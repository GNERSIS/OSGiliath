/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Line width state attribute. Sets GL line width for
 * GL_LINES, GL_LINE_STRIP, and GL_LINE_LOOP primitives.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/state/StateAttribute.hpp>

namespace osg
{

    /** LineWidth - encapsulates the OpenGL glLineWidth for setting the width of lines in
     * pixels. */
    class OSG_EXPORT LineWidth : public osg::Inherit<StateAttribute, LineWidth>
    {
        public:

            LineWidth( float width = 1.0F );

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            LineWidth( const LineWidth& lw,
                       const CopyOp&    copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( lw,
                         copyop ),
                _width( lw._width )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               LineWidth )

            Type
            getType() const override
            {
                return Type::LINEWIDTH;
            }

            /** return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            int
            compare( const StateAttribute& sa ) const override
            {
                // check if the types are equal and then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( LineWidth, sa )

                    // compare each parameter in turn against the rhs.
                    COMPARE_StateAttribute_Parameter(
                        _width
                    ) return 0;    // passed all the above comparison macros, must be
                                   // equal.
            }

            void
            setWidth( float width );

            inline float
            getWidth() const
            {
                return _width;
            }

            void
            apply( State& state ) const override;

        protected:

            virtual ~LineWidth();

            float _width;
    };

}
