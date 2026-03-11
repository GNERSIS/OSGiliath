/* Adapted from VulkanSceneGraph — MIT License, Copyright(c) 2018 Robert Osfield */

#pragma once

// we can't implement the anonymous union/structs combination without causing warnings,
// so disable them for just this header
#if defined( __GNUC__ )
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
#endif
#if defined( __clang__ )
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
    #pragma clang diagnostic ignored "-Wnested-anon-types"
#endif

#include <cmath>
#include <cstdint>
#include <osg/core/type_name.hpp>
#include <osg/maths/numbers.hpp>
#include <type_traits>

// Guarantee fixed-width floating-point semantics: float=32-bit, double=64-bit IEEE 754.
// On GCC/Clang, std::float32_t/_Float32 is a distinct type from float, which breaks
// pointer compatibility with OpenGL (GLfloat*/GLdouble*). We use float/double directly
// and enforce width with static_assert rather than using <stdfloat> types.
static_assert( sizeof( float ) == 4,
               "float must be 32-bit (IEEE 754 binary32)" );
static_assert( sizeof( double ) == 8,
               "double must be 64-bit (IEEE 754 binary64)" );
static_assert( std::numeric_limits<float>::is_iec559,
               "float must be IEEE 754" );
static_assert( std::numeric_limits<double>::is_iec559,
               "double must be IEEE 754" );

namespace osg
{

    /// t_vec2 template class that represents a 2D vector
    template<typename T>
    struct t_vec2
    {
            using value_type = T;

            union
            {
                    value_type value[2];

                    struct
                    {
                            value_type x, y;
                    };

                    struct
                    {
                            value_type r, g;
                    };

                    struct
                    {
                            value_type s, t;
                    };
            };

            constexpr t_vec2() :
                value{}
            {
            }

            constexpr t_vec2( const t_vec2& v ) :
                value{
                    v.x,
                    v.y
                }
            {
            }

            constexpr t_vec2&
            operator=( const t_vec2& ) = default;

            constexpr t_vec2( value_type in_x,
                              value_type in_y ) :
                value{
                    in_x,
                    in_y
                }
            {
            }

            template<typename R>
            constexpr explicit t_vec2( const t_vec2<R>& v ) :
                value{
                    static_cast<T>( v.x ),
                    static_cast<T>( v.y )
                }
            {
            }

            constexpr std::size_t
            size() const
            {
                return 2;
            }

            value_type&
            operator[]( std::size_t i )
            {
                return value[i];
            }

            value_type
            operator[]( std::size_t i ) const
            {
                return value[i];
            }

            template<typename R>
            t_vec2&
            operator=( const t_vec2<R>& rhs )
            {
                value[0] = static_cast<value_type>( rhs[0] );
                value[1] = static_cast<value_type>( rhs[1] );
                return *this;
            }

            T*
            data()
            {
                return value;
            }

            const T*
            data() const
            {
                return value;
            }

            void
            set( value_type in_x,
                 value_type in_y )
            {
                x = in_x;
                y = in_y;
            }

            inline t_vec2&
            operator+=( const t_vec2& rhs )
            {
                value[0] += rhs.value[0];
                value[1] += rhs.value[1];
                return *this;
            }

            inline t_vec2&
            operator-=( const t_vec2& rhs )
            {
                value[0] -= rhs.value[0];
                value[1] -= rhs.value[1];
                return *this;
            }

            inline t_vec2&
            operator*=( value_type rhs )
            {
                value[0] *= rhs;
                value[1] *= rhs;
                return *this;
            }

            inline t_vec2&
            operator*=( const t_vec2& rhs )
            {
                value[0] *= rhs.value[0];
                value[1] *= rhs.value[1];
                return *this;
            }

            friend constexpr t_vec2<T>
            operator*( const t_vec2<T>& lhs,
                       T                rhs )
            {
                return t_vec2<T>( lhs[0] * rhs, lhs[1] * rhs );
            }

            friend constexpr t_vec2<T>
            operator*( T                lhs,
                       const t_vec2<T>& rhs )
            {
                return t_vec2<T>( lhs * rhs[0], lhs * rhs[1] );
            }

            inline t_vec2&
            operator/=( value_type rhs )
            {
                if constexpr( std::is_floating_point_v<value_type> )
                {
                    value_type inv  = numbers<value_type>::one() / rhs;
                    value[0]       *= inv;
                    value[1]       *= inv;
                }
                else
                {
                    value[0] /= rhs;
                    value[1] /= rhs;
                }
                return *this;
            }

            explicit
            operator bool() const noexcept
            {
                return value[0] !=
                       numbers<value_type>::zero() ||
                       value[1] != numbers<value_type>::zero();
            }
    };

