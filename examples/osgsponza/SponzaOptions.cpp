/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#include "SponzaOptions.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <osg/core/ApplicationUsage.hpp>
#include <osg/core/ArgumentParser.hpp>
#include <osgDB/registry/Options.hpp>
#include <osgDB/registry/Registry.hpp>

namespace
{

    constexpr float t12EnvRotationDefault = 0.436332315F;

    void
    configureSponzaUsage( osg::ArgumentParser& arguments )
    {
        arguments.getApplicationUsage()->addCommandLineOption(
            "--ambient <level>",
            "Fallback ambient level; ignored when the HDRI environment loads."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--ambient-color <r> <g> <b>",
            "Fallback ambient color; ignored when the HDRI environment loads."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--shadow-cast-glass <on|off>",
            "Let named Sponza glass materials cast into the shadow map."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--shadow-taps <count>",
            "Raster shadow PCF samples per shaded fragment, 1..16; default 16."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--shadow-filter <hard|pcf>",
            "Raster shadow filter mode; default pcf."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--shadow-update-rate <frames>",
            "Update the raster shadow map every N frames; default 1."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--rt-shadows <on|off>",
            "Enable ray-traced shadows when the renderer supports them."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--rt-shadow-normal-offset <value>",
            "Normal offset for ray-traced shadow rays."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--rt-shadow-max-distance <value>",
            "Maximum ray-traced shadow distance; 0 uses the scene diagonal."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--rt-shadow-samples <count>",
            "Ray-traced sun shadow samples per shaded fragment, 1..8."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--rt-sun-angular-radius <radians>",
            "Angular radius for multi-sample ray-traced sun shadows."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--rt-shadow-debug <on|off>",
            "Show ray-traced shadow debug output."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--rt-raster-gate-min <value>",
            "Skip RT refinement at or below this raster shadow visibility."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--rt-raster-gate-max <value>",
            "Skip RT refinement at or above this raster shadow visibility."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--fxaa <on|off>",
            "Enable final-pass FXAA."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--fxaa-mode <inline|resolved>",
            "FXAA input path; inline preserves the legacy one-pass resolve."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--resolve-scale <value>",
            "Resolved FXAA LDR texture scale relative to output size; default 1."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--indirect-target <rgba16f|rg11b10f|off>",
            "Indirect-light MRT format; default rgba16f preserves the legacy path."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--normal-map <on|off>",
            "Enable material normal-map sampling and tangent basis work."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--direct-specular <on|off>",
            "Enable direct-light GGX specular evaluation."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--ssao-scale <value>",
            "SSAO render scale relative to the main render target, (0, 1]."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--ssao-samples <count>",
            "SSAO near-field samples per shaded fragment, 1..16."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--ssao-room-samples <count>",
            "SSAO room-scale samples per shaded fragment, 1..16."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--ssao-room <on|off>",
            "Enable room-scale SSAO."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--vis-bake <on|off>",
            "Enable load-time per-vertex hemisphere visibility bake."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--vis-bake-rays <count>",
            "Hemisphere rays per vertex for the visibility bake."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--vis-bake-strength <value>",
            "Blend strength for baked visibility on indirect light."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--vis-bake-power <value>",
            "Power curve for baked visibility on indirect light."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--vis-bake-refresh",
            "Ignore any existing visibility bake cache and rebuild it."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--vis-bent-strength <value>",
            "Blend strength for bent-normal irradiance lookup."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--radiance-bake <on|off>",
            "Enable first-bounce baked diffuse radiance."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--radiance-multibounce <on|off>",
            "Apply analytic multi-bounce closure to baked diffuse radiance."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--radiance-debug <on|off>",
            "Show baked irradiance directly in the PBR output."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--radiance-scale <value>",
            "Scale applied to first-bounce baked diffuse radiance."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--gpu-profile <on|off>",
            "Emit per-pass GPU timer-query averages for Sponza passes."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--bake-densify <on|off>",
            "Subdivide long triangles before the visibility/radiance bake."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--bake-densify-max-edge <value>",
            "World-space edge length target for bake densification."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--bake-densify-max-subdivisions <count>",
            "Maximum subdivisions per triangle edge for bake densification."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--render-scale <N>",
            "Internal supersampling scale; headless defaults to 2, windowed to 1."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--output-size <width> <height>",
            "Headless/window render output size; defaults to 1920 1080."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--benchmark-frames <count>",
            "Headless benchmark frames to time after warmup; 0 disables benchmarking."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--benchmark-warmup-frames <count>",
            "Untimed headless frames before benchmark timing."
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--run-max-frame-rate <fps>",
            "Windowed run-loop cap; 0 disables sleeping."
        );
    }

