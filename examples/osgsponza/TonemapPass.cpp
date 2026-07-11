/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#include "FullscreenQuad.hpp"
#include "SponzaFrameContext.hpp"
#include "SponzaOptions.hpp"
#include "SponzaPassOrder.hpp"
#include "SponzaTargets.hpp"
#include "TonemapPass.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <osg/GL>
#include <osg/nodes/Camera.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/state/StateSet.hpp>
#include <osg/state/Uniform.hpp>
#include <osg/textures/Texture2D.hpp>

namespace
{

    constexpr char tonemapFragmentShader[]      = R"glsl(
#version 460 core

uniform sampler2D uHdr;
uniform sampler2D uIndirect;
uniform sampler2D uAo;
uniform sampler2D uDepth;
uniform sampler2D uEnvMap;
uniform float uExposure;
uniform float uAoStrength;
uniform float uEnvRotation;
uniform float uSkyGain;
uniform vec3 uWhiteBalance;
uniform mat4 uInvProj;
uniform mat3 uViewToWorldRot;
uniform int uTonemapMode;
uniform bool uSkyEnabled;
uniform bool uFxaaEnabled;
uniform bool uIndirectEnabled;
uniform int uRenderScale;

in vec2 vUV;
out vec4 o;

const float PI = 3.14159265358979323846;

vec3 acesFilmic(vec3 c)
{
    return (c * (2.51 * c + 0.03)) / (c * (2.43 * c + 0.59) + 0.14);
}

vec3 rrtAndOdtFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

vec3 acesHill(vec3 c)
{
    const mat3 acesInputMat = mat3(
        vec3(0.59719, 0.07600, 0.02840),
        vec3(0.35458, 0.90834, 0.13383),
        vec3(0.04823, 0.01566, 0.83777));
    const mat3 acesOutputMat = mat3(
        vec3(1.60475, -0.10208, -0.00327),
        vec3(-0.53108, 1.10813, -0.07276),
        vec3(-0.07367, -0.00605, 1.07602));

    c = acesInputMat * c;
    c = rrtAndOdtFit(c);
    c = acesOutputMat * c;
    return c;
}

vec3 agxDefaultContrastApprox(vec3 x)
{
    vec3 x2 = x * x;
    vec3 x4 = x2 * x2;
    return 15.5 * x4 * x2
           - 40.14 * x4 * x
           + 31.96 * x4
           - 6.868 * x2 * x
           + 0.4298 * x2
           + 0.1191 * x
           - 0.00232;
}

vec3 agx(vec3 c)
{
    const mat3 linearSrgbToLinearRec2020 = mat3(
        vec3(0.6274, 0.0691, 0.0164),
        vec3(0.3293, 0.9195, 0.0880),
        vec3(0.0433, 0.0113, 0.8956));
    const mat3 linearRec2020ToLinearSrgb = mat3(
        vec3(1.6605, -0.1246, -0.0182),
        vec3(-0.5876, 1.1329, -0.1006),
        vec3(-0.0728, -0.0083, 1.1187));
    const float minEv = -12.47393;
    const float maxEv = 4.026069;

    c = linearSrgbToLinearRec2020 * c;
    c = max(c, vec3(1e-10));
    c = log2(c);
    c = (c - minEv) / (maxEv - minEv);
    c = clamp(c, 0.0, 1.0);
    c = agxDefaultContrastApprox(c);
    c = linearRec2020ToLinearSrgb * c;
    return c;
}

vec3 applyTonemap(vec3 c)
{
    if (uTonemapMode == 1) {
        return acesHill(c);
    }
    if (uTonemapMode == 2) {
        return agx(c);
    }
    return acesFilmic(c);
}

vec3 skyRayWorld(vec2 uv)
{
    vec2 ndc = vec2(uv.x * 2.0 - 1.0, 1.0 - uv.y * 2.0);
    vec4 view = uInvProj * vec4(ndc, 1.0, 1.0);
    vec3 viewDir = normalize(view.xyz / view.w);
    vec3 worldDir = uViewToWorldRot * viewDir;
    return (length(worldDir) > 1e-6) ? normalize(worldDir) : vec3(0.0, 1.0, 0.0);
}

vec3 sampleSky(vec3 dirWorld, float lod)
{
    vec3 d = dirWorld;
    float len = length(d);
    d = (len > 1e-6) ? d / len : vec3(0.0, 1.0, 0.0);
    float lon = (abs(d.x) + abs(d.z) < 1e-5) ? 0.0 : atan(d.z, d.x);
    float u = lon / (2.0 * PI) + 0.5 + uEnvRotation / (2.0 * PI);
    float v = acos(clamp(d.y, -1.0, 1.0)) / PI;
    return textureLod(uEnvMap, vec2(u, v), lod).rgb;
}

float luma(vec3 c)
{
    return dot(c, vec3(0.299, 0.587, 0.114));
}

vec3 resolveSingleColor(vec2 uv)
{
    bool skyPixel = uSkyEnabled && texture(uDepth, uv).r >= 1.0;
    vec3 hdr = texture(uHdr, uv).rgb;
    if (skyPixel) {
        hdr = sampleSky(skyRayWorld(uv), 1.0) * uSkyGain;
    } else if (uIndirectEnabled) {
        vec3 indirect = texture(uIndirect, uv).rgb;
        float ao = texture(uAo, uv).r;
        ao = mix(1.0, ao, uAoStrength);
        hdr += ao * indirect;
    }

    vec3 c = hdr * uWhiteBalance * uExposure;
    c = applyTonemap(c);
    c = clamp(c, 0.0, 1.0);
    c = pow(c, vec3(1.0 / 2.2));
    return c;
}

vec3 resolveColor(vec2 uv)
{
    int scale = clamp(uRenderScale, 1, 4);
    if (scale <= 1) {
        return resolveSingleColor(uv);
    }

    vec2 sourceTexel = 1.0 / vec2(textureSize(uHdr, 0));
    vec3 sum = vec3(0.0);
    float count = 0.0;
    for (int y = 0; y < 4; ++y) {
        if (y >= scale) {
            break;
        }
        for (int x = 0; x < 4; ++x) {
            if (x >= scale) {
                break;
            }
            vec2 sampleOffset =
                (vec2(float(x), float(y)) + vec2(0.5)) / float(scale) -
                vec2(0.5);
            sum += resolveSingleColor(uv + sampleOffset * sourceTexel * float(scale));
            count += 1.0;
        }
    }
    return sum / max(count, 1.0);
}

vec3 resolveFxaa(vec2 uv)
{
    int scale = clamp(uRenderScale, 1, 4);
    vec2 texel = 1.0 / vec2(textureSize(uHdr, 0)) * float(scale);
    vec3 rgbM = resolveColor(uv);
    vec3 rgbNW = resolveColor(uv + texel * vec2(-1.0, -1.0));
    vec3 rgbNE = resolveColor(uv + texel * vec2( 1.0, -1.0));
    vec3 rgbSW = resolveColor(uv + texel * vec2(-1.0,  1.0));
    vec3 rgbSE = resolveColor(uv + texel * vec2( 1.0,  1.0));

    float lumaM = luma(rgbM);
    float lumaNW = luma(rgbNW);
    float lumaNE = luma(rgbNE);
    float lumaSW = luma(rgbSW);
    float lumaSE = luma(rgbSE);
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
    float range = lumaMax - lumaMin;

    if (range < max(0.0312, lumaMax * 0.125)) {
        return rgbM;
    }

    vec2 direction = vec2(
        -((lumaNW + lumaNE) - (lumaSW + lumaSE)),
         ((lumaNW + lumaSW) - (lumaNE + lumaSE)));
    float directionReduce = max(
        (lumaNW + lumaNE + lumaSW + lumaSE) * 0.03125,
        0.0078125);
    float inverseDirectionAdjustment =
        1.0 / (min(abs(direction.x), abs(direction.y)) + directionReduce);
    direction = clamp(direction * inverseDirectionAdjustment, vec2(-8.0), vec2(8.0))
        * texel;

    vec3 rgbA = 0.5 * (
        resolveColor(uv + direction * (1.0 / 3.0 - 0.5)) +
        resolveColor(uv + direction * (2.0 / 3.0 - 0.5)));
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        resolveColor(uv + direction * -0.5) +
        resolveColor(uv + direction * 0.5));
    float lumaB = luma(rgbB);
    if (lumaB < lumaMin || lumaB > lumaMax) {
        return rgbA;
    }
    return rgbB;
}

