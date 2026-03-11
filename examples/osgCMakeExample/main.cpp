/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * main example application
 */
#include <iostream>
#include <osg/core/Types.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/CoordinateSystemNode.hpp>
#include <osg/nodes/Switch.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/events/Device.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgGA/manipulators/AnimationPathManipulator.hpp>
#include <osgGA/manipulators/DriveManipulator.hpp>
#include <osgGA/manipulators/FlightManipulator.hpp>
#include <osgGA/manipulators/KeySwitchMatrixManipulator.hpp>
#include <osgGA/manipulators/SphericalManipulator.hpp>
#include <osgGA/manipulators/TerrainManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgText/Text>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

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
        "--login <url> <username> <password>",
        "Provide authentication information for http file access."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "-p <filename>",
        "Play specified camera path animation file, previously saved with 'z' key."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--speed <factor>",
        "Speed factor for animation playing (1 == normal speed)."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--device <device-name>",
        "add named device to the viewer"
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

    osgViewer::Viewer viewer( arguments );

    unsigned int      helpType = 0;
    if( ( helpType = arguments.readHelpType() ) )
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

    std::string url, username, password;
    while( arguments.read( "--login", url, username, password ) )
    {
        if( !osgDB::Registry::instance()->getAuthenticationMap() )
        {
            osgDB::Registry::instance()->setAuthenticationMap(
                new osgDB::AuthenticationMap
            );
            osgDB::Registry::instance()
                ->getAuthenticationMap()
                ->addAuthenticationDetails(
                    url,
                    new osgDB::AuthenticationDetails( username, password )
                );
        }
    }

    std::string device;
    while( arguments.read( "--device", device ) )
    {
        osg::ref_ptr<osgGA::Device> dev = osgDB::readRefFile<osgGA::Device>( device );
        if( dev.valid() )
        {
            viewer.addDevice( dev );
        }
    }

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
        keyswitchManipulator->addMatrixManipulator( '5',
                                                    "Orbit",
                                                    new osgGA::OrbitManipulator() );
        keyswitchManipulator->addMatrixManipulator(
            '6',
            "FirstPerson",
            new osgGA::FirstPersonManipulator()
        );
        keyswitchManipulator->addMatrixManipulator( '7',
                                                    "Spherical",
                                                    new osgGA::SphericalManipulator() );

        std::string pathfile;
        double      animationSpeed = 1.0;
        while( arguments.read( "--speed", animationSpeed ) )
        {
        }
        char keyForAnimationPath = '8';
        while( arguments.read( "-p", pathfile ) )
        {
            osgGA::AnimationPathManipulator* apm =
                new osgGA::AnimationPathManipulator( pathfile );
            if( apm && !apm->getAnimationPath()->empty() )
            {
                apm->setTimeScale( animationSpeed );

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

    // add the record camera path handler
    viewer.addEventHandler( new osgViewer::RecordCameraPathHandler );

    // add the LOD Scale handler
    viewer.addEventHandler( new osgViewer::LODScaleHandler );

    // add the screen capture handler
    viewer.addEventHandler( new osgViewer::ScreenCaptureHandler );

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
    optimizer.optimize( loadedModel );

    viewer.setSceneData( loadedModel );

    viewer.realize();
    return viewer.run();
}
