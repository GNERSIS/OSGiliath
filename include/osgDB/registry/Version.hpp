/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Library version query for osgDB.
 * Returns version strings matching the core osg library.
 */
#pragma once

#include <osgDB/Export.hpp>

extern "C"
{

    /**
     * osgDBGetVersion() returns the library version number.
     * Numbering convention : OpenSceneGraph-1.0 will return 1.0 from osgDBGetVersion.
     *
     * This C function can be also used to check for the existence of the OpenSceneGraph
     * library using autoconf and its m4 macro AC_CHECK_LIB.
     *
     * Here is the code to add to your configure.in:
     \verbatim
     #
     # Check for the OpenSceneGraph (OSG) DB library
     #
     AC_CHECK_LIB(osg, osgDBGetVersion, ,
        [AC_MSG_ERROR(OpenSceneGraph DB library not found. See
     http://www.openscenegraph.org)],)
     \endverbatim
    */
    extern OSGDB_EXPORT const char*
    osgDBGetVersion();

    /**
     * getLibraryName() returns the library name in human friendly form.
     */
    extern OSGDB_EXPORT const char*
    osgDBGetLibraryName();
}
