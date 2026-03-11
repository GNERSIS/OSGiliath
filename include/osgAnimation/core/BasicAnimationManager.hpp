/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Simple animation manager that plays and blends animations.
 * Registered as an update callback on the animation root.
 */
#pragma once

#include <osg/core/FrameStamp.hpp>
#include <osg/core/Inherit.hpp>
#include <osg/nodes/Group.hpp>
#include <osgAnimation/core/AnimationManagerBase.hpp>
#include <osgAnimation/core/Export.hpp>

namespace osgAnimation
{

    class OSGANIMATION_EXPORT BasicAnimationManager
        : public osg::Inherit<AnimationManagerBase, BasicAnimationManager>
    {
        public:

            OSG_REGISTER_TYPE( osgAnimation,
                               BasicAnimationManager )

            BasicAnimationManager();
            BasicAnimationManager( const BasicAnimationManager& b,
                                   const osg::CopyOp&           copyop =
                                       osg::CopyOp::SHALLOW_COPY );
            BasicAnimationManager( const AnimationManagerBase& b,
                                   const osg::CopyOp&          copyop =
                                       osg::CopyOp::SHALLOW_COPY );
            virtual ~BasicAnimationManager();

            void
            update( double time );

            void
            playAnimation( Animation* pAnimation,
                           int        priority = 0,
                           float      weight   = 1.0 );
            bool
            stopAnimation( Animation* pAnimation );

            bool
            findAnimation( Animation* pAnimation );
            bool
            isPlaying( Animation* pAnimation );
            bool
            isPlaying( const std::string& animationName );

            void
            stopAll();

        protected:

            typedef std::map<int, AnimationList> AnimationLayers;
            AnimationLayers                      _animationsPlaying;
            double                               _lastUpdate;
    };

}
