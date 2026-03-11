/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Library version query for osgTerrain.
 * Returns version strings matching the core osg library.
 */
#include <osgTerrain/Version>

#include <osg/Version>

extern "C"
{

    const char*
    osgTerrainGetVersion()
    {
        return osgGetVersion();
    }

    const char*
    osgTerrainGetLibraryName()
    {
        return "OpenSceneGraph Terrain Library";
    }
}
