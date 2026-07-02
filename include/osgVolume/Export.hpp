/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * DLL export/import macros for the osgVolume library.
 * Defines OSGVOLUME_EXPORT for shared library symbol visibility.
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
        #define OSGVOLUME_EXPORT
    #elif defined( OSGVOLUME_LIBRARY )
        #define OSGVOLUME_EXPORT __declspec( dllexport )
    #else
        #define OSGVOLUME_EXPORT __declspec( dllimport )
    #endif
#else
    #define OSGVOLUME_EXPORT
#endif

/**

\namespace osgVolume

The osgVolume library is a NodeKit that extends the core scene graph to support volume
rendering.
*/
