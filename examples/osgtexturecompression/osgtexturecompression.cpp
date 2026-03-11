/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgtexturecompression example application
 */
#include <iostream>
#include <osg/core/Notify.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/Node.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgText/Text>
#include <osgViewer/core/CompositeViewer.hpp>

osg::Camera*
createHUD( const std::string& label )
{
    // create a camera to set up the projection and model view matrices, and the subgraph
    // to drawn in the HUD
    osg::Camera* camera = new osg::Camera;

    // set the projection matrix
    camera->setProjectionMatrix( osg::ortho2D( 0.0, 1280.0, 0.0, 1024.0 ) );

    // set the view matrix
    camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    camera->setViewMatrix( osg::dmat4() );

    // only clear the depth buffer
    camera->setClearMask( GL_DEPTH_BUFFER_BIT );

    // draw subgraph after main camera view.
    camera->setRenderOrder( osg::Camera::POST_RENDER );

    // we don't want the camera to grab event focus from the viewers main camera(s).
    camera->setAllowEventFocus( false );

    // add to this camera a subgraph to render
    {

        osg::Geode*    geode = new osg::Geode();

        std::string    font( "fonts/arial.ttf" );

        osg::StateSet* stateset = geode->getOrCreateStateSet();

        osg::vec3      position( 150.0F, 150.0F, 0.0F );

        osgText::Text* text = new osgText::Text;
        geode->addDrawable( text );

        text->setFont( font );
        text->setPosition( position );
        text->setCharacterSize( 100.0F );
        text->setText( label );

        camera->addChild( geode );
    }

    return camera;
}

