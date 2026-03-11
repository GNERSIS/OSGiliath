/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Front-face winding order attribute. Selects whether CW or CCW
 * vertex winding defines the front face for culling and lighting.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/GL>
#include <osg/state/StateAttribute.hpp>

namespace osg
{

    /** Class to specify the orientation of front-facing polygons.
     */
    class OSG_EXPORT FrontFace : public osg::Inherit<StateAttribute, FrontFace>
    {
        public:

            enum class Mode
            {
                CLOCKWISE         = GL_CW,
                COUNTER_CLOCKWISE = GL_CCW,
            };

            FrontFace( Mode face = Mode::COUNTER_CLOCKWISE );

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            FrontFace( const FrontFace& ff,
                       const CopyOp&    copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( ff,
                         copyop ),
                _mode( ff._mode )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               FrontFace )

            Type
            getType() const override
            {
                return Type::FRONTFACE;
            }

            /** return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs.*/
            int
            compare( const StateAttribute& sa ) const override
            {
                // check the types are equal and then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( FrontFace, sa )

                    // compare each parameter in turn against the rhs.
                    COMPARE_StateAttribute_Parameter(
                        _mode
                    ) return 0;    // passed all the above comparison macros, must be
                                   // equal.
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

            virtual ~FrontFace();

            Mode _mode;
    };

}