    using vec2   = t_vec2<float>;          // float (32-bit) 2D vector
    using dvec2  = t_vec2<double>;         // double (64-bit) 2D vector
    using ldvec2 = t_vec2<long double>;    // long double 2D vector
    using bvec2  = t_vec2<int8_t>;         // signed 8 bit integer 2D vector
    using svec2  = t_vec2<int16_t>;        // signed 16 bit integer 2D vector
    using ivec2  = t_vec2<int32_t>;        // signed 32 bit integer 2D vector
    using ubvec2 = t_vec2<uint8_t>;        // unsigned 8 bit integer 2D vector
    using usvec2 = t_vec2<uint16_t>;       // unsigned 16 bit integer 2D vector
    using uivec2 = t_vec2<uint32_t>;       // unsigned 32 bit integer 2D vector

    OSG_type_name( osg::vec2 );
    OSG_type_name( osg::dvec2 );
    OSG_type_name( osg::bvec2 );
    OSG_type_name( osg::svec2 );
    OSG_type_name( osg::ivec2 );
    OSG_type_name( osg::ubvec2 );
    OSG_type_name( osg::usvec2 );
    OSG_type_name( osg::uivec2 );

    template<typename T>
    constexpr bool
    operator==( const t_vec2<T>& lhs,
                const t_vec2<T>& rhs )
    {
        return lhs[0] == rhs[0] && lhs[1] == rhs[1];
    }

    template<typename T>
    constexpr bool
    operator!=( const t_vec2<T>& lhs,
                const t_vec2<T>& rhs )
    {
        return lhs[0] != rhs[0] || lhs[1] != rhs[1];
    }

    template<typename T>
    constexpr bool
    operator<( const t_vec2<T>& lhs,
               const t_vec2<T>& rhs )
    {
        if( lhs[0] < rhs[0] )
        {
            return true;
        }
        if( lhs[0] > rhs[0] )
        {
            return false;
        }
        return lhs[1] < rhs[1];
    }

    template<typename T>
    constexpr t_vec2<T>
    operator-( const t_vec2<T>& lhs,
               const t_vec2<T>& rhs )
    {
        return t_vec2<T>( lhs[0] - rhs[0], lhs[1] - rhs[1] );
    }

    template<typename T>
    constexpr t_vec2<T>
    operator-( const t_vec2<T>& v )
    {
        return t_vec2<T>( -v[0], -v[1] );
    }

    template<typename T>
    constexpr t_vec2<T>
    operator+( const t_vec2<T>& lhs,
               const t_vec2<T>& rhs )
    {
        return t_vec2<T>( lhs[0] + rhs[0], lhs[1] + rhs[1] );
    }

    template<typename T>
    constexpr t_vec2<T>
    operator*( const t_vec2<T>& lhs,
               const t_vec2<T>& rhs )
    {
        return t_vec2<T>( lhs[0] * rhs[0], lhs[1] * rhs[1] );
    }

    template<typename T>
    constexpr t_vec2<T>
    operator/( const t_vec2<T>& lhs,
               T                rhs )
    {
        if constexpr( std::is_floating_point_v<T> )
        {
            T inv = numbers<T>::one() / rhs;
            return t_vec2<T>( lhs[0] * inv, lhs[1] * inv );
        }
        else
        {
            return t_vec2<T>( lhs[0] / rhs, lhs[1] / rhs );
        }
    }

    template<typename T>
    constexpr T
    length( const t_vec2<T>& v )
    {
        return std::sqrt( v[0] * v[0] + v[1] * v[1] );
    }

    template<typename T>
    constexpr T
    length2( const t_vec2<T>& v )
    {
        return v[0] * v[0] + v[1] * v[1];
    }

    template<typename T>
    constexpr t_vec2<T>
    normalize( const t_vec2<T>& v )
    {
        return v / length( v );
    }

    template<typename T>
    constexpr T
    dot( const t_vec2<T>& lhs,
         const t_vec2<T>& rhs )
    {
        return lhs[0] * rhs[0] + lhs[1] * rhs[1];
    }

    /// cross product of a vec2 can be thought of as cross product of vec3's with the z
    /// value of 0.0/vec3's in the xy plane. The returned value is the length of the
    /// resulting vec3 cross product, and can be treated as the signed area of the
    /// parallelogram, negative if rhs is clockwise from lhs when looking down on xy
    /// plane.
    template<typename T>
    constexpr T
    cross( const t_vec2<T>& lhs,
           const t_vec2<T>& rhs )
    {
        return ( lhs[0] * rhs[1] - rhs[0] * lhs[1] );
    }

    template<typename T>
    constexpr t_vec2<T>
    mix( const t_vec2<T>& start,
         const t_vec2<T>& end,
         T                r )
    {
        T one_minus_r = numbers<T>::one() - r;
        return t_vec2<T>( start[0] * one_minus_r + end[0] * r,
                          start[1] * one_minus_r + end[1] * r );
    }

}    // namespace osg

#if defined( __clang__ )
    #pragma clang diagnostic pop
#endif
#if defined( __GNUC__ )
    #pragma GCC diagnostic pop
#endif
