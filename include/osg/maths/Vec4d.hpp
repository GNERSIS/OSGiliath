/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Convenience typedef for 4D double vector (dvec4).
 * Alias for osg::dvec4 bridging old API code.
 */
#pragma once

#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>

namespace osg
{

    /** General purpose double quad. Uses include representation
     * of color coordinates.
     * No support yet added for double * dvec4 - is it necessary?
     * Need to define a non-member non-friend operator*  etc.
     * dvec4 * double is okay
     */
    class dvec4
    {
        public:

            /** Data type of vector components.*/
            using value_type = double;

            /** Number of vector components. */
            enum
            {
                num_components = 4,
            };

            value_type _v[4];

            /** Constructor that sets all components of the vector to zero */
            constexpr dvec4() noexcept :
                _v{ 0.0,
                    0.0,
                    0.0,
                    0.0 }
            {
            }

            constexpr dvec4( value_type x,
                             value_type y,
                             value_type z,
                             value_type w ) noexcept :
                _v{ x,
                    y,
                    z,
                    w }
            {
            }

            constexpr dvec4( const dvec3& v3,
                             value_type   w ) noexcept :
                _v{ v3._v[0],
                    v3._v[1],
                    v3._v[2],
                    w }
            {
            }

            constexpr dvec4( const vec4& vec ) noexcept :
                _v{ vec._v[0],
                    vec._v[1],
                    vec._v[2],
                    vec._v[3] }
            {
            }

            constexpr
            operator vec4() const noexcept
            {
                return vec4( static_cast<float>( _v[0] ),
                             static_cast<float>( _v[1] ),
                             static_cast<float>( _v[2] ),
                             static_cast<float>( _v[3] ) );
            }

            constexpr bool
            operator==( const dvec4& v ) const noexcept
            {
                return _v[0] ==
                       v._v[0] &&
                       _v[1] ==
                       v._v[1] &&
                       _v[2] ==
                       v._v[2] &&
                       _v[3] == v._v[3];
            }

            constexpr bool
            operator!=( const dvec4& v ) const noexcept
            {
                return _v[0] !=
                       v._v[0] ||
                       _v[1] !=
                       v._v[1] ||
                       _v[2] !=
                       v._v[2] ||
                       _v[3] != v._v[3];
            }

            constexpr bool
            operator<( const dvec4& v ) const noexcept
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
                else if( _v[2] < v._v[2] )
                {
                    return true;
                }
                else if( _v[2] > v._v[2] )
                {
                    return false;
                }
                else
                {
                    return ( _v[3] < v._v[3] );
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
                 value_type z,
                 value_type w ) noexcept
            {
                _v[0] = x;
                _v[1] = y;
                _v[2] = z;
                _v[3] = w;
            }

            constexpr value_type&
            operator[]( unsigned int i ) noexcept
            {
                return _v[i];
            }

            constexpr value_type
            operator[]( unsigned int i ) const noexcept
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

