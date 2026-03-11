/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Timeline action that blends an animation out over a duration.
 * Ramps the animation weight from current to 0.
 */
#pragma once

#include <osgAnimation/core/Action.hpp>
#include <osgAnimation/core/Export.hpp>

namespace osgAnimation
{

    /// blend out from weight to 0 in duration
    class OSGANIMATION_EXPORT ActionBlendOut : public Action
    {
        public:

            META_Action( osgAnimation,
                         ActionBlendOut );
            ActionBlendOut();
            ActionBlendOut( const ActionBlendOut& a,
                            const osg::CopyOp&    c );
            ActionBlendOut( Animation* animation,
                            double     duration );

            Animation*
            getAnimation()
            {
                return _animation.get();
            }

            double
            getWeight() const
            {
                return _weight;
            }

            void
            computeWeight( unsigned int frame );

        protected:

            double                  _weight;
            osg::ref_ptr<Animation> _animation;
    };

}
