/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#pragma once

#include <array>
#include <osg/maths/mat4.hpp>
#include <osg/maths/MatrixTemplate.hpp>
#include <osg/maths/vec3.hpp>

namespace osg
{

    class ArgumentParser;

}

namespace sponza
{

    struct CameraPreset
    {
            const char* name;
            osg::dvec3  eye;
            osg::dvec3  forward;
            osg::dvec3  up;
            double      fovDeg;
            float       exposureTrim = 1.0F;
    };

    struct CameraSettings
    {
            osg::dvec3 eye;
            osg::dvec3 center;
            osg::dvec3 up;
            double     fovDeg       = 0.0;
            float      exposureTrim = 1.0F;
    };

    extern const std::array<CameraPreset, 6> cameraPresets;

    CameraSettings
    makeCameraSettings( const CameraPreset& preset );

    float
    readExposureTrimArgument( osg::ArgumentParser&  arguments,
                              const CameraSettings& camera,
                              float&                exposureTrim );

    osg::dmat4
    makeViewMatrix( const CameraSettings& camera );

    osg::dmat4
    makeProjectionMatrix( const CameraSettings& camera );

}
