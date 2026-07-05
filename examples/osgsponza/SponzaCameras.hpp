/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#pragma once

#include <array>
#include <osg/maths/mat4.hpp>
#include <osg/maths/MatrixTemplate.hpp>
#include <osg/maths/vec3.hpp>

namespace sponza
{

    struct CameraPreset
    {
            const char* name;
            osg::dvec3  eye;
            osg::dvec3  forward;
            osg::dvec3  up;
            double      fovDeg;
    };

    struct CameraSettings
    {
            osg::dvec3 eye;
            osg::dvec3 center;
            osg::dvec3 up;
            double     fovDeg = 0.0;
    };

    extern const std::array<CameraPreset, 6> cameraPresets;

    CameraSettings
    makeCameraSettings( const CameraPreset& preset );

    osg::dmat4
    makeViewMatrix( const CameraSettings& camera );

    osg::dmat4
    makeProjectionMatrix( const CameraSettings& camera );

}
