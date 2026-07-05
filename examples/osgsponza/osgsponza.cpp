#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <osg/GL>
#include <osg/core/ArgumentParser.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/images/Image.hpp>
#include <osg/lighting/Light.hpp>
#include <osg/maths/Math.hpp>
#include <osg/maths/MatrixTemplate.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Camera.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/Node.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/Program.hpp>
#include <osg/state/Shader.hpp>
#include <osg/state/StateSet.hpp>
#include <osg/state/Uniform.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/manipulators/FirstPersonManipulator.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <string>

namespace
{
    constexpr int    renderWidth            = 1'920;
    constexpr int    renderHeight           = 1'080;
    constexpr double renderAspect           = 16.0 / 9.0;
    constexpr double nearZ                  = 0.1;
    constexpr double farZ                   = 2'000.0;

    constexpr double defaultSunAzimuthDeg   = 75.0;
    constexpr double defaultSunElevationDeg = 52.0;
    constexpr float  defaultSunIntensity    = 3.5F;
    constexpr float  defaultAmbientLevel    = 0.7F;
    constexpr float  defaultExposure        = 1.3F;
    constexpr float  defaultIblIntensity    = 1.0F;
    constexpr float  defaultIblDiffuse      = 0.5F;
    constexpr float  defaultIblSpecular     = 0.12F;
    constexpr float  defaultIblClamp        = 20.0F;
    constexpr float  defaultEnvRotation     = 0.0F;
    constexpr float  defaultAoRadius        = 0.5F;
    constexpr float  defaultAoStrength      = 0.8F;
    constexpr float  defaultAoPower         = 1.5F;
    constexpr float  defaultAoBias          = 0.025F;
    constexpr int    defaultCameraIndex     = 0;
    constexpr unsigned int environmentTextureUnit = 5U;

    struct CameraPreset
    {
            const char* name;
            osg::dvec3 eye;
            osg::dvec3 forward;
            osg::dvec3 up;
            double     fovDeg;
    };

    struct CameraSettings
    {
            osg::dvec3 eye;
            osg::dvec3 center;
            osg::dvec3 up;
            double     fovDeg;
    };

    const std::array<CameraPreset, 6> cameraPresets = {
        { { "PhysCamera001",
            osg::dvec3( -8.80743, 1.59221947, -0.85825783 ),
            osg::dvec3( 0.9639, 0.1943, 0.1820 ),
            osg::dvec3( -0.1909, 0.9809, -0.0360 ),
            58.51 },
          { "PhysCamera002",
            osg::dvec3( 10.5311031, 7.352985, 1.52825129 ),
            osg::dvec3( -0.9919, 0.0225, -0.1252 ),
            osg::dvec3( 0.0223, 0.9997, 0.0028 ),
            58.63 },
          { "PhysCamera003",
            osg::dvec3( 2.70301843, 1.35489559, 2.11138153 ),
            osg::dvec3( -0.9395, 0.0864, -0.3316 ),
            osg::dvec3( 0.0815, 0.9963, 0.0288 ),
            36.25 },
          { "PhysCamera004",
            osg::dvec3( -9.594356, 6.921101, 5.35278 ),
            osg::dvec3( 0.9413, -0.0202, -0.3368 ),
            osg::dvec3( 0.0190, 0.9998, -0.0068 ),
            58.24 },
          { "PhysCamera005",
            osg::dvec3( -2.23714, 0.7440546, 2.3608017 ),
            osg::dvec3( 0.8684, 0.2434, -0.4320 ),
            osg::dvec3( -0.2179, 0.9699, 0.1084 ),
            37.22 },
          { "PhysCamera006",
            osg::dvec3( -6.59548569, 10.8791218, -0.6290613 ),
            osg::dvec3( 0.9628, -0.2697, 0.0174 ),
            osg::dvec3( 0.2697, 0.9629, 0.0049 ),
            37.24 } }
    };

    constexpr char fullscreenVertexShader[] = R"glsl(
#version 460 core

layout(location = 0) in vec4 osg_Vertex;
out vec2 vUV;

void main()
{
    vUV = osg_Vertex.xy * 0.5 + 0.5;
    gl_Position = vec4(osg_Vertex.xy, 0.0, 1.0);
}
)glsl";

    constexpr char ssaoFragmentShader[] = R"glsl(
