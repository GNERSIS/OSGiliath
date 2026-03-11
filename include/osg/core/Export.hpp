/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * DLL export/import macros for the osg library.
 * Defines OSG_EXPORT for shared library symbol visibility.
 */
#pragma once

#include <osg/Config>

// disable VisualStudio warnings
#if defined( _MSC_VER ) && defined( OSG_DISABLE_MSVC_WARNINGS )
    #pragma warning( disable : 4'244 )
    #pragma warning( disable : 4'251 )
    #pragma warning( disable : 4'275 )
    #pragma warning( disable : 4'512 )
    #pragma warning( disable : 4'267 )
    #pragma warning( disable : 4'702 )
    #pragma warning( disable : 4'511 )
#endif

#if defined( _MSC_VER ) ||       \
    defined( __CYGWIN__ ) ||     \
    defined( __MINGW32__ ) ||    \
    defined( __BCPLUSPLUS__ ) || \
    defined( __MWERKS__ )
    #if defined( OSG_LIBRARY_STATIC )
        #define OSG_EXPORT
    #elif defined( OSG_LIBRARY )
        #define OSG_EXPORT __declspec( dllexport )
    #else
        #define OSG_EXPORT __declspec( dllimport )
    #endif
#else
    #define OSG_EXPORT
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

// helper macro's for quieten unused variable warnings
#define OSG_UNUSED( VAR ) ( void )( VAR )
#define OSG_UNUSED2( VAR1, VAR2 ) \
    ( void )( VAR1 );             \
    ( void )( VAR2 );
#define OSG_UNUSED3( VAR1, VAR2, VAR3 ) \
    ( void )( VAR1 );                   \
    ( void )( VAR2 );                   \
    ( void )( VAR2 );
#define OSG_UNUSED4( VAR1, VAR2, VAR3, VAR4 ) \
    ( void )( VAR1 );                         \
    ( void )( VAR2 );                         \
    ( void )( VAR3 );                         \
    ( void )( VAR4 );
#define OSG_UNUSED5( VAR1, VAR2, VAR3, VAR4, VAR5 ) \
    ( void )( VAR1 );                               \
    ( void )( VAR2 );                               \
    ( void )( VAR3 );                               \
    ( void )( VAR4 );                               \
    ( void )( VAR5 );

/**

\namespace osg

The core osg library provides the basic scene graph classes such as Nodes,
State and Drawables, and maths and general helper classes.
*/
