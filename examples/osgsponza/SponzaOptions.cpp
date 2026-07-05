/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#include "SponzaOptions.hpp"

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
            "--render-scale <N>",
            "Integer supersampling scale for headless render targets."
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
        return renderWidth * scale;
    }

    int
    renderTargetHeight( const SponzaOptions& options )
    {
        const int scale =
            options.renderScale > 0 ? options.renderScale : defaultRenderScale;
        return renderHeight * scale;
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
        arguments.read( "--shadow-map-size", options.shadowMapSize );
        arguments.read( "--shadow-softness", options.shadowSoftness );
        arguments.read( "--shadow-bias", options.shadowBias );
        arguments.read( "--shadow-normal-offset", options.shadowNormalOffset );
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
        if( !readOnOffArgument( arguments, "--shadow", options.shadowEnabled ) ||
            !readOnOffArgument( arguments,
                                "--shadow-cast-glass",
                                options.shadowCastGlass ) ||
            !readOnOffArgument( arguments, "--sky", options.skyEnabled ) ||
            !readOnOffArgument( arguments, "--ibl-sh", options.iblShEnabled ) ||
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
        arguments.read( "--render-scale", options.renderScale );
        options.visBakeRefresh = arguments.read( "--vis-bake-refresh" );
        if( options.visBakeRays <= 0 )
        {
            std::cerr << "--vis-bake-rays must be greater than 0" << std::endl;
            return false;
        }
        if( options.renderScale <= 0 )
        {
            std::cerr << "--render-scale must be greater than 0" << std::endl;
            return false;
        }
        if( options.renderScale >
            std::numeric_limits<int>::max() /
            renderWidth ||
            options.renderScale >
            std::numeric_limits<int>::max() /
            renderHeight )
        {
            std::cerr << "--render-scale is too large" << std::endl;
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
