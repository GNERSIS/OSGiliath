/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * GPU-based morph target blending. Uploads target deltas
 * to shaders for hardware-accelerated shape interpolation.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/geometry/Array.hpp>
#include <osg/maths/mat4.hpp>
#include <osgAnimation/core/Export.hpp>
#include <osgAnimation/skeletal/Bone.hpp>
#include <osgAnimation/skeletal/RigTransform.hpp>
#include <osgAnimation/skeletal/VertexInfluence.hpp>

/// texture unit reserved for morphtarget TBO
#define MORPHTRANSHW_DEFAULTMORPHTEXTUREUNIT 7

namespace osgAnimation
{

    class MorphGeometry;

    /// This class manage format for hardware morphing
    class OSGANIMATION_EXPORT MorphTransformHardware
        : public osg::Inherit<MorphTransform, MorphTransformHardware>
    {
        public:

            MorphTransformHardware();

            MorphTransformHardware( const MorphTransformHardware& rth,
                                    const osg::CopyOp&            copyop );

            OSG_REGISTER_TYPE( osgAnimation,
                               MorphTransformHardware )

            virtual void
            operator()( MorphGeometry& );

            inline void
            setShader( osg::Shader* s )
            {
                _shader = s;
            }

            inline const osg::Shader*
            getShader() const
            {
                return _shader.get();
            }

            inline osg::Shader*
            getShader()
            {
                return _shader.get();
            }

            /// texture unit reserved for morphtarget TBO default is 7
            void
            setReservedTextureUnit( unsigned int t )
            {
                _reservedTextureUnit = t;
            }

            unsigned int
            getReservedTextureUnit() const
            {
                return _reservedTextureUnit;
            }

        protected:

            bool
                                       init( MorphGeometry& );

            osg::ref_ptr<osg::Uniform> _uniformTargetsWeight;
            osg::ref_ptr<osg::Shader>  _shader;

            bool                       _needInit;
            unsigned int               _reservedTextureUnit;
    };

}
