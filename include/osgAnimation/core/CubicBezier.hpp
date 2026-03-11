/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Cubic Bezier interpolation data for animation keyframes.
 * Stores value with in/out tangent control points.
 */
#pragma once

#include <osg/maths/Math.hpp>
#include <osg/maths/vec2.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>

namespace osgAnimation
{

    template<class T>
    class TemplateCubicBezier
    {
        public:

            TemplateCubicBezier() :
                _position( osg::default_value<T>() ),
                _controlPointIn( osg::default_value<T>() ),
                _controlPointOut( osg::default_value<T>() )
            {
            }

            TemplateCubicBezier( const T& p,
                                 const T& i,
                                 const T& o ) :
                _position( p ),
                _controlPointIn( i ),
                _controlPointOut( o )
            {
            }

            // Constructor with value only
            TemplateCubicBezier( const T& p ) :
                _position( p ),
                _controlPointIn( p ),
                _controlPointOut( p )
            {
            }

            const T&
            getPosition() const
            {
                return _position;
            }

            const T&
            getControlPointIn() const
            {
                return _controlPointIn;
            }

            const T&
            getControlPointOut() const
            {
                return _controlPointOut;
            }

            T&
            getPosition()
            {
                return _position;
            }

            T&
            getControlPointIn()
            {
                return _controlPointIn;
            }

            T&
            getControlPointOut()
            {
                return _controlPointOut;
            }

            void
            setPosition( const T& v )
            {
                _position = v;
            }

            void
            setControlPointIn( const T& v )
            {
                _controlPointIn = v;
            }

            void
            setControlPointOut( const T& v )
            {
                _controlPointOut = v;
            }

            bool
            operator==( const TemplateCubicBezier<T>& other ) const
            {
                return _position ==
                       other._position &&
                       _controlPointIn ==
                       other._controlPointIn &&
                       _controlPointOut == other._controlPointOut;
            }

            // steaming operators.
            friend std::ostream&
            operator<<( std::ostream&                 output,
                        const TemplateCubicBezier<T>& tcb )
            {
                output << tcb._position << " " << tcb._controlPointIn << " "
                       << tcb._controlPointOut;
                return output;    // to enable cascading
            }

            friend std::istream&
            operator>>( std::istream&           input,
                        TemplateCubicBezier<T>& tcb )
            {
                input >> tcb._position >> tcb._controlPointIn >> tcb._controlPointOut;
                return input;
            }

        protected:

            T _position, _controlPointIn, _controlPointOut;
    };

    typedef TemplateCubicBezier<float>     FloatCubicBezier;
    typedef TemplateCubicBezier<double>    DoubleCubicBezier;
    typedef TemplateCubicBezier<osg::vec2> Vec2CubicBezier;
    typedef TemplateCubicBezier<osg::vec3> Vec3CubicBezier;
    typedef TemplateCubicBezier<osg::vec4> Vec4CubicBezier;

}