    bool
    readOnOffArgument( osg::ArgumentParser& arguments,
                       const char*          option,
                       bool&                value )
    {
        std::string mode = value ? "on" : "off";
        if( !arguments.read( option, mode ) )
        {
            return true;
        }

        if( mode == "on" )
        {
            value = true;
            return true;
        }
        if( mode == "off" )
        {
            value = false;
            return true;
        }

        std::cerr << option << " must be 'on' or 'off'" << std::endl;
        return false;
    }

    bool
    readTonemapArgument( osg::ArgumentParser& arguments,
                         sponza::TonemapMode& mode )
    {
        std::string value = "narkowicz";
        if( !arguments.read( "--tonemap", value ) )
        {
            return true;
        }

        if( value == "narkowicz" )
        {
            mode = sponza::TonemapMode::Narkowicz;
            return true;
        }
        if( value == "hill" )
        {
            mode = sponza::TonemapMode::Hill;
            return true;
        }
        if( value == "agx" )
        {
            mode = sponza::TonemapMode::Agx;
            return true;
        }

        std::cerr << "--tonemap must be 'narkowicz', 'hill', or 'agx'" << std::endl;
        return false;
    }

    bool
    readFxaaModeArgument( osg::ArgumentParser& arguments,
                          sponza::FxaaMode&    mode )
    {
        std::string value = "inline";
        if( !arguments.read( "--fxaa-mode", value ) )
        {
            return true;
        }

        if( value == "inline" )
        {
            mode = sponza::FxaaMode::Inline;
            return true;
        }
        if( value == "resolved" )
        {
            mode = sponza::FxaaMode::Resolved;
            return true;
        }

        std::cerr << "--fxaa-mode must be 'inline' or 'resolved'" << std::endl;
        return false;
    }

    bool
    readShadowFilterArgument( osg::ArgumentParser&  arguments,
                              sponza::ShadowFilter& filter )
    {
        std::string value = "pcf";
        if( !arguments.read( "--shadow-filter", value ) )
        {
            return true;
        }

        if( value == "hard" )
        {
            filter = sponza::ShadowFilter::Hard;
            return true;
        }
        if( value == "pcf" )
        {
            filter = sponza::ShadowFilter::Pcf;
            return true;
        }

        std::cerr << "--shadow-filter must be 'hard' or 'pcf'" << std::endl;
        return false;
    }

    bool
    readIndirectTargetArgument( osg::ArgumentParser&          arguments,
                                sponza::IndirectTargetFormat& format )
    {
        std::string value = "rgba16f";
        if( !arguments.read( "--indirect-target", value ) )
        {
            return true;
        }

        if( value == "rgba16f" )
        {
            format = sponza::IndirectTargetFormat::Rgba16f;
            return true;
        }
        if( value == "rg11b10f" )
        {
            format = sponza::IndirectTargetFormat::Rg11b10f;
            return true;
        }
        if( value == "off" )
        {
            format = sponza::IndirectTargetFormat::Off;
            return true;
        }

        std::cerr << "--indirect-target must be 'rgba16f', 'rg11b10f', or 'off'"
                  << std::endl;
        return false;
    }

