/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Library version query for osgManipulator.
 * Returns version strings matching the core osg library.
 */
#include <osgManipulator/Version>

#include <osg/Version>

extern "C"
{

    const char*
    osgManipulatorGetVersion()
    {
        return osgGetVersion();
    }

    const char*
    osgManipulatorGetLibraryName()
    {
        return "OpenSceneGraph Manipulator Library";
    }
}
