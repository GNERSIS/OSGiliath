/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgdepthpartition example application
 */
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/PositionAttitudeTransform.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgUtil/culling/UpdateVisitor.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

const double r_earth = 6378.137;
const double r_sun   = 695990.0;
const double AU      = 149697900.0;

osg::Node*
createScene()
{
    // Create the Earth, in blue
    osg::ShapeDrawable* earth_sd     = new osg::ShapeDrawable;
    osg::Sphere*        earth_sphere = new osg::Sphere;
    earth_sphere->setName( "EarthSphere" );
    earth_sphere->setRadius( r_earth );
    earth_sd->setShape( earth_sphere );
    earth_sd->setColor( osg::vec4( 0, 0, 1.0, 1.0 ) );

    osg::Geode* earth_geode = new osg::Geode;
    earth_geode->setName( "EarthGeode" );
    earth_geode->addDrawable( earth_sd );

    // Create the Sun, in yellow
    osg::ShapeDrawable* sun_sd     = new osg::ShapeDrawable;
    osg::Sphere*        sun_sphere = new osg::Sphere;
    sun_sphere->setName( "SunSphere" );
    sun_sphere->setRadius( r_sun );
    sun_sd->setShape( sun_sphere );
    sun_sd->setColor( osg::vec4( 1.0, 0.0, 0.0, 1.0 ) );

    osg::Geode* sun_geode = new osg::Geode;
    sun_geode->setName( "SunGeode" );
    sun_geode->addDrawable( sun_sd );

    // Move the sun behind the earth
    osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
    pat->setPosition( osg::dvec3( 0.0, AU, 0.0 ) );
    pat->addChild( sun_geode );

    osg::Geometry* unitCircle = new osg::Geometry();
    {
        osg::Vec4Array* colours = new osg::Vec4Array( 1 );
        ( *colours )[0]         = osg::dvec4( 1.0, 1.0, 1.0, 1.0 );
        unitCircle->setColorArray( colours, osg::Array::BIND_OVERALL );
        const unsigned int n_points = 1'024;
        osg::Vec3Array*    coords   = new osg::Vec3Array( n_points );
        const double       dx       = 2.0 * osg::PI / n_points;
        double             s, c;
        for( unsigned int j = 0; j < n_points; ++j )
        {
            s = sin( dx * j );
            c = cos( dx * j );
            ( *coords )[j].set( ( float )c, ( float )s, 0.0F );
        }
        unitCircle->setVertexArray( coords );
        unitCircle->addPrimitiveSet(
            new osg::DrawArrays( osg::PrimitiveSet::LINE_LOOP, 0, n_points )
        );
    }

    osg::Geometry* axes = new osg::Geometry;
    {
        osg::Vec4Array* colours = new osg::Vec4Array( 1 );
        ( *colours )[0]         = osg::vec4( 1.0F, 0.0F, 0.0F, 1.0F );
        axes->setColorArray( colours, osg::Array::BIND_OVERALL );
        osg::Vec3Array* coords = new osg::Vec3Array( 6 );
        ( *coords )[0].set( 0.0F, 0.0F, 0.0F );
        ( *coords )[1].set( 0.5F, 0.0F, 0.0F );
        ( *coords )[2].set( 0.0F, 0.0F, 0.0F );
        ( *coords )[3].set( 0.0F, 0.5F, 0.0F );
        ( *coords )[4].set( 0.0F, 0.0F, 0.0F );
        ( *coords )[5].set( 0.0F, 0.0F, 0.5F );
        axes->setVertexArray( coords );
        axes->addPrimitiveSet( new osg::DrawArrays( osg::PrimitiveSet::LINES, 0, 6 ) );
    }

    // Earth orbit
    osg::Geode* earthOrbitGeode = new osg::Geode;
    earthOrbitGeode->addDrawable( unitCircle );
    earthOrbitGeode->addDrawable( axes );
    earthOrbitGeode->setName( "EarthOrbitGeode" );

    osg::PositionAttitudeTransform* earthOrbitPAT = new osg::PositionAttitudeTransform;
    earthOrbitPAT->setScale( osg::dvec3( AU, AU, AU ) );
    earthOrbitPAT->setPosition( osg::dvec3( 0.0, AU, 0.0 ) );
    earthOrbitPAT->addChild( earthOrbitGeode );
    earthOrbitPAT->setName( "EarthOrbitPAT" );

    osg::Group* scene = new osg::Group;
    scene->setName( "SceneGroup" );
    scene->addChild( earth_geode );
    scene->addChild( pat );
    scene->addChild( earthOrbitPAT );

    return scene;
}

int
main( int    argc,
      char** argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc, argv );

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

    osgViewer::Viewer viewer( arguments );

    // add the state manipulator
    viewer.addEventHandler(
        new osgGA::StateSetManipulator( viewer.getCamera()->getOrCreateStateSet() )
    );

    // add stats
    viewer.addEventHandler( new osgViewer::StatsHandler() );

    bool                    needToSetHomePosition = false;

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> scene = osgDB::readRefNodeFiles( arguments );

    // if one hasn't been loaded create an earth and sun test model.
    if( !scene )
    {
        scene                 = createScene();
        needToSetHomePosition = true;
    }
    // pass the loaded scene graph to the viewer.
    viewer.setSceneData( scene.get() );

    viewer.setCameraManipulator( new osgGA::TrackballManipulator );

    if( needToSetHomePosition )
    {
        viewer.getCameraManipulator()->setHomePosition(
            osg::dvec3( 0.0, -5.0 * r_earth, 0.0 ),
            osg::dvec3( 0.0, 0.0, 0.0 ),
            osg::dvec3( 0.0, 0.0, 1.0 )
        );
    }

    double zNear = 1.0, zMid = 10.0, zFar = 1000.0;
    if( arguments.read( "--depth-partition", zNear, zMid, zFar ) )
    {
        // set up depth partitioning
        osg::ref_ptr<osgViewer::DepthPartitionSettings> dps =
            new osgViewer::DepthPartitionSettings;
        dps->_mode  = osgViewer::DepthPartitionSettings::FIXED_RANGE;
        dps->_zNear = zNear;
        dps->_zMid  = zMid;
        dps->_zFar  = zFar;
        viewer.setUpDepthPartition( dps.get() );
    }
    else
    {
        // set up depth partitioning with default settings
        viewer.setUpDepthPartition();
    }

    return viewer.run();
}