    osg::vec3
    readColorArgument( osg::ArgumentParser& arguments,
                       const char*          option,
                       const osg::vec3&     fallback )
    {
        osg::vec3 color = fallback;
        float     red   = color.r;
        float     green = color.g;
        float     blue  = color.b;
        if( arguments.read( option, red, green, blue ) )
        {
            color.set( red, green, blue );
        }
        return color;
    }

    void
    readDVec3Argument( osg::ArgumentParser& arguments,
                       const char*          option,
                       osg::dvec3&          value )
    {
        double x = value.x;
        double y = value.y;
        double z = value.z;
        if( arguments.read( option, x, y, z ) )
        {
            value.set( x, y, z );
        }
    }

    void
    publishPluginOptions( const sponza::SponzaOptions& options )
    {
        osgDB::Options* registryOptions = osgDB::Registry::instance()->getOptions();
        if( registryOptions == nullptr )
        {
            registryOptions = new osgDB::Options;
            osgDB::Registry::instance()->setOptions( registryOptions );
        }

        registryOptions->setPluginStringData(
            "sponzaGlassReflection",
            std::to_string( options.glassReflection )
        );
    }

}

namespace sponza
{

    int
    renderTargetWidth( const SponzaOptions& options )
    {
        const int scale =
            options.renderScale > 0 ? options.renderScale : defaultRenderScale;
        return options.outputWidth * scale;
    }

    int
    renderTargetHeight( const SponzaOptions& options )
    {
        const int scale =
            options.renderScale > 0 ? options.renderScale : defaultRenderScale;
        return options.outputHeight * scale;
    }

    int
    ssaoTargetDimension( int   fullResolutionDimension,
                         float ssaoScale )
    {
        const int   dimension = std::max( fullResolutionDimension, 1 );
        const float scale     = ssaoScale > 0.0F ? ssaoScale : defaultSsaoScale;
        return std::max( static_cast<int>( std::ceil( static_cast<float>( dimension ) *
                                                      scale ) ),
                         1 );
    }

    int
    ssaoTargetWidth( const SponzaOptions& options )
    {
        return ssaoTargetDimension( renderTargetWidth( options ), options.ssaoScale );
    }

    int
    ssaoTargetHeight( const SponzaOptions& options )
    {
        return ssaoTargetDimension( renderTargetHeight( options ), options.ssaoScale );
    }

    int
    outputWidth( const SponzaOptions& options )
    {
        return options.outputWidth;
    }

    int
    outputHeight( const SponzaOptions& options )
    {
        return options.outputHeight;
    }

    bool
    indirectTargetEnabled( const SponzaOptions& options )
    {
        return options.indirectTargetFormat != IndirectTargetFormat::Off;
    }

