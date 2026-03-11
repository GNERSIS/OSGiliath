/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Convenience typedef for 2D float vector (vec2).
 * Alias for osg::vec2 bridging old API code.
 */
#pragma once

#include <osg/maths/Math.hpp>

namespace osg
{

    /** General purpose float pair. Uses include representation of
     * texture coordinates.
     * No support yet added for float * vec2 - is it necessary?
     * Need to define a non-member non-friend operator* etc.
     * BTW: vec2 * float is okay
     */

    class vec2
    {
        public:

            /** Data type of vector components.*/
            using value_type = float;

            /** Number of vector components. */
            enum
            {
                num_components = 2,
            };

            /** Vec member variable. */
            value_type _v[2];

            /** Constructor that sets all components of the vector to zero */
            constexpr vec2() noexcept :
                _v{ 0.0F,
                    0.0F }
            {
            }

            constexpr vec2( value_type x,
                            value_type y ) noexcept :
                _v{ x,
                    y }
            {
            }

            constexpr bool
            operator==( const vec2& v ) const noexcept
            {
                return _v[0] == v._v[0] && _v[1] == v._v[1];
            }

            constexpr bool
            operator!=( const vec2& v ) const noexcept
            {
                return _v[0] != v._v[0] || _v[1] != v._v[1];
            }

            constexpr bool
            operator<( const vec2& v ) const noexcept
            {
                if( _v[0] < v._v[0] )
                {
                    return true;
                }
                else if( _v[0] > v._v[0] )
                {
                    return false;
                }
                else
                {
                    return ( _v[1] < v._v[1] );
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
                 value_type y ) noexcept
            {
                _v[0] = x;
                _v[1] = y;
            }

            inline void
            set( const vec2& rhs ) noexcept
            {
                _v[0] = rhs._v[0];
                _v[1] = rhs._v[1];
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
                return osg::isNaN( _v[0] ) || osg::isNaN( _v[1] );
            }

            /** Dot product. */
            constexpr value_type
            operator*( const vec2& rhs ) const noexcept
            {
                return _v[0] * rhs._v[0] + _v[1] * rhs._v[1];
            }

            /** Multiply by scalar. */
            constexpr vec2
            operator*( value_type rhs ) const noexcept
            {
                return vec2( _v[0] * rhs, _v[1] * rhs );
            }

            /** Unary multiply by scalar. */
            constexpr vec2&
            operator*=( value_type rhs ) noexcept
            {
                _v[0] *= rhs;
                _v[1] *= rhs;
                return *this;
            }

            /** Divide by scalar. */
            constexpr vec2
            operator/( value_type rhs ) const noexcept
            {
                return vec2( _v[0] / rhs, _v[1] / rhs );
            }

            /** Unary divide by scalar. */
            constexpr vec2&
            operator/=( value_type rhs ) noexcept
            {
                _v[0] /= rhs;
                _v[1] /= rhs;
                return *this;
            }

            /** Binary vector add. */
            constexpr vec2
            operator+( const vec2& rhs ) const noexcept
            {
                return vec2( _v[0] + rhs._v[0], _v[1] + rhs._v[1] );
            }

            /** Unary vector add. Slightly more efficient because no temporary
             * intermediate object.
             */
            constexpr vec2&
            operator+=( const vec2& rhs ) noexcept
            {
                _v[0] += rhs._v[0];
                _v[1] += rhs._v[1];
                return *this;
            }

            /** Binary vector subtract. */
            constexpr vec2
            operator-( const vec2& rhs ) const noexcept
            {
                return vec2( _v[0] - rhs._v[0], _v[1] - rhs._v[1] );
            }

            /** Unary vector subtract. */
            constexpr vec2&
            operator-=( const vec2& rhs ) noexcept
            {
                _v[0] -= rhs._v[0];
                _v[1] -= rhs._v[1];
                return *this;
            }

            /** Negation operator. Returns the negative of the vec2. */
            constexpr vec2
            operator-() const noexcept
            {
                return vec2( -_v[0], -_v[1] );
            }

            /** Length of the vector = sqrt( vec . vec ) */
            inline value_type
            length() const noexcept
            {
                return sqrtf( _v[0] * _v[0] + _v[1] * _v[1] );
            }

            /** Length squared of the vector = vec . vec */
            constexpr value_type
            length2( void ) const noexcept
            {
                return _v[0] * _v[0] + _v[1] * _v[1];
            }

            /** Normalize the vector so that it has length unity.
             * Returns the previous length of the vector.
             */
            inline value_type
            normalize() noexcept
            {
                value_type norm = vec2::length();
                if( norm > 0.0 )
                {
                    value_type inv  = 1.0F / norm;
                    _v[0]          *= inv;
                    _v[1]          *= inv;
                }
                return ( norm );
            }

    };    // end of class vec2

    /** multiply by vector components. */
    constexpr vec2
    componentMultiply( const vec2& lhs,
                       const vec2& rhs ) noexcept
    {
        return vec2( lhs[0] * rhs[0], lhs[1] * rhs[1] );
    }

    /** divide rhs components by rhs vector components. */
    constexpr vec2
    componentDivide( const vec2& lhs,
                     const vec2& rhs ) noexcept
    {
        return vec2( lhs[0] / rhs[0], lhs[1] / rhs[1] );
    }

}    // end of namespace osg
