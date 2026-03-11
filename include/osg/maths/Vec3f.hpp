/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Convenience typedef for 3D float vector (vec3).
 * Alias for osg::vec3 bridging old API code.
 */
#pragma once

#include <osg/maths/Math.hpp>
#include <osg/maths/vec2.hpp>

namespace osg
{

    /** General purpose float triple for use as vertices, vectors and normals.
     * Provides general math operations from addition through to cross products.
     * No support yet added for float * vec3 - is it necessary?
     * Need to define a non-member non-friend operator*  etc.
     * vec3 * float is okay
     */
    class vec3
    {
        public:

            /** Data type of vector components.*/
            using value_type = float;

            /** Number of vector components. */
            enum
            {
                num_components = 3,
            };

            value_type _v[3];

            /** Constructor that sets all components of the vector to zero */
            constexpr vec3() noexcept :
                _v{ 0.0F,
                    0.0F,
                    0.0F }
            {
            }

            constexpr vec3( value_type x,
                            value_type y,
                            value_type z ) noexcept :
                _v{ x,
                    y,
                    z }
            {
            }

            constexpr vec3( const vec2& v2,
                            value_type  zz ) noexcept :
                _v{ v2._v[0],
                    v2._v[1],
                    zz }
            {
            }

            constexpr bool
            operator==( const vec3& v ) const noexcept
            {
                return _v[0] == v._v[0] && _v[1] == v._v[1] && _v[2] == v._v[2];
            }

            constexpr bool
            operator!=( const vec3& v ) const noexcept
            {
                return _v[0] != v._v[0] || _v[1] != v._v[1] || _v[2] != v._v[2];
            }

            constexpr bool
            operator<( const vec3& v ) const noexcept
            {
                if( _v[0] < v._v[0] )
                {
                    return true;
                }
                else if( _v[0] > v._v[0] )
                {
                    return false;
                }
                else if( _v[1] < v._v[1] )
                {
                    return true;
                }
                else if( _v[1] > v._v[1] )
                {
                    return false;
                }
                else
                {
                    return ( _v[2] < v._v[2] );
                }
            }

            inline value_type*
            ptr() noexcept
            {
                return _v;
            }

            inline const value_type*
            ptr() const noexcept
            {
                return _v;
            }

            inline void
            set( value_type x,
                 value_type y,
                 value_type z ) noexcept
            {
                _v[0] = x;
                _v[1] = y;
                _v[2] = z;
            }

            inline void
            set( const vec3& rhs ) noexcept
            {
                _v[0] = rhs._v[0];
                _v[1] = rhs._v[1];
                _v[2] = rhs._v[2];
            }

            constexpr value_type&
            operator[]( int i ) noexcept
            {
                return _v[i];
            }

            constexpr value_type
            operator[]( int i ) const noexcept
            {
                return _v[i];
            }

            constexpr value_type&
            x() noexcept
            {
                return _v[0];
            }

            constexpr value_type&
            y() noexcept
            {
                return _v[1];
            }

            constexpr value_type&
            z() noexcept
            {
                return _v[2];
            }

            constexpr value_type
            x() const noexcept
            {
                return _v[0];
            }

            constexpr value_type
            y() const noexcept
            {
                return _v[1];
            }

            constexpr value_type
            z() const noexcept
            {
                return _v[2];
            }

            /** Returns true if all components have values that are not NaN. */
            inline bool
            valid() const noexcept
            {
                return !isNaN();
            }

            /** Returns true if at least one component has value NaN. */
            inline bool
            isNaN() const noexcept
            {
                return osg::isNaN( _v[0] ) || osg::isNaN( _v[1] ) || osg::isNaN( _v[2] );
            }

            /** Dot product. */
            constexpr value_type
            operator*( const vec3& rhs ) const noexcept
            {
                return _v[0] * rhs._v[0] + _v[1] * rhs._v[1] + _v[2] * rhs._v[2];
            }

