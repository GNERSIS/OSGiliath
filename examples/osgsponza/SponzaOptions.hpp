/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#pragma once

#include "SponzaCameras.hpp"

#include <osg/maths/vec3.hpp>
#include <string>

namespace osg
{

    class ArgumentParser;

}

namespace sponza
{

    constexpr int          renderWidth               = 1'920;
    constexpr int          renderHeight              = 1'080;
    constexpr double       renderAspect              = 16.0 / 9.0;
    constexpr double       nearZ                     = 0.1;
    constexpr double       farZ                      = 2'000.0;

    constexpr double       defaultSunAzimuthDeg      = 10.8;
    constexpr double       defaultSunElevationDeg    = 55.9;
    constexpr float        defaultSunIntensity       = 3.5F;
    constexpr float        defaultAmbientLevel       = 0.7F;
    constexpr float        defaultExposure           = 1.3F;
    constexpr float        defaultIblIntensity       = 1.0F;
    constexpr float        defaultIblDiffuse         = 0.5F;
    constexpr float        defaultIblSpecular        = 0.12F;
    constexpr float        defaultIblClamp           = 20.0F;
    constexpr float        defaultEnvRotation        = 0.0F;
    constexpr float        defaultAoRadius           = 0.5F;
    constexpr float        defaultAoStrength         = 0.8F;
    constexpr float        defaultAoPower            = 1.5F;
    constexpr float        defaultAoBias             = 0.025F;
    constexpr int          defaultCameraIndex        = 0;
    constexpr unsigned int environmentTextureUnit    = 5U;

    constexpr bool         defaultShadowEnabled      = true;
    constexpr int          defaultShadowMapSize      = 4'096;
    constexpr float        defaultShadowSoftness     = 0.05F;
    constexpr float        defaultShadowBias         = 0.0008F;
    constexpr float        defaultShadowNormalOffset = 0.02F;
    constexpr bool         defaultSkyEnabled         = true;
    constexpr bool         defaultIblShEnabled       = true;
    constexpr float        defaultBounceStrength     = 0.25F;
    constexpr float        defaultExposureTrim       = 1.0F;

    inline const osg::vec3 defaultSunColor{ 1.0F, 0.98F, 0.95F };
    inline const osg::vec3 defaultAmbientColor{ 0.5F, 0.6F, 0.75F };
    inline const osg::vec3 defaultWhiteBalance{ 1.0F, 1.0F, 1.0F };
    inline const osg::vec3 defaultBounceColor{ 0.62F, 0.48F, 0.35F };

    enum class TonemapMode
    {
        Narkowicz,
        Hill,
        Agx
    };

    struct SponzaOptions
    {
            std::string    headlessOutput;
            bool           headless    = false;
            int            cameraIndex = defaultCameraIndex;
            CameraSettings camera;

            double         sunAzimuthDeg      = defaultSunAzimuthDeg;
            double         sunElevationDeg    = defaultSunElevationDeg;
            float          sunIntensity       = defaultSunIntensity;
            osg::vec3      sunColor           = defaultSunColor;
            float          ambientLevel       = defaultAmbientLevel;
            osg::vec3      ambientColor       = defaultAmbientColor;
            float          exposure           = defaultExposure;
            float          iblIntensity       = defaultIblIntensity;
            float          iblDiffuse         = defaultIblDiffuse;
            float          iblSpecular        = defaultIblSpecular;
            float          iblClamp           = defaultIblClamp;
            float          envRotation        = defaultEnvRotation;
            float          aoRadius           = defaultAoRadius;
            float          aoStrength         = defaultAoStrength;
            float          aoPower            = defaultAoPower;
            float          aoBias             = defaultAoBias;
            bool           ssaoEnabled        = true;

            bool           shadowEnabled      = defaultShadowEnabled;
            int            shadowMapSize      = defaultShadowMapSize;
            float          shadowSoftness     = defaultShadowSoftness;
            float          shadowBias         = defaultShadowBias;
            float          shadowNormalOffset = defaultShadowNormalOffset;
            bool           skyEnabled         = defaultSkyEnabled;
            TonemapMode    tonemapMode        = TonemapMode::Narkowicz;
            osg::vec3      whiteBalance       = defaultWhiteBalance;
            bool           iblShEnabled       = defaultIblShEnabled;
            float          bounceStrength     = defaultBounceStrength;
            osg::vec3      bounceColor        = defaultBounceColor;
            float          exposureTrim       = defaultExposureTrim;

            std::string    modelPath          = "NewSponza_Main_glTF_003.gltf";
    };

    bool
    parseSponzaOptions( osg::ArgumentParser& arguments,
                        SponzaOptions&       options );

}
