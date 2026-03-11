/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Visitor that connects animation channels to their target
 * nodes. Resolves named targets in the animation graph.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/state/StateSet.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osgAnimation/core/Animation.hpp>

namespace osgAnimation
{

    class AnimationUpdateCallbackBase;

    /** This class is instancied by the AnimationManagerBase, it will link animation
     * target to updatecallback that have the same name
     */
    class OSGANIMATION_EXPORT LinkVisitor : public osg::DualModeVisitor
    {
        public:

            LinkVisitor();

            OSG_REGISTER_TYPE( osgAnimation,
                               LinkVisitor )

            using DualModeVisitor::apply;

            void
            apply( osg::Node& node ) override;
            void
            apply( osg::Geode& node ) override;

            AnimationList&
            getAnimationList();
            void
            reset() override;

            unsigned int
            getNbLinkedTarget() const
            {
                return _nbLinkedTarget;
            }

        protected:

            void
            handle_stateset( osg::StateSet* stateset );
            void
                          link( osgAnimation::AnimationUpdateCallbackBase* cb );

            // animation list to link
            AnimationList _animations;

            // number of success link done
            unsigned int  _nbLinkedTarget;
    };

}
