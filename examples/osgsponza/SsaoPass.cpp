/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#include "FullscreenQuad.hpp"
#include "SponzaFrameContext.hpp"
#include "SponzaOptions.hpp"
#include "SponzaPassOrder.hpp"
#include "SponzaTargets.hpp"
#include "SsaoPass.hpp"

#include <osg/GL>
#include <osg/nodes/Camera.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/state/StateSet.hpp>
#include <osg/state/Uniform.hpp>

namespace
{

    constexpr char ssaoFragmentShader[] = R"glsl(
#version 460 core

uniform sampler2D uDepth;
uniform mat4 uProj;
uniform mat4 uInvProj;
uniform vec2 uResolution;
uniform float uRadius;
uniform float uPower;
uniform float uBias;
uniform float uRoomRadius;
uniform float uRoomStrength;
uniform int uSampleCount;
uniform int uRoomSampleCount;
uniform bool uRoomEnabled;

in vec2 vUV;
out vec4 o;

const int kernelSize = 16;
const vec3 kernel[kernelSize] = vec3[kernelSize](
    vec3( 0.052,  0.032, 0.078),
    vec3(-0.061,  0.049, 0.087),
    vec3( 0.042, -0.083, 0.101),
    vec3(-0.097, -0.043, 0.122),
    vec3( 0.125,  0.071, 0.153),
    vec3(-0.145,  0.098, 0.181),
    vec3( 0.082, -0.173, 0.209),
    vec3(-0.194, -0.126, 0.237),
    vec3( 0.246,  0.108, 0.286),
    vec3(-0.273,  0.181, 0.332),
    vec3( 0.161, -0.322, 0.384),
    vec3(-0.354, -0.238, 0.431),
    vec3( 0.426,  0.251, 0.522),
    vec3(-0.486,  0.317, 0.607),
    vec3( 0.322, -0.542, 0.692),
    vec3(-0.611, -0.421, 0.813)
);

const int roomKernelSize = 16;
const vec3 roomKernel[roomKernelSize] = vec3[roomKernelSize](
    vec3( 0.196,  0.089, 0.180),
    vec3(-0.220,  0.151, 0.220),
    vec3( 0.073, -0.314, 0.200),
    vec3( 0.326,  0.247, 0.280),
    vec3(-0.417, -0.112, 0.240),
    vec3(-0.127,  0.502, 0.340),
    vec3( 0.526, -0.265, 0.300),
    vec3(-0.540, -0.361, 0.380),
    vec3( 0.209,  0.675, 0.320),
    vec3( 0.690, -0.049, 0.460),
    vec3(-0.707,  0.427, 0.360),
    vec3( 0.089, -0.833, 0.420),
    vec3( 0.812,  0.368, 0.400),
    vec3(-0.391, -0.808, 0.480),
    vec3(-0.916,  0.168, 0.340),
    vec3( 0.516, -0.793, 0.300)
);

vec3 reconstructViewPosition(vec2 uv, float depth)
{
    vec4 ndc = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 vp = uInvProj * ndc;
    return vp.xyz / vp.w;
}

float computeRoomAo(vec3 P, mat3 TBN)
{
    float occlusion = 0.0;
    float roomBias = max(uBias, uRoomRadius * 0.035);
    int sampleCount = clamp(uRoomSampleCount, 1, roomKernelSize);

    for(int i = 0; i < sampleCount; ++i)
    {
        vec3 s = P + TBN * roomKernel[i] * uRoomRadius;
        vec4 clip = uProj * vec4(s, 1.0);
        if(clip.w <= 0.0)
        {
            continue;
        }

        vec3 ndc = clip.xyz / clip.w;
        vec2 sampleUV = ndc.xy * 0.5 + 0.5;
        if(any(lessThan(sampleUV, vec2(0.0))) ||
           any(greaterThan(sampleUV, vec2(1.0))))
        {
            continue;
        }

        float sampleDepthRaw = texture(uDepth, sampleUV).r;
        if(sampleDepthRaw >= 1.0)
        {
            continue;
        }

        vec3 sampleP = reconstructViewPosition(sampleUV, sampleDepthRaw);
        float depthDelta = abs(P.z - sampleP.z);
        float rangeCheck =
            1.0 - smoothstep(uRoomRadius * 0.25, uRoomRadius, depthDelta);
        occlusion += (sampleP.z >= s.z + roomBias ? 1.0 : 0.0) * rangeCheck;
    }

    return clamp(1.0 - (occlusion / float(sampleCount)), 0.0, 1.0);
}

void main()
{
    float centerDepth = texture(uDepth, vUV).r;
    if(centerDepth >= 1.0)
    {
        o = vec4(1.0, 1.0, 1.0, 1.0);
        return;
    }

    vec3 P = reconstructViewPosition(vUV, centerDepth);
    vec3 N = normalize(cross(dFdx(P), dFdy(P)));

    float angle = fract(
        sin(dot(vUV * uResolution, vec2(12.9898, 78.233))) * 43758.5453
    ) * 6.28318530718;
    vec3 randomVec = vec3(cos(angle), sin(angle), 0.0);
    vec3 T = normalize(randomVec - N * dot(randomVec, N));
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);

