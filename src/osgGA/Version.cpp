/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Library version query for osgGA.
 * Returns version strings matching the core osg library.
 */
#include <osgGA/Version>

#include <osg/Version>

extern "C"
{

    const char*
    osgGAGetVersion()
    {
        return osgGetVersion();
    }

    const char*
    osgGAGetLibraryName()
    {
        return "OpenSceneGraph GA (Gui Adapter) Library";
    }
}
