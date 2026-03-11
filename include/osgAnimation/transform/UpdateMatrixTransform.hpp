/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Update callback applying blended animation channels to
 * a MatrixTransform's matrix during the update traversal.
 */
#pragma once

#include <osg/core/Callback.hpp>
#include <osg/core/Inherit.hpp>
#include <osgAnimation/core/AnimationUpdateCallback.hpp>
#include <osgAnimation/core/Export.hpp>
#include <osgAnimation/transform/StackedTransform.hpp>

namespace osgAnimation
{

    class OSGANIMATION_EXPORT UpdateMatrixTransform
        : public osg::Inherit<AnimationUpdateCallback<osg::NodeCallback>,
                              UpdateMatrixTransform>
    {
        public:

            OSG_REGISTER_TYPE( osgAnimation,
                               UpdateMatrixTransform )

            UpdateMatrixTransform( const std::string& name = "" );
            UpdateMatrixTransform( const UpdateMatrixTransform& apc,
                                   const osg::CopyOp&           copyop =
                                       osg::CopyOp::SHALLOW_COPY );

            // Callback method called by the NodeVisitor when visiting a node.
            virtual void
            operator()( osg::Node*        node,
                        osg::NodeVisitor* nv );
            virtual bool
            link( osgAnimation::Channel* channel );

            StackedTransform&
            getStackedTransforms()
            {
                return _transforms;
            }

            const StackedTransform&
            getStackedTransforms() const
            {
                return _transforms;
            }

        protected:

            StackedTransform _transforms;
    };

}
