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

    constexpr int          renderWidth                  = 1'920;
    constexpr int          renderHeight                 = 1'080;
    constexpr double       renderAspect                 = 16.0 / 9.0;
    constexpr double       nearZ                        = 0.1;
    constexpr double       farZ                         = 2'000.0;

    constexpr double       defaultSunAzimuthDeg         = 14.0;
    constexpr double       defaultSunElevationDeg       = 62.0;
    constexpr float        defaultSunIntensity          = 5.0F;
    constexpr float        defaultAmbientLevel          = 0.7F;
    constexpr float        defaultExposure              = 3.2F;
    constexpr float        defaultIblIntensity          = 1.0F;
    constexpr float        defaultIblDiffuse            = 1.0F;
    constexpr float        defaultIblSpecular           = 0.12F;
    constexpr float        defaultIblClamp              = 20.0F;
    constexpr float        defaultEnvRotation           = 0.0F;
    constexpr float        defaultAoRadius              = 0.5F;
    constexpr float        defaultAoStrength            = 0.8F;
    constexpr float        defaultAoPower               = 1.5F;
    constexpr float        defaultAoBias                = 0.025F;
    constexpr float        defaultAoRoomRadius          = 4.0F;
    constexpr float        defaultAoRoomStrength        = 0.7F;
    constexpr float        defaultSsaoScale             = 1.0F;
    constexpr int          defaultSsaoSamples           = 16;
    constexpr int          defaultSsaoRoomSamples       = 16;
    constexpr bool         defaultSsaoRoomEnabled       = true;
    constexpr int          defaultCameraIndex           = 0;
    constexpr unsigned int environmentTextureUnit       = 5U;

    constexpr bool         defaultShadowEnabled         = true;
    constexpr int          defaultShadowMapSize         = 4'096;
    constexpr float        defaultShadowSoftness        = 0.05F;
    constexpr float        defaultShadowBias            = 0.0008F;
    constexpr float        defaultShadowNormalOffset    = 0.02F;
    constexpr bool         defaultShadowCastGlass       = false;
    constexpr int          defaultShadowTaps            = 16;
    constexpr int          defaultShadowUpdateRate      = 1;
    constexpr bool         defaultRtShadowsEnabled      = false;
    constexpr float        defaultRtShadowNormalOffset  = defaultShadowNormalOffset;
    constexpr float        defaultRtShadowMaxDistance   = 0.0F;
    constexpr int          defaultRtShadowSamples       = 1;
    constexpr float        defaultRtSunAngularRadius    = 0.00465F;
    constexpr bool         defaultRtShadowDebug         = false;
    constexpr float        defaultRtRasterGateMin       = 0.49F;
    constexpr float        defaultRtRasterGateMax       = 0.51F;
    constexpr bool         defaultFxaaEnabled           = true;
    constexpr float        defaultResolveScale          = 1.0F;
    constexpr bool         defaultSkyEnabled            = true;
    constexpr float        defaultSkyGain               = 3.4F;
    constexpr bool         defaultIblShEnabled          = true;
    constexpr bool         defaultNormalMapEnabled      = true;
    constexpr bool         defaultDirectSpecularEnabled = true;
    constexpr float        defaultBounceStrength        = 0.45F;
    constexpr float        defaultExposureTrim          = 1.0F;
    constexpr float        defaultGlassReflection       = 1.0F;
    constexpr bool         defaultVisBakeEnabled        = true;
    constexpr int          defaultVisBakeRays           = 48;
    constexpr float        defaultVisBakeStrength       = 0.55F;
    constexpr float        defaultVisBakePower          = 1.0F;
    constexpr float        defaultVisBakeDistance       = 10.0F;
    constexpr float        defaultVisBentStrength       = 0.8F;
    constexpr bool         defaultRadianceBakeEnabled   = true;
    constexpr bool         defaultRadianceMultibounce   = true;
    constexpr bool         defaultRadianceDebug         = false;
    constexpr float        defaultRadianceScale         = 5.0F;
    constexpr bool         defaultGpuProfileEnabled     = false;
    constexpr bool         defaultBakeDensifyEnabled    = false;
    constexpr float        defaultBakeDensifyMaxEdge    = 0.5F;
    constexpr int          defaultBakeDensifyMaxSubdiv  = 6;
    constexpr int          defaultRenderScale           = 1;
    constexpr int          defaultHeadlessRenderScale   = 2;
    constexpr int          defaultBenchmarkFrames       = 0;
    constexpr int          defaultBenchmarkWarmupFrames = 60;
    constexpr double       defaultRunMaxFrameRate       = 60.0;

    inline const osg::vec3 defaultSunColor{ 1.0F, 0.98F, 0.95F };
    inline const osg::vec3 defaultAmbientColor{ 0.5F, 0.5F, 0.5F };
    inline const osg::vec3 defaultWhiteBalance{ 1.045F, 1.0F, 0.955F };
    inline const osg::vec3 defaultBounceColor{ 0.60F, 0.49F, 0.37F };

    enum class TonemapMode
    {
        Narkowicz,
        Hill,
        Agx
    };

    enum class FxaaMode
    {
        Inline,
        Resolved
    };

    enum class IndirectTargetFormat
    {
        Rgba16f,
        Rg11b10f,
        Off
    };

    enum class ShadowFilter
    {
        Hard,
        Pcf
    };

    constexpr IndirectTargetFormat defaultIndirectTargetFormat =
        IndirectTargetFormat::Rgba16f;

    struct SponzaOptions
    {
            std::string          headlessOutput;
            bool                 headless    = false;
            int                  cameraIndex = defaultCameraIndex;
            CameraSettings       camera;

            double               sunAzimuthDeg         = defaultSunAzimuthDeg;
            double               sunElevationDeg       = defaultSunElevationDeg;
            float                sunIntensity          = defaultSunIntensity;
            osg::vec3            sunColor              = defaultSunColor;
            float                ambientLevel          = defaultAmbientLevel;
            osg::vec3            ambientColor          = defaultAmbientColor;
            float                exposure              = defaultExposure;
            float                iblIntensity          = defaultIblIntensity;
            float                iblDiffuse            = defaultIblDiffuse;
            float                iblSpecular           = defaultIblSpecular;
            float                iblClamp              = defaultIblClamp;
            float                envRotation           = defaultEnvRotation;
            float                aoRadius              = defaultAoRadius;
            float                aoStrength            = defaultAoStrength;
            float                aoPower               = defaultAoPower;
            float                aoBias                = defaultAoBias;
            float                aoRoomRadius          = defaultAoRoomRadius;
            float                aoRoomStrength        = defaultAoRoomStrength;
            bool                 ssaoEnabled           = true;
            bool                 ssaoRoomEnabled       = defaultSsaoRoomEnabled;
            float                ssaoScale             = defaultSsaoScale;
            int                  ssaoSamples           = defaultSsaoSamples;
            int                  ssaoRoomSamples       = defaultSsaoRoomSamples;

            bool                 shadowEnabled         = defaultShadowEnabled;
            int                  shadowMapSize         = defaultShadowMapSize;
            float                shadowSoftness        = defaultShadowSoftness;
            float                shadowBias            = defaultShadowBias;
            float                shadowNormalOffset    = defaultShadowNormalOffset;
            bool                 shadowCastGlass       = defaultShadowCastGlass;
            int                  shadowTaps            = defaultShadowTaps;
            ShadowFilter         shadowFilter          = ShadowFilter::Pcf;
            int                  shadowUpdateRate      = defaultShadowUpdateRate;
            bool                 rtShadowsEnabled      = defaultRtShadowsEnabled;
            float                rtShadowNormalOffset  = defaultRtShadowNormalOffset;
            float                rtShadowMaxDistance   = defaultRtShadowMaxDistance;
            int                  rtShadowSamples       = defaultRtShadowSamples;
            float                rtSunAngularRadius    = defaultRtSunAngularRadius;
            bool                 rtShadowDebug         = defaultRtShadowDebug;
            float                rtRasterGateMin       = defaultRtRasterGateMin;
            float                rtRasterGateMax       = defaultRtRasterGateMax;
            bool                 fxaaEnabled           = defaultFxaaEnabled;
            FxaaMode             fxaaMode              = FxaaMode::Inline;
            float                resolveScale          = defaultResolveScale;
            bool                 skyEnabled            = defaultSkyEnabled;
            float                skyGain               = defaultSkyGain;
            TonemapMode          tonemapMode           = TonemapMode::Narkowicz;
            osg::vec3            whiteBalance          = defaultWhiteBalance;
            bool                 iblShEnabled          = defaultIblShEnabled;
            bool                 normalMapEnabled      = defaultNormalMapEnabled;
            bool                 directSpecularEnabled = defaultDirectSpecularEnabled;
            float                bounceStrength        = defaultBounceStrength;
            osg::vec3            bounceColor           = defaultBounceColor;
            float                exposureTrim          = defaultExposureTrim;
            float                glassReflection       = defaultGlassReflection;
            bool                 visBakeEnabled        = defaultVisBakeEnabled;
            int                  visBakeRays           = defaultVisBakeRays;
            float                visBakeStrength       = defaultVisBakeStrength;
            float                visBakePower          = defaultVisBakePower;
            float                visBakeDistance       = defaultVisBakeDistance;
            float                visBentStrength       = defaultVisBentStrength;
            bool                 radianceBakeEnabled   = defaultRadianceBakeEnabled;
            bool                 radianceMultibounce   = defaultRadianceMultibounce;
            bool                 radianceDebug         = defaultRadianceDebug;
            float                radianceScale         = defaultRadianceScale;
            bool                 gpuProfileEnabled     = defaultGpuProfileEnabled;
            bool                 bakeDensifyEnabled    = defaultBakeDensifyEnabled;
            float                bakeDensifyMaxEdge    = defaultBakeDensifyMaxEdge;
            int                  bakeDensifyMaxSubdiv  = defaultBakeDensifyMaxSubdiv;
            int                  renderScale           = defaultRenderScale;
            int                  outputWidth           = renderWidth;
            int                  outputHeight          = renderHeight;
            IndirectTargetFormat indirectTargetFormat  = defaultIndirectTargetFormat;
            bool                 visBakeRefresh        = false;
            int                  benchmarkFrames       = defaultBenchmarkFrames;
            int                  benchmarkWarmupFrames = defaultBenchmarkWarmupFrames;
            double               runMaxFrameRate       = defaultRunMaxFrameRate;

            std::string          modelPath             = "NewSponza_Main_glTF_003.gltf";
    };

    int
    renderTargetWidth( const SponzaOptions& options );

    int
    renderTargetHeight( const SponzaOptions& options );

    int
    ssaoTargetDimension( int   fullResolutionDimension,
                         float ssaoScale );

    int
    ssaoTargetWidth( const SponzaOptions& options );

    int
    ssaoTargetHeight( const SponzaOptions& options );

    int
    outputWidth( const SponzaOptions& options );

    int
    outputHeight( const SponzaOptions& options );

    bool
    indirectTargetEnabled( const SponzaOptions& options );

    bool
    parseSponzaOptions( osg::ArgumentParser& arguments,
                        SponzaOptions&       options );

}