            constexpr value_type&
            w() noexcept
            {
                return _v[3];
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

            constexpr value_type
            w() const noexcept
            {
                return _v[3];
            }

            constexpr value_type&
            r() noexcept
            {
                return _v[0];
            }

            constexpr value_type&
            g() noexcept
            {
                return _v[1];
            }

            constexpr value_type&
            b() noexcept
            {
                return _v[2];
            }

            constexpr value_type&
            a() noexcept
            {
                return _v[3];
            }

            constexpr value_type
            r() const noexcept
            {
                return _v[0];
            }

            constexpr value_type
            g() const noexcept
            {
                return _v[1];
            }

            constexpr value_type
            b() const noexcept
            {
                return _v[2];
            }

            constexpr value_type
            a() const noexcept
            {
                return _v[3];
            }

            inline unsigned int
            asABGR() const noexcept
            {
                return ( unsigned int )clampTo( _v[0] * 255.0, 0.0, 255.0 ) << 24 |
                       ( unsigned int )clampTo( _v[1] * 255.0, 0.0, 255.0 ) << 16 |
                       ( unsigned int )clampTo( _v[2] * 255.0, 0.0, 255.0 ) << 8 |
                       ( unsigned int )clampTo( _v[3] * 255.0, 0.0, 255.0 );
            }

            inline unsigned int
            asRGBA() const noexcept
            {
                return ( unsigned int )clampTo( _v[3] * 255.0, 0.0, 255.0 ) << 24 |
                       ( unsigned int )clampTo( _v[2] * 255.0, 0.0, 255.0 ) << 16 |
                       ( unsigned int )clampTo( _v[1] * 255.0, 0.0, 255.0 ) << 8 |
                       ( unsigned int )clampTo( _v[0] * 255.0, 0.0, 255.0 );
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
                return osg::isNaN( _v[0] ) ||
                       osg::isNaN( _v[1] ) ||
                       osg::isNaN( _v[2] ) ||
                       osg::isNaN( _v[3] );
            }

            /** Dot product. */
            constexpr value_type
            operator*( const dvec4& rhs ) const noexcept
            {
                return _v[0] *
                       rhs._v[0] +
                       _v[1] *
                       rhs._v[1] +
                       _v[2] *
                       rhs._v[2] +
                       _v[3] *
                       rhs._v[3];
            }

            /** Multiply by scalar. */
            constexpr dvec4
            operator*( value_type rhs ) const noexcept
            {
                return dvec4( _v[0] * rhs, _v[1] * rhs, _v[2] * rhs, _v[3] * rhs );
            }

            /** Unary multiply by scalar. */
            constexpr dvec4&
            operator*=( value_type rhs ) noexcept
            {
                _v[0] *= rhs;
                _v[1] *= rhs;
                _v[2] *= rhs;
                _v[3] *= rhs;
                return *this;
            }

            /** Divide by scalar. */
            constexpr dvec4
            operator/( value_type rhs ) const noexcept
            {
                return dvec4( _v[0] / rhs, _v[1] / rhs, _v[2] / rhs, _v[3] / rhs );
            }

            /** Unary divide by scalar. */
            constexpr dvec4&
            operator/=( value_type rhs ) noexcept
            {
                _v[0] /= rhs;
                _v[1] /= rhs;
                _v[2] /= rhs;
                _v[3] /= rhs;
                return *this;
            }

            /** Binary vector add. */
            constexpr dvec4
            operator+( const dvec4& rhs ) const noexcept
            {
                return dvec4( _v[0] + rhs._v[0],
                              _v[1] + rhs._v[1],
                              _v[2] + rhs._v[2],
                              _v[3] + rhs._v[3] );
            }

            /** Unary vector add. Slightly more efficient because no temporary
             * intermediate object.
             */
            constexpr dvec4&
            operator+=( const dvec4& rhs ) noexcept
            {
                _v[0] += rhs._v[0];
                _v[1] += rhs._v[1];
                _v[2] += rhs._v[2];
                _v[3] += rhs._v[3];
                return *this;
            }

            /** Binary vector subtract. */
            constexpr dvec4
            operator-( const dvec4& rhs ) const noexcept
            {
                return dvec4( _v[0] - rhs._v[0],
                              _v[1] - rhs._v[1],
                              _v[2] - rhs._v[2],
                              _v[3] - rhs._v[3] );
            }

            /** Unary vector subtract. */
            constexpr dvec4&
            operator-=( const dvec4& rhs ) noexcept
            {
                _v[0] -= rhs._v[0];
                _v[1] -= rhs._v[1];
                _v[2] -= rhs._v[2];
                _v[3] -= rhs._v[3];
                return *this;
            }

            /** Negation operator. Returns the negative of the dvec4. */
            constexpr dvec4
            operator-() const noexcept
            {
                return dvec4( -_v[0], -_v[1], -_v[2], -_v[3] );
            }

            /** Length of the vector = sqrt( vec . vec ) */
            inline value_type
            length() const noexcept
            {
                return sqrt(
                    _v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2] + _v[3] * _v[3]
                );
            }

            /** Length squared of the vector = vec . vec */
            constexpr value_type
            length2() const noexcept
            {
                return _v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2] + _v[3] * _v[3];
            }

            /** Normalize the vector so that it has length unity.
             * Returns the previous length of the vector.
             */
            inline value_type
            normalize() noexcept
            {
                value_type norm = dvec4::length();
                if( norm > 0.0F )
                {
                    value_type inv  = 1.0 / norm;
                    _v[0]          *= inv;
                    _v[1]          *= inv;
                    _v[2]          *= inv;
                    _v[3]          *= inv;
                }
                return ( norm );
            }

    };    // end of class dvec4

    /** Compute the dot product of a (vec3,1.0) and a dvec4. */
    constexpr dvec4::value_type
    operator*( const dvec3& lhs,
               const dvec4& rhs ) noexcept
    {
        return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2] + rhs[3];
    }

    /** Compute the dot product of a (vec3,1.0) and a dvec4. */
    constexpr dvec4::value_type
    operator*( const vec3&  lhs,
               const dvec4& rhs ) noexcept
    {
        return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2] + rhs[3];
    }

    /** Compute the dot product of a (vec3,1.0) and a dvec4. */
    constexpr dvec4::value_type
    operator*( const dvec3& lhs,
               const vec4&  rhs ) noexcept
    {
        return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2] + rhs[3];
    }

    /** Compute the dot product of a dvec4 and a (vec3,1.0). */
    constexpr dvec4::value_type
    operator*( const dvec4& lhs,
               const dvec3& rhs ) noexcept
    {
        return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2] + lhs[3];
    }

    /** Compute the dot product of a dvec4 and a (vec3,1.0). */
    constexpr dvec4::value_type
    operator*( const dvec4& lhs,
               const vec3&  rhs ) noexcept
    {
        return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2] + lhs[3];
    }

    /** Compute the dot product of a dvec4 and a (vec3,1.0). */
    constexpr dvec4::value_type
    operator*( const vec4&  lhs,
               const dvec3& rhs ) noexcept
    {
        return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2] + lhs[3];
    }

    /** multiply by vector components. */
    constexpr dvec4
    componentMultiply( const dvec4& lhs,
                       const dvec4& rhs ) noexcept
    {
        return dvec4( lhs[0] * rhs[0],
                      lhs[1] * rhs[1],
                      lhs[2] * rhs[2],
                      lhs[3] * rhs[3] );
    }

    /** divide rhs components by rhs vector components. */
    constexpr dvec4
    componentDivide( const dvec4& lhs,
                     const dvec4& rhs ) noexcept
    {
        return dvec4( lhs[0] / rhs[0],
                      lhs[1] / rhs[1],
                      lhs[2] / rhs[2],
                      lhs[3] / rhs[3] );
    }

}    // end of namespace osg
