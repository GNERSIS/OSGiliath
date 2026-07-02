/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgprecipitation example application
 */
#include <iostream>
#include <osg/maths/compat.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgGA/manipulators/AnimationPathManipulator.hpp>
#include <osgGA/manipulators/DriveManipulator.hpp>
#include <osgGA/manipulators/FlightManipulator.hpp>
#include <osgGA/manipulators/KeySwitchMatrixManipulator.hpp>
#include <osgGA/manipulators/TerrainManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgParticle/PrecipitationEffect.hpp>
#include <osgUtil/culling/CullVisitor.hpp>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgUtil/utils/TransformCallback.hpp>
#include <osgViewer/core/Viewer.hpp>

class MyGustCallback : public osg::NodeCallback
{

    public:

        MyGustCallback()
        {
        }

        virtual void
        operator()( osg::Node*        node,
                    osg::NodeVisitor* nv )
        {
            osgParticle::PrecipitationEffect* pe =
                dynamic_cast<osgParticle::PrecipitationEffect*>( node );

            float value = sin( nv->getFrameStamp()->getSimulationTime() );
            if( value < -0.5 )
            {
                pe->snow( 1.0 );
            }
            else
            {
                pe->rain( 0.5 );
            }

            traverse( node, nv );
        }
};

int
main( int    argc,
      char** argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc, argv );

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(
        arguments.getApplicationName()
    );
    arguments.getApplicationUsage()->setDescription(
        arguments.getApplicationName() +
        " example provides an interactive viewer for visualising point clouds.."
    );
    arguments.getApplicationUsage()->setCommandLineUsage(
        arguments.getApplicationName() + " [options] filename ..."
    );
    arguments.getApplicationUsage()->addCommandLineOption( "-h or --help",
                                                           "Display this information" );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--snow <density>",
        "Set the snow with a density between 0 and 1.0"
    );
    arguments.getApplicationUsage()->addCommandLineOption( "--rain <density>", "" );
    arguments.getApplicationUsage()->addCommandLineOption( "--particleSize <size>", "" );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--particleColour <red> <green> <blue> <alpha>",
        ""
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--wind <x> <y> <z>",
        "Set the wind speed in model coordinates"
    );
    arguments.getApplicationUsage()->addCommandLineOption( "--particleSpeed <float>",
                                                           "Set the particle speed" );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--nearTransition <distance>",
        "Set the near transition distance"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--farTransition  <distance>",
        "Set the far transition distance"
    );
    arguments.getApplicationUsage()->addCommandLineOption( "--particleDensity <density>",
                                                           "Set the particle density" );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--cellSize <x> <y> <z>",
        "Set the cell size in model coordinates"
    );
    arguments.getApplicationUsage()->addCommandLineOption( "--fogDensity <density>",
                                                           "Set the fog density" );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--fogColour <red> <green> <blue> <alpha>",
        "Set fog colour."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "-useFarLineSegments",
        "Switch on the use of line segments"
    );

    // Headless capture: early check before viewer construction
    {
        std::string headlessOutput;
        if( arguments.read( "--headless", headlessOutput ) )
        {
            auto node = osgDB::readRefNodeFiles( arguments );
            if( !node )
            {
                node = osgDB::readRefNodeFile( "duck.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    osgViewer::Viewer viewer;

    // set up the camera manipulators.
    {
        osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator =
            new osgGA::KeySwitchMatrixManipulator;

        keyswitchManipulator->addMatrixManipulator( '1',
                                                    "Trackball",
                                                    new osgGA::TrackballManipulator() );
        keyswitchManipulator->addMatrixManipulator( '2',
                                                    "Flight",
                                                    new osgGA::FlightManipulator() );
        keyswitchManipulator->addMatrixManipulator( '3',
                                                    "Drive",
                                                    new osgGA::DriveManipulator() );
        keyswitchManipulator->addMatrixManipulator( '4',
                                                    "Terrain",
                                                    new osgGA::TerrainManipulator() );

        std::string pathfile;
        char        keyForAnimationPath = '5';
        while( arguments.read( "-p", pathfile ) )
        {
            osgGA::AnimationPathManipulator* apm =
                new osgGA::AnimationPathManipulator( pathfile );
            if( apm || !apm->valid() )
            {
                unsigned int num = keyswitchManipulator->getNumMatrixManipulators();
                keyswitchManipulator->addMatrixManipulator( keyForAnimationPath,
                                                            "Path",
                                                            apm );
                keyswitchManipulator->selectMatrixManipulator( num );
                ++keyForAnimationPath;
            }
        }

        viewer.setCameraManipulator( keyswitchManipulator.get() );
    }

    osg::ref_ptr<osgParticle::PrecipitationEffect> precipitationEffect =
        new osgParticle::PrecipitationEffect;

    float intensity;
    while( arguments.read( "--snow", intensity ) )
    {
        precipitationEffect->snow( intensity );
    }
    while( arguments.read( "--rain", intensity ) )
    {
        precipitationEffect->rain( intensity );
    }

    float value;
    while( arguments.read( "--particleSize", value ) )
    {
        precipitationEffect->setParticleSize( value );
    }

    osg::vec4 color;
    while( arguments.read( "--particleColor", color.r, color.g, color.b, color.a ) )
    {
        precipitationEffect->setParticleColor( color );
    }
    while( arguments.read( "--particleColour", color.r, color.g, color.b, color.a ) )
    {
        precipitationEffect->setParticleColor( color );
    }

    osg::vec3 wind;
    while( arguments.read( "--wind", wind.x, wind.y, wind.z ) )
    {
        precipitationEffect->setWind( wind );
    }

    while( arguments.read( "--particleSpeed", value ) )
    {
        precipitationEffect->setParticleSpeed( value );
    }

    while( arguments.read( "--nearTransition", value ) )
    {
        precipitationEffect->setNearTransition( value );
    }
    while( arguments.read( "--farTransition", value ) )
    {
        precipitationEffect->setFarTransition( value );
    }

    while( arguments.read( "--particleDensity", value ) )
    {
        precipitationEffect->setMaximumParticleDensity( value );
    }

    osg::vec3 cellSize;
    while( arguments.read( "--cellSize", cellSize.x, cellSize.y, cellSize.z ) )
    {
        precipitationEffect->setCellSize( cellSize );
    }

    osg::box bb;
    while( arguments.read( "--boundingBox",
                           bb.min.x,
                           bb.min.y,
                           bb.min.z,
                           bb.max.x,
                           bb.max.y,
                           bb.max.z ) )
    {
    }

    while( arguments.read( "--fogDensity", value ) )
    {
        precipitationEffect->setFogDensity( value );
    }
    while( arguments.read( "--fogColor", color.r, color.g, color.b, color.a ) )
    {
        precipitationEffect->setFogColor( color );
    }
    while( arguments.read( "--fogColour", color.r, color.g, color.b, color.a ) )
    {
        precipitationEffect->setFogColor( color );
    }

    while( arguments.read( "--useFarLineSegments" ) )
    {
        precipitationEffect->setUseFarLineSegments( true );
    }

    viewer.getCamera()->setClearColor( precipitationEffect->getFogColor() );

    // if user request help write it out to cout.
    if( arguments.read( "-h" ) || arguments.read( "--help" ) )
    {
        arguments.getApplicationUsage()->write( std::cout );
        return 1;
    }

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles( arguments );
    if( !loadedModel )
    {
        std::cout << arguments.getApplicationName() << ": No data loaded" << std::endl;
        return 1;
    }

    // precipitationEffect->setUpdateCallback(new MyGustCallback);

    osg::ref_ptr<osg::Group> group = new osg::Group;

    group->addChild( precipitationEffect.get() );

    group->addChild( loadedModel.get() );

    // create the light
    osg::LightSource* lightSource = new osg::LightSource;
    group->addChild( lightSource );

    osg::Light* light = lightSource->getLight();
    light->setLightNum( 0 );
    light->setPosition(
        osg::vec4( 0.0F, 0.0F, 1.0F, 0.0F )
    );    // directional light from above
    light->setAmbient( osg::vec4( 0.8F, 0.8F, 0.8F, 1.0F ) );
    light->setDiffuse( osg::vec4( 0.2F, 0.2F, 0.2F, 1.0F ) );
    light->setSpecular( osg::vec4( 0.2F, 0.2F, 0.2F, 1.0F ) );

    // set the scene to render
    viewer.setSceneData( group.get() );
    return viewer.run();
}
