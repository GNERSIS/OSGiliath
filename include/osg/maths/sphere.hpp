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

#include <osg/maths/box.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>

namespace osg
{

    /// template sphere class
    template<typename T>
    struct t_sphere
    {
            using value_type  = T;
            using vec_type    = t_vec4<T>;
            using center_type = t_vec3<T>;

            union
            {
                    value_type value[4];

                    vec_type   vec;

                    struct
                    {
                            value_type x, y, z, r;
                    };

                    struct
                    {
                            center_type center;
                            value_type  radius;
                    };
            };

            constexpr t_sphere() :
                value{
                    numbers<value_type>::zero(),
                    numbers<value_type>::zero(),
                    numbers<value_type>::zero(),
                    numbers<value_type>::minus_one()
                }
            {
            }

            constexpr t_sphere( const t_sphere& s ) :
                value{
                    s[0],
                    s[1],
                    s[2],
                    s[3]
                }
            {
            }

            constexpr t_sphere&
            operator=( const t_sphere& ) = default;

            template<typename R>
            constexpr explicit t_sphere( const t_sphere<R>& s ) :
                value{
                    static_cast<value_type>( s[0] ),
                    static_cast<value_type>( s[1] ),
                    static_cast<value_type>( s[2] ),
                    static_cast<value_type>( s[3] )
                }
            {
            }

            template<typename R>
            constexpr t_sphere( const t_vec3<R>& c,
                                T                rad ) :
                value{
                    static_cast<value_type>( c.x ),
                    static_cast<value_type>( c.y ),
                    static_cast<value_type>( c.z ),
                    rad
                }
            {
            }

            template<typename R>
            constexpr t_sphere( R sx,
                                R sy,
                                R sz,
                                R sd ) :
                value{
                    sx,
                    sy,
                    sz,
                    sd
                }
            {
            }

            /// Construct from a bounding box — compute enclosing sphere
            template<typename BT>
            explicit t_sphere( const t_box<BT>& bb ) :
                value{
                    numbers<value_type>::zero(),
                    numbers<value_type>::zero(),
                    numbers<value_type>::zero(),
                    numbers<value_type>::minus_one()
                }
            {
                if( bb.valid() )
                {
                    center = t_vec3<value_type>(
                        static_cast<value_type>( ( bb.min.x + bb.max.x ) * 0.5 ),
                        static_cast<value_type>( ( bb.min.y + bb.max.y ) * 0.5 ),
                        static_cast<value_type>( ( bb.min.z + bb.max.z ) * 0.5 )
                    );
                    t_vec3<value_type> d( static_cast<value_type>( bb.max.x - bb.min.x ),
                                          static_cast<value_type>( bb.max.y - bb.min.y ),
                                          static_cast<value_type>( bb.max.z -
                                                                   bb.min.z ) );
                    radius = length( d ) * static_cast<value_type>( 0.5 );
                }
            }

