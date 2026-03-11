/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Face culling state attribute. Configures front/back face
 * culling for correct rendering of closed meshes.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/GL>
#include <osg/state/StateAttribute.hpp>

namespace osg
{

    /** Class to globally enable/disable OpenGL's polygon culling mode.
     */
    class OSG_EXPORT CullFace : public osg::Inherit<StateAttribute, CullFace>
    {
        public:

            enum class Mode
            {
                FRONT          = GL_FRONT,
                BACK           = GL_BACK,
                FRONT_AND_BACK = GL_FRONT_AND_BACK,
            };

            CullFace( Mode mode = Mode::BACK ) :
                _mode( mode )
            {
            }

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            CullFace( const CullFace& cf,
                      const CopyOp&   copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( cf,
                         copyop ),
                _mode( cf._mode )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               CullFace )

            Type
            getType() const override
            {
                return Type::CULLFACE;
            }

            /** return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs.*/
            int
            compare( const StateAttribute& sa ) const override
            {
                // check the types are equal and then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( CullFace, sa )

                    // compare each parameter in turn against the rhs.
                    COMPARE_StateAttribute_Parameter(
                        _mode
                    ) return 0;    // passed all the above comparison macros, must be
                                   // equal.
            }

            bool
            getModeUsage( StateAttribute::ModeUsage& usage ) const override
            {
                usage.usesMode( GL_CULL_FACE );
                return true;
            }

            inline void
            setMode( Mode mode )
            {
                _mode = mode;
            }

            inline Mode
            getMode() const
            {
                return _mode;
            }

            void
            apply( State& state ) const override;

        protected:

            virtual ~CullFace();

            Mode _mode;
    };

}
