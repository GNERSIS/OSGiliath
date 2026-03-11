/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgshadergen example application
 */
#include <iostream>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/io/WriteFile.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgGA/manipulators/AnimationPathManipulator.hpp>
#include <osgGA/manipulators/DriveManipulator.hpp>
#include <osgGA/manipulators/FlightManipulator.hpp>
#include <osgGA/manipulators/KeySwitchMatrixManipulator.hpp>
#include <osgGA/manipulators/TerrainManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgUtil/utils/ShaderGen.hpp>
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
        " is an example of conversion of fixed function pipeline to GLSL"
    );
    arguments.getApplicationUsage()->setCommandLineUsage(
        arguments.getApplicationName() + " [options] filename ..."
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

    // add the record camera path handler
    viewer.addEventHandler( new osgViewer::RecordCameraPathHandler );

    // add the LOD Scale handler
    viewer.addEventHandler( new osgViewer::LODScaleHandler );

    // add the screen capture handler
    viewer.addEventHandler( new osgViewer::ScreenCaptureHandler );

    osg::ref_ptr<osg::Program> uberProgram = new osg::Program;
    std::string                shaderFilename;
    while( arguments.read( "--shader", shaderFilename ) )
    {
        osg::ref_ptr<osg::Shader> shader = osgDB::readRefShaderFile( shaderFilename );
        if( shader.valid() )
        {
            uberProgram->addShader( shader.get() );
        }
    }

    std::string outputFilename;
    if( arguments.read( "-o", outputFilename ) )
    {
    }

    // load the data
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles( arguments );
    if( !loadedModel )
    {
        loadedModel = osgDB::readRefNodeFile( "duck.glb" );
    }
    if( !loadedModel )
    {
        std::cout << arguments.getApplicationName() << ": No data loaded" << std::endl;
        return 1;
    }

    osg::ref_ptr<osg::StateSet> rootStateSet = viewer.getCamera()->getStateSet();

    // run the shadergen on the loaded scene graph, and assign the uber shader
    osgUtil::ShaderGenVisitor   shadergen;

    if( uberProgram->getNumShaders() > 0 )
    {
        rootStateSet->setAttribute( uberProgram.get() );
        rootStateSet->addUniform( new osg::Uniform( "diffuseMap", 0 ) );

        shadergen.remapStateSet( rootStateSet.get() );
    }
    else
    {
        shadergen.assignUberProgram( rootStateSet.get() );
    }

    loadedModel->accept( shadergen );

    if( !outputFilename.empty() )
    {
        osgDB::writeNodeFile( *loadedModel, outputFilename );
        osgDB::writeObjectFile( *( viewer.getCamera()->getStateSet() ),
                                "rootStateSet.osgt" );
        return 0;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occurred when parsing the program arguments.
    if( arguments.errors() )
    {
        arguments.writeErrorMessages( std::cout );
        return 1;
    }

    viewer.setSceneData( loadedModel.get() );

    viewer.realize();
    return viewer.run();
}
