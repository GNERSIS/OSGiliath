/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Library version query for osgShadow.
 * Returns version strings matching the core osg library.
 */
#include <osgShadow/Version.hpp>

#include <osg/Version>

extern "C"
{

    const char*
    osgShadowGetVersion()
    {
        return osgGetVersion();
    }

    const char*
    osgShadowGetLibraryName()
    {
        return "OpenSceneGraph Shadow Library";
    }
}