void main()
{
    vec3 c = uFxaaEnabled ? resolveFxaa(vUV) : resolveColor(vUV);
    o = vec4(c, 1.0);
}
)glsl";

    constexpr char resolvedFxaaFragmentShader[] = R"glsl(
#version 460 core

uniform sampler2D uResolvedColor;
uniform bool uFxaaEnabled;

in vec2 vUV;
out vec4 o;

float luma(vec3 c)
{
    return dot(c, vec3(0.299, 0.587, 0.114));
}

vec3 sampleResolvedColor(vec2 uv)
{
    return texture(uResolvedColor, uv).rgb;
}

vec3 resolveFxaa(vec2 uv)
{
    vec2 texel = 1.0 / vec2(textureSize(uResolvedColor, 0));
    vec3 rgbM = sampleResolvedColor(uv);
    vec3 rgbNW = sampleResolvedColor(uv + texel * vec2(-1.0, -1.0));
    vec3 rgbNE = sampleResolvedColor(uv + texel * vec2( 1.0, -1.0));
    vec3 rgbSW = sampleResolvedColor(uv + texel * vec2(-1.0,  1.0));
    vec3 rgbSE = sampleResolvedColor(uv + texel * vec2( 1.0,  1.0));

    float lumaM = luma(rgbM);
    float lumaNW = luma(rgbNW);
    float lumaNE = luma(rgbNE);
    float lumaSW = luma(rgbSW);
    float lumaSE = luma(rgbSE);
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
    float range = lumaMax - lumaMin;

    if (range < max(0.0312, lumaMax * 0.125)) {
        return rgbM;
    }

    vec2 direction = vec2(
        -((lumaNW + lumaNE) - (lumaSW + lumaSE)),
         ((lumaNW + lumaSW) - (lumaNE + lumaSE)));
    float directionReduce = max(
        (lumaNW + lumaNE + lumaSW + lumaSE) * 0.03125,
        0.0078125);
    float inverseDirectionAdjustment =
        1.0 / (min(abs(direction.x), abs(direction.y)) + directionReduce);
    direction = clamp(direction * inverseDirectionAdjustment, vec2(-8.0), vec2(8.0))
        * texel;

    vec3 rgbA = 0.5 * (
        sampleResolvedColor(uv + direction * (1.0 / 3.0 - 0.5)) +
        sampleResolvedColor(uv + direction * (2.0 / 3.0 - 0.5)));
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        sampleResolvedColor(uv + direction * -0.5) +
        sampleResolvedColor(uv + direction * 0.5));
    float lumaB = luma(rgbB);
    if (lumaB < lumaMin || lumaB > lumaMax) {
        return rgbA;
    }
    return rgbB;
}