#version 460 core

uniform sampler2D uDepth;
uniform mat4 uProj;
uniform mat4 uInvProj;
uniform vec2 uResolution;
uniform float uRadius;
uniform float uPower;
uniform float uBias;

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

vec3 reconstructViewPosition(vec2 uv, float depth)
{
    vec4 ndc = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 vp = uInvProj * ndc;
    return vp.xyz / vp.w;
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
    for(int i = 0; i < kernelSize; ++i)
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

    float ao = 1.0 - (occlusion / float(kernelSize));
    ao = pow(clamp(ao, 0.0, 1.0), uPower);
    o = vec4(ao, ao, ao, 1.0);
}
)glsl";

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

    osg::vec3
    scaledColor( const osg::vec3& color,
                 float            scale )
    {
        return osg::vec3( color.r * scale, color.g * scale, color.b * scale );
    }

    float
    computeMaxMipLevel( const osg::Image& image )
    {
        const int maxDimension = std::max( image.s(), image.t() );
        return maxDimension > 0
                 ? std::floor( std::log2( static_cast<float>( maxDimension ) ) )
                 : 0.0F;
    }

    osg::ref_ptr<osg::Image>
    loadEnvironmentImage()
    {
        osg::ref_ptr<osg::Image> image =
            osgDB::readRefImageFile( "textures/kloppenheim_05_4k.hdr" );
        if( !image )
        {
            image = osgDB::readRefImageFile( "kloppenheim_05_4k.hdr" );
        }
        return image;
    }

    osg::ref_ptr<osg::Texture2D>
    createEnvironmentTexture( osg::Image* image )
    {
        osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
        texture->setImage( image );
        texture->setInternalFormat( GL_RGB16F );
        texture->setFilter( osg::Texture2D::MIN_FILTER,
                            osg::Texture2D::LINEAR_MIPMAP_LINEAR );
        texture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
        texture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT );
        texture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE );
        texture->setUseHardwareMipMapGeneration( true );
        texture->setMaxAnisotropy( 4.0F );
        return texture;
    }

    osg::Matrix3
    makeViewToWorldRotation( const osg::dmat4& viewMatrix )
    {
        return osg::Matrix3(
            static_cast<float>( viewMatrix[0][0] ),
            static_cast<float>( viewMatrix[1][0] ),
            static_cast<float>( viewMatrix[2][0] ),
            static_cast<float>( viewMatrix[0][1] ),
            static_cast<float>( viewMatrix[1][1] ),
            static_cast<float>( viewMatrix[2][1] ),
            static_cast<float>( viewMatrix[0][2] ),
            static_cast<float>( viewMatrix[1][2] ),
            static_cast<float>( viewMatrix[2][2] )
        );
    }

    CameraSettings
    makeCameraSettings( const CameraPreset& preset )
    {
        return CameraSettings{ preset.eye, preset.eye + preset.forward, preset.up,
                               preset.fovDeg };
    }

    osg::ref_ptr<osg::Texture2D>
    createHdrColorTexture()
    {
        osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
        texture->setTextureSize( renderWidth, renderHeight );
        texture->setInternalFormat( GL_RGBA16F );
        texture->setSourceFormat( GL_RGBA );
        texture->setSourceType( GL_HALF_FLOAT );
        texture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
        texture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
        texture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE );
        texture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE );
        return texture;
    }

    osg::ref_ptr<osg::Texture2D>
    createSceneDepthTexture()
    {
        osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
        texture->setTextureSize( renderWidth, renderHeight );
        texture->setInternalFormat( GL_DEPTH_COMPONENT24 );
        texture->setSourceFormat( GL_DEPTH_COMPONENT );
        texture->setSourceType( GL_FLOAT );
        texture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST );
        texture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST );
        texture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE );
        texture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE );
        return texture;
    }

    osg::ref_ptr<osg::Texture2D>
    createAoTexture()
    {
        osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
        texture->setTextureSize( renderWidth, renderHeight );
        texture->setInternalFormat( GL_R8 );
        texture->setSourceFormat( GL_RED );
        texture->setSourceType( GL_UNSIGNED_BYTE );
        texture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
        texture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
        texture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE );
        texture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE );
        return texture;
    }

    osg::ref_ptr<osg::Geometry>
    createFullscreenQuadGeometry()
    {
        osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
        vertices->reserve( 6 );
        vertices->push_back( osg::vec3( -1.0F, -1.0F, 0.0F ) );
        vertices->push_back( osg::vec3( 1.0F, -1.0F, 0.0F ) );
        vertices->push_back( osg::vec3( 1.0F, 1.0F, 0.0F ) );
        vertices->push_back( osg::vec3( -1.0F, -1.0F, 0.0F ) );
        vertices->push_back( osg::vec3( 1.0F, 1.0F, 0.0F ) );
        vertices->push_back( osg::vec3( -1.0F, 1.0F, 0.0F ) );

        osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
        geometry->setUseVertexBufferObjects( true );
        geometry->setVertexArray( vertices.get() );
        geometry->addPrimitiveSet( new osg::DrawArrays( GL_TRIANGLES, 0, 6 ) );
        return geometry;
    }

    osg::ref_ptr<osg::Camera>
    createRttCamera( osg::Node*            model,
                     osg::Texture2D*       hdrColor,
                     osg::Texture2D*       sceneDepth,
                     const osg::dmat4&     projectionMatrix,
                     const osg::dmat4&     viewMatrix )
    {
        osg::ref_ptr<osg::Camera> rtt = new osg::Camera;
        rtt->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
        rtt->attach( osg::Camera::COLOR_BUFFER0, hdrColor );
        rtt->attach( osg::Camera::DEPTH_BUFFER, sceneDepth );
        rtt->setViewport( 0, 0, renderWidth, renderHeight );
        rtt->setRenderOrder( osg::Camera::PRE_RENDER, 0 );
        rtt->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
        rtt->setClearColor( osg::vec4( 0.0F, 0.0F, 0.0F, 1.0F ) );
        rtt->setClearMask( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        rtt->setProjectionMatrix( projectionMatrix );
        rtt->setViewMatrix( viewMatrix );
        rtt->addChild( model );
        return rtt;
    }

    osg::ref_ptr<osg::Geode>
    createSsaoQuad( osg::Texture2D*   sceneDepth,
                    const osg::mat4&  projectionMatrix,
                    const osg::mat4&  inverseProjectionMatrix,
                    float             aoRadius,
                    float             aoPower )
    {
        osg::ref_ptr<osg::Program> program = new osg::Program;
        program->addBindAttribLocation( "osg_Vertex", 0U );
        program->addShader( new osg::Shader( osg::Shader::VERTEX,
                                             fullscreenVertexShader ) );
        program->addShader( new osg::Shader( osg::Shader::FRAGMENT,
                                             ssaoFragmentShader ) );

        osg::ref_ptr<osg::Geometry> geometry = createFullscreenQuadGeometry();
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        geode->setCullingActive( false );
        geode->addDrawable( geometry.get() );

        osg::StateSet* stateSet = geode->getOrCreateStateSet();
        stateSet->setAttributeAndModes( program.get(), osg::StateAttribute::ON );
        stateSet->setTextureAttributeAndModes( 0, sceneDepth, osg::StateAttribute::ON );
        stateSet->addUniform( new osg::Uniform( "uDepth", 0 ) );
        stateSet->addUniform( new osg::Uniform( "uProj", projectionMatrix ) );
        stateSet->addUniform( new osg::Uniform( "uInvProj", inverseProjectionMatrix ) );
        stateSet->addUniform(
            new osg::Uniform(
                "uResolution",
                osg::vec2( static_cast<float>( renderWidth ),
                           static_cast<float>( renderHeight ) )
            )
        );
        stateSet->addUniform( new osg::Uniform( "uRadius", aoRadius ) );
        stateSet->addUniform( new osg::Uniform( "uPower", aoPower ) );
        stateSet->addUniform( new osg::Uniform( "uBias", defaultAoBias ) );
        stateSet->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );
        stateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

        return geode;
    }

    osg::ref_ptr<osg::Camera>
    createSsaoCamera( osg::Texture2D*  sceneDepth,
                      osg::Texture2D*  aoTexture,
                      const osg::mat4& projectionMatrix,
                      const osg::mat4& inverseProjectionMatrix,
                      float            aoRadius,
                      float            aoPower,
                      bool             ssaoEnabled )
    {
        osg::ref_ptr<osg::Camera> ssao = new osg::Camera;
        ssao->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
        ssao->attach( osg::Camera::COLOR_BUFFER0, aoTexture );
        ssao->setViewport( 0, 0, renderWidth, renderHeight );
        ssao->setRenderOrder( osg::Camera::PRE_RENDER, 1 );
        ssao->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
        ssao->setProjectionMatrix( osg::mat4() );
        ssao->setViewMatrix( osg::mat4() );
        ssao->setClearColor( osg::vec4( 1.0F, 1.0F, 1.0F, 1.0F ) );
        ssao->setClearMask( GL_COLOR_BUFFER_BIT );
        ssao->setAllowEventFocus( false );

        if( ssaoEnabled )
        {
            osg::ref_ptr<osg::Geode> quad =
                createSsaoQuad( sceneDepth,
                                projectionMatrix,
                                inverseProjectionMatrix,
                                aoRadius,
                                aoPower );
            ssao->addChild( quad.get() );
        }

        return ssao;
    }

    osg::ref_ptr<osg::Geode>
    createTonemapQuad( osg::Texture2D* hdrColor,
                       osg::Texture2D* aoTexture,
                       float           exposure,
                       float           aoStrength )
    {
        osg::ref_ptr<osg::Program> program = new osg::Program;
        program->addBindAttribLocation( "osg_Vertex", 0U );
        program->addShader( new osg::Shader( osg::Shader::VERTEX,
                                             fullscreenVertexShader ) );
        program->addShader( new osg::Shader( osg::Shader::FRAGMENT,
                                             tonemapFragmentShader ) );

        osg::ref_ptr<osg::Geometry> geometry = createFullscreenQuadGeometry();
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        geode->setCullingActive( false );
        geode->addDrawable( geometry.get() );

        osg::StateSet* stateSet = geode->getOrCreateStateSet();
        stateSet->setAttributeAndModes( program.get(), osg::StateAttribute::ON );
        stateSet->setTextureAttributeAndModes( 0, hdrColor, osg::StateAttribute::ON );
        stateSet->setTextureAttributeAndModes( 1, aoTexture, osg::StateAttribute::ON );
        stateSet->addUniform( new osg::Uniform( "uHdr", 0 ) );
        stateSet->addUniform( new osg::Uniform( "uAo", 1 ) );
        stateSet->addUniform( new osg::Uniform( "uExposure", exposure ) );
        stateSet->addUniform( new osg::Uniform( "uAoStrength", aoStrength ) );
        stateSet->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );
        stateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

        return geode;
    }

    osg::ref_ptr<osg::Camera>
    createTonemapCamera( osg::Texture2D* hdrColor,
                         osg::Texture2D* aoTexture,
                         float           exposure,
                         float           aoStrength )
    {
        osg::ref_ptr<osg::Geode>  quad =
            createTonemapQuad( hdrColor, aoTexture, exposure, aoStrength );
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

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    std::string headlessOutput;
    const bool  headless = arguments.read( "--headless", headlessOutput );

    int cameraIndex = defaultCameraIndex;
    arguments.read( "--camera-index", cameraIndex );
    if( cameraIndex < 0 ||
        cameraIndex >= static_cast<int>( cameraPresets.size() ) )
    {
        std::cerr << "--camera-index must be in the range 0..5" << std::endl;
        return 1;
    }

    CameraSettings camera =
        makeCameraSettings( cameraPresets[static_cast<size_t>( cameraIndex )] );

    double sunAzimuthDeg   = defaultSunAzimuthDeg;
    double sunElevationDeg = defaultSunElevationDeg;
    float  sunIntensity    = defaultSunIntensity;
    float  ambientLevel    = defaultAmbientLevel;
    float  exposure        = defaultExposure;
    float  iblIntensity    = defaultIblIntensity;
    float  iblDiffuse      = defaultIblDiffuse;
    float  iblSpecular     = defaultIblSpecular;
    float  iblClamp        = defaultIblClamp;
    float  envRotation     = defaultEnvRotation;
    float  aoRadius        = defaultAoRadius;
    float  aoStrength      = defaultAoStrength;
    float  aoPower         = defaultAoPower;
    bool   ssaoEnabled     = true;

    arguments.read( "--sun-azimuth", sunAzimuthDeg );
    arguments.read( "--sun-elevation", sunElevationDeg );
    arguments.read( "--sun-intensity", sunIntensity );
    arguments.read( "--ambient", ambientLevel );
    arguments.read( "--exposure", exposure );
    arguments.read( "--ibl-intensity", iblIntensity );
    arguments.read( "--ibl-diffuse", iblDiffuse );
    arguments.read( "--ibl-specular", iblSpecular );
    arguments.read( "--ibl-clamp", iblClamp );
    arguments.read( "--env-rotation", envRotation );
    arguments.read( "--ao-radius", aoRadius );
    arguments.read( "--ao-strength", aoStrength );
    arguments.read( "--ao-power", aoPower );
    std::string ssaoMode = "on";
    if( arguments.read( "--ssao", ssaoMode ) )
    {
        if( ssaoMode == "off" )
        {
            ssaoEnabled = false;
        }
        else if( ssaoMode != "on" )
        {
            std::cerr << "--ssao must be 'on' or 'off'" << std::endl;
            return 1;
        }
    }
    readDVec3Argument( arguments, "--eye", camera.eye );
    readDVec3Argument( arguments, "--center", camera.center );
    readDVec3Argument( arguments, "--up", camera.up );
    arguments.read( "--fov", camera.fovDeg );

    const osg::vec3 sunColor = readColorArgument(
        arguments, "--sun-color", osg::vec3( 1.0F, 0.95F, 0.85F )
    );
    const osg::vec3 ambientColor = readColorArgument(
        arguments, "--ambient-color", osg::vec3( 0.5F, 0.6F, 0.75F )
    );

    std::string modelPath = "NewSponza_Main_glTF_003.gltf";
    for( int i = 1; i < arguments.argc(); ++i )
    {
        if( !arguments.isOption( i ) )
        {
            modelPath = arguments[i];
            break;
        }
    }

    osg::ref_ptr<osg::Node> model = osgDB::readRefNodeFile( modelPath );
    if( !model )
    {
        std::cerr << "Failed to load " << modelPath << std::endl;
        return 1;
    }

    const double azimuthRad   = osg::DegreesToRadians( sunAzimuthDeg );
    const double elevationRad = osg::DegreesToRadians( sunElevationDeg );
    const osg::dvec3 dirWorld = osg::normalize(
        osg::dvec3( std::cos( elevationRad ) * std::cos( azimuthRad ),
                    std::sin( elevationRad ),
                    std::cos( elevationRad ) * std::sin( azimuthRad ) )
    );
    const osg::dmat4 rttView = osg::lookAt( camera.eye, camera.center, camera.up );
    const osg::dvec3 dirView = osg::normalize(
        osg::transform3x3( rttView, dirWorld )
    );
    const osg::dmat4 projectionMatrix =
        osg::perspective( camera.fovDeg, renderAspect, nearZ, farZ );
    const osg::mat4 projectionMatrixUniform( projectionMatrix );
    const osg::mat4 inverseProjectionMatrixUniform(
        osg::inverse( projectionMatrix )
    );

    osg::ref_ptr<osg::Light> sun = new osg::Light;
    const osg::vec3          sunRadiance = scaledColor( sunColor, sunIntensity );
    sun->setLightNum( 0 );
    sun->setPosition( osg::vec4( static_cast<float>( dirView.x ),
                                 static_cast<float>( dirView.y ),
                                 static_cast<float>( dirView.z ),
                                 0.0F ) );
    sun->setDiffuse(
        osg::vec4( sunRadiance.r, sunRadiance.g, sunRadiance.b, 1.0F )
    );
    sun->setSpecular(
        osg::vec4( sunRadiance.r, sunRadiance.g, sunRadiance.b, 1.0F )
    );
    sun->setAmbient( osg::vec4( 0.0F, 0.0F, 0.0F, 1.0F ) );

    osg::StateSet* modelStateSet = model->getOrCreateStateSet();
    modelStateSet->setAttributeAndModes( sun.get(), osg::StateAttribute::ON );
    modelStateSet->setMode( GL_LIGHTING, osg::StateAttribute::ON );

    const osg::vec3 ambientRadiance = scaledColor( ambientColor, ambientLevel );
    modelStateSet->addUniform(
        new osg::Uniform(
            "osg_LightModel_ambient",
            osg::vec4( ambientRadiance.r,
                       ambientRadiance.g,
                       ambientRadiance.b,
                       1.0F )
        )
    );
    modelStateSet->addUniform(
        new osg::Uniform( "uViewToWorldRot", makeViewToWorldRotation( rttView ) )
    );
    modelStateSet->addUniform(
        new osg::Uniform(
            "uEnvMap", static_cast<int>( environmentTextureUnit )
        )
    );
    modelStateSet->addUniform( new osg::Uniform( "uEnvMaxLod", 0.0F ) );
    modelStateSet->addUniform( new osg::Uniform( "uEnvClamp", iblClamp ) );
    modelStateSet->addUniform( new osg::Uniform( "uIblIntensity", iblIntensity ) );
    modelStateSet->addUniform( new osg::Uniform( "uIblDiffuse", iblDiffuse ) );
    modelStateSet->addUniform( new osg::Uniform( "uIblSpecular", iblSpecular ) );
    modelStateSet->addUniform( new osg::Uniform( "uEnvRotation", envRotation ) );
    modelStateSet->addUniform( new osg::Uniform( "uHasEnv", false ) );

    osg::ref_ptr<osg::Image> envImage = loadEnvironmentImage();
    if( envImage )
    {
        osg::ref_ptr<osg::Texture2D> envTexture =
            createEnvironmentTexture( envImage.get() );
        modelStateSet->setTextureAttributeAndModes(
            environmentTextureUnit, envTexture.get(), osg::StateAttribute::ON
        );
        modelStateSet->getUniform( "uEnvMaxLod" )->set(
            computeMaxMipLevel( *envImage )
        );
        modelStateSet->getUniform( "uHasEnv" )->set( true );
    }
    else
    {
        std::cerr << "Warning: failed to load textures/kloppenheim_05_4k.hdr "
                     "or kloppenheim_05_4k.hdr; IBL disabled"
                  << std::endl;
    }

    osg::ref_ptr<osg::Texture2D> hdrColor   = createHdrColorTexture();
    osg::ref_ptr<osg::Texture2D> sceneDepth = createSceneDepthTexture();
    osg::ref_ptr<osg::Texture2D> aoTexture  = createAoTexture();
    osg::ref_ptr<osg::Camera>    rtt =
        createRttCamera(
            model.get(),
            hdrColor.get(),
            sceneDepth.get(),
            projectionMatrix,
            rttView
        );
    osg::ref_ptr<osg::Camera> ssao =
        createSsaoCamera(
            sceneDepth.get(),
            aoTexture.get(),
            projectionMatrixUniform,
            inverseProjectionMatrixUniform,
            aoRadius,
            aoPower,
            ssaoEnabled
        );
    osg::ref_ptr<osg::Camera> tonemapCamera =
        createTonemapCamera(
            hdrColor.get(),
            aoTexture.get(),
            exposure,
            ssaoEnabled ? aoStrength : 0.0F
        );

    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( rtt.get() );
    root->addChild( ssao.get() );
    root->addChild( tonemapCamera.get() );

    if( headless )
    {
        return osg::headlessCapture( root.get(),
                                     headlessOutput,
                                     renderWidth,
                                     renderHeight,
                                     camera.eye,
                                     camera.center,
                                     camera.up )
                 ? 0
                 : 1;
    }

    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    viewer.setCameraManipulator( new osgGA::FirstPersonManipulator );
    viewer.getCameraManipulator()->setHomePosition( camera.eye, camera.center, camera.up );

    return viewer.run();
}
