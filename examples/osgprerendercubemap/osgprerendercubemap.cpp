/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgprerendercubemap example application
 */
#include <iostream>
#include <osg/core/ArgumentParser.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/lighting/Light.hpp>
#include <osg/lighting/LightSource.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/nodes/Camera.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/nodes/PositionAttitudeTransform.hpp>
#include <osg/nodes/Projection.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/CullFace.hpp>
#include <osg/state/Material.hpp>
#include <osg/state/PolygonOffset.hpp>
#include <osg/textures/Texture.hpp>
#include <osg/textures/TextureCubeMap.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgViewer/core/Viewer.hpp>

using namespace osg;

ref_ptr<Group>
_create_scene()
{
    ref_ptr<Group> scene   = new Group;
    ref_ptr<Geode> geode_1 = new Geode;
    scene->addChild( geode_1.get() );

    ref_ptr<Geode>           geode_2     = new Geode;
    ref_ptr<MatrixTransform> transform_2 = new MatrixTransform;
    transform_2->addChild( geode_2.get() );
    transform_2->setUpdateCallback(
        new osg::AnimationPathCallback( dvec3( 0, 0, 0 ),
                                        osg::dvec3( 0, 1, 0 ),
                                        inDegrees( 45.0F ) )
    );
    scene->addChild( transform_2.get() );

    ref_ptr<Geode>           geode_3     = new Geode;
    ref_ptr<MatrixTransform> transform_3 = new MatrixTransform;
    transform_3->addChild( geode_3.get() );
    transform_3->setUpdateCallback(
        new osg::AnimationPathCallback( dvec3( 0, 0, 0 ),
                                        osg::dvec3( 0, 1, 0 ),
                                        inDegrees( -22.5F ) )
    );
    scene->addChild( transform_3.get() );

    const float                radius = 0.8F;
    const float                height = 1.0F;
    ref_ptr<TessellationHints> hints  = new TessellationHints;
    hints->setDetailRatio( 2.0F );
    ref_ptr<ShapeDrawable> shape;

    shape = new ShapeDrawable( new Box( vec3( 0.0F, -2.0F, 0.0F ), 10, 0.1F, 10 ),
                               hints.get() );
    shape->setColor( vec4( 0.5F, 0.5F, 0.7F, 1.0F ) );
    geode_1->addDrawable( shape.get() );

    shape = new ShapeDrawable( new Sphere( vec3( -3.0F, 0.0F, 0.0F ), radius ),
                               hints.get() );
    shape->setColor( vec4( 0.6F, 0.8F, 0.8F, 1.0F ) );
    geode_2->addDrawable( shape.get() );

    shape = new ShapeDrawable( new Box( vec3( 3.0F, 0.0F, 0.0F ), 2 * radius ),
                               hints.get() );
    shape->setColor( vec4( 0.4F, 0.9F, 0.3F, 1.0F ) );
    geode_2->addDrawable( shape.get() );

    shape = new ShapeDrawable( new Cone( vec3( 0.0F, 0.0F, -3.0F ), radius, height ),
                               hints.get() );
    shape->setColor( vec4( 0.2F, 0.5F, 0.7F, 1.0F ) );
    geode_2->addDrawable( shape.get() );

    shape = new ShapeDrawable( new Cylinder( vec3( 0.0F, 0.0F, 3.0F ), radius, height ),
                               hints.get() );
    shape->setColor( vec4( 1.0F, 0.3F, 0.3F, 1.0F ) );
    geode_2->addDrawable( shape.get() );

    shape = new ShapeDrawable( new Box( vec3( 0.0F, 3.0F, 0.0F ), 2, 0.1F, 2 ),
                               hints.get() );
    shape->setColor( vec4( 0.8F, 0.8F, 0.4F, 1.0F ) );
    geode_3->addDrawable( shape.get() );

    // material
    ref_ptr<Material> matirial = new Material;
    matirial->setColorMode( Material::DIFFUSE );
    matirial->setAmbient( Material::FRONT_AND_BACK, vec4( 0, 0, 0, 1 ) );
    matirial->setSpecular( Material::FRONT_AND_BACK, vec4( 1, 1, 1, 1 ) );
    matirial->setShininess( Material::FRONT_AND_BACK, 64.0F );
    scene->getOrCreateStateSet()->setAttributeAndModes( matirial.get(),
                                                        StateAttribute::ON );

    return scene;
}

