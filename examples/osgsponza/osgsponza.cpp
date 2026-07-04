#include <cmath>
#include <iostream>
#include <osg/core/ArgumentParser.hpp>
#include <osg/lighting/Light.hpp>
#include <osg/maths/Math.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Node.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/StateSet.hpp>
#include <osg/state/Uniform.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/manipulators/FirstPersonManipulator.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <string>

namespace
{
    constexpr double defaultSunAzimuthDeg   = 60.0;
    constexpr double defaultSunElevationDeg = 55.0;
    constexpr float  defaultSunIntensity    = 3.0F;
    constexpr float  defaultAmbientLevel    = 0.3F;
    constexpr float  defaultExposure        = 1.0F;

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

    osg::vec3
    scaledColor( const osg::vec3& color,
                 float            scale )
    {
        return osg::vec3( color.r * scale, color.g * scale, color.b * scale );
    }
}

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    std::string headlessOutput;
    const bool  headless = arguments.read( "--headless", headlessOutput );

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

    const osg::dvec3 eye( -8.80743, 1.59221947, -0.85825783 );
    const osg::dvec3 center( -7.84353, 1.78652, -0.67626 );
    const osg::dvec3 up( -0.1909, 0.9809, -0.0360 );

    const double azimuthRad   = osg::DegreesToRadians( sunAzimuthDeg );
    const double elevationRad = osg::DegreesToRadians( sunElevationDeg );
    const osg::dvec3 dirWorld = osg::normalize(
        osg::dvec3( std::cos( elevationRad ) * std::cos( azimuthRad ),
                    std::sin( elevationRad ),
                    std::cos( elevationRad ) * std::sin( azimuthRad ) )
    );
    const osg::dmat4 view = osg::lookAt( eye, center, up );
    const osg::dvec3 dirView = osg::normalize(
        osg::transform3x3( view, dirWorld )
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

    osg::StateSet* rootStateSet = model->getOrCreateStateSet();
    rootStateSet->setAttributeAndModes( sun.get(), osg::StateAttribute::ON );
    rootStateSet->setMode( GL_LIGHTING, osg::StateAttribute::ON );

    const osg::vec3 ambientRadiance = scaledColor( ambientColor, ambientLevel );
    rootStateSet->addUniform(
        new osg::Uniform(
            "osg_LightModel_ambient",
            osg::vec4( ambientRadiance.r,
                       ambientRadiance.g,
                       ambientRadiance.b,
                       1.0F )
        )
    );
    rootStateSet->addUniform( new osg::Uniform( "uExposure", exposure ) );

    if( headless )
    {
        return osg::headlessCapture(
                   model.get(), headlessOutput, 1920, 1080, eye, center, up
               )
                 ? 0
                 : 1;
    }

    osgViewer::Viewer viewer;
    viewer.setSceneData( model.get() );
    viewer.setCameraManipulator( new osgGA::FirstPersonManipulator );
    viewer.getCameraManipulator()->setHomePosition( eye, center, up );

    return viewer.run();
}
