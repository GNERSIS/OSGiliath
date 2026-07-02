/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Library version query for osgText.
 * Returns version strings matching the core osg library.
 */
#pragma once

#include <osgText/Export.hpp>

extern "C"
{

    /**
     * osgTextGetVersion() returns the library version number.
     * Numbering convention : OpenSceneGraph-1.0 will return 1.0 from osgTextGetVersion.
     *
     * This C function can be also used to check for the existence of the OpenSceneGraph
     * library using autoconf and its m4 macro AC_CHECK_LIB.
     *
     * Here is the code to add to your configure.in:
     \verbatim
     #
     # Check for the OpenSceneGraph (OSG) Text library
     #
     AC_CHECK_LIB(osg, osgTextGetVersion, ,
        [AC_MSG_ERROR(OpenSceneGraph Text library not found. See
     http://www.openscenegraph.org)],)
     \endverbatim
    */
    extern OSGTEXT_EXPORT const char*
    osgTextGetVersion();

    /**
     * osgTextGetLibraryName() returns the library name in human friendly form.
     */
    extern OSGTEXT_EXPORT const char*
    osgTextGetLibraryName();
}
