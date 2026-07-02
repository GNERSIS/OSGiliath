/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Library version query for osgVolume.
 * Returns version strings matching the core osg library.
 */
#include <osgVolume/Version.hpp>

#include <osg/Version>

extern "C"
{

    const char*
    osgVolumeGetVersion()
    {
        return osgGetVersion();
    }

    const char*
    osgVolumeGetLibraryName()
    {
        return "OpenSceneGraph Volume Library";
    }
}
