/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Blending equation mode (ADD, SUBTRACT, MIN, MAX, etc.).
 * Controls how source and destination colors are combined.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/state/StateAttribute.hpp>

namespace osg
{

    /** Encapsulates OpenGL BlendEquation state. */
    class OSG_EXPORT BlendEquation : public osg::Inherit<StateAttribute, BlendEquation>
    {
        public:

            enum Equation
            {
                RGBA_MIN              = GL_MIN,
                RGBA_MAX              = GL_MAX,
                ALPHA_MIN             = GL_ALPHA_MIN_SGIX,
                ALPHA_MAX             = GL_ALPHA_MAX_SGIX,
                LOGIC_OP              = GL_LOGIC_OP,
                FUNC_ADD              = GL_FUNC_ADD,
                FUNC_SUBTRACT         = GL_FUNC_SUBTRACT,
                FUNC_REVERSE_SUBTRACT = GL_FUNC_REVERSE_SUBTRACT,
            };

            BlendEquation();

            BlendEquation( Equation equation );

            BlendEquation( Equation equationRGB,
                           Equation equationAlpha );

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            BlendEquation( const BlendEquation& trans,
                           const CopyOp&        copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( trans,
                         copyop ),
                _equationRGB( trans._equationRGB ),
                _equationAlpha( trans._equationAlpha )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               BlendEquation )

            Type
            getType() const override
            {
                return Type::BLENDEQUATION;
            }

            /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            int
            compare( const StateAttribute& sa ) const override
            {
                // Check for equal types, then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( BlendEquation, sa )

                    // Compare each parameter in turn against the rhs.
                    COMPARE_StateAttribute_Parameter( _equationRGB )
                        COMPARE_StateAttribute_Parameter(
                            _equationAlpha
                        ) return 0;    // Passed all the above comparison macros, so must
                                       // be equal.
            }

            bool
            getModeUsage( StateAttribute::ModeUsage& usage ) const override
            {
                usage.usesMode( GL_BLEND );
                return true;
            }

            inline void
            setEquation( Equation equation )
            {
                _equationRGB = _equationAlpha = equation;
            }

            inline Equation
            getEquation() const
            {
                return _equationRGB;
            }

            inline void
            setEquationRGB( Equation equation )
            {
                _equationRGB = equation;
            }

            inline Equation
            getEquationRGB() const
            {
                return _equationRGB;
            }

            inline void
            setEquationAlpha( Equation equation )
            {
                _equationAlpha = equation;
            }

            inline Equation
            getEquationAlpha() const
            {
                return _equationAlpha;
            }

            void
            apply( State& state ) const override;

        protected:

            virtual ~BlendEquation();

            Equation _equationRGB, _equationAlpha;
    };

}
