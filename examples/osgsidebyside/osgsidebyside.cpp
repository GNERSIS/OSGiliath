/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgsidebyside example application
 */
#include <iostream>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Switch.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/BlendFunc.hpp>
#include <osg/state/Depth.hpp>
#include <osg/state/PolygonMode.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/io/WriteFile.hpp>
#include <osgGA/events/GUIEventHandler.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgGA/manipulators/AnimationPathManipulator.hpp>
#include <osgGA/manipulators/FlightManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgSim/DOFTransform.hpp>
#include <osgSim/MultiSwitch.hpp>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

using namespace osg;
using namespace osgGA;

class SwitchDOFHandler : public osgGA::GUIEventHandler
{
    public:

        SwitchDOFHandler()
        {
        }

        void
        nextSwitch()
        {
            for( size_t i = 0; i < mSwitches.size(); i++ )
            {
                if( mSwitches[i]->getSwitchSetList().size() > 1 )
                {
                    // Toggle through switchsets
                    unsigned int nextSwitchSet = mSwitches[i]->getActiveSwitchSet();
                    nextSwitchSet++;
                    if( nextSwitchSet >= mSwitches[i]->getSwitchSetList().size() )
                    {
                        nextSwitchSet = 0;
                    }
                    mSwitches[i]->setActiveSwitchSet( nextSwitchSet );
                }
                else if( mSwitches[i]->getSwitchSetList().size() == 1 )
                {
                    // If we have only one switchset, toggle values within this switchset
                    osgSim::MultiSwitch::ValueList values =
                        mSwitches[i]->getValueList( 0 );
                    for( size_t j = 0; j < values.size(); j++ )
                    {
                        if( values[j] )
                        {
                            unsigned int nextValue = j + 1;
                            if( nextValue >= values.size() )
                            {
                                nextValue = 0;
                            }
                            mSwitches[i]->setSingleChildOn( 0, nextValue );
                        }
                    }
                }
            }
        }

        void
        multiplyAnimation( float scale )
        {
            for( size_t i = 0; i < mDofs.size(); i++ )
            {
                mDofs[i]->setIncrementHPR( mDofs[i]->getIncrementHPR() * scale );
                mDofs[i]->setIncrementScale( mDofs[i]->getIncrementScale() * scale );
                mDofs[i]->setIncrementTranslate( mDofs[i]->getIncrementTranslate() *
                                                 scale );
            }
        }

        bool
        handle( const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter&      aa )
        {
            osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>( &aa );
            if( !viewer )
            {
                return false;
            }

            if( ea.getHandled() )
            {
                return false;
            }

            if( ea.getEventType() == GUIEventAdapter::KEYDOWN )
            {

                switch( ea.getKey() )
                {
                    case osgGA::GUIEventAdapter::KEY_Right :
                        // Toggle next switch
                        nextSwitch();
                        return true;
                        break;
                    case osgGA::GUIEventAdapter::KEY_Up :
                        // Increase animation speed
                        multiplyAnimation( 2 );
                        return true;
                        break;
                    case osgGA::GUIEventAdapter::KEY_Down :
                        // Decrease animation speed
                        multiplyAnimation( 0.5 );
                        return true;
                        break;
                }
            }
            return false;
        }

        void
        collectNodesOfInterest( osg::Node* node )
        {
            CollectNodes cn( this );
            node->accept( cn );
        }

    private:

        friend class CollectNodes;

        class CollectNodes : public osg::DualModeVisitor
        {
            public:

                SwitchDOFHandler* _parent;

                CollectNodes( SwitchDOFHandler* parent ) :
                    osg::DualModeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
                    _parent( parent )
                {
                }

                virtual void
                apply( Group& node )
                {
                    osgSim::MultiSwitch* pMSwitch =
                        dynamic_cast<osgSim::MultiSwitch*>( &node );
                    if( pMSwitch )
                    {
                        _parent->mSwitches.push_back( pMSwitch );
                    }

                    traverse( node );
                }

                virtual void
                apply( Transform& node )
                {
                    osgSim::DOFTransform* pDof =
                        dynamic_cast<osgSim::DOFTransform*>( &node );
                    if( pDof )
                    {
                        _parent->mDofs.push_back( pDof );
                        pDof->setAnimationOn( true );
                    }

                    traverse( node );
                }
        };

        std::vector<osg::ref_ptr<osgSim::MultiSwitch>>  mSwitches;
        std::vector<osg::ref_ptr<osgSim::DOFTransform>> mDofs;
};

