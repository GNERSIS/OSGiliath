/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osganimationtimeline example application
 */
#include <iostream>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgAnimation/core/ActionAnimation.hpp>
#include <osgAnimation/core/ActionBlendIn.hpp>
#include <osgAnimation/core/ActionBlendOut.hpp>
#include <osgAnimation/core/ActionStripAnimation.hpp>
#include <osgAnimation/core/AnimationManagerBase.hpp>
#include <osgAnimation/core/Timeline.hpp>
#include <osgAnimation/core/TimelineAnimationManager.hpp>
#include <osgAnimation/skeletal/Bone.hpp>
#include <osgAnimation/skeletal/RigGeometry.hpp>
#include <osgAnimation/skeletal/Skeleton.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgGA/manipulators/AnimationPathManipulator.hpp>
#include <osgGA/manipulators/DriveManipulator.hpp>
#include <osgGA/manipulators/FlightManipulator.hpp>
#include <osgGA/manipulators/KeySwitchMatrixManipulator.hpp>
#include <osgGA/manipulators/TerrainManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

struct NoseBegin : public osgAnimation::Action::Callback
{
        virtual void
        operator()( osgAnimation::Action* action,
                    osgAnimation::ActionVisitor* /*nv*/ )
        {
            std::cout << "sacrebleu, it scratches my nose, let me scratch it"
                      << std::endl;
            std::cout << "process NoseBegin call back " << action->getName() << std::endl
                      << std::endl;
        }
};

struct NoseEnd : public osgAnimation::Action::Callback
{
        virtual void
        operator()( osgAnimation::Action* action,
                    osgAnimation::ActionVisitor* /*nv*/ )
        {
            std::cout << "shhhrt shrrrrt shhhhhhrrrrt, haaa it's better" << std::endl;
            std::cout << "process NoseEnd call back " << action->getName() << std::endl
                      << std::endl;
        }
};

struct ExampleTimelineUsage : public osgGA::GUIEventHandler
{
        osg::ref_ptr<osgAnimation::ActionStripAnimation>     _mainLoop;
        osg::ref_ptr<osgAnimation::ActionStripAnimation>     _scratchHead;
        osg::ref_ptr<osgAnimation::ActionStripAnimation>     _scratchNose;
        osg::ref_ptr<osgAnimation::TimelineAnimationManager> _manager;

        bool                                                 _releaseKey;

        ExampleTimelineUsage( osgAnimation::TimelineAnimationManager* manager )
        {
            _releaseKey                             = false;
            _manager                                = manager;

            const osgAnimation::AnimationList& list = _manager->getAnimationList();
            osgAnimation::AnimationMap         map;
            for( osgAnimation::AnimationList::const_iterator it = list.begin();
                 it != list.end();
                 it++ )
            {
                map[( *it )->getName()] = *it;
            }

            _mainLoop = new osgAnimation::ActionStripAnimation( map["Idle_Main"].get(),
                                                                0.0,
                                                                0.0 );
            _mainLoop->setLoop( 0 );    // means forever

            _scratchHead = new osgAnimation::ActionStripAnimation(
                map["Idle_Head_Scratch.02"].get(),
                0.2,
                0.3
            );
            _scratchHead->setLoop( 1 );    // one time

            map["Idle_Nose_Scratch.01"]->setDuration(
                10.0
            );    // set this animation duration to 10 seconds
            _scratchNose = new osgAnimation::ActionStripAnimation(
                map["Idle_Nose_Scratch.01"].get(),
                0.2,
                0.3
            );
            _scratchNose->setLoop( 1 );    // one time

            // add the main loop at priority 0 at time 0.

            osgAnimation::Timeline* tml = _manager->getTimeline();
            tml->play();
            tml->addActionAt( 0.0, _mainLoop.get(), 0 );

            // add a scratch head priority 1 at 3.0 second.
            tml->addActionAt( 5.0, _scratchHead.get(), 1 );

            // populate time with scratch head
            for( int i = 1; i < 20; i++ )
            {
                // we add a scratch head priority 1 each 10 second
                // note:
                //      it's possible to add the same instance more than once on the
                //      timeline the only things you need to take care is if you remove
                //      it. It will remove all instance that exist on the timeline. If
                //      you need to differentiate it's better to create a new instance
                tml->addActionAt( 5.0 + 10.0 * i, _scratchHead.get(), 1 );
            }

            // we will add the scratch nose action only when the player hit a key
            // in the operator()

            // now we will add callback at end and begin of animation of
            // Idle_Nose_Scratch.02
            _scratchNose->setCallback( 0.0, new NoseBegin );
            _scratchNose->setCallback( _scratchNose->getNumFrames() - 1, new NoseEnd );
        }

        bool
        handle( const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter& )
        {
            if( ea.getEventType() == osgGA::GUIEventAdapter::KEYUP )
            {
                _releaseKey = true;
            }
            return false;
        }

        virtual void
        operator()( osg::Node*        node,
                    osg::NodeVisitor* nv )
        {
            if( nv && nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR )
            {
                if( _releaseKey )    // we hit a key and release it execute an action
                {
                    osgAnimation::Timeline* tml = _manager->getTimeline();
                    // don't play if already playing
                    if( !tml->isActive( _scratchNose.get() ) )
                    {
                        // add this animation on top of two other
                        // we add one to evaluate the animation at the next frame, else
                        // we will miss the current frame
                        tml->addActionAt( tml->getCurrentFrame() + 1,
                                          _scratchNose.get(),
                                          2 );
                    }
                    _releaseKey = false;
                }
                traverse( node, nv );
            }
            else
            {
                osgGA::GUIEventHandler::operator()( node, nv );
            }
        }
};

int
main( int   argc,
      char* argv[] )
{
    osg::ArgumentParser arguments( &argc, argv );
    std::cerr << "This example requires a character animation file (e.g. nathan model)"
              << std::endl;

    osg::ArgumentParser psr( &argc, argv );

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

    osgViewer::Viewer viewer( psr );

    std::string       file = "nathan.glb";
    if( argc >= 2 )
    {
        file = psr[1];
    }

    // replace the manager
    osg::ref_ptr<osg::Node> loadedmodel = osgDB::readRefNodeFile( file );
    osg::Group*             root        = dynamic_cast<osg::Group*>( loadedmodel.get() );
    if( !root )
    {
        osg::notify( osg::FATAL ) << "can't read file " << file << std::endl;
        return 1;
    }
    osgAnimation::AnimationManagerBase* animationManager =
        dynamic_cast<osgAnimation::AnimationManagerBase*>( root->getUpdateCallback() );
    if( !animationManager )
    {
        osg::notify( osg::FATAL ) << "Did not find AnimationManagerBase updateCallback "
                                     "needed to animate elements"
                                  << std::endl;
        return 1;
    }

    osg::ref_ptr<osgAnimation::TimelineAnimationManager> tl =
        new osgAnimation::TimelineAnimationManager( *animationManager );
    root->setUpdateCallback( tl.get() );

    ExampleTimelineUsage* callback = new ExampleTimelineUsage( tl.get() );
    root->setEventCallback( callback );
    root->getUpdateCallback()->addNestedCallback( callback );

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
    viewer.addEventHandler( new osgViewer::HelpHandler( psr.getApplicationUsage() ) );

    // add the LOD Scale handler
    viewer.addEventHandler( new osgViewer::LODScaleHandler );

    // add the screen capture handler
    viewer.addEventHandler( new osgViewer::ScreenCaptureHandler );

    viewer.setSceneData( root );
    return viewer.run();
}
