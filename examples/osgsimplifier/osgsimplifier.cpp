/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgsimplifier example application
 */
#include <iostream>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/io/WriteFile.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgUtil/optimization/Simplifier.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

class KeyboardEventHandler : public osgGA::GUIEventHandler
{
    public:

        KeyboardEventHandler( unsigned int&      flag,
                              const std::string& filename ) :
            _flag( flag ),
            _outputFilename( filename )
        {
        }

        virtual bool
        handle( const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter&      aa )
        {
            switch( ea.getEventType() )
            {
                case( osgGA::GUIEventAdapter::KEYDOWN ) :
                    {
                        if( ea.getKey() == 'n' )
                        {
                            _flag = 1;
                            return true;
                        }
                        if( ea.getKey() == 'p' )
                        {
                            _flag = 2;
                            return true;
                        }
                        if( ea.getKey() == 'o' )
                        {
                            osgViewer::View* view =
                                dynamic_cast<osgViewer::Viewer*>( aa.asView() );
                            osg::Node* sceneData = view ? view->getSceneData() : 0;
                            if( sceneData )
                            {
                                OSG_NOTICE << "Witten model to file: " << _outputFilename
                                           << std::endl;
                                osgDB::writeNodeFile( *sceneData, _outputFilename );
                            }
                            return true;
                        }
                        break;
                    }
                default :
                    break;
            }
            return false;
        }

    private:

        unsigned int& _flag;
        std::string   _outputFilename;
};

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
        " examples illustrates simplification of triangle meshes."
    );
    arguments.getApplicationUsage()->setCommandLineUsage(
        arguments.getApplicationName() + " [options] filename ..."
    );
    arguments.getApplicationUsage()->addCommandLineOption( "-h or --help",
                                                           "Display this information" );
    arguments.getApplicationUsage()->addCommandLineOption( "--ratio <ratio>",
                                                           "Specify the sample ratio",
                                                           "0.5]" );
    arguments.getApplicationUsage()->addCommandLineOption( "--max-error <error>",
                                                           "Specify the maximum error",
                                                           "4.0" );

    std::string outputFilename = "model.osgt";

    float       sampleRatio    = 0.5F;
    float       maxError       = 4.0F;

    // construct the viewer.
    // Headless capture: early check before viewer construction
    {
        std::string headlessOutput;
        if( arguments.read( "--headless", headlessOutput ) )
        {
            auto node = osgDB::readRefNodeFiles( arguments );
            if( !node )
            {
                node = osgDB::readRefNodeFile( "milk_truck.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    osgViewer::Viewer viewer;

    // read the sample ratio if one is supplied
    while( arguments.read( "--ratio", sampleRatio ) )
    {
    }
    while( arguments.read( "--max-error", maxError ) )
    {
    }
    while( arguments.read( "-o", outputFilename ) )
    {
    }

    // if user request help write it out to cout.
    if( arguments.read( "-h" ) || arguments.read( "--help" ) )
    {
        arguments.getApplicationUsage()->write( std::cout );
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

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles( arguments );

    // if not loaded assume no arguments passed in, try use default mode instead.
    if( !loadedModel )
    {
        loadedModel = osgDB::readRefNodeFile( "milk_truck.glb" );
    }

    // if no model has been successfully loaded report failure.
    if( !loadedModel )
    {
        std::cout << arguments.getApplicationName() << ": No data loaded" << std::endl;
        return 1;
    }

    // loadedModel->accept(simplifier);

    unsigned int keyFlag = 0;
    viewer.addEventHandler( new KeyboardEventHandler( keyFlag, outputFilename ) );

    // add the state manipulator
    viewer.addEventHandler(
        new osgGA::StateSetManipulator( viewer.getCamera()->getOrCreateStateSet() )
    );

    // add the window size toggle handler
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );

    // add the stats handler
    viewer.addEventHandler( new osgViewer::StatsHandler );

    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );

    // set the scene to render
    viewer.setSceneData( loadedModel.get() );

    // create the windows and run the threads.
    viewer.realize();

    float multiplier = 0.8F;
    float minRatio   = 0.001F;
    float ratio      = sampleRatio;

    while( !viewer.done() )
    {
        // fire off the cull and draw traversals of the scene.
        viewer.frame();

        if( keyFlag == 1 || keyFlag == 2 )
        {
            if( keyFlag == 1 )
            {
                ratio *= multiplier;
            }
            if( keyFlag == 2 )
            {
                ratio /= multiplier;
            }
            if( ratio < minRatio )
            {
                ratio = minRatio;
            }

            osgUtil::Simplifier simplifier( ratio, maxError );

            std::cout << "Running osgUtil::Simplifier with SampleRatio=" << ratio
                      << " maxError=" << maxError << " ...";
            std::cout.flush();

            osg::ref_ptr<osg::Node> root =
                ( osg::Node* )loadedModel->clone( osg::CopyOp::DEEP_COPY_ALL );

            root->accept( simplifier );

            std::cout << "done" << std::endl;

            viewer.setSceneData( root.get() );
            keyFlag = 0;
        }
    }

    return 0;
}
