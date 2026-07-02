/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * DLL export/import macros for the osgWidget library.
 * Defines OSGWIDGET_EXPORT for shared library symbol visibility.
 */
// Code by: Jeremy Moles (cubicool) 2007-2008

#pragma once

#include <osg/Config>

#if defined( _MSC_VER ) && defined( OSG_DISABLE_MSVC_WARNINGS )
    #pragma warning( disable : 4'121 )
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
        #define OSGWIDGET_EXPORT
    #elif defined( OSGWIDGET_LIBRARY )
        #define OSGWIDGET_EXPORT __declspec( dllexport )
    #else
        #define OSGWIDGET_EXPORT __declspec( dllimport )
    #endif
#else
    #define OSGWIDGET_EXPORT
#endif

/**

\namespace osgWidget

The osgWidget library is a NodeKit that extends the core scene graph to support a 2D (and
eventually 3D) GUI widget set.
*/
