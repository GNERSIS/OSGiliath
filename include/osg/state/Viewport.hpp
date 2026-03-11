/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Rectangular viewport mapping NDC to window coordinates.
 * Defines the rendering area within the graphics context.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/maths/transform.hpp>
#include <osg/state/StateAttribute.hpp>

namespace osg
{

    /** Encapsulate OpenGL glViewport. */
    class OSG_EXPORT Viewport : public osg::Inherit<StateAttribute, Viewport>
    {
        public:

#if 0
        typedef int value_type;
#else
            typedef double value_type;
#endif
            Viewport();

            Viewport( value_type x,
                      value_type y,
                      value_type width,
                      value_type height ) :
                _x( x ),
                _y( y ),
                _width( width ),
                _height( height )
            {
            }

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            Viewport( const Viewport& vp,
                      const CopyOp&   copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( vp,
                         copyop ),
                _x( vp._x ),
                _y( vp._y ),
                _width( vp._width ),
                _height( vp._height )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               Viewport )

            Type
            getType() const override
            {
                return Type::VIEWPORT;
            }

            /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            virtual int
            compare( const StateAttribute& sa ) const override
            {
                // check the types are equal and then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( Viewport, sa )

                    // compare each parameter in turn against the rhs.
                    COMPARE_StateAttribute_Parameter( _x )
                        COMPARE_StateAttribute_Parameter( _y )
                            COMPARE_StateAttribute_Parameter( _width )
                                COMPARE_StateAttribute_Parameter(
                                    _height
                                ) return 0;    // passed all the above comparison macros,
                                               // must be equal.
            }

            Viewport&
            operator=( const Viewport& rhs )
            {
                if( &rhs == this )
                {
                    return *this;
                }

                _x      = rhs._x;
                _y      = rhs._y;
                _width  = rhs._width;
                _height = rhs._height;

                return *this;
            }

            inline void
            setViewport( value_type x,
                         value_type y,
                         value_type width,
                         value_type height )
            {
                _x      = x;
                _y      = y;
                _width  = width;
                _height = height;
            }

#if 0
        void getViewport(int& x,int& y,int& width,int& height) const
        {
            x = _x;
            y = _y;
            width = _width;
            height = _height;
        }

        void getViewport(double& x,double& y,double& width,double& height) const
        {
            x = _x;
            y = _y;
            width = _width;
            height = _height;
        }
#endif
            inline value_type&
            x()
            {
                return _x;
            }

            inline value_type
            x() const
            {
                return _x;
            }

            inline value_type&
            y()
            {
                return _y;
            }

            inline value_type
            y() const
            {
                return _y;
            }

            inline value_type&
            width()
            {
                return _width;
            }

            inline value_type
            width() const
            {
                return _width;
            }

            inline value_type&
            height()
            {
                return _height;
            }

            inline value_type
            height() const
            {
                return _height;
            }

            inline bool
            valid() const
            {
                return _width > 0 && _height > 0;
            }

            /** Return the aspectRatio of the viewport, which is equal to width/height.
             * If height is zero, the potential division by zero is avoided by simply
             * returning 1.0f.
             */
            inline double
            aspectRatio() const
            {
                if( _height != 0 )
                {
                    return ( double )_width / ( double )_height;
                }
                else
                {
                    return 1.0;
                }
            }

            /** Compute the Window dmat4 which takes projected coords into Window
             * coordinates. To convert local coordinates into window coordinates use
             * v_window = v_local * MVPW matrix, where the MVPW matrix is ModelViewMatrix
             * * ProjectionMatrix * WindowMatrix, the latter supplied by
             * Viewport::computeWindowMatrix(), the ModelView and Projection dmat4 can
             * either be sourced from the current osg::State object, via
             * osgUtil::SceneView or CullVisitor.
             */
            inline const osg::dmat4
            computeWindowMatrix() const
            {
                return osg::translate( 1.0, 1.0, 1.0 ) *
                       osg::scale( 0.5 * width(), 0.5 * height(), 0.5 ) *
                       osg::translate( static_cast<double>( x() ),
                                       static_cast<double>( y() ),
                                       0.0 );
            }

            virtual void
            apply( State& state ) const override;

        protected:

            virtual ~Viewport();

            value_type _x;
            value_type _y;
            value_type _width;
            value_type _height;
    };

}
