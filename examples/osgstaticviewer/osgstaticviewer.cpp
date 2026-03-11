/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgstaticviewer example application
 */
#include <iostream>
#include <osg/maths/compat.hpp>
#include <osg/nodes/CoordinateSystemNode.hpp>
#include <osg/nodes/Switch.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgGA/manipulators/AnimationPathManipulator.hpp>
#include <osgGA/manipulators/DriveManipulator.hpp>
#include <osgGA/manipulators/FlightManipulator.hpp>
#include <osgGA/manipulators/KeySwitchMatrixManipulator.hpp>
#include <osgGA/manipulators/TerrainManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgText/Text>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

// include the plugins we need
USE_OSGPLUGIN( ive )
USE_OSGPLUGIN( osg )
USE_OSGPLUGIN( osg2 )
USE_OSGPLUGIN( rgb )
USE_OSGPLUGIN( OpenFlight )

#ifdef USE_FREETYPE
USE_OSGPLUGIN( freetype )
#endif

USE_DOTOSGWRAPPER_LIBRARY( osg )
USE_DOTOSGWRAPPER_LIBRARY( osgFX )
USE_DOTOSGWRAPPER_LIBRARY( osgParticle )
USE_DOTOSGWRAPPER_LIBRARY( osgShadow )
USE_DOTOSGWRAPPER_LIBRARY( osgSim )
USE_DOTOSGWRAPPER_LIBRARY( osgTerrain )
USE_DOTOSGWRAPPER_LIBRARY( osgText )
USE_DOTOSGWRAPPER_LIBRARY( osgViewer )
USE_DOTOSGWRAPPER_LIBRARY( osgVolume )
USE_DOTOSGWRAPPER_LIBRARY( osgWidget )

USE_SERIALIZER_WRAPPER_LIBRARY( osg )
USE_SERIALIZER_WRAPPER_LIBRARY( osgAnimation )
USE_SERIALIZER_WRAPPER_LIBRARY( osgFX )
USE_SERIALIZER_WRAPPER_LIBRARY( osgManipulator )
USE_SERIALIZER_WRAPPER_LIBRARY( osgParticle )
USE_SERIALIZER_WRAPPER_LIBRARY( osgShadow )
USE_SERIALIZER_WRAPPER_LIBRARY( osgSim )
USE_SERIALIZER_WRAPPER_LIBRARY( osgTerrain )
USE_SERIALIZER_WRAPPER_LIBRARY( osgText )
USE_SERIALIZER_WRAPPER_LIBRARY( osgVolume )

// include the platform specific GraphicsWindow implementation.
USE_GRAPHICSWINDOW()

