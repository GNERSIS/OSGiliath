/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * DLL export/import macros for the osgManipulator library.
 * Defines OSGMANIPULATOR_EXPORT for shared library symbol visibility.
 */
// osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#pragma once

#if defined( _MSC_VER ) ||       \
    defined( __CYGWIN__ ) ||     \
    defined( __MINGW32__ ) ||    \
    defined( __BCPLUSPLUS__ ) || \
    defined( __MWERKS__ )
    #if defined( OSG_LIBRARY_STATIC )
        #define OSGMANIPULATOR_EXPORT
    #elif defined( OSGMANIPULATOR_LIBRARY )
        #define OSGMANIPULATOR_EXPORT __declspec( dllexport )
    #else
        #define OSGMANIPULATOR_EXPORT __declspec( dllimport )
    #endif
#else
    #define OSGMANIPULATOR_EXPORT
#endif

#define META_OSGMANIPULATOR_Object( library, name )           \
    virtual osg::Object* cloneType() const                    \
    {                                                         \
        return new name();                                    \
    }                                                         \
    virtual bool isSameKindAs( const osg::Object* obj ) const \
    {                                                         \
        return dynamic_cast<const name*>( obj ) != NULL;      \
    }                                                         \
    virtual const char* libraryName() const                   \
    {                                                         \
        return #library;                                      \
    }                                                         \
    virtual const char* className() const                     \
    {                                                         \
        return #name;                                         \
    }

/**

\namespace osgManipulator

The osgManipulator library is a NodeKit that extends the core scene graph to support 3D
interactive manipulators.
*/
