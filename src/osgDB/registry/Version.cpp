/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Library version query for osgDB.
 * Returns version strings matching the core osg library.
 */
#include <osgDB/registry/Version.hpp>

#include <osg/Version>

extern "C"
{

    const char*
    osgDBGetVersion()
    {
        return osgGetVersion();
    }

    const char*
    osgDBGetLibraryName()
    {
        return "OpenSceneGraph DB (Data Base) Library";
    }
}
