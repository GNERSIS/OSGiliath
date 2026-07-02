/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgcompositeviewer example application
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
#include <osgFX/Scribe.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgGA/manipulators/FlightManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgText/Text.hpp>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgViewer/core/CompositeViewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

// class to handle events with a pick
class PickHandler : public osgGA::GUIEventHandler
{
    public:

        PickHandler() :
            _mx( 0.0F ),
            _my( 0.0F )
        {
        }

        ~PickHandler()
        {
        }

        bool
        handle( const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter&      aa )
        {
            osgViewer::View* view = dynamic_cast<osgViewer::View*>( &aa );
            if( !view )
            {
                return false;
            }

            switch( ea.getEventType() )
            {
                case( osgGA::GUIEventAdapter::PUSH ) :
                    {
                        _mx = ea.getX();
                        _my = ea.getY();
                        break;
                    }
                case( osgGA::GUIEventAdapter::RELEASE ) :
                    {
                        if( _mx == ea.getX() && _my == ea.getY() )
                        {
                            pick( view, ea );
                        }
                        break;
                    }
                default :
                    break;
            }
            return false;
        }

        void
        pick( osgViewer::View*              view,
              const osgGA::GUIEventAdapter& event )
        {
            osg::Node*                                     node   = 0;
            osg::Group*                                    parent = 0;

            osgUtil::LineSegmentIntersector::Intersections intersections;
            if( view->computeIntersections( event, intersections ) )
            {
                osgUtil::LineSegmentIntersector::Intersection intersection =
                    *intersections.begin();
                osg::NodePath& nodePath = intersection.nodePath;
                node   = ( nodePath.size() >= 1 ) ? nodePath[nodePath.size() - 1] : 0;
                parent = ( nodePath.size() >= 2 )
                           ? dynamic_cast<osg::Group*>( nodePath[nodePath.size() - 2] )
                           : 0;
            }

            // now we try to decorate the hit node by the osgFX::Scribe to show that its
            // been "picked"
            if( parent && node )
            {
                osgFX::Scribe* parentAsScribe = dynamic_cast<osgFX::Scribe*>( parent );
                if( !parentAsScribe )
                {
                    // node not already picked, so highlight it with an osgFX::Scribe
                    osgFX::Scribe* scribe = new osgFX::Scribe();
                    scribe->addChild( node );
                    parent->replaceChild( node, scribe );
                }
                else
                {
                    // node already picked so we want to remove scribe to unpick it.
                    osg::Node::ParentList parentList = parentAsScribe->getParents();
                    for( osg::Node::ParentList::iterator itr = parentList.begin();
                         itr != parentList.end();
                         ++itr )
                    {
                        ( *itr )->replaceChild( parentAsScribe, node );
                    }
                }
            }
        }

        float _mx, _my;
};

class EventHandler : public osgGA::GUIEventHandler
{
    public:

        EventHandler()
        {
        }

        ~EventHandler()
        {
        }

        bool
        handle( const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter&      aa )
        {
            osgViewer::View* view = dynamic_cast<osgViewer::View*>( &aa );
            if( !view )
            {
                return false;
            }

            switch( ea.getEventType() )
            {
                case( osgGA::GUIEventAdapter::KEYDOWN ) :
                case( osgGA::GUIEventAdapter::KEYUP ) :
                    OSG_NOTICE << "View " << view << ", name=" << view->getName()
                               << " keyboard event " << ea.getEventType()
                               << " key=" << ea.getKey() << " ea.getX()=" << ea.getX()
                               << " ea.getY()=" << ea.getY() << std::endl;
                    break;

                case( osgGA::GUIEventAdapter::MOVE ) :
                case( osgGA::GUIEventAdapter::DRAG ) :
                case( osgGA::GUIEventAdapter::PUSH ) :
                case( osgGA::GUIEventAdapter::RELEASE ) :
                    OSG_NOTICE << "View " << view << ", name=" << view->getName()
                               << " mouse event " << ea.getEventType()
                               << " ea.getX()=" << ea.getX()
                               << " ea.getY()=" << ea.getY() << std::endl;
                    break;

                default :
                    // OSG_NOTICE<<"View "<<view<<", name="<<view->getName()<<" general
                    // event "<<ea.getEventType()<<std::endl;
                    break;
            }

            return false;
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
        std::cout << argv[0] << ": requires filename argument." << std::endl;
        return 1;
    }