int
main( int    argc,
      char** argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc, argv );

    arguments.getApplicationUsage()->setApplicationName(
        arguments.getApplicationName()
    );
    arguments.getApplicationUsage()->setDescription(
        arguments.getApplicationName() +
        " is the standard OpenSceneGraph example which loads and visualises 3d models."
    );
    arguments.getApplicationUsage()->setCommandLineUsage(
        arguments.getApplicationName() + " [options] filename ..."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--image <filename>",
        "Load an image and render it on a quad"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--dem <filename>",
        "Load an image/DEM and render it on a HeightField"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "-h or --help",
        "Display command line parameters"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--help-env",
        "Display environmental variables available"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--help-keys",
        "Display keyboard & mouse bindings available"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--help-all",
        "Display all command line, env vars and keyboard & mouse bindings."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--SingleThreaded",
        "Select SingleThreaded threading model for viewer."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--CullDrawThreadPerContext",
        "Select CullDrawThreadPerContext threading model for viewer."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--DrawThreadPerContext",
        "Select DrawThreadPerContext threading model for viewer."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--CullThreadPerCameraDrawThreadPerContext",
        "Select CullThreadPerCameraDrawThreadPerContext threading model for viewer."
    );

    // if user request help write it out to cout.
    bool         helpAll = arguments.read( "--help-all" );
    unsigned int helpType =
        ( ( helpAll || arguments.read( "-h" ) || arguments.read( "--help" ) )
              ? osg::ApplicationUsage::COMMAND_LINE_OPTION
              : 0 ) |
        ( ( helpAll || arguments.read( "--help-env" ) )
              ? osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE
              : 0 ) |
        ( ( helpAll || arguments.read( "--help-keys" ) )
              ? osg::ApplicationUsage::KEYBOARD_MOUSE_BINDING
              : 0 );
    if( helpType )
    {
        arguments.getApplicationUsage()->write( std::cout, helpType );
        return 1;
    }

    // report any errors if they have occurred when parsing the program arguments.
    if( arguments.errors() )
    {
        arguments.writeErrorMessages( std::cout );
        return 1;
    }

    if( arguments.argc() <= 1 )
    {
        arguments.getApplicationUsage()->write(
            std::cout,
            osg::ApplicationUsage::COMMAND_LINE_OPTION
        );
        return 1;
    }

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

    // set up the camera manipulators.
    {
        osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator =
            new osgGA::KeySwitchMatrixManipulator;

        keyswitchManipulator->addMatrixManipulator( '1',
                                                    "Trackball",
                                                    new osgGA::TrackballManipulator() );
        keyswitchManipulator->addMatrixManipulator( '2',
                                                    "Flight",
                                                    new osgGA::FlightManipulator() );
        keyswitchManipulator->addMatrixManipulator( '3',
                                                    "Drive",
                                                    new osgGA::DriveManipulator() );
        keyswitchManipulator->addMatrixManipulator( '4',
                                                    "Terrain",
                                                    new osgGA::TerrainManipulator() );

        std::string pathfile;
        char        keyForAnimationPath = '5';
        while( arguments.read( "-p", pathfile ) )
        {
            osgGA::AnimationPathManipulator* apm =
                new osgGA::AnimationPathManipulator( pathfile );
            if( apm || !apm->valid() )
            {
                unsigned int num = keyswitchManipulator->getNumMatrixManipulators();
                keyswitchManipulator->addMatrixManipulator( keyForAnimationPath,
                                                            "Path",
                                                            apm );
                keyswitchManipulator->selectMatrixManipulator( num );
                ++keyForAnimationPath;
            }
        }

        viewer.setCameraManipulator( keyswitchManipulator.get() );
    }

    // add the state manipulator
    viewer.addEventHandler(
        new osgGA::StateSetManipulator( viewer.getCamera()->getOrCreateStateSet() )
    );

    // add the thread model handler
    viewer.addEventHandler( new osgViewer::ThreadingHandler );

    // add the window size toggle handler
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );

    // add the stats handler
    viewer.addEventHandler( new osgViewer::StatsHandler );

    // add the help handler
    viewer.addEventHandler(
        new osgViewer::HelpHandler( arguments.getApplicationUsage() )
    );

    while( arguments.read( "--SingleThreaded" ) )
    {
        viewer.setThreadingModel( osgViewer::Viewer::SingleThreaded );
    }
    while( arguments.read( "--CullDrawThreadPerContext" ) )
    {
        viewer.setThreadingModel( osgViewer::Viewer::CullDrawThreadPerContext );
    }
    while( arguments.read( "--DrawThreadPerContext" ) )
    {
        viewer.setThreadingModel( osgViewer::Viewer::DrawThreadPerContext );
    }
    while( arguments.read( "--CullThreadPerCameraDrawThreadPerContext" ) )
    {
        viewer.setThreadingModel(
            osgViewer::Viewer::CullThreadPerCameraDrawThreadPerContext
        );
    }

    viewer.setUpViewInWindow( 1'000, 100, 640, 480 );

    unsigned int screenNum;
    while( arguments.read( "--screen", screenNum ) )
    {
        viewer.setUpViewOnSingleScreen( screenNum );
    }

    // load the data
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles( arguments );
    if( !loadedModel )
    {
        std::cout << arguments.getApplicationName() << ": No data loaded" << std::endl;
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occurred when parsing the program arguments.
    if( arguments.errors() )
    {
        arguments.writeErrorMessages( std::cout );
        return 1;
    }

    // optimize the scene graph, remove redundant nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize( loadedModel.get() );

    viewer.setSceneData( loadedModel.get() );
    return viewer.run();
}
