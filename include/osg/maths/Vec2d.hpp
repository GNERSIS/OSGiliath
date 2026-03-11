/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Convenience typedef for 2D double vector (dvec2).
 * Alias for osg::dvec2 bridging old API code.
 */
#pragma once

#include <osg/maths/vec2.hpp>

namespace osg
{

    /** General purpose double pair, uses include representation of
     * texture coordinates.
     * No support yet added for double * dvec2 - is it necessary?
     * Need to define a non-member non-friend operator* etc.
     * BTW: dvec2 * double is okay
     */

    class dvec2
    {
        public:

            /** Data type of vector components.*/
            using value_type = double;

            /** Number of vector components. */
            enum
            {
                num_components = 2,
            };

            value_type _v[2];

            /** Constructor that sets all components of the vector to zero */
            constexpr dvec2() noexcept :
                _v{ 0.0,
                    0.0 }
            {
            }

            constexpr dvec2( value_type x,
                             value_type y ) noexcept :
                _v{ x,
                    y }
            {
            }

            constexpr dvec2( const vec2& vec ) noexcept :
                _v{ vec._v[0],
                    vec._v[1] }
            {
            }

            constexpr
            operator vec2() const noexcept
            {
                return vec2( static_cast<float>( _v[0] ), static_cast<float>( _v[1] ) );
            }

            constexpr bool
            operator==( const dvec2& v ) const noexcept
            {
                return _v[0] == v._v[0] && _v[1] == v._v[1];
            }

            constexpr bool
            operator!=( const dvec2& v ) const noexcept
            {
                return _v[0] != v._v[0] || _v[1] != v._v[1];
            }

            constexpr bool
            operator<( const dvec2& v ) const noexcept
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
            operator*( const dvec2& rhs ) const noexcept
            {
                return _v[0] * rhs._v[0] + _v[1] * rhs._v[1];
            }

            /** Multiply by scalar. */
            constexpr dvec2
            operator*( value_type rhs ) const noexcept
            {
                return dvec2( _v[0] * rhs, _v[1] * rhs );
            }

            /** Unary multiply by scalar. */
            constexpr dvec2&
            operator*=( value_type rhs ) noexcept
            {
                _v[0] *= rhs;
                _v[1] *= rhs;
                return *this;
            }

            /** Divide by scalar. */
            constexpr dvec2
            operator/( value_type rhs ) const noexcept
            {
                return dvec2( _v[0] / rhs, _v[1] / rhs );
            }

            /** Unary divide by scalar. */
            constexpr dvec2&
            operator/=( value_type rhs ) noexcept
            {
                _v[0] /= rhs;
                _v[1] /= rhs;
                return *this;
            }

            /** Binary vector add. */
            constexpr dvec2
            operator+( const dvec2& rhs ) const noexcept
            {
                return dvec2( _v[0] + rhs._v[0], _v[1] + rhs._v[1] );
            }

            /** Unary vector add. Slightly more efficient because no temporary
             * intermediate object.
             */
            constexpr dvec2&
            operator+=( const dvec2& rhs ) noexcept
            {
                _v[0] += rhs._v[0];
                _v[1] += rhs._v[1];
                return *this;
            }

            /** Binary vector subtract. */
            constexpr dvec2
            operator-( const dvec2& rhs ) const noexcept
            {
                return dvec2( _v[0] - rhs._v[0], _v[1] - rhs._v[1] );
            }

            /** Unary vector subtract. */
            constexpr dvec2&
            operator-=( const dvec2& rhs ) noexcept
            {
                _v[0] -= rhs._v[0];
                _v[1] -= rhs._v[1];
                return *this;
            }

            /** Negation operator. Returns the negative of the dvec2. */
            constexpr dvec2
            operator-() const noexcept
            {
                return dvec2( -_v[0], -_v[1] );
            }

            /** Length of the vector = sqrt( vec . vec ) */
            inline value_type
            length() const noexcept
            {
                return sqrt( _v[0] * _v[0] + _v[1] * _v[1] );
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
                value_type norm = dvec2::length();
                if( norm > 0.0 )
                {
                    value_type inv  = 1.0 / norm;
                    _v[0]          *= inv;
                    _v[1]          *= inv;
                }
                return ( norm );
            }

    };    // end of class dvec2

    /** multiply by vector components. */
    constexpr dvec2
    componentMultiply( const dvec2& lhs,
                       const dvec2& rhs ) noexcept
    {
        return dvec2( lhs[0] * rhs[0], lhs[1] * rhs[1] );
    }

    /** divide rhs components by rhs vector components. */
    constexpr dvec2
    componentDivide( const dvec2& lhs,
                     const dvec2& rhs ) noexcept
    {
        return dvec2( lhs[0] / rhs[0], lhs[1] / rhs[1] );
    }

}    // end of namespace osg
