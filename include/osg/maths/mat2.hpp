/* Adapted from VulkanSceneGraph — MIT License, Copyright(c) 2018 Robert Osfield */

#pragma once

#include <osg/maths/vec2.hpp>

namespace osg
{

    /// t_mat2 template class that represents a 2x2 matrix.
    template<typename T>
    struct t_mat2
    {
        public:

            using value_type  = T;
            using column_type = t_vec2<T>;

            column_type value[2];

            constexpr t_mat2() :
                value{
                    { numbers<value_type>::one(),
                     numbers<value_type>::zero()},
                    {numbers<value_type>::zero(),
                     numbers<value_type>::one() }
            }
            {
            }

            constexpr explicit t_mat2( value_type v ) :
                value{
                    {                          v,
                     numbers<value_type>::zero()},
                    {numbers<value_type>::zero(),
                     v                          }
            }
            {
            }

            constexpr t_mat2( value_type v0,
                              value_type v1, /* column 0 */
                              value_type v2,
                              value_type v3 ) /* column 1 */ :
                value{
                    {v0,
                     v1},
                    {v2,
                     v3}
            }
            {
            }

            constexpr explicit t_mat2( value_type v[4] ) :
                value{
                    {v[0],
                     v[1]},
                    {v[2],
                     v[3]}
            }
            {
            }

            constexpr t_mat2( const column_type& c0,
                              const column_type& c1 ) :
                value{
                    c0,
                    c1
                }
            {
            }

            template<typename R>
            explicit t_mat2( const t_mat2<R>& rhs )
            {
                value[0] = rhs[0];
                value[1] = rhs[1];
            }

            constexpr std::size_t
            size() const
            {
                return 4;
            }

            constexpr std::size_t
            columns() const
            {
                return 2;
            }

            constexpr std::size_t
            rows() const
            {
                return 2;
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
            t_mat2&
            operator=( const t_mat2<R>& rhs )
            {
                value[0] = rhs[0];
                value[1] = rhs[1];
                return *this;
            }

            void
            set( value_type v0,
                 value_type v1,  /* column 0 */
                 value_type v2,
                 value_type v3 ) /* column 1 */
            {
                value[0].set( v0, v1 );
                value[1].set( v2, v3 );
            }

            template<typename R>
            void
            set( const t_mat2<R>& rhs )
            {
                value[0] = rhs[0];
                value[1] = rhs[1];
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

    using mat2  = t_mat2<float>;     /// float (32-bit) 2x2 matrix
    using dmat2 = t_mat2<double>;    /// double (64-bit) 2x2 matrix

    OSG_type_name( osg::mat2 );
    OSG_type_name( osg::dmat2 );

    template<typename T>
    bool
    operator==( const t_mat2<T>& lhs,
                const t_mat2<T>& rhs )
    {
        return lhs.value[0] == rhs.value[0] && lhs.value[1] == rhs.value[1];
    }

    template<typename T>
    bool
    operator!=( const t_mat2<T>& lhs,
                const t_mat2<T>& rhs )
    {
        return lhs.value[0] != rhs.value[0] || lhs.value[1] != rhs.value[1];
    }

    template<typename T>
    bool
    operator<( const t_mat2<T>& lhs,
               const t_mat2<T>& rhs )
    {
        if( lhs.value[0] < rhs.value[0] )
        {
            return true;
        }
        if( rhs.value[0] < lhs.value[0] )
        {
            return false;
        }
        return lhs.value[1] < rhs.value[1];
    }

    template<typename T>
    T
    dot( const t_mat2<T>& lhs,
         const t_mat2<T>& rhs,
         int              c,
         int              r )
    {
        return lhs[0][r] * rhs[c][0] + lhs[1][r] * rhs[c][1];
    }

    template<typename T>
    t_mat2<T>
    operator*( const t_mat2<T>& lhs,
               const t_mat2<T>& rhs )
    {
        return t_mat2<T>( dot( lhs, rhs, 0, 0 ),
                          dot( lhs, rhs, 0, 1 ),
                          dot( lhs, rhs, 1, 0 ),
                          dot( lhs, rhs, 1, 1 ) );
    }

    template<typename T>
    t_vec2<T>
    operator*( const t_mat2<T>& lhs,
               const t_vec2<T>& rhs )
    {
        return t_vec2<T>( lhs[0][0] * rhs[0] + lhs[1][0] * rhs[1],
                          lhs[0][1] * rhs[0] + lhs[1][1] * rhs[1] );
    }

    template<typename T>
    t_vec2<T>
    operator*( const t_vec2<T>& lhs,
               const t_mat2<T>& rhs )
    {
        return t_vec2<T>( lhs[0] * rhs[0][0] + lhs[1] * rhs[0][1],
                          lhs[0] * rhs[1][0] + lhs[1] * rhs[1][1] );
    }

}    // namespace osg
