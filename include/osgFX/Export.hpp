/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * DLL export/import macros for the osgFX library.
 * Defines OSGFX_EXPORT for shared library symbol visibility.
 */
// osgFX - Copyright (C) 2003 Marco Jez

#pragma once

#if defined( _MSC_VER ) ||       \
    defined( __CYGWIN__ ) ||     \
    defined( __MINGW32__ ) ||    \
    defined( __BCPLUSPLUS__ ) || \
    defined( __MWERKS__ )
    #if defined( OSG_LIBRARY_STATIC )
        #define OSGFX_EXPORT
    #elif defined( OSGFX_LIBRARY )
        #define OSGFX_EXPORT __declspec( dllexport )
    #else
        #define OSGFX_EXPORT __declspec( dllimport )
    #endif
#else
    #define OSGFX_EXPORT
#endif

/**

\namespace osgFX

The osgFX library is a NodeKit that extends the core scene graph to provide a special
effects framework. osgFX's framework allows multiple rendering techniques to be provide
for each effect, thereby provide the use appropriate rendering techniques for each
different class of graphics hardware, i.e. support for both modern programmable graphics
hardware and still have standard OpenGL 1.1 support as a fallback.
*/
