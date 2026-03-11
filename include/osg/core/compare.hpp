/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Function objects for pointer comparison and content-based
 * comparison of Referenced objects. Used in STL containers.
 */
#pragma once

#include <cstddef>
#include <cstring>
#include <osg/core/ref_ptr.hpp>

namespace osg
{

    /** Three-way value comparison. Returns -1, 0, or 1. */
    template<typename T>
    inline int
    compare_value( const T& lhs,
                   const T& rhs ) noexcept
    {
        if( lhs < rhs )
        {
            return -1;
        }
        if( rhs < lhs )
        {
            return 1;
        }
        return 0;
    }

    /** Three-way comparison of ref_ptr-held objects via their compare() method.
     *  Handles null: null < non-null, null == null. */
    template<typename T,
             typename R>
    inline int
    compare_pointer( const ref_ptr<T>& lhs,
                     const ref_ptr<R>& rhs )
    {
        if( lhs == rhs )
        {
            return 0;
        }
        if( !lhs )
        {
            return -1;
        }
        if( !rhs )
        {
            return 1;
        }
        return lhs->compare( *rhs );
    }

    /** Binary comparison of a POD value via memcmp. */
    template<typename T>
    inline int
    compare_memory( const T& lhs,
                    const T& rhs ) noexcept
    {
        return std::memcmp( &lhs, &rhs, sizeof( T ) );
    }

    /** Compare a contiguous region of memory between two struct member addresses.
     *  Useful for comparing all fields between `start` and `end` (inclusive) in
     *  a flat struct. Both structs must have identical layout. */
    template<typename S,
             typename E>
    inline int
    compare_region( const S& lhs_start,
                    const E& lhs_end,
                    const S& rhs_start ) noexcept
    {
        const char* lhs_ptr = reinterpret_cast<const char*>( &lhs_start );
        const char* rhs_ptr = reinterpret_cast<const char*>( &rhs_start );
        std::size_t size =
            static_cast<std::size_t>( reinterpret_cast<const char*>( &lhs_end ) -
                                      lhs_ptr ) +
            sizeof( E );
        return std::memcmp( lhs_ptr, rhs_ptr, size );
    }

    /** Compare containers of ref_ptr element-wise via compare_pointer(). */
    template<typename C>
    inline int
    compare_pointer_container( const C& lhs,
                               const C& rhs )
    {
        if( lhs.size() < rhs.size() )
        {
            return -1;
        }
        if( lhs.size() > rhs.size() )
        {
            return 1;
        }
        if( lhs.empty() )
        {
            return 0;
        }

        auto rhs_itr = rhs.begin();
        for( const auto& elem : lhs )
        {
            int result = compare_pointer( elem, *rhs_itr++ );
            if( result != 0 )
            {
                return result;
            }
        }
        return 0;
    }

    /** Compare containers of contiguous values via a single memcmp.
     *  Only valid for containers with contiguous storage (vector, array). */
    template<typename C>
    inline int
    compare_value_container( const C& lhs,
                             const C& rhs ) noexcept
    {
        if( lhs.size() < rhs.size() )
        {
            return -1;
        }
        if( lhs.size() > rhs.size() )
        {
            return 1;
        }
        if( lhs.empty() )
        {
            return 0;
        }
        return std::memcmp( lhs.data(),
                            rhs.data(),
                            lhs.size() * sizeof( typename C::value_type ) );
    }

    /** Functor for std::set/std::map that dereferences pointers before comparing.
     *  Enables sorted containers of ref_ptr ordered by pointed-to value. */
    struct DereferenceLess
    {
            template<typename P>
            bool
            operator()( const P& lhs,
                        const P& rhs ) const
            {
                if( !lhs )
                {
                    return static_cast<bool>( rhs );
                }
                if( !rhs )
                {
                    return false;
                }
                return lhs->compare( *rhs ) < 0;
            }
    };

}    // namespace osg
