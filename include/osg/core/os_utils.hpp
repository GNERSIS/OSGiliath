/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Operating system utility functions. Provides environment variable
 * access, temporary file paths, and platform detection.
 */
#pragma once

#include <osg/core/Export.hpp>

#ifdef __cplusplus
extern "C"
{
#endif

    /** Cross platform version of C system() function. */
    extern OSG_EXPORT int
    osg_system( const char* str );

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

    #include <string>

    #if defined( OSG_ENVVAR_SUPPORTED )
        #include <sstream>
        #include <stdlib.h>
    #endif

namespace osg
{

    inline unsigned int
    getClampedLength( const char*  str,
                      unsigned int maxNumChars = 4'096 )
    {
        unsigned int i = 0;
        while( i < maxNumChars && str[i] != 0 )
        {
            ++i;
        }
        return i;
    }

    inline std::string
    getEnvVar( const char* name )
    {
    #ifdef OSG_ENVVAR_SUPPORTED
        std::string value;
        const char* ptr = getenv( name );
        if( ptr )
        {
            value.assign( ptr, getClampedLength( ptr ) );
        }
        return value;
    #else
        OSG_UNUSED( name );
        return std::string();
    #endif
    }

    template<typename T>
    inline bool
    getEnvVar( const char* name,
               T&          value )
    {
    #ifdef OSG_ENVVAR_SUPPORTED
        const char* ptr = getenv( name );
        if( !ptr )
        {
            return false;
        }

        std::istringstream str( std::string( ptr, getClampedLength( ptr ) ) );
        str >> value;
        return !str.fail();
    #else
        OSG_UNUSED2( name, value );
        return false;
    #endif
    }

    template<>
    inline bool
    getEnvVar( const char*  name,
               std::string& value )
    {
    #ifdef OSG_ENVVAR_SUPPORTED
        const char* ptr = getenv( name );
        if( !ptr )
        {
            return false;
        }

        value.assign( ptr, getClampedLength( ptr ) );
        return true;
    #else
        OSG_UNUSED2( name, value );
        return false;
    #endif
    }

    template<typename T1,
             typename T2>
    inline bool
    getEnvVar( const char* name,
               T1&         value1,
               T2&         value2 )
    {
    #ifdef OSG_ENVVAR_SUPPORTED
        const char* ptr = getenv( name );
        if( !ptr )
        {
            return false;
        }

        std::istringstream str( std::string( ptr, getClampedLength( ptr ) ) );
        str >> value1 >> value2;
        return !str.fail();
    #else
        OSG_UNUSED3( name, value1, value2 );
        return false;
    #endif
    }

    template<typename T1,
             typename T2,
             typename T3>
    inline bool
    getEnvVar( const char* name,
               T1&         value1,
               T2&         value2,
               T3&         value3 )
    {
    #ifdef OSG_ENVVAR_SUPPORTED
        const char* ptr = getenv( name );
        if( !ptr )
        {
            return false;
        }

        std::istringstream str( std::string( ptr, getClampedLength( ptr ) ) );
        str >> value1 >> value2 >> value3;
        return !str.fail();
    #else
        OSG_UNUSED4( name, value1, value2, value3 );
        return false;
    #endif
    }

    template<typename T1,
             typename T2,
             typename T3,
             typename T4>
    inline bool
    getEnvVar( const char* name,
               T1&         value1,
               T2&         value2,
               T3&         value3,
               T4&         value4 )
    {
    #ifdef OSG_ENVVAR_SUPPORTED
        const char* ptr = getenv( name );
        if( !ptr )
        {
            return false;
        }

        std::istringstream str( std::string( ptr, getClampedLength( ptr ) ) );
        str >> value1 >> value2 >> value3 >> value4;
        return !str.fail();
    #else
        OSG_UNUSED5( name, value1, value2, value3, value4 );
        return false;
    #endif
    }

}

#endif
