/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Convenience typedef for 3D double vector (dvec3).
 * Alias for osg::dvec3 bridging old API code.
 */
#pragma once

#include <osg/maths/vec2.hpp>
#include <osg/maths/vec3.hpp>

namespace osg
{

    /** General purpose double triple for use as vertices, vectors and normals.
     * Provides general math operations from addition through to cross products.
     * No support yet added for double * dvec3 - is it necessary?
     * Need to define a non-member non-friend operator*  etc.
     *    dvec3 * double is okay
     */

    class dvec3
    {
        public:

            /** Data type of vector components.*/
            using value_type = double;

            /** Number of vector components. */
            enum
            {
                num_components = 3,
            };

            value_type _v[3];

            /** Constructor that sets all components of the vector to zero */
            constexpr dvec3() noexcept :
                _v{ 0.0,
                    0.0,
                    0.0 }
            {
            }

            constexpr dvec3( const vec3& vec ) noexcept :
                _v{ vec._v[0],
                    vec._v[1],
                    vec._v[2] }
            {
            }

            constexpr
            operator vec3() const noexcept
            {
                return vec3( static_cast<float>( _v[0] ),
                             static_cast<float>( _v[1] ),
                             static_cast<float>( _v[2] ) );
            }

            constexpr dvec3( value_type x,
                             value_type y,
                             value_type z ) noexcept :
                _v{ x,
                    y,
                    z }
            {
            }

            constexpr dvec3( const dvec2& v2,
                             value_type   zz ) noexcept :
                _v{ v2._v[0],
                    v2._v[1],
                    zz }
            {
            }

            constexpr bool
            operator==( const dvec3& v ) const noexcept
            {
                return _v[0] == v._v[0] && _v[1] == v._v[1] && _v[2] == v._v[2];
            }

            constexpr bool
            operator!=( const dvec3& v ) const noexcept
            {
                return _v[0] != v._v[0] || _v[1] != v._v[1] || _v[2] != v._v[2];
            }

            constexpr bool
            operator<( const dvec3& v ) const noexcept
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
            set( const dvec3& rhs ) noexcept
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
            operator*( const dvec3& rhs ) const noexcept
            {
                return _v[0] * rhs._v[0] + _v[1] * rhs._v[1] + _v[2] * rhs._v[2];
            }

            /** Cross product. */
            constexpr dvec3
            operator^( const dvec3& rhs ) const noexcept
            {
                return dvec3( _v[1] * rhs._v[2] - _v[2] * rhs._v[1],
                              _v[2] * rhs._v[0] - _v[0] * rhs._v[2],
                              _v[0] * rhs._v[1] - _v[1] * rhs._v[0] );
            }

            /** Multiply by scalar. */
            constexpr dvec3
            operator*( value_type rhs ) const noexcept
            {
                return dvec3( _v[0] * rhs, _v[1] * rhs, _v[2] * rhs );
            }

            /** Unary multiply by scalar. */
            constexpr dvec3&
            operator*=( value_type rhs ) noexcept
            {
                _v[0] *= rhs;
                _v[1] *= rhs;
                _v[2] *= rhs;
                return *this;
            }

            /** Divide by scalar. */
            constexpr dvec3
            operator/( value_type rhs ) const noexcept
            {
                return dvec3( _v[0] / rhs, _v[1] / rhs, _v[2] / rhs );
            }

            /** Unary divide by scalar. */
            constexpr dvec3&
            operator/=( value_type rhs ) noexcept
            {
                _v[0] /= rhs;
                _v[1] /= rhs;
                _v[2] /= rhs;
                return *this;
            }

            /** Binary vector add. */
            constexpr dvec3
            operator+( const dvec3& rhs ) const noexcept
            {
                return dvec3( _v[0] + rhs._v[0], _v[1] + rhs._v[1], _v[2] + rhs._v[2] );
            }

            /** Unary vector add. Slightly more efficient because no temporary
             * intermediate object.
             */
            constexpr dvec3&
            operator+=( const dvec3& rhs ) noexcept
            {
                _v[0] += rhs._v[0];
                _v[1] += rhs._v[1];
                _v[2] += rhs._v[2];
                return *this;
            }

            /** Binary vector subtract. */
            constexpr dvec3
            operator-( const dvec3& rhs ) const noexcept
            {
                return dvec3( _v[0] - rhs._v[0], _v[1] - rhs._v[1], _v[2] - rhs._v[2] );
            }

            /** Unary vector subtract. */
            constexpr dvec3&
            operator-=( const dvec3& rhs ) noexcept
            {
                _v[0] -= rhs._v[0];
                _v[1] -= rhs._v[1];
                _v[2] -= rhs._v[2];
                return *this;
            }

            /** Negation operator. Returns the negative of the dvec3. */
            constexpr dvec3
            operator-() const noexcept
            {
                return dvec3( -_v[0], -_v[1], -_v[2] );
            }

            /** Length of the vector = sqrt( vec . vec ) */
            inline value_type
            length() const noexcept
            {
                return sqrt( _v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2] );
            }

            /** Length squared of the vector = vec . vec */
            constexpr value_type
            length2() const noexcept
            {
                return _v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2];
            }

            /** Normalize the vector so that it has length unity.
             * Returns the previous length of the vector.
             * If the vector is zero length, it is left unchanged and zero is returned.
             */
            inline value_type
            normalize() noexcept
            {
                value_type norm = dvec3::length();
                if( norm > 0.0 )
                {
                    value_type inv  = 1.0 / norm;
                    _v[0]          *= inv;
                    _v[1]          *= inv;
                    _v[2]          *= inv;
                }
                return ( norm );
            }

    };    // end of class dvec3

    /** multiply by vector components. */
    constexpr dvec3
    componentMultiply( const dvec3& lhs,
                       const dvec3& rhs ) noexcept
    {
        return dvec3( lhs[0] * rhs[0], lhs[1] * rhs[1], lhs[2] * rhs[2] );
    }

    /** divide rhs components by rhs vector components. */
    constexpr dvec3
    componentDivide( const dvec3& lhs,
                     const dvec3& rhs ) noexcept
    {
        return dvec3( lhs[0] / rhs[0], lhs[1] / rhs[1], lhs[2] / rhs[2] );
    }

}    // end of namespace osg
