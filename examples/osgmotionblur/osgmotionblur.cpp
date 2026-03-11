/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgmotionblur example application
 */
#include <iostream>
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/BlendFunc.hpp>
#include <osg/state/Depth.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgViewer/core/Viewer.hpp>

// Create a camera that renders a full-screen fade quad before the main scene.
// This replaces the deprecated accumulation buffer approach with a simple
// framebuffer persistence technique:
//   - The color buffer is never fully cleared, so previous frames persist.
//   - A semi-transparent quad is drawn each frame to gradually fade old content.
//   - The scene then renders on top, giving a motion blur / ghosting effect.
osg::Camera*
createFadeOverlayCamera( double persistence )
{
    // The persistence parameter controls how long trails last.
    // Convert it to a fade alpha: higher persistence = less fade per frame.
    // The original code computed: s = pow(0.2, dt/persistence)
    // At 60fps, dt ~ 0.0167, so s ~ pow(0.2, 0.0667) ~ 0.90 for persistence=0.25
    // The fade alpha is (1 - s), so about 0.10
    // We use a fixed approximation assuming ~60fps; the visual result is similar.
    double       dt        = 1.0 / 60.0;
    double       s         = pow( 0.2, dt / persistence );
    float        fadeAlpha = static_cast<float>( 1.0 - s );

    osg::Camera* camera    = new osg::Camera;

    // Use absolute reference frame so it is independent of the main camera
    camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    camera->setProjectionMatrixAsOrtho2D( 0, 1, 0, 1 );
    camera->setViewMatrix( osg::dmat4() );

    // Render before the main scene but after the depth buffer is cleared
    camera->setRenderOrder( osg::Camera::NESTED_RENDER, -1 );

    // Do not clear anything -- we want the previous frame to persist
    camera->setClearMask( 0 );

    // Allow this camera to receive events but mainly we need it to render
    camera->setAllowEventFocus( false );

    // Create the full-screen fade quad
    osg::Geometry*  quad = osg::createTexturedQuadGeometry( osg::vec3( 0, 0, 0 ),
                                                            osg::vec3( 1, 0, 0 ),
                                                            osg::vec3( 0, 1, 0 ) );

    // Set the fade color: black with the computed alpha
    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back( osg::vec4( 0.0F, 0.0F, 0.0F, fadeAlpha ) );
    quad->setColorArray( colors, osg::Array::BIND_OVERALL );

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable( quad );

    // Set up state for blending
    osg::StateSet*  ss = geode->getOrCreateStateSet();

    // Enable alpha blending
    osg::BlendFunc* blendFunc = new osg::BlendFunc;
    blendFunc->setFunction( osg::BlendFunc::SRC_ALPHA,
                            osg::BlendFunc::ONE_MINUS_SRC_ALPHA );
    ss->setAttributeAndModes( blendFunc, osg::StateAttribute::ON );

    // Disable depth testing and writing for the fade quad
    osg::Depth* depth = new osg::Depth;
    depth->setWriteMask( false );
    depth->setFunction( osg::Depth::Function::ALWAYS );
    ss->setAttributeAndModes( depth, osg::StateAttribute::ON );

    // GL_LIGHTING removed: not in core profile (lighting is shader-controlled)

    camera->addChild( geode );

    return camera;
}

int
main( int    argc,
      char** argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc, argv );

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(
        arguments.getApplicationName()
    );
    arguments.getApplicationUsage()->setDescription(
        arguments.getApplicationName() +
        " is an OpenSceneGraph example that shows how to achieve a simple motion blur "
        "effect using framebuffer persistence."
    );
    arguments.getApplicationUsage()->setCommandLineUsage(
        arguments.getApplicationName() + " [options] filename ..."
    );
    arguments.getApplicationUsage()->addCommandLineOption( "-h or --help",
                                                           "Display this information" );
    arguments.getApplicationUsage()->addCommandLineOption(
        "-P or --persistence",
        "Set the motion blur persistence time"
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

    double persistence = 0.25;
    arguments.read( "-P", persistence ) ||
        arguments.read( "--persistence", persistence );

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles( arguments );

    // if not loaded assume no arguments passed in, try use default mode instead.
    if( !loadedModel )
    {
        loadedModel = osgDB::readRefNodeFile( "duck.glb" );
    }

    // if no model has been successfully loaded report failure.
    if( !loadedModel )
    {
        std::cout << arguments.getApplicationName() << ": No data loaded" << std::endl;
        return 1;
    }

    // Build the scene graph:
    // root
    //   |-- fadeOverlayCamera (draws semi-transparent black quad to fade old content)
    //   |-- loadedModel (the actual scene)
    osg::Group* root = new osg::Group;
    root->addChild( createFadeOverlayCamera( persistence ) );
    root->addChild( loadedModel.get() );

    // pass the scene graph to the viewer.
    viewer.setSceneData( root );

    // Set the main camera to only clear the depth buffer, not the color buffer.
    // This lets previous frame content persist, creating the motion blur trails.
    viewer.getCamera()->setClearMask( GL_DEPTH_BUFFER_BIT );
    return viewer.run();
}
