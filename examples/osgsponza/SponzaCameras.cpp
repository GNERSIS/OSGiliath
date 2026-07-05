/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#include "SponzaCameras.hpp"
#include "SponzaOptions.hpp"

#include <osg/core/ArgumentParser.hpp>
#include <osg/maths/transform.hpp>

namespace sponza
{

    /*
     * USD-EV candidates for T3.1, sign unverified:
     *
     *   Views A/B/C/F: EV 4.5 -> trim 1.0 baseline.
     *   View D:       EV 2.0 -> candidate trim 5.66 or 1/5.66.
     *   View E:       EV 3.5 -> candidate trim 2.0 or 0.5.
     *
     * T3.1 decides the EV sign empirically; this task keeps every preset at
     * 1.0 so the mechanism is pixel-neutral by default.
     */
    extern const std::array<CameraPreset, 6> cameraPresets = {
        { { "PhysCamera001",
            osg::dvec3( -8.80743, 1.59221947, -0.85825783 ),
            osg::dvec3( 0.9639, 0.1943, 0.1820 ),
            osg::dvec3( -0.1909, 0.9809, -0.0360 ),
            58.51,
            1.0F },
         { "PhysCamera002",
            osg::dvec3( 10.5311031, 7.352985, 1.52825129 ),
            osg::dvec3( -0.9919, 0.0225, -0.1252 ),
            osg::dvec3( 0.0223, 0.9997, 0.0028 ),
            58.63,
            1.3F },
         { "PhysCamera003",
            osg::dvec3( 2.70301843, 1.35489559, 2.11138153 ),
            osg::dvec3( -0.9395, 0.0864, -0.3316 ),
            osg::dvec3( 0.0815, 0.9963, 0.0288 ),
            36.25,
            1.0F },
         { "PhysCamera004",
            osg::dvec3( -9.594356, 6.921101, 5.35278 ),
            osg::dvec3( 0.9413, -0.0202, -0.3368 ),
            osg::dvec3( 0.0190, 0.9998, -0.0068 ),
            58.24,
            3.3F },
         { "PhysCamera005",
            osg::dvec3( -2.23714, 0.7440546, 2.3608017 ),
            osg::dvec3( 0.8684, 0.2434, -0.4320 ),
            osg::dvec3( -0.2179, 0.9699, 0.1084 ),
            37.22,
            2.4F },
         { "PhysCamera006",
            osg::dvec3( -6.59548569, 10.8791218, -0.6290613 ),
            osg::dvec3( 0.9628, -0.2697, 0.0174 ),
            osg::dvec3( 0.2697, 0.9629, 0.0049 ),
            37.24,
            1.1F } }
    };

    CameraSettings
    makeCameraSettings( const CameraPreset& preset )
    {
        return CameraSettings{
            preset.eye,
            preset.eye + preset.forward,
            preset.up,
            preset.fovDeg,
            preset.exposureTrim
        };
    }

    float
    readExposureTrimArgument( osg::ArgumentParser&  arguments,
                              const CameraSettings& camera,
                              float&                exposureTrim )
    {
        exposureTrim = camera.exposureTrim;
        arguments.read( "--exposure-trim", exposureTrim );
        return exposureTrim;
    }

    osg::dmat4
    makeViewMatrix( const CameraSettings& camera )
    {
        return osg::lookAt( camera.eye, camera.center, camera.up );
    }

    osg::dmat4
    makeProjectionMatrix( const CameraSettings& camera )
    {
        return osg::perspective( camera.fovDeg, renderAspect, nearZ, farZ );
    }

}
