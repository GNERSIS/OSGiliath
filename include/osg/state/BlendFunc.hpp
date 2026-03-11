/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Alpha blending function state attribute. Configures source and
 * destination blend factors for transparency rendering.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/state/StateAttribute.hpp>

namespace osg
{

    /** Encapsulates OpenGL blend/transparency state.
     *
     *  Blending combines incoming fragment with a fragment
     *  already present in the target buffer.
     *
     *  OpenGL 1.1 supports following source and destination blending factors:
     *  GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
     *  GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA,
     *  GL_ZERO, GL_ONE.
     *
     *  Moreover, there are three source-only blending factors:
     *  GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA_SATURATE
     *  and two destination-only blending factors:
     *  GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR.
     *  OpenGL 1.4 allowed to use these five blending factors
     *  as both - source and destination blending factors.
     *
     *  Following four source and destination blending factors
     *  were added by Imaging subset of OpenGL 1.2
     *  and made mandatory by OpenGL 1.4:
     *  GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR,
     *  GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA
     *
     *  OpenGL 1.4 further provides glBlendFuncSeparate
     *  (promoted from GL_EXT_blend_func_separate).
     *  It makes possible to set blending functions for RGB and Alpha separately.
     *  Before, it was possible to set just one blending function for RGBA.
     */
    class OSG_EXPORT BlendFunc : public osg::Inherit<StateAttribute, BlendFunc>
    {
        public:

            BlendFunc();

            BlendFunc( GLenum source,
                       GLenum destination );
            BlendFunc( GLenum source,
                       GLenum destination,
                       GLenum source_alpha,
                       GLenum destination_alpha );

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            BlendFunc( const BlendFunc& trans,
                       const CopyOp&    copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( trans,
                         copyop ),
                _source_factor( trans._source_factor ),
                _destination_factor( trans._destination_factor ),
                _source_factor_alpha( trans._source_factor_alpha ),
                _destination_factor_alpha( trans._destination_factor_alpha )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               BlendFunc )

            Type
            getType() const override
            {
                return Type::BLENDFUNC;
            }

            /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            int
            compare( const StateAttribute& sa ) const override
            {
                // Check for equal types, then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( BlendFunc, sa )

                    // Compare each parameter in turn against the rhs.
                    COMPARE_StateAttribute_Parameter( _source_factor )
                        COMPARE_StateAttribute_Parameter( _destination_factor )
                            COMPARE_StateAttribute_Parameter( _source_factor_alpha )
                                COMPARE_StateAttribute_Parameter(
                                    _destination_factor_alpha
                                ) return 0;    // Passed all the above comparison macros,
                                               // so must be equal.
            }

            bool
            getModeUsage( StateAttribute::ModeUsage& usage ) const override
            {
                usage.usesMode( GL_BLEND );
                return true;
            }

            enum BlendFuncMode
            {
                DST_ALPHA                = GL_DST_ALPHA,
                DST_COLOR                = GL_DST_COLOR,
                ONE                      = GL_ONE,
                ONE_MINUS_DST_ALPHA      = GL_ONE_MINUS_DST_ALPHA,
                ONE_MINUS_DST_COLOR      = GL_ONE_MINUS_DST_COLOR,
                ONE_MINUS_SRC_ALPHA      = GL_ONE_MINUS_SRC_ALPHA,
                ONE_MINUS_SRC_COLOR      = GL_ONE_MINUS_SRC_COLOR,
                SRC_ALPHA                = GL_SRC_ALPHA,
                SRC_ALPHA_SATURATE       = GL_SRC_ALPHA_SATURATE,
                SRC_COLOR                = GL_SRC_COLOR,
                CONSTANT_COLOR           = GL_CONSTANT_COLOR,
                ONE_MINUS_CONSTANT_COLOR = GL_ONE_MINUS_CONSTANT_COLOR,
                CONSTANT_ALPHA           = GL_CONSTANT_ALPHA,
                ONE_MINUS_CONSTANT_ALPHA = GL_ONE_MINUS_CONSTANT_ALPHA,
                ZERO                     = GL_ZERO,
            };

            inline void
            setFunction( GLenum source,
                         GLenum destination )
            {
                _source_factor            = source;
                _destination_factor       = destination;
                _source_factor_alpha      = source;
                _destination_factor_alpha = destination;
            }

            inline void
            setFunction( GLenum source_rgb,
                         GLenum destination_rgb,
                         GLenum source_alpha,
                         GLenum destination_alpha )
            {
                _source_factor            = source_rgb;
                _destination_factor       = destination_rgb;
                _source_factor_alpha      = source_alpha;
                _destination_factor_alpha = destination_alpha;
            }

            void
            setSource( GLenum source )
            {
                _source_factor = _source_factor_alpha = source;
            }

            inline GLenum
            getSource() const
            {
                return _source_factor;
            }

            void
            setSourceRGB( GLenum source )
            {
                _source_factor = source;
            }

            inline GLenum
            getSourceRGB() const
            {
                return _source_factor;
            }

            void
            setSourceAlpha( GLenum source )
            {
                _source_factor_alpha = source;
            }

            inline GLenum
            getSourceAlpha() const
            {
                return _source_factor_alpha;
            }

            void
            setDestination( GLenum destination )
            {
                _destination_factor = _destination_factor_alpha = destination;
            }

            inline GLenum
            getDestination() const
            {
                return _destination_factor;
            }

            void
            setDestinationRGB( GLenum destination )
            {
                _destination_factor = destination;
            }

            inline GLenum
            getDestinationRGB() const
            {
                return _destination_factor;
            }

            void
            setDestinationAlpha( GLenum destination )
            {
                _destination_factor_alpha = destination;
            }

            inline GLenum
            getDestinationAlpha() const
            {
                return _destination_factor_alpha;
            }

            void
            apply( State& state ) const override;

        protected:

            virtual ~BlendFunc();

            GLenum _source_factor;
            GLenum _destination_factor;
            GLenum _source_factor_alpha;
            GLenum _destination_factor_alpha;
    };

}
