/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Animation manager driven by a Timeline rather than simple
 * play/stop. Evaluates scheduled Actions each frame.
 */
#include <osgAnimation/core/TimelineAnimationManager.hpp>

#include <osgAnimation/core/Timeline.hpp>

using namespace osgAnimation;

TimelineAnimationManager::TimelineAnimationManager()
{
    _timeline = new Timeline;
}

TimelineAnimationManager::TimelineAnimationManager(
    const AnimationManagerBase& manager
) :
    Inherit( manager,
             osg::CopyOp::SHALLOW_COPY )
{
    _timeline = new Timeline;
}

TimelineAnimationManager::TimelineAnimationManager( const TimelineAnimationManager& nc,
                                                    const osg::CopyOp& co ) :
    Inherit( nc,
             co )
{
    _timeline = new Timeline( *nc.getTimeline(), co );
}

void
TimelineAnimationManager::update( double time )
{
    // clearTargets();
    _timeline->setAnimationManager( this );
    _timeline->update( time );
}
