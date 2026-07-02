/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Controls blending weights between multiple texture layers.
 * Used for terrain multi-texturing with smooth transitions.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/nodes/Group.hpp>
#include <osgFX/Export.hpp>

namespace osgFX
{

    /**
      This node provides control over the which texture units are active and the
      blending weighting between them.
     */
    class OSGFX_EXPORT MultiTextureControl
        : public osg::Inherit<osg::Group, MultiTextureControl>
    {
        public:

            MultiTextureControl();
            MultiTextureControl( const MultiTextureControl& copy,
                                 const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgFX,
                               MultiTextureControl )

            typedef osg::FloatArray TextureWeights;

            void
            setTextureWeights( TextureWeights* twl )
            {
                _textureWeights = twl;
            }

            TextureWeights*
            getTextureWeights()
            {
                return _textureWeights.get();
            }

            const TextureWeights*
            getTextureWeights() const
            {
                return _textureWeights.get();
            }

            void
            setTextureWeight( unsigned int unit,
                              float        weight );

            float
            getTextureWeight( unsigned int unit ) const
            {
                return ( unit < _textureWeights->size() ) ? ( *_textureWeights )[unit]
                                                          : 0.0F;
            }

            unsigned int
            getNumTextureWeights() const
            {
                return static_cast<unsigned int>( _textureWeights->size() );
            }

            void
            setUseTextureWeightsUniform( bool flag )
            {
                _useTextureWeightsUniform = flag;
            }

            bool
            getUseTextureWeightsUniform() const
            {
                return _useTextureWeightsUniform;
            }

        protected:

            virtual ~MultiTextureControl()
            {
            }

            MultiTextureControl&
            operator=( const MultiTextureControl& )
            {
                return *this;
            }

            void
                                         updateStateSet();

            osg::ref_ptr<TextureWeights> _textureWeights;

            bool                         _useTextureWeightsUniform;
    };

}
