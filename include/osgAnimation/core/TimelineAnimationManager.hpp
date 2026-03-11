/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Animation manager driven by a Timeline rather than simple
 * play/stop. Evaluates scheduled Actions each frame.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osgAnimation/core/AnimationManagerBase.hpp>
#include <osgAnimation/core/Export.hpp>
#include <osgAnimation/core/Timeline.hpp>

namespace osgAnimation
{

    class OSGANIMATION_EXPORT TimelineAnimationManager
        : public osg::Inherit<AnimationManagerBase, TimelineAnimationManager>
    {
        protected:

            osg::ref_ptr<Timeline> _timeline;

        public:

            OSG_REGISTER_TYPE( osgAnimation,
                               TimelineAnimationManager )
            TimelineAnimationManager();
            TimelineAnimationManager( const AnimationManagerBase& manager );
            TimelineAnimationManager( const TimelineAnimationManager& nc,
                                      const osg::CopyOp& );

            Timeline*
            getTimeline()
            {
                return _timeline.get();
            }

            const Timeline*
            getTimeline() const
            {
                return _timeline.get();
            }

            void
            update( double time );
    };

}
