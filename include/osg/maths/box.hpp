/* Adapted from VulkanSceneGraph — MIT License, Copyright(c) 2018 Robert Osfield */

#pragma once

#include <algorithm>
#include <limits>
#include <osg/maths/vec3.hpp>

namespace osg
{

    // forward declaration for expandBy(t_sphere)
    template<typename T>
    struct t_sphere;

    /// t_box template class that represents an axis aligned bounding box
    template<typename T>
    struct t_box
    {
            using value_type  = T;
            using vec_type    = t_vec3<T>;

            vec_type min      = vec_type( std::numeric_limits<value_type>::max(),
                                          std::numeric_limits<value_type>::max(),
                                          std::numeric_limits<value_type>::max() );
            vec_type max      = vec_type( std::numeric_limits<value_type>::lowest(),
                                          std::numeric_limits<value_type>::lowest(),
                                          std::numeric_limits<value_type>::lowest() );

            constexpr t_box() = default;
            constexpr t_box( const t_box& s ) = default;

            template<typename R>
            constexpr explicit t_box( const t_box<R>& s ) :
                min( s.min ),
                max( s.max )
            {
            }

            constexpr t_box( const vec_type& in_min,
                             const vec_type& in_max ) :
                min( in_min ),
                max( in_max )
            {
            }

            constexpr t_box( value_type xmin,
                             value_type ymin,
                             value_type zmin,
                             value_type xmax,
                             value_type ymax,
                             value_type zmax ) :
                min( xmin,
                     ymin,
                     zmin ),
                max( xmax,
                     ymax,
                     zmax )
            {
            }

            constexpr t_box&
            operator=( const t_box& ) = default;

            constexpr std::size_t
            size() const
            {
                return 6;
            }

            value_type&
            operator[]( std::size_t i )
            {
                return data()[i];
            }

            value_type
            operator[]( std::size_t i ) const
            {
                return data()[i];
            }

            bool
            valid() const
            {
                return min.x <= max.x && min.y <= max.y && min.z <= max.z;
            }

            explicit
            operator bool() const noexcept
            {
                return valid();
            }

            T*
            data()
            {
                return min.data();
            }

            const T*
            data() const
            {
                return min.data();
            }

            void
            init()
            {
                min = vec_type( std::numeric_limits<value_type>::max(),
                                std::numeric_limits<value_type>::max(),
                                std::numeric_limits<value_type>::max() );
                max = vec_type( std::numeric_limits<value_type>::lowest(),
                                std::numeric_limits<value_type>::lowest(),
                                std::numeric_limits<value_type>::lowest() );
            }

            void
            reset()
            {
                init();
            }

            /// Set the bounding box extents.
            void
            set( value_type xmin,
                 value_type ymin,
                 value_type zmin,
                 value_type xmax,
                 value_type ymax,
                 value_type zmax )
            {
                min.set( xmin, ymin, zmin );
                max.set( xmax, ymax, zmax );
            }

            /// Set the bounding box extents.
            void
            set( const vec_type& in_min,
                 const vec_type& in_max )
            {
                min = in_min;
                max = in_max;
            }

            /// Accessor methods
            value_type
            xMin() const
            {
                return min.x;
            }

            value_type
            yMin() const
            {
                return min.y;
            }

            value_type
            zMin() const
            {
                return min.z;
            }

            value_type
            xMax() const
            {
                return max.x;
            }

            value_type
            yMax() const
            {
                return max.y;
            }

            value_type
            zMax() const
            {
                return max.z;
            }

            /// Calculates and returns the bounding box center.
            vec_type
            center() const
            {
                return ( min + max ) * value_type( 0.5 );
            }

            /// Calculates and returns the bounding box radius.
            value_type
            radius() const
            {
                return length( max - min ) * value_type( 0.5 );
            }

            /// Calculates and returns the squared length of the bounding box radius.
            value_type
            radius2() const
            {
                return length2( max - min ) * value_type( 0.25 );
            }

