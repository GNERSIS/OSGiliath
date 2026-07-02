/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Library version query for osgManipulator.
 * Returns version strings matching the core osg library.
 */
#pragma once

#include <osgManipulator/Export.hpp>

extern "C"
{

    /**
     * osgManipulatorGetVersion() returns the library version number.
     * Numbering convention : OpenSceneGraph-1.0 will return 1.0 from
     osgManipulatorGetVersion.
     *
     * This C function can be also used to check for the existence of the OpenSceneGraph
     * library using autoconf and its m4 macro AC_CHECK_LIB.
     *
     * Here is the code to add to your configure.in:
     \verbatim
     #
     # Check for the OpenSceneGraph (OSG) Manipulator library
     #
     AC_CHECK_LIB(osg, osgManipulatorGetVersion, ,
        [AC_MSG_ERROR(OpenSceneGraph Manipulator library not found. See
     http://www.openscenegraph.org)],)
     \endverbatim
    */
    extern OSGMANIPULATOR_EXPORT const char*
    osgManipulatorGetVersion();

    /**
     * osgManipulatorGetLibraryName() returns the library name in human friendly form.
     */
    extern OSGMANIPULATOR_EXPORT const char*
    osgManipulatorGetLibraryName();
}
