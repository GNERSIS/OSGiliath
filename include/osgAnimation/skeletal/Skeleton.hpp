/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Root node of a bone hierarchy. Manages the skeleton's update
 * traversal for computing bone world matrices.
 */
#pragma once

#include <osg/core/Callback.hpp>
#include <osg/core/Inherit.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osgAnimation/core/Export.hpp>

namespace osgAnimation
{

    class OSGANIMATION_EXPORT Skeleton
        : public osg::Inherit<osg::MatrixTransform, Skeleton>
    {
        public:

            OSG_REGISTER_TYPE( osgAnimation,
                               Skeleton )

            class OSGANIMATION_EXPORT UpdateSkeleton
                : public osg::Inherit<osg::NodeCallback, UpdateSkeleton>
            {
                public:

                    OSG_REGISTER_TYPE( osgAnimation,
                                       UpdateSkeleton )
                    UpdateSkeleton();
                    UpdateSkeleton( const UpdateSkeleton&,
                                    const osg::CopyOp& copyop =
                                        osg::CopyOp::SHALLOW_COPY );
                    virtual void
                    operator()( osg::Node*        node,
                                osg::NodeVisitor* nv );
                    bool
                    needToValidate() const;

                protected:

                    bool _needValidate;
            };

            Skeleton();
            Skeleton( const Skeleton&,
                      const osg::CopyOp& );
            void
            setDefaultUpdateCallback();
    };

}
