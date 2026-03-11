/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Compile-time type name deduction. Provides type_name<T>()
 * returning a string_view of the unmangled type name.
 */
#pragma once

#include <typeinfo>

namespace osg
{

    /** Return a human-readable name for type T.
     *  The default returns the compiler's mangled name via typeid.
     *  Specialize via OSG_type_name() macro for demangled names. */
    template<typename T>
    constexpr const char*
    type_name() noexcept
    {
        return typeid( T ).name();
    }

}    // namespace osg

/** Register a demangled type name for use with osg::type_name<T>().
 *  Place in a header after the class declaration, inside namespace osg.
 *  Example: OSG_type_name(osg::Node) */
#define OSG_type_name( T )                                     \
    template<>                                                 \
    inline constexpr const char* type_name<T>() noexcept       \
    {                                                          \
        return #T;                                             \
    }                                                          \
    template<>                                                 \
    inline constexpr const char* type_name<const T>() noexcept \
    {                                                          \
        return "const " #T;                                    \
    }
