/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Timeline action invoking a user callback at a scheduled time.
 * Used for event triggers in animation timelines.
 */
#pragma once

#include <osgAnimation/core/Action.hpp>
#include <osgAnimation/core/Export.hpp>

namespace osgAnimation
{

    /** Callback used to run new action on the timeline.*/
    class OSGANIMATION_EXPORT RunAction : public Action::Callback
    {
        public:

            RunAction( Action* a,
                       int     priority = 0 ) :
                _action( a ),
                _priority( priority )
            {
            }

            virtual void
            operator()( Action*        action,
                        ActionVisitor* visitor );

            Action*
            getAction() const
            {
                return _action.get();
            }

            int
            getPriority() const
            {
                return _priority;
            }

        protected:

            osg::ref_ptr<Action> _action;
            int                  _priority;
    };

}
