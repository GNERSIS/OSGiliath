/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Library version query for osgFX.
 * Returns version strings matching the core osg library.
 */
#pragma once

#include <osgFX/Export.hpp>

extern "C"
{

    /**
     * osgFXGetVersion() returns the library version number.
     * Numbering convention : OpenSceneGraph-1.0 will return 1.0 from osgFXGetVersion.
     *
     * This C function can be also used to check for the existence of the OpenSceneGraph
     * library using autoconf and its m4 macro AC_CHECK_LIB.
     *
     * Here is the code to add to your configure.in:
     \verbatim
     #
     # Check for the OpenSceneGraph (OSG) FX library
     #
     AC_CHECK_LIB(osg, osgFXGetVersion, ,
        [AC_MSG_ERROR(OpenSceneGraph FX library not found. See
     http://www.openscenegraph.org)],)
     \endverbatim
    */
    extern OSGFX_EXPORT const char*
    osgFXGetVersion();

    /**
     * getLibraryName() returns the library name in human friendly form.
     */
    extern OSGFX_EXPORT const char*
    osgFXGetLibraryName();
}
