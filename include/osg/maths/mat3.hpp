/* Adapted from VulkanSceneGraph — MIT License, Copyright(c) 2018 Robert Osfield */

#pragma once

#include <osg/maths/vec3.hpp>

namespace osg
{

    /// t_mat3 template class that represents a 3x3 matrix.
    template<typename T>
    struct t_mat3
    {
        public:

            using value_type  = T;
            using column_type = t_vec3<T>;

            column_type value[3];

            constexpr t_mat3() :
                value{
                    { numbers<value_type>::one(),
                     numbers<value_type>::zero(),
                     numbers<value_type>::zero()},
                    {numbers<value_type>::zero(),
                     numbers<value_type>::one(),
                     numbers<value_type>::zero()},
                    {numbers<value_type>::zero(),
                     numbers<value_type>::zero(),
                     numbers<value_type>::one() }
            }
            {
            }

            constexpr explicit t_mat3( value_type v ) :
                value{
                    {                          v,
                     numbers<value_type>::zero(),
                     numbers<value_type>::zero()   },
                    {numbers<value_type>::zero(),
                     v, numbers<value_type>::zero()},
                    {numbers<value_type>::zero(),
                     numbers<value_type>::zero(),
                     v                             }
            }
            {
            }

            constexpr t_mat3( value_type v0,
                              value_type v1,
                              value_type v2, /* column 0 */
                              value_type v3,
                              value_type v4,
                              value_type v5, /* column 1 */
                              value_type v6,
                              value_type v7,
                              value_type v8 ) /* column 2 */ :
                value{
                    {v0,
                     v1, v2},
                    {v3,
                     v4, v5},
                    {v6,
                     v7, v8}
            }
            {
            }

            constexpr explicit t_mat3( value_type v[9] ) :
                value{
                    {v[0],
                     v[1],
                     v[2]},
                    {v[3],
                     v[4],
                     v[5]},
                    {v[6],
                     v[7],
                     v[8]}
            }
            {
            }

            constexpr t_mat3( const column_type& c0,
                              const column_type& c1,
                              const column_type& c2 ) :
                value{
                    c0,
                    c1,
                    c2
                }
            {
            }

            template<typename R>
            explicit t_mat3( const t_mat3<R>& rhs )
            {
                value[0] = rhs[0];
                value[1] = rhs[1];
                value[2] = rhs[2];
            }

            constexpr std::size_t
            size() const
            {
                return 9;
            }

            constexpr std::size_t
            columns() const
            {
                return 3;
            }

            constexpr std::size_t
            rows() const
            {
                return 3;
            }

            column_type&
            operator[]( std::size_t c )
            {
                return value[c];
            }

            const column_type&
            operator[]( std::size_t c ) const
            {
                return value[c];
            }

            value_type&
            operator()( std::size_t c,
                        std::size_t r )
            {
                return value[c][r];
            }

            value_type
            operator()( std::size_t c,
                        std::size_t r ) const
            {
                return value[c][r];
            }

            template<typename R>
            t_mat3&
            operator=( const t_mat3<R>& rhs )
            {
                value[0] = rhs[0];
                value[1] = rhs[1];
                value[2] = rhs[2];
                return *this;
            }

            void
            set( value_type v0,
                 value_type v1,
                 value_type v2,  /* column 0 */
                 value_type v3,
                 value_type v4,
                 value_type v5,  /* column 1 */
                 value_type v6,
                 value_type v7,
                 value_type v8 ) /* column 2 */
            {
                value[0].set( v0, v1, v2 );
                value[1].set( v3, v4, v5 );
                value[2].set( v6, v7, v8 );
            }

            template<typename R>
            void
            set( const t_mat3<R>& rhs )
            {
                value[0] = rhs[0];
                value[1] = rhs[1];
                value[2] = rhs[2];
            }

            T*
            data()
            {
                return value[0].data();
            }

            const T*
            data() const
            {
                return value[0].data();
            }
    };

    using mat3  = t_mat3<float>;     /// float (32-bit) 3x3 matrix
    using dmat3 = t_mat3<double>;    /// double (64-bit) 3x3 matrix

    OSG_type_name( osg::mat3 );
    OSG_type_name( osg::dmat3 );

    template<typename T>
    bool
    operator==( const t_mat3<T>& lhs,
                const t_mat3<T>& rhs )
    {
        return lhs.value[0] ==
               rhs.value[0] &&
               lhs.value[1] ==
               rhs.value[1] &&
               lhs.value[2] == rhs.value[2];
    }

    template<typename T>
    bool
    operator!=( const t_mat3<T>& lhs,
                const t_mat3<T>& rhs )
    {
        return lhs.value[0] !=
               rhs.value[0] ||
               lhs.value[1] !=
               rhs.value[1] ||
               lhs.value[2] != rhs.value[2];
    }

    template<typename T>
    bool
    operator<( const t_mat3<T>& lhs,
               const t_mat3<T>& rhs )
    {
        if( lhs.value[0] < rhs.value[0] )
        {
            return true;
        }
        if( rhs.value[0] < lhs.value[0] )
        {
            return false;
        }
        if( lhs.value[1] < rhs.value[1] )
        {
            return true;
        }
        if( rhs.value[1] < lhs.value[1] )
        {
            return false;
        }
        return lhs.value[2] < rhs.value[2];
    }

    template<typename T>
    T
    dot( const t_mat3<T>& lhs,
         const t_mat3<T>& rhs,
         int              c,
         int              r )
    {
        return lhs[0][r] * rhs[c][0] + lhs[1][r] * rhs[c][1] + lhs[2][r] * rhs[c][2];
    }

    template<typename T>
    t_mat3<T>
    operator*( const t_mat3<T>& lhs,
               const t_mat3<T>& rhs )
    {
        return t_mat3<T>( dot( lhs, rhs, 0, 0 ),
                          dot( lhs, rhs, 0, 1 ),
                          dot( lhs, rhs, 0, 2 ),
                          dot( lhs, rhs, 1, 0 ),
                          dot( lhs, rhs, 1, 1 ),
                          dot( lhs, rhs, 1, 2 ),
                          dot( lhs, rhs, 2, 0 ),
                          dot( lhs, rhs, 2, 1 ),
                          dot( lhs, rhs, 2, 2 ) );
    }

    template<typename T>
    t_vec3<T>
    operator*( const t_mat3<T>& lhs,
               const t_vec3<T>& rhs )
    {
        return t_vec3<T>( lhs[0][0] * rhs[0] + lhs[1][0] * rhs[1] + lhs[2][0] * rhs[2],
                          lhs[0][1] * rhs[0] + lhs[1][1] * rhs[1] + lhs[2][1] * rhs[2],
                          lhs[0][2] * rhs[0] + lhs[1][2] * rhs[1] + lhs[2][2] * rhs[2] );
    }

    template<typename T>
    t_vec3<T>
    operator*( const t_vec3<T>& lhs,
               const t_mat3<T>& rhs )
    {
        return t_vec3<T>( lhs[0] * rhs[0][0] + lhs[1] * rhs[0][1] + lhs[2] * rhs[0][2],
                          lhs[0] * rhs[1][0] + lhs[1] * rhs[1][1] + lhs[2] * rhs[1][2],
                          lhs[0] * rhs[2][0] + lhs[1] * rhs[2][1] + lhs[2] * rhs[2][2] );
    }

}    // namespace osg
