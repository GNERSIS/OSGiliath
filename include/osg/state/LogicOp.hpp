/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Logical pixel operation between fragment and framebuffer.
 * Supports AND, OR, XOR, and other bitwise operations.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/state/StateAttribute.hpp>

namespace osg
{

    /** Encapsulates OpenGL LogicOp state. */
    class OSG_EXPORT LogicOp : public osg::Inherit<StateAttribute, LogicOp>
    {
        public:

            enum class Opcode
            {
                CLEAR         = GL_CLEAR,
                SET           = GL_SET,
                COPY          = GL_COPY,
                COPY_INVERTED = GL_COPY_INVERTED,
                NOOP          = GL_NOOP,
                INVERT        = GL_INVERT,
                AND           = GL_AND,
                NAND          = GL_NAND,
                OR            = GL_OR,
                NOR           = GL_NOR,
                XOR           = GL_XOR,
                EQUIV         = GL_EQUIV,
                AND_REVERSE   = GL_AND_REVERSE,
                AND_INVERTED  = GL_AND_INVERTED,
                OR_REVERSE    = GL_OR_REVERSE,
                OR_INVERTED   = GL_OR_INVERTED,
            };

            LogicOp();

            LogicOp( Opcode opcode );

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            LogicOp( const LogicOp& trans,
                     const CopyOp&  copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( trans,
                         copyop ),
                _opcode( trans._opcode )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               LogicOp )

            Type
            getType() const override
            {
                return Type::LOGICOP;
            }

            /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            int
            compare( const StateAttribute& sa ) const override
            {
                // Check for equal types, then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( LogicOp, sa )

                    // Compare each parameter in turn against the rhs.
                    COMPARE_StateAttribute_Parameter(
                        _opcode
                    ) return 0;    // Passed all the above comparison macros, so must be
                                   // equal.
            }

            bool
            getModeUsage( StateAttribute::ModeUsage& usage ) const override
            {
                usage.usesMode( GL_COLOR_LOGIC_OP );
                return true;
            }

            inline void
            setOpcode( Opcode opcode )
            {
                _opcode = opcode;
            }

            inline Opcode
            getOpcode() const
            {
                return _opcode;
            }

            void
            apply( State& state ) const override;

        protected:

            virtual ~LogicOp();

            Opcode _opcode;
    };

}