            /** Cross product. */
            constexpr vec3
            operator^( const vec3& rhs ) const noexcept
            {
                return vec3( _v[1] * rhs._v[2] - _v[2] * rhs._v[1],
                             _v[2] * rhs._v[0] - _v[0] * rhs._v[2],
                             _v[0] * rhs._v[1] - _v[1] * rhs._v[0] );
            }

            /** Multiply by scalar. */
            constexpr vec3
            operator*( value_type rhs ) const noexcept
            {
                return vec3( _v[0] * rhs, _v[1] * rhs, _v[2] * rhs );
            }

            /** Unary multiply by scalar. */
            constexpr vec3&
            operator*=( value_type rhs ) noexcept
            {
                _v[0] *= rhs;
                _v[1] *= rhs;
                _v[2] *= rhs;
                return *this;
            }

            /** Divide by scalar. */
            constexpr vec3
            operator/( value_type rhs ) const noexcept
            {
                return vec3( _v[0] / rhs, _v[1] / rhs, _v[2] / rhs );
            }

            /** Unary divide by scalar. */
            constexpr vec3&
            operator/=( value_type rhs ) noexcept
            {
                _v[0] /= rhs;
                _v[1] /= rhs;
                _v[2] /= rhs;
                return *this;
            }

            /** Binary vector add. */
            constexpr vec3
            operator+( const vec3& rhs ) const noexcept
            {
                return vec3( _v[0] + rhs._v[0], _v[1] + rhs._v[1], _v[2] + rhs._v[2] );
            }

            /** Unary vector add. Slightly more efficient because no temporary
             * intermediate object.
             */
            constexpr vec3&
            operator+=( const vec3& rhs ) noexcept
            {
                _v[0] += rhs._v[0];
                _v[1] += rhs._v[1];
                _v[2] += rhs._v[2];
                return *this;
            }

            /** Binary vector subtract. */
            constexpr vec3
            operator-( const vec3& rhs ) const noexcept
            {
                return vec3( _v[0] - rhs._v[0], _v[1] - rhs._v[1], _v[2] - rhs._v[2] );
            }

            /** Unary vector subtract. */
            constexpr vec3&
            operator-=( const vec3& rhs ) noexcept
            {
                _v[0] -= rhs._v[0];
                _v[1] -= rhs._v[1];
                _v[2] -= rhs._v[2];
                return *this;
            }

            /** Negation operator. Returns the negative of the vec3. */
            constexpr vec3
            operator-() const noexcept
            {
                return vec3( -_v[0], -_v[1], -_v[2] );
            }

            /** Length of the vector = sqrt( vec . vec ) */
            inline value_type
            length() const noexcept
            {
                return sqrtf( _v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2] );
            }

            /** Length squared of the vector = vec . vec */
            constexpr value_type
            length2() const noexcept
            {
                return _v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2];
            }

            /** Normalize the vector so that it has length unity.
             * Returns the previous length of the vector.
             */
            inline value_type
            normalize() noexcept
            {
                value_type norm = vec3::length();
                if( norm > 0.0 )
                {
                    value_type inv  = 1.0F / norm;
                    _v[0]          *= inv;
                    _v[1]          *= inv;
                    _v[2]          *= inv;
                }
                return ( norm );
            }

    };    // end of class vec3

    /** multiply by vector components. */
    constexpr vec3
    componentMultiply( const vec3& lhs,
                       const vec3& rhs ) noexcept
    {
        return vec3( lhs[0] * rhs[0], lhs[1] * rhs[1], lhs[2] * rhs[2] );
    }

    /** divide rhs components by rhs vector components. */
    constexpr vec3
    componentDivide( const vec3& lhs,
                     const vec3& rhs ) noexcept
    {
        return vec3( lhs[0] / rhs[0], lhs[1] / rhs[1], lhs[2] / rhs[2] );
    }

    constexpr vec3 X_AXIS( 1.0,
                           0.0,
                           0.0 );
    constexpr vec3 Y_AXIS( 0.0,
                           1.0,
                           0.0 );
    constexpr vec3 Z_AXIS( 0.0,
                           0.0,
                           1.0 );

}    // end of namespace osg
