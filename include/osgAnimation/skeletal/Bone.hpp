/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Skeleton bone node carrying bind-pose and inverse-bind matrices.
 * Forms the bone hierarchy for skeletal animation.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osgAnimation/core/Export.hpp>

namespace osgAnimation
{

    // A bone can't have more than one parent Bone, so sharing a part of Bone's hierarchy
    // makes no sense. You can share the entire hierarchy but not only a part of it.
    class OSGANIMATION_EXPORT Bone : public osg::Inherit<osg::MatrixTransform, Bone>
    {
        public:

            typedef osg::dmat4 MatrixType;

            OSG_REGISTER_TYPE( osgAnimation,
                               Bone )
            Bone( const Bone&        b,
                  const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );
            Bone( const std::string& name = "" );

            void
            setDefaultUpdateCallback( const std::string& name = "" );

            Bone*
            getBoneParent();
            const Bone*
            getBoneParent() const;

            const osg::dmat4&
            getMatrixInBoneSpace() const
            {
                return getMatrix();
            }

            const osg::dmat4&
            getMatrixInSkeletonSpace() const
            {
                return _boneInSkeletonSpace;
            }

            const osg::dmat4&
            getInvBindMatrixInSkeletonSpace() const
            {
                return _invBindInSkeletonSpace;
            }

            void
            setMatrixInSkeletonSpace( const osg::dmat4& matrix )
            {
                _boneInSkeletonSpace = matrix;
            }

            void
            setInvBindMatrixInSkeletonSpace( const osg::dmat4& matrix )
            {
                _invBindInSkeletonSpace = matrix;
            }

        protected:

            // bind data
            osg::dmat4 _invBindInSkeletonSpace;

            // bone updated
            osg::dmat4 _boneInSkeletonSpace;
    };

    typedef std::map<std::string, osg::ref_ptr<Bone>> BoneMap;

}
