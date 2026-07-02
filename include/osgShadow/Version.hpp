/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Library version query for osgShadow.
 * Returns version strings matching the core osg library.
 */
#pragma once

#include <osgShadow/Export.hpp>

extern "C"
{

    /**
     * osgShadowGetVersion() returns the library version number.
     * Numbering convention : OpenSceneGraph-1.0 will return 1.0 from
     osgShadowGetVersion.
     *
     * This C function can be also used to check for the existence of the OpenSceneGraph
     * library using autoconf and its m4 macro AC_CHECK_LIB.
     *
     * Here is the code to add to your configure.in:
     \verbatim
     #
     # Check for the OpenSceneGraph (OSG) Shadow library
     #
     AC_CHECK_LIB(osg, osgShadowGetVersion, ,
        [AC_MSG_ERROR(OpenSceneGraph Shadow library not found. See
     http://www.openscenegraph.org)],)
     \endverbatim
    */
    extern OSGSHADOW_EXPORT const char*
    osgShadowGetVersion();

    /**
     * osgShadowGetLibraryName() returns the library name in human friendly form.
     */
    extern OSGSHADOW_EXPORT const char*
    osgShadowGetLibraryName();
}
