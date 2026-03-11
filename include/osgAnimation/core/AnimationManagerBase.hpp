/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract animation manager handling animation blending
 * and timeline control. Base for BasicAnimationManager.
 */
#pragma once

#include <osg/core/FrameStamp.hpp>
#include <osg/nodes/Group.hpp>
#include <osgAnimation/core/Animation.hpp>
#include <osgAnimation/core/Export.hpp>
#include <osgAnimation/skeletal/LinkVisitor.hpp>

namespace osgAnimation
{

    class OSGANIMATION_EXPORT AnimationManagerBase : public osg::NodeCallback
    {
        public:

            typedef std::set<osg::ref_ptr<Target>> TargetSet;

            AnimationManagerBase();
            AnimationManagerBase( const AnimationManagerBase& b,
                                  const osg::CopyOp&          copyop =
                                      osg::CopyOp::SHALLOW_COPY );
            virtual ~AnimationManagerBase();
            virtual void
            buildTargetReference();
            virtual void
            registerAnimation( Animation* );
            virtual void
            unregisterAnimation( Animation* );
            virtual void
            link( osg::Node* subgraph );
            virtual void
            update( double t ) = 0;
            virtual bool
            needToLink() const;

            const AnimationList&
            getAnimationList() const
            {
                return _animations;
            }

            AnimationList&
            getAnimationList()
            {
                return _animations;
            }

            // uniformisation of the API
            inline Animation*
            getRegisteredAnimation( unsigned int i )
            {
                return _animations[i].get();
            }

            inline unsigned int
            getNumRegisteredAnimations() const
            {
                return static_cast<unsigned int>( _animations.size() );
            }

            inline void
            addRegisteredAnimation( Animation* animation )
            {
                _needToLink = true;
                _animations.push_back( animation );
                buildTargetReference();
            }

            void
            removeRegisteredAnimation( Animation* animation );

            /** Callback method called by the NodeVisitor when visiting a node.*/
            virtual void
            operator()( osg::Node*        node,
                        osg::NodeVisitor* nv );

            /** Reset the value of targets
                this Operation must be done each frame */
            void
            clearTargets();

            LinkVisitor*
            getOrCreateLinkVisitor();
            void
            setLinkVisitor( LinkVisitor* );

            /// set a flag to define the behaviour
            void
            setAutomaticLink( bool );
            bool
            getAutomaticLink() const;

            bool
            isAutomaticLink() const
            {
                return getAutomaticLink();
            }

            void
            dirty();

        protected:

            osg::ref_ptr<LinkVisitor> _linker;
            AnimationList             _animations;
            TargetSet                 _targets;
            bool                      _needToLink;
            bool                      _automaticLink;
    };

}
