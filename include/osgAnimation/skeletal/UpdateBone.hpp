/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Update callback applying blended animation channels to
 * a Bone's local transform during the update traversal.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osgAnimation/core/Export.hpp>
#include <osgAnimation/transform/UpdateMatrixTransform.hpp>

namespace osgAnimation
{

    class OSGANIMATION_EXPORT UpdateBone
        : public osg::Inherit<UpdateMatrixTransform, UpdateBone>
    {
        public:

            OSG_REGISTER_TYPE( osgAnimation,
                               UpdateBone )

            UpdateBone( const std::string& name = "" );
            UpdateBone( const UpdateBone&,
                        const osg::CopyOp& );
            void
            operator()( osg::Node*        node,
                        osg::NodeVisitor* nv );
    };

}
