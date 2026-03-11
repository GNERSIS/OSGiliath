/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Preprocessor definitions and configuration macros for the
 * osgPlugins library.
 */
#pragma once

/*
    Make sure either OSC_HOST_LITTLE_ENDIAN or OSC_HOST_BIG_ENDIAN is defined

    If you know a way to enhance the detection below for Linux and/or MacOSX
    please let me know! I've tried a few things which don't work.
*/

#if defined( OSC_HOST_LITTLE_ENDIAN ) || defined( OSC_HOST_BIG_ENDIAN )

// you can define one of the above symbols from the command line
// then you don't have to edit this file.

#elif defined( __WIN32__ ) || defined( _WIN32 ) || defined( WINCE )

// assume that __WIN32__ is only defined on little endian systems

    #define OSC_HOST_LITTLE_ENDIAN 1
    #undef OSC_HOST_BIG_ENDIAN

#else

    #if defined( __GLIBC__ ) || defined( __ANDROID__ ) || defined( __CYGWIN__ )
        #include <endian.h>
        #if ( __BYTE_ORDER == __LITTLE_ENDIAN )
            #ifndef __LITTLE_ENDIAN__
                #define __LITTLE_ENDIAN__
            #endif
        #elif ( __BYTE_ORDER == __BIG_ENDIAN )
            #ifndef __BIG_ENDIAN__
                #define __BIG_ENDIAN__
            #endif
        #else
            #error Unknown machine endianness detected.
        #endif
    #elif defined( __FreeBSD__ )
        #include <sys/endian.hpp>
        #if ( _BYTE_ORDER == _LITTLE_ENDIAN )
            #ifndef __LITTLE_ENDIAN__
                #define __LITTLE_ENDIAN__
            #endif
        #elif ( _BYTE_ORDER == _BIG_ENDIAN )
            #ifndef __BIG_ENDIAN__
                #define __BIG_ENDIAN__
            #endif
        #else
            #error Unknown machine endianness detected.
        #endif
    #endif

    #if defined( __LITTLE_ENDIAN__ )

        #define OSC_HOST_LITTLE_ENDIAN 1
        #undef OSC_HOST_BIG_ENDIAN

    #elif defined( __BIG_ENDIAN__ )

        #define OSC_HOST_BIG_ENDIAN 1
        #undef OSC_HOST_LITTLE_ENDIAN

    #else

        #error please edit OscHostEndianness.h to configure endianness

    #endif

#endif