            constexpr std::size_t
            size() const
            {
                return 4;
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
            t_sphere&
            operator=( const t_sphere<R>& rhs )
            {
                value[0] = static_cast<value_type>( rhs[0] );
                value[1] = static_cast<value_type>( rhs[1] );
                value[2] = static_cast<value_type>( rhs[2] );
                value[3] = static_cast<value_type>( rhs[3] );
                return *this;
            }

            void
            set( value_type in_x,
                 value_type in_y,
                 value_type in_z,
                 value_type in_r )
            {
                x = in_x;
                y = in_y;
                z = in_z;
                r = in_r;
            }

            template<typename R>
            void
            set( const t_vec3<R>& c,
                 T                rad )
            {
                x = static_cast<value_type>( c.x );
                y = static_cast<value_type>( c.y );
                z = static_cast<value_type>( c.z );
                r = rad;
            }

            bool
            valid() const
            {
                return radius >= numbers<value_type>::zero();
            }

            explicit
            operator bool() const noexcept
            {
                return valid();
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

            /// Clear the bounding sphere. Reset to default values.
            void
            init()
            {
                center.set( numbers<value_type>::zero(),
                            numbers<value_type>::zero(),
                            numbers<value_type>::zero() );
                radius = numbers<value_type>::minus_one();
            }

            void
            reset()
            {
                init();
            }

            /// Returns the squared length of the radius.
            value_type
            radius2() const
            {
                return radius * radius;
            }

            /// Expands the sphere to encompass the given point.
            /// Repositions the sphere center to minimize the radius increase.
            void
            expandBy( const center_type& v )
            {
                if( valid() )
                {
                    center_type dv = v - center;
                    value_type  r  = length( dv );
                    if( r > radius )
                    {
                        value_type dr  = ( r - radius ) * value_type( 0.5 );
                        center        += dv * ( dr / r );
                        radius        += dr;
                    }
                }
                else
                {
                    center = v;
                    radius = value_type( 0.0 );
                }
            }

            /// Expands the sphere to encompass the given sphere.
            /// Repositions the sphere center to minimize the radius increase.
            void
            expandBy( const t_sphere& sh )
            {
                if( !sh.valid() )
                {
                    return;
                }

                if( !valid() )
                {
                    center = sh.center;
                    radius = sh.radius;
                    return;
                }

                double d = length( center - sh.center );

                // New sphere is already inside this one
                if( d + sh.radius <= radius )
                {
                    return;
                }

                // New sphere completely contains this one
                if( d + radius <= sh.radius )
                {
                    center = sh.center;
                    radius = sh.radius;
                    return;
                }

                // Build a new sphere that completely contains both
                double new_radius = ( radius + d + sh.radius ) * 0.5;
                double ratio      = ( new_radius - radius ) / d;

                center[0] +=
                    static_cast<value_type>( ( sh.center[0] - center[0] ) * ratio );
                center[1] +=
                    static_cast<value_type>( ( sh.center[1] - center[1] ) * ratio );
                center[2] +=
                    static_cast<value_type>( ( sh.center[2] - center[2] ) * ratio );

                radius = static_cast<value_type>( new_radius );
            }

            /// Expands the sphere to encompass the given box.
            /// Repositions the sphere center to minimize the radius increase.
            template<typename BT>
            void
            expandBy( const t_box<BT>& bb )
            {
                if( bb.valid() )
                {
                    if( valid() )
                    {
                        t_box<value_type> newbb( bb );

                        for( unsigned int c = 0; c < 8; ++c )
                        {
                            center_type v   = bb.corner( c ) - center;
                            value_type  len = length( v );
                            if( len > value_type( 0.0 ) )
                            {
                                v = v * ( value_type( 1.0 ) / len );    // normalize
                            }
                            v = v *
                                ( -radius );    // move in opposite direction by radius
                            v = v + center;     // move to absolute position
                            newbb.expandBy( v );
                        }

                        center = newbb.center();
                        radius = newbb.radius();
                    }
                    else
                    {
                        center = bb.center();
                        radius = bb.radius();
                    }
                }
            }

            /// Expands the sphere radius to encompass the given point.
            /// Does not reposition the sphere center.
            void
            expandRadiusBy( const center_type& v )
            {
                if( valid() )
                {
                    value_type r = length( v - center );
                    if( r > radius )
                    {
                        radius = r;
                    }
                }
                else
                {
                    center = v;
                    radius = value_type( 0.0 );
                }
            }

            /// Expands the sphere radius to encompass the given sphere.
            /// Does not reposition the sphere center.
            void
            expandRadiusBy( const t_sphere& sh )
            {
                if( sh.valid() )
                {
                    if( valid() )
                    {
                        value_type r = length( sh.center - center ) + sh.radius;
                        if( r > radius )
                        {
                            radius = r;
                        }
                    }
                    else
                    {
                        center = sh.center;
                        radius = sh.radius;
                    }
                }
            }

            /// Returns true if v is within the sphere.
            bool
            contains( const center_type& v ) const
            {
                return valid() && ( length2( v - center ) <= radius2() );
            }

            /// Returns true if there is a non-empty intersection with the given bounding
            /// sphere.
            bool
            intersects( const t_sphere& bs ) const
            {
                return valid() &&
                       bs.valid() &&
                       ( length2( center - bs.center ) <=
                         ( radius + bs.radius ) *
                         ( radius + bs.radius ) );
            }
    };

    using sphere   = t_sphere<float>;          /// float (32-bit) sphere class
    using dsphere  = t_sphere<double>;         /// double (64-bit) sphere class
    using ldsphere = t_sphere<long double>;    /// long double sphere class

    OSG_type_name( osg::sphere );
    OSG_type_name( osg::dsphere );
    OSG_type_name( osg::ldsphere );

    template<typename T>
    constexpr bool
    operator==( const t_sphere<T>& lhs,
                const t_sphere<T>& rhs )
    {
        return ( lhs.center == rhs.center ) && ( lhs.radius == rhs.radius );
    }

    template<typename T>
    constexpr bool
    operator!=( const t_sphere<T>& lhs,
                const t_sphere<T>& rhs )
    {
        return ( lhs.center != rhs.center ) || ( lhs.radius != rhs.radius );
    }

    template<typename T>
    constexpr bool
    operator<( const t_sphere<T>& lhs,
               const t_sphere<T>& rhs )
    {
        if( lhs.center < rhs.center )
        {
            return true;
        }
        if( rhs.center < lhs.center )
        {
            return false;
        }
        return lhs.radius < rhs.radius;
    }

}    // namespace osg

#if defined( __clang__ )
    #pragma clang diagnostic pop
#endif
#if defined( __GNUC__ )
    #pragma GCC diagnostic pop
#endif