osg::NodePath
createReflector()
{
    osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
    pat->setPosition( osg::dvec3( 0.0F, 0.0F, 0.0F ) );
    pat->setAttitude( osg::quat( osg::radians( 0.0F ), osg::vec3( 0.0F, 0.0F, 1.0F ) ) );

    Geode* geode_1 = new Geode;
    pat->addChild( geode_1 );

    const float                radius = 0.8F;
    ref_ptr<TessellationHints> hints  = new TessellationHints;
    hints->setDetailRatio( 2.0F );
    ShapeDrawable* shape =
        new ShapeDrawable( new Sphere( vec3( 0.0F, 0.0F, 0.0F ), radius * 1.5F ),
                           hints.get() );
    shape->setColor( vec4( 0.8F, 0.8F, 0.8F, 1.0F ) );
    geode_1->addDrawable( shape );

    osg::NodePath nodeList;
    nodeList.push_back( pat );
    nodeList.push_back( geode_1 );

    return nodeList;
}

class UpdateCameraAndTexGenCallback : public osg::NodeCallback
{
    public:

        typedef std::vector<osg::ref_ptr<osg::Camera>> CameraList;

        UpdateCameraAndTexGenCallback( osg::NodePath& reflectorNodePath,
                                       CameraList&    Cameras ) :
            _reflectorNodePath( reflectorNodePath ),
            _Cameras( Cameras )
        {
        }

        virtual void
        operator()( osg::Node*        node,
                    osg::NodeVisitor* nv )
        {
            // first update subgraph to make sure objects are all moved into position
            traverse( node, nv );

            // compute the position of the center of the reflector subgraph
            osg::dmat4  worldToLocal = osg::computeWorldToLocal( _reflectorNodePath );
            osg::sphere bs           = _reflectorNodePath.back()->getBound();
            osg::vec3   position     = bs.center;

            typedef std::pair<osg::vec3, osg::vec3> ImageData;
            const ImageData                         id[] = {
                ImageData( osg::vec3( 1, 0, 0 ), osg::vec3( 0, -1, 0 ) ),     // +X
                ImageData( osg::vec3( -1, 0, 0 ), osg::vec3( 0, -1, 0 ) ),    // -X
                ImageData( osg::vec3( 0, 1, 0 ), osg::vec3( 0, 0, 1 ) ),      // +Y
                ImageData( osg::vec3( 0, -1, 0 ), osg::vec3( 0, 0, -1 ) ),    // -Y
                ImageData( osg::vec3( 0, 0, 1 ), osg::vec3( 0, -1, 0 ) ),     // +Z
                ImageData( osg::vec3( 0, 0, -1 ), osg::vec3( 0, -1, 0 ) )     // -Z
            };

            for( unsigned int i = 0; i < 6 && i < _Cameras.size(); ++i )
            {
                osg::dmat4 localOffset;
                localOffset =
                    osg::lookAt( position, position + id[i].first, id[i].second );

                osg::dmat4 viewMatrix = worldToLocal * localOffset;

                _Cameras[i]->setReferenceFrame( osg::Camera::ABSOLUTE_RF );
                _Cameras[i]
                    ->setProjectionMatrixAsFrustum( -1.0, 1.0, -1.0, 1.0, 1.0, 10000.0 );
                _Cameras[i]->setViewMatrix( viewMatrix );
            }
        }

    protected:

        virtual ~UpdateCameraAndTexGenCallback()
        {
        }

        osg::NodePath _reflectorNodePath;
        CameraList    _Cameras;
};