    // construct the viewer.
    // Headless capture: early check before viewer construction
    {
        std::string headlessOutput;
        if( arguments.read( "--headless", headlessOutput ) )
        {
            auto node = osgDB::readRefNodeFiles( arguments );
            if( !node )
            {
                node = osgDB::readRefNodeFile( "lantern.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    osgViewer::CompositeViewer viewer( arguments );

    if( arguments.read( "-1" ) )
    {
        {
            osgViewer::View* view = new osgViewer::View;
            view->setName( "Single view" );
            view->setSceneData( osgDB::readRefNodeFile( "lantern.glb" ) );

            view->addEventHandler( new osgViewer::StatsHandler );

            view->setUpViewInWindow( 1'000, 100, 640, 480 );
            view->setCameraManipulator( new osgGA::TrackballManipulator );
            viewer.addView( view );
        }
    }

    if( arguments.read( "-2" ) )
    {

        // view one
        {
            osgViewer::View* view = new osgViewer::View;
            view->setName( "View one" );
            viewer.addView( view );

            view->setUpViewInWindow( 1'000, 100, 640, 480 );
            view->setSceneData( scene.get() );
            view->setCameraManipulator( new osgGA::TrackballManipulator );

            // add the state manipulator
            osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator =
                new osgGA::StateSetManipulator;
            statesetManipulator->setStateSet( view->getCamera()->getOrCreateStateSet() );

            view->addEventHandler( statesetManipulator.get() );
        }

        // view two
        {
            osgViewer::View* view = new osgViewer::View;
            view->setName( "View two" );
            viewer.addView( view );

            view->setUpViewInWindow( 1'000, 600, 640, 480 );
            view->setSceneData( scene.get() );
            view->setCameraManipulator( new osgGA::TrackballManipulator );

            view->addEventHandler( new osgViewer::StatsHandler );

            // add the handler for doing the picking
            view->addEventHandler( new PickHandler() );
        }
    }

    if( arguments.read( "-4" ) )
    {

        // view one
        {
            osgViewer::View* view = new osgViewer::View;
            view->setName( "View one" );
            viewer.addView( view );

            view->setUpViewInWindow( 1'000, 100, 640, 480 );
            view->setSceneData( scene.get() );
            view->setCameraManipulator( new osgGA::TrackballManipulator );

            // add the state manipulator
            osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator =
                new osgGA::StateSetManipulator;
            statesetManipulator->setStateSet( view->getCamera()->getOrCreateStateSet() );

            view->addEventHandler( statesetManipulator.get() );

            view->addEventHandler( new EventHandler() );
        }

        // view two
        {
            osgViewer::View* view = new osgViewer::View;
            view->setName( "View two" );
            viewer.addView( view );

            view->setUpViewInWindow( 1'000, 600, 640, 480 );
            view->setSceneData( scene.get() );
            view->setCameraManipulator( new osgGA::TrackballManipulator );

            view->addEventHandler( new osgViewer::StatsHandler );

            // add the handler for doing the picking
            view->addEventHandler( new PickHandler() );

            view->addEventHandler( new EventHandler() );
        }
    }

    if( arguments.read( "-3" ) || viewer.getNumViews() == 0 )
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

            view->setSceneData( scene.get() );
            view->getCamera()->setName( "Cam one" );
            view->getCamera()->setViewport(
                new osg::Viewport( 0, 0, traits->width / 2, traits->height / 2 )
            );
            view->getCamera()->setGraphicsContext( gc.get() );
            view->setCameraManipulator( new osgGA::TrackballManipulator );

            // add the state manipulator
            osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator =
                new osgGA::StateSetManipulator;
            statesetManipulator->setStateSet( view->getCamera()->getOrCreateStateSet() );

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

            view->setSceneData( scene.get() );
            view->getCamera()->setName( "Cam two" );
            view->getCamera()->setViewport( new osg::Viewport( traits->width / 2,
                                                               0,
                                                               traits->width / 2,
                                                               traits->height / 2 ) );
            view->getCamera()->setGraphicsContext( gc.get() );
            view->setCameraManipulator( new osgGA::TrackballManipulator );

            // add the handler for doing the picking
            view->addEventHandler( new PickHandler() );
        }

        // view three
        {
            osgViewer::View* view = new osgViewer::View;
            view->setName( "View three" );
            viewer.addView( view );

            view->setSceneData( osgDB::readRefNodeFile( "damaged_helmet.glb" ) );

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
            view->setCameraManipulator( new osgGA::TrackballManipulator );
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
