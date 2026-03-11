/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgmultiviewpaging example application
 */
#include <iostream>
#include <osg/core/io_utils.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Camera.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/nodes/Projection.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/BlendFunc.hpp>
#include <osg/state/Depth.hpp>
#include <osg/state/FrontFace.hpp>
#include <osg/state/Material.hpp>
#include <osg/state/PolygonOffset.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgGA/manipulators/FlightManipulator.hpp>
#include <osgGA/manipulators/TerrainManipulator.hpp>
#include <osgText/Text>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgViewer/core/CompositeViewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

class MyPager : public osgDB::DatabasePager
{
    public:

        virtual void
        updateSceneGraph( const osg::FrameStamp& frameStamp )
        {
            if( frameStamp.getFrameNumber() % 60 == 0 )
            {
                osg::Timer_t start = osg::Timer::instance()->tick();
                osgDB::DatabasePager::updateSceneGraph( frameStamp );
                double d =
                    osg::Timer::instance()->delta_m( start,
                                                     osg::Timer::instance()->tick() );
                std::cout << "DatabasePager update took " << d
                          << " ms. Length of active nodes = "
                          << _activePagedLODList->size() << std::endl;
            }
        }
};

int
main( int    argc,
      char** argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser     arguments( &argc, argv );

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> scene = osgDB::readRefNodeFiles( arguments );

    if( !scene )
    {
        scene = osgDB::readRefNodeFile(
            "http://www.openscenegraph.org/data/earth_bayarea/earth.ive"
        );
    }

    if( !scene )
    {
        std::cout << argv[0] << ": requires filename argument." << std::endl;
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

    osgViewer::CompositeViewer viewer( arguments );

    if( viewer.getNumViews() == 0 )
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

        osg::ref_ptr<osg::GraphicsContext::Traits> traits =
            new osg::GraphicsContext::Traits;
        traits->x                = 1'000;
        traits->y                = 100;
        traits->width            = 640;
        traits->height           = 480;
        traits->windowDecoration = true;
        traits->doubleBuffer     = true;
        traits->sharedContext    = 0;
        traits->readDISPLAY();
        traits->setUndefinedScreenDetailsToDefaultScreen();

        osg::ref_ptr<osg::GraphicsContext> gc =
            osg::GraphicsContext::createGraphicsContext( traits.get() );
        if( gc.valid() )
        {
            osg::notify( osg::INFO )
                << "  GraphicsWindow has been created successfully." << std::endl;

            // need to ensure that the window is cleared make sure that the complete
            // window is set the correct colour rather than just the parts of the window
            // that are under the camera's viewports
            gc->setClearColor( osg::vec4( 0.2F, 0.2F, 0.6F, 1.0F ) );
            gc->setClearMask( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        }
        else
        {
            osg::notify( osg::NOTICE )
                << "  GraphicsWindow has not been created successfully." << std::endl;
        }

        // view one
        {
            osgViewer::View* view = new osgViewer::View;
            view->setName( "View one" );
            viewer.addView( view );

            view->setSceneData( scene );
            view->getCamera()->setName( "Cam one" );
            view->getCamera()->setViewport(
                new osg::Viewport( 0, 0, traits->width / 2, traits->height / 2 )
            );
            view->getCamera()->setGraphicsContext( gc.get() );

            // add the state manipulator
            osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator =
                new osgGA::StateSetManipulator;
            statesetManipulator->setStateSet( view->getCamera()->getOrCreateStateSet() );

            view->setCameraManipulator( new osgGA::TerrainManipulator );
            view->addEventHandler( statesetManipulator.get() );

            view->addEventHandler( new osgViewer::StatsHandler );
            view->addEventHandler( new osgViewer::HelpHandler );
            view->addEventHandler( new osgViewer::WindowSizeHandler );
            view->addEventHandler( new osgViewer::ThreadingHandler );
            view->addEventHandler( new osgViewer::RecordCameraPathHandler );
        }

        // view two
        {
            osgViewer::View* view = new osgViewer::View;
            view->setName( "View two" );
            viewer.addView( view );

            view->setSceneData( scene );
            view->getCamera()->setName( "Cam two" );
            view->getCamera()->setViewport( new osg::Viewport( traits->width / 2,
                                                               0,
                                                               traits->width / 2,
                                                               traits->height / 2 ) );
            view->getCamera()->setGraphicsContext( gc.get() );
            view->setCameraManipulator( new osgGA::TerrainManipulator );
        }

        // view three
        {
            osgViewer::View* view = new osgViewer::View;
            view->setName( "View three" );
            viewer.addView( view );

            view->setSceneData( scene );

            view->getCamera()->setName( "Cam three" );
            view->getCamera()->setProjectionMatrixAsPerspective(
                30.0,
                double( traits->width ) / double( traits->height / 2 ),
                1.0,
                1000.0
            );
            view->getCamera()->setViewport( new osg::Viewport( 0,
                                                               traits->height / 2,
                                                               traits->width,
                                                               traits->height / 2 ) );
            view->getCamera()->setGraphicsContext( gc.get() );
            view->setCameraManipulator( new osgGA::TerrainManipulator );

            // attach custom database pager
            view->setDatabasePager( new MyPager );
            view->getDatabasePager()->setTargetMaximumNumberOfPageLOD( 1 );
        }
    }

    while( arguments.read( "-s" ) )
    {
        viewer.setThreadingModel( osgViewer::CompositeViewer::SingleThreaded );
    }
    while( arguments.read( "-g" ) )
    {
        viewer.setThreadingModel( osgViewer::CompositeViewer::CullDrawThreadPerContext );
    }
    while( arguments.read( "-c" ) )
    {
        viewer.setThreadingModel(
            osgViewer::CompositeViewer::CullThreadPerCameraDrawThreadPerContext
        );
    }

    // run the viewer's main frame loop
    return viewer.run();
}
