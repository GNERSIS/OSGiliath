/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Scissor test attribute defining a rectangular clip region
 * in window coordinates. Pixels outside are discarded.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/state/StateAttribute.hpp>

namespace osg
{

    /** Encapsulate OpenGL glScissor. */
    class OSG_EXPORT Scissor : public osg::Inherit<StateAttribute, Scissor>
    {
        public:

            Scissor();

            Scissor( int x,
                     int y,
                     int width,
                     int height ) :
                _x( x ),
                _y( y ),
                _width( width ),
                _height( height )
            {
            }

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            Scissor( const Scissor& vp,
                     const CopyOp&  copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( vp,
                         copyop ),
                _x( vp._x ),
                _y( vp._y ),
                _width( vp._width ),
                _height( vp._height )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               Scissor )

            Type
            getType() const override
            {
                return Type::SCISSOR;
            }

            /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            int
            compare( const StateAttribute& sa ) const override
            {
                // check the types are equal and then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( Scissor, sa )

                    // compare each parameter in turn against the rhs.
                    COMPARE_StateAttribute_Parameter( _x )
                        COMPARE_StateAttribute_Parameter( _y )
                            COMPARE_StateAttribute_Parameter( _width )
                                COMPARE_StateAttribute_Parameter(
                                    _height
                                ) return 0;    // passed all the above comparison macros,
                                               // must be equal.
            }

            bool
            getModeUsage( StateAttribute::ModeUsage& usage ) const override
            {
                usage.usesMode( GL_SCISSOR_TEST );
                return true;
            }

            inline void
            setScissor( int x,
                        int y,
                        int width,
                        int height )
            {
                _x      = x;
                _y      = y;
                _width  = width;
                _height = height;
            }

            void
            getScissor( int& x,
                        int& y,
                        int& width,
                        int& height ) const
            {
                x      = _x;
                y      = _y;
                width  = _width;
                height = _height;
            }

            inline int&
            x()
            {
                return _x;
            }

            inline int
            x() const
            {
                return _x;
            }

            inline int&
            y()
            {
                return _y;
            }

            inline int
            y() const
            {
                return _y;
            }

            inline int&
            width()
            {
                return _width;
            }

            inline int
            width() const
            {
                return _width;
            }

            inline int&
            height()
            {
                return _height;
            }

            inline int
            height() const
            {
                return _height;
            }

            void
            apply( State& state ) const override;

        protected:

            virtual ~Scissor();

            int _x;
            int _y;
            int _width;
            int _height;
    };

}