            /// Returns a specific corner of the bounding box.
            /// pos specifies the corner as a number between 0 and 7.
            /// Each bit selects an axis, X, Y, or Z from least- to most-significant.
            /// Unset bits select the minimum value for that axis, and set bits select
            /// the maximum.
            vec_type
            corner( unsigned int pos ) const
            {
                return vec_type( pos & 1 ? max.x : min.x,
                                 pos & 2 ? max.y : min.y,
                                 pos & 4 ? max.z : min.z );
            }

            template<typename R>
            void
            add( const t_vec3<R>& v )
            {
                add( v.x, v.y, v.z );
            }

            void
            add( value_type x,
                 value_type y,
                 value_type z )
            {
                if( x < min.x )
                {
                    min.x = x;
                }
                if( y < min.y )
                {
                    min.y = y;
                }
                if( z < min.z )
                {
                    min.z = z;
                }
                if( x > max.x )
                {
                    max.x = x;
                }
                if( y > max.y )
                {
                    max.y = y;
                }
                if( z > max.z )
                {
                    max.z = z;
                }
            }

            template<typename R>
            void
            add( const t_box<R>& bb )
            {
                if( bb.min.x < min.x )
                {
                    min.x = bb.min.x;
                }
                if( bb.min.y < min.y )
                {
                    min.y = bb.min.y;
                }
                if( bb.min.z < min.z )
                {
                    min.z = bb.min.z;
                }
                if( bb.max.x > max.x )
                {
                    max.x = bb.max.x;
                }
                if( bb.max.y > max.y )
                {
                    max.y = bb.max.y;
                }
                if( bb.max.z > max.z )
                {
                    max.z = bb.max.z;
                }
            }

            /// Expands the bounding box to include the given coordinate.
            void
            expandBy( const vec_type& v )
            {
                if( v.x < min.x )
                {
                    min.x = v.x;
                }
                if( v.x > max.x )
                {
                    max.x = v.x;
                }
                if( v.y < min.y )
                {
                    min.y = v.y;
                }
                if( v.y > max.y )
                {
                    max.y = v.y;
                }
                if( v.z < min.z )
                {
                    min.z = v.z;
                }
                if( v.z > max.z )
                {
                    max.z = v.z;
                }
            }

            /// Expands the bounding box to include the given coordinate (cross-type).
            template<typename R>
            void
            expandBy( const t_vec3<R>& v )
            {
                value_type vx = static_cast<value_type>( v.x );
                value_type vy = static_cast<value_type>( v.y );
                value_type vz = static_cast<value_type>( v.z );
                if( vx < min.x )
                {
                    min.x = vx;
                }
                if( vx > max.x )
                {
                    max.x = vx;
                }
                if( vy < min.y )
                {
                    min.y = vy;
                }
                if( vy > max.y )
                {
                    max.y = vy;
                }
                if( vz < min.z )
                {
                    min.z = vz;
                }
                if( vz > max.z )
                {
                    max.z = vz;
                }
            }

            /// Expands the bounding box to include the given coordinate.
            void
            expandBy( value_type x,
                      value_type y,
                      value_type z )
            {
                if( x < min.x )
                {
                    min.x = x;
                }
                if( x > max.x )
                {
                    max.x = x;
                }
                if( y < min.y )
                {
                    min.y = y;
                }
                if( y > max.y )
                {
                    max.y = y;
                }
                if( z < min.z )
                {
                    min.z = z;
                }
                if( z > max.z )
                {
                    max.z = z;
                }
            }

            /// Expands this bounding box to include the given bounding box.
            void
            expandBy( const t_box& bb )
            {
                if( !bb.valid() )
                {
                    return;
                }

                if( bb.min.x < min.x )
                {
                    min.x = bb.min.x;
                }
                if( bb.max.x > max.x )
                {
                    max.x = bb.max.x;
                }
                if( bb.min.y < min.y )
                {
                    min.y = bb.min.y;
                }
                if( bb.max.y > max.y )
                {
                    max.y = bb.max.y;
                }
                if( bb.min.z < min.z )
                {
                    min.z = bb.min.z;
                }
                if( bb.max.z > max.z )
                {
                    max.z = bb.max.z;
                }
            }

