/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Library version query for osgUtil.
 * Returns version strings matching the core osg library.
 */
#include <osgUtil/utils/Version.hpp>

#include <osg/Version>

extern "C"
{

    const char*
    osgUtilGetVersion()
    {
        return osgGetVersion();
    }

    const char*
    osgUtilGetLibraryName()
    {
        return "OpenSceneGraph Util (Utility) Library";
    }
}
