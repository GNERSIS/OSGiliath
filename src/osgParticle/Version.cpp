/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Library version query for osgParticle.
 * Returns version strings matching the core osg library.
 */
#include <osgParticle/Version>

#include <osg/Version>

extern "C"
{

    const char*
    osgParticleGetVersion()
    {
        return osgGetVersion();
    }

    const char*
    osgParticleGetLibraryName()
    {
        return "OpenSceneGraph Particle Library";
    }
}