void main()
{
    vec3 c = uFxaaEnabled ? resolveFxaa(vUV) : sampleResolvedColor(vUV);
    o = vec4(c, 1.0);
}
)glsl";

}

namespace sponza
{

    namespace
    {

        bool
        useResolvedFxaa( const SponzaOptions& options )
        {
            return options.fxaaEnabled && options.fxaaMode == FxaaMode::Resolved;
        }

        int
        scaledResolveDimension( int   outputDimension,
                                float resolveScale )
        {
            const double scaled =
                std::round( static_cast<double>( std::max( outputDimension, 1 ) ) *
                            static_cast<double>( resolveScale ) );
            const double clamped =
                std::clamp( scaled,
                            1.0,
                            static_cast<double>( std::numeric_limits<int>::max() ) );
            return static_cast<int>( clamped );
        }

        int
        resolveTargetWidth( const SponzaOptions& options )
        {
            return scaledResolveDimension( outputWidth( options ),
                                           options.resolveScale );
        }

        int
        resolveTargetHeight( const SponzaOptions& options )
        {
            return scaledResolveDimension( outputHeight( options ),
                                           options.resolveScale );
        }

        osg::ref_ptr<osg::Texture2D>
        createResolvedColorTexture( int width,
                                    int height )
        {
            osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
            texture->setTextureSize( width, height );
            texture->setInternalFormat( GL_RGBA8 );
            texture->setSourceFormat( GL_RGBA );
            texture->setSourceType( GL_UNSIGNED_BYTE );
            texture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
            texture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
            texture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE );
            texture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE );
            return texture;
        }

        int
        tonemapModeUniformValue( TonemapMode mode )
        {
            switch( mode )
            {
                case TonemapMode::Hill :
                    return 1;
                case TonemapMode::Agx :
                    return 2;
                case TonemapMode::Narkowicz :
                    return 0;
            }
            return 0;
        }

        osg::ref_ptr<osg::Geode>
        createTonemapQuadWithFxaa( const SponzaOptions&      options,
                                   SponzaTargets&            targets,
                                   const SponzaFrameContext& frame,
                                   bool                      fxaaEnabled )
        {
            osg::ref_ptr<osg::Geode> geode =
                makeFullscreenPassGeode( tonemapFragmentShader );

            osg::StateSet* stateSet = geode->getOrCreateStateSet();
            stateSet->setTextureAttributeAndModes( 0,
                                                   targets.hdrColor.get(),
                                                   osg::StateAttribute::ON );
            stateSet->setTextureAttributeAndModes( 1,
                                                   targets.aoTexture.get(),
                                                   osg::StateAttribute::ON );
            stateSet->setTextureAttributeAndModes( 2,
                                                   targets.sceneDepth.get(),
                                                   osg::StateAttribute::ON );
            if( frame.envTexture )
            {
                stateSet->setTextureAttributeAndModes( 3,
                                                       frame.envTexture.get(),
                                                       osg::StateAttribute::ON );
            }
            osg::Texture2D* indirectTexture = targets.indirectColor.valid()
                                                ? targets.indirectColor.get()
                                                : targets.hdrColor.get();
            stateSet->setTextureAttributeAndModes( 4,
                                                   indirectTexture,
                                                   osg::StateAttribute::ON );
            stateSet->addUniform( new osg::Uniform( "uHdr", 0 ) );
            stateSet->addUniform( new osg::Uniform( "uIndirect", 4 ) );
            stateSet->addUniform( new osg::Uniform( "uAo", 1 ) );
            stateSet->addUniform( new osg::Uniform( "uDepth", 2 ) );
            stateSet->addUniform( new osg::Uniform( "uEnvMap", 3 ) );
            stateSet->addUniform( new osg::Uniform( "uExposure", options.exposure ) );
            stateSet->addUniform(
                new osg::Uniform( "uAoStrength",
                                  options.ssaoEnabled ? options.aoStrength : 0.0F )
            );
            stateSet->addUniform( new osg::Uniform( "uEnvRotation",
                                                    frame.envRotation ) );
            stateSet->addUniform( new osg::Uniform( "uSkyGain", options.skyGain ) );
            stateSet->addUniform( new osg::Uniform( "uWhiteBalance",
                                                    options.whiteBalance ) );
            stateSet->addUniform( new osg::Uniform( "uInvProj",
                                                    osg::mat4( frame.invProj ) ) );
            stateSet->addUniform( new osg::Uniform( "uViewToWorldRot",
                                                    frame.viewToWorldRot ) );
            stateSet->addUniform(
                new osg::Uniform( "uTonemapMode",
                                  tonemapModeUniformValue( options.tonemapMode ) )
            );
            stateSet->addUniform( new osg::Uniform( "uSkyEnabled",
                                                    options.skyEnabled &&
                                                        frame.envTexture.valid() ) );
            stateSet->addUniform( new osg::Uniform( "uFxaaEnabled", fxaaEnabled ) );
            stateSet->addUniform( new osg::Uniform(
                "uIndirectEnabled",
                indirectTargetEnabled( options ) && targets.indirectColor.valid()
            ) );
            stateSet->addUniform( new osg::Uniform( "uRenderScale",
                                                    options.renderScale ) );

            return geode;
        }

        osg::ref_ptr<osg::Geode>
        createResolvedFxaaQuad( osg::Texture2D* resolvedColor,
                                bool            fxaaEnabled )
        {
            osg::ref_ptr<osg::Geode> geode =
                makeFullscreenPassGeode( resolvedFxaaFragmentShader );

            osg::StateSet* stateSet = geode->getOrCreateStateSet();
            stateSet->setTextureAttributeAndModes( 0,
                                                   resolvedColor,
                                                   osg::StateAttribute::ON );
            stateSet->addUniform( new osg::Uniform( "uResolvedColor", 0 ) );
            stateSet->addUniform( new osg::Uniform( "uFxaaEnabled", fxaaEnabled ) );
            return geode;
        }

        osg::ref_ptr<osg::Camera>
        createOutputCamera( const SponzaOptions& options,
                            osg::Geode*          quad )
        {
            osg::ref_ptr<osg::Camera> post = new osg::Camera;
            post->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
            post->setRenderOrder( osg::Camera::POST_RENDER );
            post->setProjectionMatrix( osg::mat4() );
            post->setViewMatrix( osg::mat4() );
            post->setClearMask( 0 );
            post->setAllowEventFocus( false );
            post->setViewport( 0, 0, outputWidth( options ), outputHeight( options ) );
            post->addChild( quad );
            return post;
        }

        osg::ref_ptr<osg::Camera>
        createResolveCamera( const SponzaOptions&      options,
                             SponzaTargets&            targets,
                             const SponzaFrameContext& frame,
                             osg::Texture2D*           resolvedColor )
        {
            osg::ref_ptr<osg::Geode> quad =
                createTonemapQuadWithFxaa( options, targets, frame, false );

            osg::ref_ptr<osg::Camera> resolve = new osg::Camera;
            resolve->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
            resolve->attach( osg::Camera::COLOR_BUFFER0, resolvedColor );
            resolve->setViewport( 0,
                                  0,
                                  resolveTargetWidth( options ),
                                  resolveTargetHeight( options ) );
            resolve->setRenderOrder( osg::Camera::PRE_RENDER, ssaoPassOrder + 1 );
            resolve->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
            resolve->setProjectionMatrix( osg::mat4() );
            resolve->setViewMatrix( osg::mat4() );
            resolve->setClearColor( osg::vec4( 0.0F, 0.0F, 0.0F, 1.0F ) );
            resolve->setClearMask( GL_COLOR_BUFFER_BIT );
            resolve->setAllowEventFocus( false );
            resolve->addChild( quad.get() );
            return resolve;
        }

    }

    osg::ref_ptr<osg::Geode>
    createTonemapQuad( const SponzaOptions&      options,
                       SponzaTargets&            targets,
                       const SponzaFrameContext& frame )
    {
        return createTonemapQuadWithFxaa( options, targets, frame, options.fxaaEnabled );
    }

    TonemapPassResult
    createTonemapPass( const SponzaOptions&      options,
                       SponzaTargets&            targets,
                       const SponzaFrameContext& frame )
    {
        TonemapPassResult result;
        result.resolvedFxaa = useResolvedFxaa( options );

        if( result.resolvedFxaa )
        {
            result.resolvedColor =
                createResolvedColorTexture( resolveTargetWidth( options ),
                                            resolveTargetHeight( options ) );
            result.resolveCamera = createResolveCamera( options,
                                                        targets,
                                                        frame,
                                                        result.resolvedColor.get() );
            osg::ref_ptr<osg::Geode> fxaaQuad =
                createResolvedFxaaQuad( result.resolvedColor.get(), true );
            result.outputCamera = createOutputCamera( options, fxaaQuad.get() );
            return result;
        }

        osg::ref_ptr<osg::Geode> quad = createTonemapQuad( options, targets, frame );
        result.outputCamera           = createOutputCamera( options, quad.get() );
        return result;
    }

    osg::ref_ptr<osg::Camera>
    createTonemapCamera( const SponzaOptions&      options,
                         SponzaTargets&            targets,
                         const SponzaFrameContext& frame )
    {
        return createTonemapPass( options, targets, frame ).outputCamera;
    }

}
