/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * DLL export/import macros for the osgAnimation library.
 * Defines OSGANIMATION_EXPORT for shared library symbol visibility.
 */
// The following symbol has a underscore suffix for compatibility.
#pragma once

#include <osg/Config>

#if defined( _MSC_VER ) && defined( OSG_DISABLE_MSVC_WARNINGS )
    #pragma warning( disable : 4'244 )
    #pragma warning( disable : 4'251 )
    #pragma warning( disable : 4'267 )
    #pragma warning( disable : 4'275 )
    #pragma warning( disable : 4'290 )
    #pragma warning( disable : 4'786 )
    #pragma warning( disable : 4'305 )
    #pragma warning( disable : 4'996 )
#endif

#if defined( _MSC_VER ) ||       \
    defined( __CYGWIN__ ) ||     \
    defined( __MINGW32__ ) ||    \
    defined( __BCPLUSPLUS__ ) || \
    defined( __MWERKS__ )
    #if defined( OSG_LIBRARY_STATIC )
        #define OSGANIMATION_EXPORT
    #elif defined( OSGANIMATION_LIBRARY )
        #define OSGANIMATION_EXPORT __declspec( dllexport )
    #else
        #define OSGANIMATION_EXPORT __declspec( dllimport )
    #endif
#else
    #define OSGANIMATION_EXPORT
#endif

// set up define for whether member templates are supported by VisualStudio compilers.
#ifdef _MSC_VER
    #if ( _MSC_VER >= 1'300 )
        #define __STL_MEMBER_TEMPLATES
    #endif
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

\namespace osgAnimation

The osgAnimation library provides general purpose utility classes for animation.

*/
