/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * DLL export/import macros for the osgShadow library.
 * Defines OSGSHADOW_EXPORT for shared library symbol visibility.
 */
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

#if defined( _MSC_VER ) ||    \
    defined( __CYGWIN__ ) ||  \
    defined( __MINGW32__ ) || \
    defined( __BCPLUSPLUS__ )
    #if defined( OSG_LIBRARY_STATIC )
        #define OSGSHADOW_EXPORT
    #elif defined( OSGSHADOW_LIBRARY )
        #define OSGSHADOW_EXPORT __declspec( dllexport )
    #else
        #define OSGSHADOW_EXPORT __declspec( dllimport )
    #endif
#else
    #define OSGSHADOW_EXPORT
#endif

/**

\namespace osgShadow

The osgShadow library is a NodeKit that extends the core scene graph to add support for a
range of shadow techniques.
*/
