/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Per-viewport scissor rectangle for multi-viewport rendering.
 * Sets independent clip regions for indexed viewports.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/state/Depth.hpp>

namespace osg
{

    /** Encapsulates glScissorIndexed function : the index version of glDepth
     */
    class OSG_EXPORT ScissorIndexed
        : public osg::Inherit<osg::StateAttribute, ScissorIndexed>
    {
        public:

            ScissorIndexed();

            ScissorIndexed( unsigned int index,
                            float        x,
                            float        y,
                            float        width,
                            float        height ) :
                _index( index ),
                _x( x ),
                _y( y ),
                _width( width ),
                _height( height )
            {
            }

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            ScissorIndexed( const ScissorIndexed& dp,
                            const CopyOp&         copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( dp,
                         copyop ),
                _index( dp._index ),
                _x( dp._x ),
                _y( dp._y ),
                _width( dp._width ),
                _height( dp._height )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               ScissorIndexed )

            Type
            getType() const override
            {
                return Type::SCISSORINDEXED;
            }

            /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            int
            compare( const StateAttribute& sa ) const override
            {
                // Check for equal types, then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( ScissorIndexed, sa )

                    COMPARE_StateAttribute_Parameter( _index );
                COMPARE_StateAttribute_Parameter( _x )
                    COMPARE_StateAttribute_Parameter( _y )
                        COMPARE_StateAttribute_Parameter( _width )
                            COMPARE_StateAttribute_Parameter( _height ) return 0;
            }

            /** Return the buffer index as the member identifier.*/
            unsigned int
            getMember() const override
            {
                return _index;
            }

            /** Set the index of the ScissorIndexed. */
            void
            setIndex( unsigned int index );

            /** Get the index of the ScissorIndexed. */
            unsigned int
            getIndex() const
            {
                return _index;
            }

            inline void
            setX( float x )
            {
                _x = x;
            }

            inline float
            getX() const
            {
                return _x;
            }

            inline void
            setY( float y )
            {
                _y = y;
            }

            inline float
            getY() const
            {
                return _y;
            }

            inline void
            setWidth( float w )
            {
                _width = w;
            }

            inline float
            getWidth() const
            {
                return _width;
            }

            inline void
            setHeight( float height )
            {
                _height = height;
            }

            inline float
            getHeight() const
            {
                return _height;
            }

            void
            apply( State& state ) const override;

        protected:

            virtual ~ScissorIndexed();

            unsigned int _index;
            float        _x;
            float        _y;
            float        _width;
            float        _height;
    };

}