osg::Group*
createShadowedScene( osg::Node*                              reflectedSubgraph,
                     osg::NodePath                           reflectorNodePath,
                     unsigned int                            unit,
                     const osg::vec4&                        clearColor,
                     unsigned                                tex_width,
                     unsigned                                tex_height,
                     osg::Camera::RenderTargetImplementation renderImplementation )
{

    osg::Group*          group   = new osg::Group;

    osg::TextureCubeMap* texture = new osg::TextureCubeMap;
    texture->setTextureSize( tex_width, tex_height );

    texture->setInternalFormat( GL_RGB );
    texture->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
    texture->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );
    texture->setWrap( osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE );
    texture->setFilter( osg::TextureCubeMap::MIN_FILTER, osg::TextureCubeMap::LINEAR );
    texture->setFilter( osg::TextureCubeMap::MAG_FILTER, osg::TextureCubeMap::LINEAR );

    // set up the render to texture cameras.
    UpdateCameraAndTexGenCallback::CameraList Cameras;
    for( unsigned int i = 0; i < 6; ++i )
    {
        // create the camera
        osg::Camera* camera = new osg::Camera;

        camera->setClearMask( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        camera->setClearColor( clearColor );

        // set viewport
        camera->setViewport( 0, 0, tex_width, tex_height );

        // set the camera to render before the main camera.
        camera->setRenderOrder( osg::Camera::PRE_RENDER );

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation( renderImplementation );

        // attach the texture and use it as the color buffer.
        camera->attach( osg::Camera::COLOR_BUFFER, texture, 0, i );

        // add subgraph to render
        camera->addChild( reflectedSubgraph );

        group->addChild( camera );

        Cameras.push_back( camera );
    }

    // set the reflected subgraph so that it uses the texture settings.
    {
        osg::Node* reflectorNode = reflectorNodePath.front();
        group->addChild( reflectorNode );

        osg::StateSet* stateset = reflectorNode->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes( unit, texture, osg::StateAttribute::ON );
    }

    // add the reflector scene to draw just as normal
    group->addChild( reflectedSubgraph );

    // set an update callback to keep moving the camera and tex gen in the right
    // direction.
    group->setUpdateCallback( new UpdateCameraAndTexGenCallback( reflectorNodePath,
                                                                 Cameras ) );

    return group;
}

int
main( int    argc,
      char** argv )
{
    // use an ArgumentParser object to manage the program arguments.
    ArgumentParser arguments( &argc, argv );

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(
        arguments.getApplicationName() +
        " is the example which demonstrates using of GL_ARB_shadow extension "
        "implemented in osg::Texture class"
    );
    arguments.getApplicationUsage()->setCommandLineUsage(
        arguments.getApplicationName()
    );
    arguments.getApplicationUsage()->addCommandLineOption( "-h or --help",
                                                           "Display this information" );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--fbo",
        "Use Frame Buffer Object for render to texture, where supported."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--fb",
        "Use FrameBuffer for render to texture."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--pbuffer",
        "Use Pixel Buffer for render to texture, where supported."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--window",
        "Use a separate Window for render to texture."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--width",
        "Set the width of the render to texture"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--height",
        "Set the height of the render to texture"
    );

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

    osgViewer::Viewer viewer;

    // if user request help write it out to cout.
    if( arguments.read( "-h" ) || arguments.read( "--help" ) )
    {
        arguments.getApplicationUsage()->write( std::cout );
        return 1;
    }

    unsigned tex_width  = 256;
    unsigned tex_height = 256;
    while( arguments.read( "--width", tex_width ) )
    {
    }
    while( arguments.read( "--height", tex_height ) )
    {
    }

    osg::Camera::RenderTargetImplementation renderImplementation =
        osg::Camera::FRAME_BUFFER_OBJECT;

    while( arguments.read( "--fbo" ) )
    {
        renderImplementation = osg::Camera::FRAME_BUFFER_OBJECT;
    }
    while( arguments.read( "--pbuffer" ) )
    {
        renderImplementation = osg::Camera::PIXEL_BUFFER;
    }
    while( arguments.read( "--fb" ) )
    {
        renderImplementation = osg::Camera::FRAME_BUFFER;
    }
    while( arguments.read( "--window" ) )
    {
        renderImplementation = osg::Camera::SEPARATE_WINDOW;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occurred when parsing the program arguments.
    if( arguments.errors() )
    {
        arguments.writeErrorMessages( std::cout );
        return 1;
    }

    ref_ptr<MatrixTransform> scene = new MatrixTransform;
    scene->setMatrix( osg::rotate( osg::radians( 125.0 ), 1.0, 0.0, 0.0 ) );

    ref_ptr<Group> reflectedSubgraph = _create_scene();
    if( !reflectedSubgraph.valid() )
    {
        return 1;
    }

    ref_ptr<Group> reflectedScene =
        createShadowedScene( reflectedSubgraph.get(),
                             createReflector(),
                             0,
                             viewer.getCamera()->getClearColor(),
                             tex_width,
                             tex_height,
                             renderImplementation );

    scene->addChild( reflectedScene.get() );

    viewer.setSceneData( scene.get() );
    return viewer.run();
}
