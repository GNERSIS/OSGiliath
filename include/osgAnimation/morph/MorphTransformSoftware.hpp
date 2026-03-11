/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * CPU-based morph target blending. Interpolates vertex
 * positions on the CPU from morph target arrays.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/observer_ptr.hpp>
#include <osgAnimation/core/Export.hpp>
#include <osgAnimation/skeletal/Bone.hpp>
#include <osgAnimation/skeletal/RigTransform.hpp>

namespace osgAnimation
{

    class MorphGeometry;

    /// This class manage format for software morphing
    class OSGANIMATION_EXPORT MorphTransformSoftware
        : public osg::Inherit<MorphTransform, MorphTransformSoftware>
    {
        public:

            MorphTransformSoftware() :
                _needInit( true )
            {
            }

            MorphTransformSoftware( const MorphTransformSoftware& rts,
                                    const osg::CopyOp&            copyop ) :
                Inherit( rts,
                         copyop ),
                _needInit( true )
            {
            }

            OSG_REGISTER_TYPE( osgAnimation,
                               MorphTransformSoftware )
            bool
            init( MorphGeometry& );
            virtual void
            operator()( MorphGeometry& );

        protected:

            bool _needInit;
    };

}
