/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * DLL export/import macros for the osgParticle library.
 * Defines OSGPARTICLE_EXPORT for shared library symbol visibility.
 */
// osgParticle - Copyright (C) 2002 Marco Jez

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
        #define OSGPARTICLE_EXPORT
    #elif defined( OSGPARTICLE_LIBRARY )
        #define OSGPARTICLE_EXPORT __declspec( dllexport )
    #else
        #define OSGPARTICLE_EXPORT __declspec( dllimport )
    #endif
#else
    #define OSGPARTICLE_EXPORT
#endif

/**

\namespace osgParticle

The osgParticle library is a NodeKit that extends the core scene graph to support
particle effects.
*/
