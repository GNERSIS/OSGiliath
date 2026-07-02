/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * DLL export/import macros for the osgSim library.
 * Defines OSGSIM_EXPORT for shared library symbol visibility.
 */
#pragma once

#include <osg/Config>

#if defined( _MSC_VER ) && defined( OSG_DISABLE_MSVC_WARNINGS )
    #pragma warning( disable : 4'244 )
    #pragma warning( disable : 4'251 )
    #pragma warning( disable : 4'275 )
    #pragma warning( disable : 4'786 )
    #pragma warning( disable : 4'290 )
    #pragma warning( disable : 4'305 )
    #pragma warning( disable : 4'996 )
#endif

#if defined( _MSC_VER ) ||       \
    defined( __CYGWIN__ ) ||     \
    defined( __MINGW32__ ) ||    \
    defined( __BCPLUSPLUS__ ) || \
    defined( __MWERKS__ )
    #if defined( OSG_LIBRARY_STATIC )
        #define OSGSIM_EXPORT
    #elif defined( OSGSIM_LIBRARY )
        #define OSGSIM_EXPORT __declspec( dllexport )
    #else
        #define OSGSIM_EXPORT __declspec( dllimport )
    #endif
#else
    #define OSGSIM_EXPORT
#endif

/* Define NULL pointer value */

#ifndef NULL
    #ifdef __cplusplus
        #define NULL 0
    #else
        #define NULL ( ( void* )0 )
    #endif
#endif

/**

\namespace osgSim

The osgSim library is a NodeKit that extends the core scene graph to support nodes and
drawables that specific to the visual simulation, such a navigational light point support
and OpenFlight style degrees of freedom transform.
*/