void
singleWindowSideBySideCameras( osgViewer::Viewer& viewer )
{
    osg::GraphicsContext::WindowingSystemInterface* wsi =
        osg::GraphicsContext::getWindowingSystemInterface();
    if( !wsi )
    {
        osg::notify( osg::NOTICE )
            << "Error, no WindowSystemInterface available, cannot create windows."
            << std::endl;
        return;
    }

    unsigned int                           width, height;
    osg::GraphicsContext::ScreenIdentifier main_screen_id;

    main_screen_id.readDISPLAY();
    main_screen_id.setUndefinedScreenDetailsToDefaultScreen();
    wsi->getScreenResolution( main_screen_id, width, height );

    // Fixed window size
    width                                             = 640;
    height                                            = 480;

    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x                                         = 1'000;
    traits->y                                         = 100;
    traits->width                                     = width;
    traits->height                                    = height;
    traits->windowDecoration                          = true;
    traits->doubleBuffer                              = true;
    traits->sharedContext                             = 0;
    traits->readDISPLAY();
    traits->setUndefinedScreenDetailsToDefaultScreen();

    osg::ref_ptr<osg::GraphicsContext> gc =
        osg::GraphicsContext::createGraphicsContext( traits.get() );
    if( gc.valid() )
    {
        osg::notify( osg::INFO )
            << "  GraphicsWindow has been created successfully." << std::endl;

        // need to ensure that the window is cleared make sure that the complete window
        // is set the correct colour rather than just the parts of the window that are
        // under the camera's viewports
        gc->setClearColor( osg::vec4( 0.2F, 0.2F, 0.6F, 1.0F ) );
        gc->setClearMask( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    }
    else
    {
        osg::notify( osg::NOTICE )
            << "  GraphicsWindow has not been created successfully." << std::endl;
    }

    osg::Camera* master = viewer.getCamera();

    // get the default settings for the camera
    double       fovy, aspectRatio, zNear, zFar;
    master->getProjectionMatrixAsPerspective( fovy, aspectRatio, zNear, zFar );

    // reset this for the actual apsect ratio of out created window
    double windowAspectRatio = double( width ) / double( height );
    master->setProjectionMatrixAsPerspective( fovy, windowAspectRatio, 1.0, 10000.0 );

    master->setName( "MasterCam" );

    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setCullMask( 1 );
    camera->setName( "Left" );
    camera->setGraphicsContext( gc.get() );
    camera->setViewport( new osg::Viewport( 0, 0, width / 2, height ) );
    GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
    camera->setDrawBuffer( buffer );
    camera->setReadBuffer( buffer );
    viewer.addSlave( camera.get(), osg::scale( 1.0, 0.5, 1.0 ), osg::dmat4() );

    camera = new osg::Camera;
    camera->setCullMask( 2 );
    camera->setName( "Right" );
    camera->setGraphicsContext( gc.get() );
    camera->setViewport( new osg::Viewport( width / 2, 0, width / 2, height ) );
    buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
    camera->setDrawBuffer( buffer );
    camera->setReadBuffer( buffer );
    viewer.addSlave( camera.get(), osg::scale( 1.0, 0.5, 1.0 ), osg::dmat4() );
}

int
main( int    argc,
      char** argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc, argv );

    if( argc < 2 )
    {
        std::cout << argv[0] << ": requires filename argument." << std::endl;
        return 1;
    }

    std::string outputfile( "output.osgt" );
    while( arguments.read( "-o", outputfile ) )
    {
    }

    // load the scene.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles( arguments );
    if( !loadedModel )
    {
        std::cout << argv[0] << ": No data loaded." << std::endl;
        return 1;
    }

    osg::Group* group  = new osg::Group;

    osg::Group* group1 = new osg::Group;
    group1->addChild( loadedModel );
    group1->setNodeMask( 1 );

    osgDB::writeNodeFile( *loadedModel, outputfile );
    osg::ref_ptr<osg::Node> convertedModel = osgDB::readRefNodeFile( outputfile );

    osg::Group*             group2         = new osg::Group;
    group2->addChild( convertedModel.get() );
    group2->setNodeMask( 2 );

    group->addChild( group1 );
    group->addChild( group2 );

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

    while( arguments.read( "-s" ) )
    {
        viewer.setThreadingModel( osgViewer::Viewer::SingleThreaded );
    }
    while( arguments.read( "-g" ) )
    {
        viewer.setThreadingModel( osgViewer::Viewer::CullDrawThreadPerContext );
    }
    while( arguments.read( "-d" ) )
    {
        viewer.setThreadingModel( osgViewer::Viewer::DrawThreadPerContext );
    }
    while( arguments.read( "-c" ) )
    {
        viewer.setThreadingModel(
            osgViewer::Viewer::CullThreadPerCameraDrawThreadPerContext
        );
    }

    singleWindowSideBySideCameras( viewer );

    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::ThreadingHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler() );
    viewer.addEventHandler( new osgViewer::LODScaleHandler() );
    viewer.addEventHandler( new osgGA::StateSetManipulator() );

    SwitchDOFHandler* switchOFHandler = new SwitchDOFHandler;
    viewer.addEventHandler( switchOFHandler );

    // Activate DOF animations and collect switches
    switchOFHandler->collectNodesOfInterest( group );

    viewer.setSceneData( group );

    viewer.setThreadingModel( osgViewer::Viewer::DrawThreadPerContext );
    viewer.realize();
    viewer.run();

    return 0;
}
