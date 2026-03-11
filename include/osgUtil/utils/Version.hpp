/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Library version query for osgUtil.
 * Returns version strings matching the core osg library.
 */
#pragma once

#include <osgUtil/Export>

extern "C"
{

    /**
     * osgUtilGetVersion() returns the library version number.
     * Numbering convention : OpenSceneGraph-1.0 will return 1.0 from osgUtilGetVersion.
     *
     * This C function can be also used to check for the existence of the OpenSceneGraph
     * library using autoconf and its m4 macro AC_CHECK_LIB.
     *
     * Here is the code to add to your configure.in:
     \verbatim
     #
     # Check for the OpenSceneGraph (OSG) Util library
     #
     AC_CHECK_LIB(osg, osgUtilGetVersion, ,
        [AC_MSG_ERROR(OpenSceneGraph Util library not found. See
     http://www.openscenegraph.org)],)
     \endverbatim
    */
    extern OSGUTIL_EXPORT const char*
    osgUtilGetVersion();

    /**
     * osgUtilGetLibraryName() returns the library name in human friendly form.
     */
    extern OSGUTIL_EXPORT const char*
    osgUtilGetLibraryName();
}
