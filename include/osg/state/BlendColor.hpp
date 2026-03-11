/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Constant blend color used with GL_CONSTANT_ALPHA and
 * GL_CONSTANT_COLOR blend factors.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/ref_ptr.hpp>
#include <osg/GL>
#include <osg/maths/vec4.hpp>
#include <osg/state/StateAttribute.hpp>

namespace osg
{

    /** Encapsulates OpenGL blend/transparency state. */
    class OSG_EXPORT BlendColor : public osg::Inherit<StateAttribute, BlendColor>
    {
        public:

            BlendColor();

            BlendColor( const osg::vec4& constantColor );

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            BlendColor( const BlendColor& trans,
                        const CopyOp&     copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( trans,
                         copyop ),
                _constantColor( trans._constantColor )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               BlendColor )

            Type
            getType() const override
            {
                return Type::BLENDCOLOR;
            }

            /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            int
            compare( const StateAttribute& sa ) const override
            {
                // Check for equal types, then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( BlendColor, sa )

                    // Compare each parameter in turn against the rhs.
                    COMPARE_StateAttribute_Parameter(
                        _constantColor
                    ) return 0;    // Passed all the above comparison macros, so must be
                                   // equal.
            }

            bool
            getModeUsage( StateAttribute::ModeUsage& usage ) const override
            {
                usage.usesMode( GL_BLEND );
                return true;
            }

            void
            setConstantColor( const osg::vec4& color )
            {
                _constantColor = color;
            }

            inline osg::vec4&
            getConstantColor()
            {
                return _constantColor;
            }

            inline const osg::vec4&
            getConstantColor() const
            {
                return _constantColor;
            }

            void
            apply( State& state ) const override;

        protected:

            virtual ~BlendColor();

            osg::vec4 _constantColor;
    };

}
