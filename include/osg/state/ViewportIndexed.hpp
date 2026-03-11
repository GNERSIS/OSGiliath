/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Per-viewport rectangle for multi-viewport rendering.
 * Sets independent viewport regions for indexed viewports.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/state/Viewport.hpp>

namespace osg
{

    /** Encapsulates glViewportIndexed function : the index version of glViewport for
     * multiple render target.
     */
    class OSG_EXPORT ViewportIndexed : public osg::Inherit<Viewport, ViewportIndexed>
    {
        public:

            ViewportIndexed();

            ViewportIndexed( unsigned int index,
                             value_type   x,
                             value_type   y,
                             value_type   width,
                             value_type   height ) :
                Inherit( x,
                         y,
                         width,
                         height ),
                _index( index )
            {
            }

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            ViewportIndexed( const ViewportIndexed& cm,
                             const CopyOp&          copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( cm,
                         copyop ),
                _index( cm._index )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               ViewportIndexed )

            Type
            getType() const override
            {
                return Type::VIEWPORTINDEXED;
            }

            /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            int
            compare( const StateAttribute& sa ) const override
            {
                // Check for equal types, then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( ViewportIndexed, sa )

                    COMPARE_StateAttribute_Parameter( _index );

                return Viewport::compare( sa );
            }

            /** Return the buffer index as the member identifier.*/
            unsigned int
            getMember() const override
            {
                return _index;
            }

            /** Set the index of the ViewportIndexed. */
            void
            setIndex( unsigned int index );

            /** Get the index of the ViewportIndexed. */
            unsigned int
            getIndex() const
            {
                return _index;
            }

            void
            apply( State& state ) const override;

        protected:

            virtual ~ViewportIndexed();

            unsigned int _index;
    };

}