    float occlusion = 0.0;
    int sampleCount = clamp(uSampleCount, 1, kernelSize);
    for(int i = 0; i < sampleCount; ++i)
    {
        vec3 s = P + TBN * kernel[i] * uRadius;
        vec4 clip = uProj * vec4(s, 1.0);
        if(clip.w <= 0.0)
        {
            continue;
        }

        vec3 ndc = clip.xyz / clip.w;
        vec2 sampleUV = ndc.xy * 0.5 + 0.5;
        if(any(lessThan(sampleUV, vec2(0.0))) ||
           any(greaterThan(sampleUV, vec2(1.0))))
        {
            continue;
        }

        float sampleDepthRaw = texture(uDepth, sampleUV).r;
        if(sampleDepthRaw >= 1.0)
        {
            continue;
        }

        vec3 sampleP = reconstructViewPosition(sampleUV, sampleDepthRaw);
        float depthDelta = max(abs(P.z - sampleP.z), 0.0001);
        float rangeCheck = smoothstep(0.0, 1.0, uRadius / depthDelta);
        occlusion += (sampleP.z >= s.z + uBias ? 1.0 : 0.0) * rangeCheck;
    }

    float ao = 1.0 - (occlusion / float(sampleCount));
    ao = pow(clamp(ao, 0.0, 1.0), uPower);
    if(uRoomEnabled && uRoomStrength > 0.0 && uRoomRadius > 0.0)
    {
        float roomAO = computeRoomAo(P, TBN);
        ao = clamp(ao * mix(1.0, roomAO, uRoomStrength), 0.0, 1.0);
    }
    o = vec4(ao, ao, ao, 1.0);
}
)glsl";

}

namespace sponza
{

    osg::ref_ptr<osg::Geode>
    createSsaoQuad( const SponzaOptions&      options,
                    SponzaTargets&            targets,
                    const SponzaFrameContext& frame )
    {
        osg::ref_ptr<osg::Geode> geode = makeFullscreenPassGeode( ssaoFragmentShader );

        osg::StateSet*           stateSet = geode->getOrCreateStateSet();
        stateSet->setTextureAttributeAndModes( 0,
                                               targets.sceneDepth.get(),
                                               osg::StateAttribute::ON );
        stateSet->addUniform( new osg::Uniform( "uDepth", 0 ) );
        stateSet->addUniform( new osg::Uniform( "uProj", osg::mat4( frame.proj ) ) );
        stateSet->addUniform( new osg::Uniform( "uInvProj",
                                                osg::mat4( frame.invProj ) ) );
        stateSet->addUniform( new osg::Uniform(
            "uResolution",
            osg::vec2( static_cast<float>( ssaoTargetWidth( options ) ),
                       static_cast<float>( ssaoTargetHeight( options ) ) )
        ) );
        stateSet->addUniform( new osg::Uniform( "uRadius", options.aoRadius ) );
        stateSet->addUniform( new osg::Uniform( "uPower", options.aoPower ) );
        stateSet->addUniform( new osg::Uniform( "uBias", options.aoBias ) );
        stateSet->addUniform( new osg::Uniform( "uRoomRadius", options.aoRoomRadius ) );
        stateSet->addUniform( new osg::Uniform( "uRoomStrength",
                                                options.aoRoomStrength ) );
        stateSet->addUniform( new osg::Uniform( "uSampleCount", options.ssaoSamples ) );
        stateSet->addUniform( new osg::Uniform( "uRoomSampleCount",
                                                options.ssaoRoomSamples ) );
        stateSet->addUniform( new osg::Uniform( "uRoomEnabled",
                                                options.ssaoRoomEnabled ) );

        return geode;
    }

    osg::ref_ptr<osg::Camera>
    createSsaoCamera( const SponzaOptions&      options,
                      SponzaTargets&            targets,
                      const SponzaFrameContext& frame )
    {
        osg::ref_ptr<osg::Camera> ssao = new osg::Camera;
        ssao->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
        ssao->attach( osg::Camera::COLOR_BUFFER0, targets.aoTexture.get() );
        ssao->setViewport( 0,
                           0,
                           ssaoTargetWidth( options ),
                           ssaoTargetHeight( options ) );
        ssao->setRenderOrder( osg::Camera::PRE_RENDER, ssaoPassOrder );
        ssao->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
        ssao->setProjectionMatrix( osg::mat4() );
        ssao->setViewMatrix( osg::mat4() );
        ssao->setClearColor( osg::vec4( 1.0F, 1.0F, 1.0F, 1.0F ) );
        ssao->setClearMask( GL_COLOR_BUFFER_BIT );
        ssao->setAllowEventFocus( false );

        if( options.ssaoEnabled )
        {
            osg::ref_ptr<osg::Geode> quad = createSsaoQuad( options, targets, frame );
            ssao->addChild( quad.get() );
        }

        return ssao;
    }

}
