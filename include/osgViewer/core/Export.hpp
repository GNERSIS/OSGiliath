/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * DLL export/import macros for the osgViewer library.
 * Defines OSGVIEWER_EXPORT for shared library symbol visibility.
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
        #define OSGVIEWER_EXPORT
    #elif defined( OSGVIEWER_LIBRARY )
        #define OSGVIEWER_EXPORT __declspec( dllexport )
    #else
        #define OSGVIEWER_EXPORT __declspec( dllimport )
    #endif
#else
    #define OSGVIEWER_EXPORT
#endif

/**

\namespace osgViewer

The osgViewer library provides high level viewer functionality designed to make it easier
to write a range of different types of viewers, from viewers embedded in existing windows
via SimpleViewer, through to highly scalable and flexible Viewer and Composite classes. A
set of event handlers add functionality to these viewers so that you can rapidly compose
the viewer functionality tailored to your needs. Finally the viewer classes can be
adapted to work with a range of different window toolkit API's via GraphicsWindow
implementations, with native Win32, X11 and Carbon implementations on Windows, Unices and
OSX respectively, and other window toolkits such as WxWidgets, Qt etc.
*/