    bool
    parseSponzaOptions( osg::ArgumentParser& arguments,
                        SponzaOptions&       options )
    {
        configureSponzaUsage( arguments );

        // sampleEnv maps u = atan(z, x) / 2pi + 0.5 + rotation / 2pi.
        // Positive rotation makes an env feature appear at a lower world
        // azimuth, so 35.8deg env sun -> 10.8deg analytic sun is +25deg.
        options.envRotation = t12EnvRotationDefault;
        options.tonemapMode = TonemapMode::Hill;

        options.headless    = arguments.read( "--headless", options.headlessOutput );

        arguments.read( "--camera-index", options.cameraIndex );
        if( options.cameraIndex <
            0 ||
            options.cameraIndex >= static_cast<int>( cameraPresets.size() ) )
        {
            std::cerr << "--camera-index must be in the range 0..5" << std::endl;
            return false;
        }

        options.camera = makeCameraSettings(
            cameraPresets[static_cast<size_t>( options.cameraIndex )]
        );

        arguments.read( "--sun-azimuth", options.sunAzimuthDeg );
        arguments.read( "--sun-elevation", options.sunElevationDeg );
        arguments.read( "--sun-intensity", options.sunIntensity );
        arguments.read( "--ambient", options.ambientLevel );
        arguments.read( "--exposure", options.exposure );
        arguments.read( "--ibl-intensity", options.iblIntensity );
        arguments.read( "--ibl-diffuse", options.iblDiffuse );
        arguments.read( "--ibl-specular", options.iblSpecular );
        arguments.read( "--ibl-clamp", options.iblClamp );
        arguments.read( "--env-rotation", options.envRotation );
        arguments.read( "--ao-radius", options.aoRadius );
        arguments.read( "--ao-strength", options.aoStrength );
        arguments.read( "--ao-power", options.aoPower );
        arguments.read( "--ao-bias", options.aoBias );
        arguments.read( "--ao-room-radius", options.aoRoomRadius );
        arguments.read( "--ao-room-strength", options.aoRoomStrength );
        arguments.read( "--ssao-scale", options.ssaoScale );
        arguments.read( "--ssao-samples", options.ssaoSamples );
        arguments.read( "--ssao-room-samples", options.ssaoRoomSamples );
        arguments.read( "--shadow-map-size", options.shadowMapSize );
        arguments.read( "--shadow-softness", options.shadowSoftness );
        arguments.read( "--shadow-bias", options.shadowBias );
        arguments.read( "--shadow-normal-offset", options.shadowNormalOffset );
        arguments.read( "--shadow-taps", options.shadowTaps );
        arguments.read( "--shadow-update-rate", options.shadowUpdateRate );
        arguments.read( "--rt-shadow-normal-offset", options.rtShadowNormalOffset );
        arguments.read( "--rt-shadow-max-distance", options.rtShadowMaxDistance );
        arguments.read( "--rt-shadow-samples", options.rtShadowSamples );
        arguments.read( "--rt-sun-angular-radius", options.rtSunAngularRadius );
        arguments.read( "--rt-raster-gate-min", options.rtRasterGateMin );
        arguments.read( "--rt-raster-gate-max", options.rtRasterGateMax );
        arguments.read( "--bounce-strength", options.bounceStrength );
        options.exposure *=
            readExposureTrimArgument( arguments, options.camera, options.exposureTrim );

        std::string ssaoMode = "on";
        if( arguments.read( "--ssao", ssaoMode ) )
        {
            if( ssaoMode == "off" )
            {
                options.ssaoEnabled = false;
            }
            else if( ssaoMode != "on" )
            {
                std::cerr << "--ssao must be 'on' or 'off'" << std::endl;
                return false;
            }
        }
        if( !readOnOffArgument( arguments, "--ssao-room", options.ssaoRoomEnabled ) ||
            !readOnOffArgument( arguments, "--shadow", options.shadowEnabled ) ||
            !readOnOffArgument( arguments,
                                "--shadow-cast-glass",
                                options.shadowCastGlass ) ||
            !readOnOffArgument( arguments, "--rt-shadows", options.rtShadowsEnabled ) ||
            !readOnOffArgument( arguments,
                                "--rt-shadow-debug",
                                options.rtShadowDebug ) ||
            !readOnOffArgument( arguments, "--fxaa", options.fxaaEnabled ) ||
            !readFxaaModeArgument( arguments, options.fxaaMode ) ||
            !readIndirectTargetArgument( arguments, options.indirectTargetFormat ) ||
            !readOnOffArgument( arguments, "--sky", options.skyEnabled ) ||
            !readOnOffArgument( arguments, "--ibl-sh", options.iblShEnabled ) ||
            !readOnOffArgument( arguments, "--normal-map", options.normalMapEnabled ) ||
            !readOnOffArgument( arguments,
                                "--direct-specular",
                                options.directSpecularEnabled ) ||
            !readShadowFilterArgument( arguments, options.shadowFilter ) ||
            !readTonemapArgument( arguments, options.tonemapMode ) )
        {
            return false;
        }

        readDVec3Argument( arguments, "--eye", options.camera.eye );
        readDVec3Argument( arguments, "--center", options.camera.center );
        readDVec3Argument( arguments, "--up", options.camera.up );
        arguments.read( "--fov", options.camera.fovDeg );

        options.sunColor =
            readColorArgument( arguments, "--sun-color", defaultSunColor );
        options.ambientColor =
            readColorArgument( arguments, "--ambient-color", defaultAmbientColor );
        options.whiteBalance =
            readColorArgument( arguments, "--white-balance", defaultWhiteBalance );
        options.bounceColor =
            readColorArgument( arguments, "--bounce-color", defaultBounceColor );
        arguments.read( "--sky-gain", options.skyGain );
        arguments.read( "--glass-reflection", options.glassReflection );
        if( !readOnOffArgument( arguments, "--vis-bake", options.visBakeEnabled ) )
        {
            return false;
        }
        arguments.read( "--vis-bake-rays", options.visBakeRays );
        arguments.read( "--vis-bake-strength", options.visBakeStrength );
        arguments.read( "--vis-bake-power", options.visBakePower );
        arguments.read( "--vis-bake-distance", options.visBakeDistance );
        arguments.read( "--vis-bent-strength", options.visBentStrength );
        if( !readOnOffArgument( arguments,
                                "--radiance-bake",
                                options.radianceBakeEnabled ) ||
            !readOnOffArgument( arguments,
                                "--radiance-multibounce",
                                options.radianceMultibounce ) ||
            !readOnOffArgument( arguments, "--radiance-debug", options.radianceDebug ) ||
            !readOnOffArgument( arguments,
                                "--gpu-profile",
                                options.gpuProfileEnabled ) ||
            !readOnOffArgument( arguments,
                                "--bake-densify",
                                options.bakeDensifyEnabled ) )
        {
            return false;
        }
        arguments.read( "--radiance-scale", options.radianceScale );
        arguments.read( "--bake-densify-max-edge", options.bakeDensifyMaxEdge );
        arguments.read( "--bake-densify-max-subdivisions",
                        options.bakeDensifyMaxSubdiv );
        arguments.read( "--resolve-scale", options.resolveScale );
        const bool explicitRenderScale =
            arguments.read( "--render-scale", options.renderScale );
        if( options.headless && !explicitRenderScale )
        {
            options.renderScale = defaultHeadlessRenderScale;
        }
        arguments.read( "--output-size", options.outputWidth, options.outputHeight );
        options.visBakeRefresh = arguments.read( "--vis-bake-refresh" );
        arguments.read( "--benchmark-frames", options.benchmarkFrames );
        arguments.read( "--benchmark-warmup-frames", options.benchmarkWarmupFrames );
        arguments.read( "--run-max-frame-rate", options.runMaxFrameRate );
        if( options.visBakeRays <= 0 )
        {
            std::cerr << "--vis-bake-rays must be greater than 0" << std::endl;
            return false;
        }
        if( options.radianceScale < 0.0F )
        {
            std::cerr << "--radiance-scale must be non-negative" << std::endl;
            return false;
        }
        if( options.bakeDensifyMaxEdge <= 0.0F )
        {
            std::cerr << "--bake-densify-max-edge must be greater than 0" << std::endl;
            return false;
        }
        if( options.bakeDensifyMaxSubdiv < 1 || options.bakeDensifyMaxSubdiv > 16 )
        {
            std::cerr << "--bake-densify-max-subdivisions must be in the range 1..16"
                      << std::endl;
            return false;
        }
        if( options.shadowTaps < 1 || options.shadowTaps > 16 )
        {
            std::cerr << "--shadow-taps must be in the range 1..16" << std::endl;
            return false;
        }
        if( options.shadowUpdateRate < 1 )
        {
            std::cerr << "--shadow-update-rate must be greater than 0" << std::endl;
            return false;
        }
        if( options.rtShadowNormalOffset < 0.0F )
        {
            std::cerr << "--rt-shadow-normal-offset must be non-negative" << std::endl;
            return false;
        }
        if( options.rtShadowMaxDistance < 0.0F )
        {
            std::cerr << "--rt-shadow-max-distance must be non-negative" << std::endl;
            return false;
        }
        if( options.rtShadowSamples < 1 || options.rtShadowSamples > 8 )
        {
            std::cerr << "--rt-shadow-samples must be in the range 1..8" << std::endl;
            return false;
        }
        if( options.ssaoScale <= 0.0F || options.ssaoScale > 1.0F )
        {
            std::cerr << "--ssao-scale must be in the range (0, 1]" << std::endl;
            return false;
        }
        if( options.ssaoSamples < 1 || options.ssaoSamples > defaultSsaoSamples )
        {
            std::cerr << "--ssao-samples must be in the range 1..16" << std::endl;
            return false;
        }
        if( options.ssaoRoomSamples <
            1 ||
            options.ssaoRoomSamples > defaultSsaoRoomSamples )
        {
            std::cerr << "--ssao-room-samples must be in the range 1..16" << std::endl;
            return false;
        }
        if( options.rtSunAngularRadius < 0.0F )
        {
            std::cerr << "--rt-sun-angular-radius must be non-negative" << std::endl;
            return false;
        }
        if( options.rtRasterGateMin < 0.0F || options.rtRasterGateMin > 1.0F )
        {
            std::cerr << "--rt-raster-gate-min must be in the range 0..1" << std::endl;
            return false;
        }
        if( options.rtRasterGateMax < 0.0F || options.rtRasterGateMax > 1.0F )
        {
            std::cerr << "--rt-raster-gate-max must be in the range 0..1" << std::endl;
            return false;
        }
        if( options.rtRasterGateMin > options.rtRasterGateMax )
        {
            std::cerr << "--rt-raster-gate-min must be <= --rt-raster-gate-max"
                      << std::endl;
            return false;
        }
        if( options.renderScale <= 0 )
        {
            std::cerr << "--render-scale must be greater than 0" << std::endl;
            return false;
        }
        if( !std::isfinite( options.resolveScale ) || options.resolveScale <= 0.0F )
        {
            std::cerr << "--resolve-scale must be finite and greater than 0"
                      << std::endl;
            return false;
        }
        if( options.outputWidth <= 0 || options.outputHeight <= 0 )
        {
            std::cerr << "--output-size dimensions must be greater than 0" << std::endl;
            return false;
        }
        if( options.renderScale >
            std::numeric_limits<int>::max() /
            options.outputWidth ||
            options.renderScale >
            std::numeric_limits<int>::max() /
            options.outputHeight )
        {
            std::cerr << "--render-scale is too large" << std::endl;
            return false;
        }
        if( static_cast<double>( options.outputWidth ) *
            static_cast<double>( options.resolveScale ) >
            static_cast<double>( std::numeric_limits<int>::max() ) ||
            static_cast<double>( options.outputHeight ) *
            static_cast<double>( options.resolveScale ) >
            static_cast<double>( std::numeric_limits<int>::max() ) )
        {
            std::cerr << "--resolve-scale is too large" << std::endl;
            return false;
        }
        if( options.benchmarkFrames < 0 )
        {
            std::cerr << "--benchmark-frames must be non-negative" << std::endl;
            return false;
        }
        if( options.benchmarkWarmupFrames < 0 )
        {
            std::cerr << "--benchmark-warmup-frames must be non-negative" << std::endl;
            return false;
        }
        if( options.runMaxFrameRate < 0.0 )
        {
            std::cerr << "--run-max-frame-rate must be non-negative" << std::endl;
            return false;
        }

        for( int i = 1; i < arguments.argc(); ++i )
        {
            if( !arguments.isOption( i ) )
            {
                options.modelPath = arguments[i];
                break;
            }
        }

        publishPluginOptions( options );

        return true;
    }

}
