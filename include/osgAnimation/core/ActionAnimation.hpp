/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Timeline action that plays an Animation clip.
 * Wraps Animation with timeline start/duration scheduling.
 */
#pragma once

#include <osgAnimation/core/Action.hpp>
#include <osgAnimation/core/Export.hpp>

namespace osgAnimation
{

    class OSGANIMATION_EXPORT ActionAnimation : public Action
    {
        public:

            META_Action( osgAnimation,
                         ActionAnimation );
            ActionAnimation();
            ActionAnimation( const ActionAnimation& a,
                             const osg::CopyOp&     c );
            ActionAnimation( Animation* animation );
            void
            updateAnimation( unsigned int frame,
                             int          priority );

            Animation*
            getAnimation()
            {
                return _animation.get();
            }

        protected:

            osg::ref_ptr<Animation> _animation;
    };

}