osg::Node*
creatQuad( const std::string&               name,
           osg::Image*                      image,
           osg::Texture::InternalFormatMode formatMode,
           osg::Texture::FilterMode         minFilter )
{

    osg::Group* group = new osg::Group;

    {
        osg::Geode* geode = new osg::Geode;

        geode->addDrawable(
            createTexturedQuadGeometry( osg::vec3( 0.0F, 0.0F, 0.0F ),
                                        osg::vec3( float( image->s() ), 0.0F, 0.0F ),
                                        osg::vec3( 0.0F, 0.0F, float( image->t() ) ) )
        );

        geode->setName( name );

        osg::StateSet*  stateset = geode->getOrCreateStateSet();

        osg::Texture2D* texture  = new osg::Texture2D( image );
        texture->setInternalFormatMode( formatMode );
        texture->setFilter( osg::Texture::MIN_FILTER, minFilter );
        stateset->setTextureAttributeAndModes( 0, texture, osg::StateAttribute::ON );

        group->addChild( geode );
    }

    {
        group->addChild( createHUD( name ) );
    }

    return group;
}

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    // Headless capture: early check before viewer construction
    {
        std::string headlessOutput;
        if( arguments.read( "--headless", headlessOutput ) )
        {
            auto node = osgDB::readRefNodeFiles( arguments );
            if( !node )
            {
                node = osgDB::readRefNodeFile( "duck.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    osgViewer::CompositeViewer viewer( arguments );

    if( arguments.argc() <= 1 )
    {
        std::cout << "Please supply an image filename on the command line." << std::endl;
        return 1;
    }

    std::string              filename = arguments[1];
    osg::ref_ptr<osg::Image> image    = osgDB::readRefImageFile( filename );

    if( !image )
    {
        std::cout << "Error: unable able to read image from " << filename << std::endl;
        return 1;
    }

    osg::GraphicsContext::WindowingSystemInterface* wsi =
        osg::GraphicsContext::getWindowingSystemInterface();
    if( !wsi )
    {
        osg::notify( osg::NOTICE )
            << "Error, no WindowSystemInterface available, cannot create windows."
            << std::endl;
        return 1;
    }

    unsigned int                           width, height;
    osg::GraphicsContext::ScreenIdentifier main_screen_id;

    main_screen_id.readDISPLAY();
    main_screen_id.setUndefinedScreenDetailsToDefaultScreen();
    wsi->getScreenResolution( main_screen_id, width, height );

    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x                                         = 1'000;
    traits->y                                         = 100;
    traits->width                                     = 640;
    traits->height                                    = 480;
    traits->windowDecoration                          = true;
    traits->doubleBuffer                              = true;
    traits->readDISPLAY();
    traits->setUndefinedScreenDetailsToDefaultScreen();

    osg::ref_ptr<osg::GraphicsContext> gc =
        osg::GraphicsContext::createGraphicsContext( traits.get() );
    if( !gc )
    {
        std::cout << "Error: GraphicsWindow has not been created successfully."
                  << std::endl;
    }

    gc->setClearColor( osg::vec4( 0.0F, 0.0F, 0.0F, 1.0F ) );
    gc->setClearMask( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    osg::ref_ptr<osgGA::TrackballManipulator> trackball =
        new osgGA::TrackballManipulator;

    typedef std::vector<osg::ref_ptr<osg::Node>> Models;

    Models                                       models;
    models.push_back( creatQuad( "no compression",
                                 image.get(),
                                 osg::Texture::USE_IMAGE_DATA_FORMAT,
                                 osg::Texture::LINEAR ) );
    models.push_back( creatQuad( "ARB compression",
                                 image.get(),
                                 osg::Texture::USE_ARB_COMPRESSION,
                                 osg::Texture::LINEAR ) );
    models.push_back( creatQuad( "DXT1 compression",
                                 image.get(),
                                 osg::Texture::USE_S3TC_DXT1_COMPRESSION,
                                 osg::Texture::LINEAR ) );
    models.push_back( creatQuad( "DXT3 compression",
                                 image.get(),
                                 osg::Texture::USE_S3TC_DXT3_COMPRESSION,
                                 osg::Texture::LINEAR ) );
    models.push_back( creatQuad( "DXT5 compression",
                                 image.get(),
                                 osg::Texture::USE_S3TC_DXT5_COMPRESSION,
                                 osg::Texture::LINEAR ) );

    int numX = 1;
    int numY = 1;

    // compute the number of views up and across that are need
    {
        float aspectRatio = float( width ) / float( height );
        float multiplier  = sqrtf( float( models.size() ) / aspectRatio );
        ;
        float multiplier_x = multiplier * aspectRatio;
        float multiplier_y = multiplier;

        if( ( multiplier_x / ceilf( multiplier_x ) ) >
            ( multiplier_y / ceilf( multiplier_y ) ) )
        {
            numX = int( ceilf( multiplier_x ) );
            numY = int( ceilf( float( models.size() ) / float( numX ) ) );
        }
        else
        {
            numY = int( ceilf( multiplier_y ) );
            numX = int( ceilf( float( models.size() ) / float( numY ) ) );
        }
    }

    // populate the view with the required view to view each model.
    for( unsigned int i = 0; i < models.size(); ++i )
    {
        osgViewer::View* view  = new osgViewer::View;

        int              xCell = i % numX;
        int              yCell = i / numX;

        int              vx = int( ( float( xCell ) / float( numX ) ) * float( width ) );
        int vy = int( ( float( yCell ) / float( numY ) ) * float( height ) );
        int vw = int( float( width ) / float( numX ) );
        int vh = int( float( height ) / float( numY ) );

        view->setSceneData( models[i].get() );
        view->getCamera()->setProjectionMatrixAsPerspective( 30.0,
                                                             double( vw ) / double( vh ),
                                                             1.0,
                                                             1000.0 );
        view->getCamera()->setViewport( new osg::Viewport( vx, vy, vw, vh ) );
        view->getCamera()->setGraphicsContext( gc.get() );
        view->getCamera()->setClearMask( 0 );
        view->setCameraManipulator( trackball.get() );

        viewer.addView( view );
    }
    return viewer.run();
}
