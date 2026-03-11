/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Timeline action that blends an animation in over a duration.
 * Ramps the animation weight from 0 to target.
 */
#pragma once

#include <osgAnimation/core/Action.hpp>
#include <osgAnimation/core/Export.hpp>

namespace osgAnimation
{

    /// blend in from 0 to weight in duration
    class OSGANIMATION_EXPORT ActionBlendIn : public Action
    {
        public:

            META_Action( osgAnimation,
                         ActionBlendIn );
            ActionBlendIn();
            ActionBlendIn( const ActionBlendIn& a,
                           const osg::CopyOp&   c );
            ActionBlendIn( Animation* animation,
                           double     duration,
                           double     weight );

            double
            getWeight() const
            {
                return _weight;
            }

            Animation*
            getAnimation()
            {
                return _animation.get();
            }

            void
            computeWeight( unsigned int frame );

        protected:

            double                  _weight;
            osg::ref_ptr<Animation> _animation;
    };

}
