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
uniform sampler2D uAo;
uniform float uExposure;
uniform float uAoStrength;

in vec2 vUV;
out vec4 o;

vec3 acesFilmic(vec3 c)
{
    return (c * (2.51 * c + 0.03)) / (c * (2.43 * c + 0.59) + 0.14);
}

void main()
{
    float ao = texture(uAo, vUV).r;
    ao = mix(1.0, ao, uAoStrength);
    vec3 c = texture(uHdr, vUV).rgb * ao * uExposure;
    c = acesFilmic(c);
    c = clamp(c, 0.0, 1.0);
    c = pow(c, vec3(1.0 / 2.2));
    o = vec4(c, 1.0);
}
)glsl";

}

namespace sponza
{

    osg::ref_ptr<osg::Geode>
    createTonemapQuad( const SponzaOptions&      options,
                       SponzaTargets&            targets,
                       const SponzaFrameContext& frame )
    {
        ( void )frame;

        osg::ref_ptr<osg::Geode> geode =
            makeFullscreenPassGeode( tonemapFragmentShader );

        osg::StateSet* stateSet = geode->getOrCreateStateSet();
        stateSet->setTextureAttributeAndModes( 0,
                                               targets.hdrColor.get(),
                                               osg::StateAttribute::ON );
        stateSet->setTextureAttributeAndModes( 1,
                                               targets.aoTexture.get(),
                                               osg::StateAttribute::ON );
        stateSet->addUniform( new osg::Uniform( "uHdr", 0 ) );
        stateSet->addUniform( new osg::Uniform( "uAo", 1 ) );
        stateSet->addUniform( new osg::Uniform( "uExposure", options.exposure ) );
        stateSet->addUniform( new osg::Uniform( "uAoStrength",
                                                options.ssaoEnabled ? options.aoStrength
                                                                    : 0.0F ) );

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
