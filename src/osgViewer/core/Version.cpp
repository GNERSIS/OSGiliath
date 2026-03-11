/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Library version query for osgViewer.
 * Returns version strings matching the core osg library.
 */
#include <osgViewer/core/Version.hpp>

#include <osg/Version>

extern "C"
{

    const char*
    osgViewerGetVersion()
    {
        return osgGetVersion();
    }

    const char*
    osgViewerGetLibraryName()
    {
        return "OpenSceneGraph Viewer Library";
    }
}
