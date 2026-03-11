/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Timeline action invoking a user callback at a scheduled time.
 * Used for event triggers in animation timelines.
 */
#include <osgAnimation/core/ActionCallback.hpp>

#include <osgAnimation/core/Timeline.hpp>

void
osgAnimation::RunAction::operator()( Action* /*action*/,
                                     ActionVisitor* visitor )
{
    Timeline* tm = visitor->getCurrentTimeline();
    tm->addActionNow( _action.get(), _priority );
}
