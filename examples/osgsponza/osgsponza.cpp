#include <array>
#include <cmath>
#include <iostream>
#include <osg/GL>
#include <osg/core/ArgumentParser.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/lighting/Light.hpp>
#include <osg/maths/Math.hpp>
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
    constexpr float  defaultSunIntensity    = 5.0F;
    constexpr float  defaultAmbientLevel    = 0.85F;
    constexpr float  defaultExposure        = 2.8F;
    constexpr int    defaultCameraIndex     = 0;

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

    constexpr char tonemapVertexShader[] = R"glsl(
#version 460 core

layout(location = 0) in vec4 osg_Vertex;
out vec2 vUV;

void main()
{
    vUV = osg_Vertex.xy * 0.5 + 0.5;
    gl_Position = vec4(osg_Vertex.xy, 0.0, 1.0);
}
)glsl";

    constexpr char tonemapFragmentShader[] = R"glsl(
#version 460 core

uniform sampler2D uHdr;
uniform float uExposure;

in vec2 vUV;
out vec4 o;

vec3 acesFilmic(vec3 c)
{
    return (c * (2.51 * c + 0.03)) / (c * (2.43 * c + 0.59) + 0.14);
}

void main()
{
    vec3 c = texture(uHdr, vUV).rgb * uExposure;
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

    osg::ref_ptr<osg::Camera>
    createRttCamera( osg::Node*            model,
                     osg::Texture2D*       hdrColor,
                     osg::Texture2D*       sceneDepth,
                     const CameraSettings& settings,
                     const osg::dmat4&     viewMatrix )
    {
        osg::ref_ptr<osg::Camera> rtt = new osg::Camera;
        rtt->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
        rtt->attach( osg::Camera::COLOR_BUFFER0, hdrColor );
        rtt->attach( osg::Camera::DEPTH_BUFFER, sceneDepth );
        rtt->setViewport( 0, 0, renderWidth, renderHeight );
        rtt->setRenderOrder( osg::Camera::PRE_RENDER );
        rtt->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
        rtt->setClearColor( osg::vec4( 0.0F, 0.0F, 0.0F, 1.0F ) );
        rtt->setClearMask( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        rtt->setProjectionMatrix(
            osg::perspective( settings.fovDeg, renderAspect, nearZ, farZ )
        );
        rtt->setViewMatrix( viewMatrix );
        rtt->addChild( model );
        return rtt;
    }

    osg::ref_ptr<osg::Geode>
    createTonemapQuad( osg::Texture2D* hdrColor,
                       float           exposure )
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

        osg::ref_ptr<osg::Program> program = new osg::Program;
        program->addBindAttribLocation( "osg_Vertex", 0U );
        program->addShader( new osg::Shader( osg::Shader::VERTEX,
                                             tonemapVertexShader ) );
        program->addShader( new osg::Shader( osg::Shader::FRAGMENT,
                                             tonemapFragmentShader ) );

        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        geode->setCullingActive( false );
        geode->addDrawable( geometry.get() );

        osg::StateSet* stateSet = geode->getOrCreateStateSet();
        stateSet->setAttributeAndModes( program.get(), osg::StateAttribute::ON );
        stateSet->setTextureAttributeAndModes( 0, hdrColor, osg::StateAttribute::ON );
        stateSet->addUniform( new osg::Uniform( "uHdr", 0 ) );
        stateSet->addUniform( new osg::Uniform( "uExposure", exposure ) );
        stateSet->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );
        stateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

        return geode;
    }

    osg::ref_ptr<osg::Camera>
    createTonemapCamera( osg::Texture2D* hdrColor,
                         float           exposure )
    {
        osg::ref_ptr<osg::Geode>  quad = createTonemapQuad( hdrColor, exposure );
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

    arguments.read( "--sun-azimuth", sunAzimuthDeg );
    arguments.read( "--sun-elevation", sunElevationDeg );
    arguments.read( "--sun-intensity", sunIntensity );
    arguments.read( "--ambient", ambientLevel );
    arguments.read( "--exposure", exposure );
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

    osg::ref_ptr<osg::Texture2D> hdrColor   = createHdrColorTexture();
    osg::ref_ptr<osg::Texture2D> sceneDepth = createSceneDepthTexture();
    osg::ref_ptr<osg::Camera>    rtt =
        createRttCamera(
            model.get(), hdrColor.get(), sceneDepth.get(), camera, rttView
        );
    osg::ref_ptr<osg::Camera> tonemapCamera =
        createTonemapCamera( hdrColor.get(), exposure );

    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( rtt.get() );
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
