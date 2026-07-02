/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgautotransform example application
 */
#include <iostream>
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/AutoTransform.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/Projection.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/BlendFunc.hpp>
#include <osg/state/Depth.hpp>
#include <osg/state/Material.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/io/WriteFile.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgText/Text.hpp>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgViewer/core/CompositeViewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

osg::Node*
createLabel( const osg::vec3&             pos,
             float                        size,
             const std::string&           label,
             osgText::Text::AxisAlignment axisAlignment )
{
    osg::Geode* geode = new osg::Geode();

    std::string timesFont( "fonts/arial.ttf" );

    {
        osgText::Text* text = new osgText::Text;
        geode->addDrawable( text );

        text->setFont( timesFont );
        text->setPosition( pos );
        text->setCharacterSize( size );
        text->setAxisAlignment( axisAlignment );
        text->setAlignment( osgText::Text::CENTER_CENTER );
        text->setText( label );
    }

    return geode;
}

osg::Node*
createLabel3( const osg::vec3&   pos,
              float              size,
              const std::string& label )
{
    osg::Geode* geode = new osg::Geode();

    std::string timesFont( "fonts/arial.ttf" );

    {
        osgText::Text* text = new osgText::Text;
        geode->addDrawable( text );

        text->setFont( timesFont );
        text->setPosition( pos );
        text->setFontResolution( 40, 40 );
        text->setCharacterSize( size );
        text->setAlignment( osgText::Text::CENTER_CENTER );
        text->setAutoRotateToScreen( true );
        text->setCharacterSizeMode(
            osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT
        );
        text->setText( label );
    }

    return geode;
}

osg::Node*
createAxis( const osg::vec3&                   s,
            const osg::vec3&                   e,
            int                                numReps,
            osg::AutoTransform::AutoRotateMode autoRotateMode,
            osgText::Text::AxisAlignment       axisAlignment,
            const std::string&                 str )
{
    osg::Group* group  = new osg::Group;

    osg::vec3   dv     = e - s;
    dv                /= float( numReps - 1 );

    osg::vec3 pos      = s;

    bool      useAuto  = true;
    if( useAuto )
    {
        osg::Vec3Array* vertices = new osg::Vec3Array;

        for( int i = 0; i < numReps; ++i )
        {
            osg::AutoTransform* at = new osg::AutoTransform;
            at->setPosition( osg::dvec3( pos ) );
            at->setAutoRotateMode( autoRotateMode );
            at->addChild( createLabel( osg::vec3( 0.0F, 0.0F, 0.0F ),
                                       osg::length( dv ) * 0.2F,
                                       str,
                                       axisAlignment ) );
            vertices->push_back( pos );
            pos += dv;

            group->addChild( at );
        }

        osg::Vec4Array* colors = new osg::Vec4Array;
        colors->push_back( osg::vec4( 1.0F, 1.0F, 1.0F, 1.0F ) );

        osg::Geometry* geom = new osg::Geometry;
        geom->setVertexArray( vertices );
        geom->setColorArray( colors, osg::Array::BIND_OVERALL );
        geom->addPrimitiveSet(
            new osg::DrawArrays( GL_LINE_STRIP, 0, vertices->size() )
        );

        osg::Geode* geode = new osg::Geode;
        geode->addDrawable( geom );

        group->addChild( geode );
    }
    else
    {
        osg::Vec3Array* vertices = new osg::Vec3Array;

        for( int i = 0; i < numReps; ++i )
        {
            group->addChild(
                createLabel3( osg::vec3( pos ), osg::length( dv ) * 0.5F, str )
            );
            vertices->push_back( pos );
            pos += dv;
        }

        osg::Vec4Array* colors = new osg::Vec4Array;
        colors->push_back( osg::vec4( 1.0F, 1.0F, 1.0F, 1.0F ) );

        osg::Geometry* geom = new osg::Geometry;
        geom->setVertexArray( vertices );
        geom->setColorArray( colors, osg::Array::BIND_OVERALL );
        geom->addPrimitiveSet(
            new osg::DrawArrays( GL_LINE_STRIP, 0, vertices->size() )
        );

        osg::Geode* geode = new osg::Geode;
        geode->addDrawable( geom );

        group->addChild( geode );
    }

    return group;
}

osg::Node*
createAutoScale( const osg::vec3&   position,
                 float              characterSize,
                 const std::string& message,
                 float              minScale = 0.0,
                 float              maxScale = FLT_MAX )
{
    std::string    timesFont( "fonts/arial.ttf" );

    osgText::Text* text = new osgText::Text;
    text->setCharacterSize( characterSize );
    text->setText( message );
    text->setFont( timesFont );
    text->setAlignment( osgText::Text::CENTER_CENTER );

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable( text );
    // GL_LIGHTING removed for Core Profile compatibility

    osg::AutoTransform* at = new osg::AutoTransform;
    at->addChild( geode );

    at->setAutoRotateMode( osg::AutoTransform::ROTATE_TO_SCREEN );
    at->setAutoScaleToScreen( true );
    at->setMinimumScale( minScale );
    at->setMaximumScale( maxScale );
    at->setPosition( osg::dvec3( position ) );

    return at;
}

