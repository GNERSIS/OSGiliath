/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Library version query for osgSim.
 * Returns version strings matching the core osg library.
 */
#include <osgSim/Version.hpp>

#include <osg/Version>

extern "C"
{

    const char*
    osgSimGetVersion()
    {
        return osgGetVersion();
    }

    const char*
    osgSimGetLibraryName()
    {
        return "OpenSceneGraph Sim (Visual Simulation) Library";
    }
}
