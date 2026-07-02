/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * DLL export/import macros for the osgGA library.
 * Defines OSGGA_EXPORT for shared library symbol visibility.
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
        #define OSGGA_EXPORT
    #elif defined( OSGGA_LIBRARY )
        #define OSGGA_EXPORT __declspec( dllexport )
    #else
        #define OSGGA_EXPORT __declspec( dllimport )
    #endif
#else
    #define OSGGA_EXPORT
#endif

/**

\namespace osgGA

The 'GA' in osgGA stands for 'GUI Abstraction'; the osgGA namespace provides facilities
to help developers write the glue to allow the osg to work with varying window systems.

As a cross-platform, window system-agnostic class library, the OpenSceneGraph
has no direct ties to any given windowing environment. Viewers, however, must at
some level interact with a window system - where Window system may refer to a windowing
API, e.g. GLUT, Qt, FLTK, MFC, ...

There is much commonality in the implementation of Viewers for varying windowing
environments. E.g. most Viewers will update a Camera position in response to a mouse
event, and may request that a timer be started as a result of a model being 'spun'.

The purpose of the osgGA namespace is to centralise the common areas of this
functionality. The viewer writer needs then only write a GUIEventAdapter, a
GUIActionAdapter, and assemble a collection of GUIEventHandlers
as appropriate for the viewer.

Events from the windowing environment are adpated, and then fed into the
GUIEventHandlers. The GUIEventHandlers analyse and take action, and make requests of the
windowing environment via the GUIActionAdapter. The viewer writer should then honour
these requests, translating them into calls to the windowing API.

*/