osg::Node*
createScene()
{
    osg::Group* root = new osg::Group;

    // int numReps = 3333;
    int         numReps = 10;
    root->addChild( createAxis( osg::vec3( 0.0, 0.0, 0.0 ),
                                osg::vec3( 1000.0, 0.0, 0.0 ),
                                numReps,
                                osg::AutoTransform::ROTATE_TO_CAMERA,
                                osgText::Text::XY_PLANE,
                                "ROTATE_TO_CAMERA" ) );
    root->addChild( createAxis( osg::vec3( 0.0, 0.0, 0.0 ),
                                osg::vec3( 0.0, 1000.0, 0.0 ),
                                numReps,
                                osg::AutoTransform::ROTATE_TO_SCREEN,
                                osgText::Text::XY_PLANE,
                                "ROTATE_TO_SCREEN" ) );
    root->addChild( createAxis( osg::vec3( 0.0, 0.0, 0.0 ),
                                osg::vec3( 0.0, 0.0, 1000.0 ),
                                numReps,
                                osg::AutoTransform::NO_ROTATION,
                                osgText::Text::XZ_PLANE,
                                "NO_ROTATION" ) );

    root->addChild( createAutoScale( osg::vec3( 500.0, 500.0, 500.0 ),
                                     25.0,
                                     "AutoScale with no min, max limits" ) );
    root->addChild( createAutoScale( osg::vec3( 500.0, 500.0, 300.0 ),
                                     25.0,
                                     "AutoScale with minScale = 1, maxScale = 2.0 ",
                                     1,
                                     2.0 ) );
    root->addChild( createAutoScale( osg::vec3( 500.0, 500.0, 700.0 ),
                                     25.0,
                                     "AutoScale with minScale = 0.0, maxScale = 5.0 ",
                                     0.0,
                                     5.0 ) );
    return root;
}

osgViewer::View*
createView( osg::ref_ptr<osg::Node>            scenegraph,
            osg::ref_ptr<osg::GraphicsContext> gc,
            unsigned int                       x,
            unsigned int                       y,
            unsigned int                       width,
            unsigned int                       height )
{
    OSG_NOTICE << "createView(....,x=" << x << ", y=" << y << ", width=" << width
               << ", height=" << height << ")" << std::endl;

    if( !gc )
    {
        osg::ref_ptr<osg::GraphicsContext::Traits> traits =
            new osg::GraphicsContext::Traits;
        traits->x                = x;
        traits->y                = y;
        traits->width            = width;
        traits->height           = height;
        traits->windowDecoration = true;
        traits->doubleBuffer     = true;
        traits->sharedContext    = 0;
        traits->readDISPLAY();
        traits->setUndefinedScreenDetailsToDefaultScreen();

        gc = osg::GraphicsContext::createGraphicsContext( traits.get() );
        if( !gc )
        {
            osg::notify( osg::NOTICE )
                << "  GraphicsWindow has not been created successfully." << std::endl;
            return 0;
        }

        x = 0;
        y = 0;
    }

    osgViewer::View* view = new osgViewer::View;
    view->getCamera()->setGraphicsContext( gc.get() );
    view->getCamera()->setViewport( new osg::Viewport( x, y, width, height ) );
    // view->getCamera()->setProjectionMatrixAsPerspective(30.0, double(width) /
    // double(height), 1.0, 1000.0);
    view->setCameraManipulator( new osgGA::TrackballManipulator );
    view->setSceneData( scenegraph );
    return view;
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

    // create the scene graph that contains osg::AutoTransform nodes, see about for
    // implementation details
    osg::ref_ptr<osg::Node>    scenegraph = createScene();

    unsigned int               numViews   = 1;
    while( arguments.read( "-n", numViews ) )
    {
    }

    bool windows = false;
    while( arguments.read( "-w" ) )
    {
        windows = true;
    }

    if( numViews <= 1 )
    {
        osgViewer::View* view = new osgViewer::View;
        view->setUpViewInWindow( 1'000, 100, 640, 480 );
        view->setSceneData( scenegraph.get() );
        view->setCameraManipulator( new osgGA::TrackballManipulator );
        view->addEventHandler( new osgViewer::StatsHandler );
        viewer.addView( view );
    }
    else
    {
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

        unsigned int x = 0, y = 0;
        while( arguments.read( "--window", x, y, width, height ) )
        {
        }

        osg::ref_ptr<osg::GraphicsContext> gc;
        if( !windows )
        {
            osg::ref_ptr<osg::GraphicsContext::Traits> traits =
                new osg::GraphicsContext::Traits;
            traits->x                = x;
            traits->y                = y;
            traits->width            = width;
            traits->height           = height;
            traits->windowDecoration = true;
            traits->doubleBuffer     = true;
            traits->sharedContext    = 0;
            traits->readDISPLAY();
            traits->setUndefinedScreenDetailsToDefaultScreen();

            gc = osg::GraphicsContext::createGraphicsContext( traits.get() );
            if( !gc )
            {
                osg::notify( osg::NOTICE )
                    << "  GraphicsWindow has not been created successfully."
                    << std::endl;
                return 1;
            }
        }

        if( numViews == 2 )
        {
            viewer.addView( createView( scenegraph, gc, 0, 0, width / 2, height ) );
            viewer.addView(
                createView( scenegraph, gc, width / 2, 0, width / 2, height )
            );
        }
        else if( numViews == 3 )
        {
            viewer.addView( createView( scenegraph, gc, 0, 0, width / 2, height / 2 ) );
            viewer.addView(
                createView( scenegraph, gc, width / 2, 0, width / 2, height / 2 )
            );
            viewer.addView(
                createView( scenegraph, gc, 0, height / 2, width, height / 2 )
            );
        }
        else
        {
            viewer.addView( createView( scenegraph, gc, 0, 0, width / 2, height / 2 ) );
            viewer.addView(
                createView( scenegraph, gc, width / 2, 0, width / 2, height / 2 )
            );
            viewer.addView(
                createView( scenegraph, gc, 0, height / 2, width / 2, height / 2 )
            );
            viewer.addView( createView( scenegraph,
                                        gc,
                                        width / 2,
                                        height / 2,
                                        width / 2,
                                        height / 2 ) );
        }

        viewer.getView( 0 )->addEventHandler( new osgViewer::StatsHandler );
    }

    // run the viewers frame loop
    return viewer.run();
}
