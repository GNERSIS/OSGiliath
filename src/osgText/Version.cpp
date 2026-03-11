/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Library version query for osgText.
 * Returns version strings matching the core osg library.
 */
#include <osgText/Version>

#include <osg/Version>

extern "C"
{

    const char*
    osgTextGetVersion()
    {
        return osgGetVersion();
    }

    const char*
    osgTextGetLibraryName()
    {
        return "OpenSceneGraph Text Library";
    }
}