            /// Expands this bounding box to include the given sphere.
            template<typename ST>
            void
            expandBy( const t_sphere<ST>& sh )
            {
                if( !sh.valid() )
                {
                    return;
                }

                if( sh.center.x - sh.radius < min.x )
                {
                    min.x = sh.center.x - sh.radius;
                }
                if( sh.center.x + sh.radius > max.x )
                {
                    max.x = sh.center.x + sh.radius;
                }
                if( sh.center.y - sh.radius < min.y )
                {
                    min.y = sh.center.y - sh.radius;
                }
                if( sh.center.y + sh.radius > max.y )
                {
                    max.y = sh.center.y + sh.radius;
                }
                if( sh.center.z - sh.radius < min.z )
                {
                    min.z = sh.center.z - sh.radius;
                }
                if( sh.center.z + sh.radius > max.z )
                {
                    max.z = sh.center.z + sh.radius;
                }
            }

            /// Returns true if this bounding box contains the specified coordinate.
            bool
            contains( const vec_type& v ) const
            {
                return valid() &&
                       ( v.x >= min.x && v.x <= max.x ) &&
                       ( v.y >= min.y && v.y <= max.y ) &&
                       ( v.z >= min.z && v.z <= max.z );
            }

            /// Returns true if this bounding box contains the specified coordinate
            /// allowing for specific epsilon.
            bool
            contains( const vec_type& v,
                      value_type      epsilon ) const
            {
                return valid() &&
                       ( ( v.x + epsilon ) >= min.x && ( v.x - epsilon ) <= max.x ) &&
                       ( ( v.y + epsilon ) >= min.y && ( v.y - epsilon ) <= max.y ) &&
                       ( ( v.z + epsilon ) >= min.z && ( v.z - epsilon ) <= max.z );
            }

            /// Returns the intersection of this bounding box and the specified bounding
            /// box.
            t_box
            intersect( const t_box& bb ) const
            {
                return t_box( std::max( xMin(), bb.xMin() ),
                              std::max( yMin(), bb.yMin() ),
                              std::max( zMin(), bb.zMin() ),
                              std::min( xMax(), bb.xMax() ),
                              std::min( yMax(), bb.yMax() ),
                              std::min( zMax(), bb.zMax() ) );
            }

            /// Returns true if this bounding box intersects the specified bounding box.
            bool
            intersects( const t_box& bb ) const
            {
                return std::max( xMin(), bb.xMin() ) <=
                       std::min( xMax(), bb.xMax() ) &&
                       std::max( yMin(), bb.yMin() ) <=
                       std::min( yMax(), bb.yMax() ) &&
                       std::max( zMin(), bb.zMin() ) <= std::min( zMax(), bb.zMax() );
            }
    };

    using box   = t_box<float>;          /// float (32-bit) box class
    using dbox  = t_box<double>;         /// double (64-bit) box class
    using ldbox = t_box<long double>;    /// long double box class

    OSG_type_name( osg::box );
    OSG_type_name( osg::dbox );
    OSG_type_name( osg::ldbox );

    template<typename T>
    constexpr bool
    operator==( const t_box<T>& lhs,
                const t_box<T>& rhs )
    {
        return ( lhs.min == rhs.min ) && ( lhs.max == rhs.max );
    }

    template<typename T>
    constexpr bool
    operator!=( const t_box<T>& lhs,
                const t_box<T>& rhs )
    {
        return ( lhs.min != rhs.min ) || ( lhs.max != rhs.max );
    }

    template<typename T>
    constexpr bool
    operator<( const t_box<T>& lhs,
               const t_box<T>& rhs )
    {
        if( lhs.min < rhs.min )
        {
            return true;
        }
        if( rhs.min < lhs.min )
        {
            return false;
        }
        return lhs.max < rhs.max;
    }

}    // namespace osg
