/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Timeline action combining blend-in, play, and blend-out
 * for smooth animation transitions.
 */
#pragma once

#include <osgAnimation/core/Action.hpp>
#include <osgAnimation/core/ActionAnimation.hpp>
#include <osgAnimation/core/ActionBlendIn.hpp>
#include <osgAnimation/core/ActionBlendOut.hpp>
#include <osgAnimation/core/Export.hpp>
#include <osgAnimation/core/FrameAction.hpp>

namespace osgAnimation
{

    // encapsulate animation with blend in blend out for classic usage
    class OSGANIMATION_EXPORT ActionStripAnimation : public Action
    {
        public:

            META_Action( osgAnimation,
                         ActionStripAnimation );

            ActionStripAnimation()
            {
            }

            ActionStripAnimation( const ActionStripAnimation& a,
                                  const osg::CopyOp&          c );
            ActionStripAnimation( Animation* animation,
                                  double     blendInDuration     = 0.0,
                                  double     blendOutDuration    = 0.0,
                                  double     blendInWeightTarget = 1.0 );
            ActionAnimation*
            getAnimation();
            ActionBlendIn*
            getBlendIn();
            ActionBlendOut*
            getBlendOut();
            const ActionAnimation*
            getAnimation() const;
            const ActionBlendIn*
            getBlendIn() const;
            const ActionBlendOut*
            getBlendOut() const;
            unsigned int
            getBlendOutStartFrame() const;

            unsigned int
            getLoop() const;
            void
            setLoop( unsigned int loop );
            void
            traverse( ActionVisitor& visitor );

        protected:

            typedef std::pair<unsigned int, osg::ref_ptr<ActionBlendOut>> FrameBlendOut;
            osg::ref_ptr<ActionBlendIn>                                   _blendIn;
            FrameBlendOut                                                 _blendOut;
            osg::ref_ptr<ActionAnimation>                                 _animation;
    };

}
