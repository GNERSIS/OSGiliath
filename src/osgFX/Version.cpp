/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Library version query for osgFX.
 * Returns version strings matching the core osg library.
 */
#include <osgFX/Version.hpp>

#include <osg/Version>

extern "C"
{

    const char*
    osgFXGetVersion()
    {
        return osgGetVersion();
    }

    const char*
    osgFXGetLibraryName()
    {
        return "OpenSceneGraph FX (Special effects) Library";
    }
}
