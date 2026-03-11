/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Library version query for osgViewer.
 * Returns version strings matching the core osg library.
 */
#pragma once

#include <osgViewer/core/Export.hpp>

extern "C"
{

    /**
     * osgViewerGetVersion() returns the library version number.
     * Numbering convention : OpenSceneGraph-1.0 will return 1.0 from
     osgViewerGetVersion.
     *
     * This C function can be also used to check for the existence of the OpenSceneGraph
     * library using autoconf and its m4 macro AC_CHECK_LIB.
     *
     * Here is the code to add to your configure.in:
     \verbatim
     #
     # Check for the OpenSceneGraph (OSG) Viewer library
     #
     AC_CHECK_LIB(osg, osgViewerGetVersion, ,
        [AC_MSG_ERROR(OpenSceneGraph Viewer library not found. See
     http://www.openscenegraph.org)],)
     \endverbatim
    */
    extern OSGVIEWER_EXPORT const char*
    osgViewerGetVersion();

    /**
     * getLibraryName_osgViewer() returns the library name in human friendly form.
     */
    extern OSGVIEWER_EXPORT const char*
    osgViewerGetLibraryName();
}
