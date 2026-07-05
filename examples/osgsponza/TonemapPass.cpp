/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#include "FullscreenQuad.hpp"
#include "SponzaFrameContext.hpp"
#include "SponzaOptions.hpp"
#include "SponzaTargets.hpp"
#include "TonemapPass.hpp"

#include <osg/GL>
#include <osg/nodes/Camera.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/state/StateSet.hpp>
#include <osg/state/Uniform.hpp>

namespace
{

    constexpr char tonemapFragmentShader[] = R"glsl(
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

vec3 skyRayWorld()
{
    vec2 ndc = vec2(vUV.x * 2.0 - 1.0, 1.0 - vUV.y * 2.0);
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

void main()
{
    bool skyPixel = uSkyEnabled && texture(uDepth, vUV).r >= 1.0;
    vec3 hdr = texture(uHdr, vUV).rgb;
    if (skyPixel) {
        hdr = sampleSky(skyRayWorld(), 1.0) * uSkyGain;
    } else {
        vec3 indirect = texture(uIndirect, vUV).rgb;
        float ao = texture(uAo, vUV).r;
        ao = mix(1.0, ao, uAoStrength);
        hdr += ao * indirect;
    }

    vec3 c = hdr * uWhiteBalance * uExposure;
    c = applyTonemap(c);
    c = clamp(c, 0.0, 1.0);
    c = pow(c, vec3(1.0 / 2.2));
    o = vec4(c, 1.0);
}
)glsl";

}

namespace sponza
{

    namespace
    {

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

    }

    osg::ref_ptr<osg::Geode>
    createTonemapQuad( const SponzaOptions&      options,
                       SponzaTargets&            targets,
                       const SponzaFrameContext& frame )
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
        stateSet->setTextureAttributeAndModes( 4,
                                               targets.indirectColor.get(),
                                               osg::StateAttribute::ON );
        stateSet->addUniform( new osg::Uniform( "uHdr", 0 ) );
        stateSet->addUniform( new osg::Uniform( "uIndirect", 4 ) );
        stateSet->addUniform( new osg::Uniform( "uAo", 1 ) );
        stateSet->addUniform( new osg::Uniform( "uDepth", 2 ) );
        stateSet->addUniform( new osg::Uniform( "uEnvMap", 3 ) );
        stateSet->addUniform( new osg::Uniform( "uExposure", options.exposure ) );
        stateSet->addUniform( new osg::Uniform( "uAoStrength",
                                                options.ssaoEnabled ? options.aoStrength
                                                                    : 0.0F ) );
        stateSet->addUniform( new osg::Uniform( "uEnvRotation", frame.envRotation ) );
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

        return geode;
    }

    osg::ref_ptr<osg::Camera>
    createTonemapCamera( const SponzaOptions&      options,
                         SponzaTargets&            targets,
                         const SponzaFrameContext& frame )
    {
        osg::ref_ptr<osg::Geode>  quad = createTonemapQuad( options, targets, frame );
        osg::ref_ptr<osg::Camera> post = new osg::Camera;
        post->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
        post->setRenderOrder( osg::Camera::POST_RENDER );
        post->setProjectionMatrix( osg::mat4() );
        post->setViewMatrix( osg::mat4() );
        post->setClearMask( 0 );
        post->setAllowEventFocus( false );
        post->setViewport( 0, 0, renderWidth, renderHeight );
        post->addChild( quad.get() );
        return post;
    }

}
