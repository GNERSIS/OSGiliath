/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osggameoflife example application
 */
#include "GameOfLifePass.hpp"

#include <iostream>
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/textures/TextureRectangle.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

GameOfLifePass*             golpass;
osg::ref_ptr<osg::StateSet> geomss;    // stateset where we can attach textures

osg::Node*
createScene( osg::Image* start_im )
{
    int                                   width   = start_im->s();
    int                                   height  = start_im->t();

    osg::Group*                           topnode = new osg::Group;

    // create quad to display image on
    osg::ref_ptr<osg::Geode>              geode = new osg::Geode();

    // each geom will contain a quad
    osg::ref_ptr<osg::DrawElementsUShort> da =
        new osg::DrawElementsUShort( GL_TRIANGLES );
    // Quad (0,1,2,3) -> triangles (0,1,3) and (1,2,3)
    da->push_back( 0 );
    da->push_back( 1 );
    da->push_back( 3 );
    da->push_back( 1 );
    da->push_back( 2 );
    da->push_back( 3 );

    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
    colors->push_back( osg::vec4( 1.0F, 1.0F, 1.0F, 1.0F ) );

    osg::ref_ptr<osg::Vec2Array> tcoords = new osg::Vec2Array;    // texture coords
    tcoords->push_back( osg::vec2( 0, 0 ) );
    tcoords->push_back( osg::vec2( width, 0 ) );
    tcoords->push_back( osg::vec2( width, height ) );
    tcoords->push_back( osg::vec2( 0, height ) );

    osg::ref_ptr<osg::Vec3Array> vcoords = new osg::Vec3Array;    // vertex coords
    osg::ref_ptr<osg::Geometry>  geom    = new osg::Geometry;

    // initial viewer camera looks along y
    vcoords->push_back( osg::vec3( 0, 0, 0 ) );
    vcoords->push_back( osg::vec3( width, 0, 0 ) );
    vcoords->push_back( osg::vec3( width, 0, height ) );
    vcoords->push_back( osg::vec3( 0, 0, height ) );

    geom->setVertexArray( vcoords.get() );
    geom->setTexCoordArray( 0, tcoords.get() );
    geom->addPrimitiveSet( da.get() );
    geom->setColorArray( colors.get(), osg::Array::BIND_OVERALL );
    geomss = geom->getOrCreateStateSet();

    geode->addDrawable( geom.get() );

    topnode->addChild( geode.get() );

    // create the ping pong processing passes
    golpass = new GameOfLifePass( start_im );
    topnode->addChild( golpass->getRoot().get() );

    // attach the output of the processing to the geom
    geomss->setTextureAttributeAndModes( 0,
                                         golpass->getOutputTexture().get(),
                                         osg::StateAttribute::ON );
    return topnode;
}

int
main( int   argc,
      char* argv[] )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc, argv );

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(
        arguments.getApplicationName() +
        " is the example which demonstrates ping pong rendering with FBOs and multiple "
        "rendering branches. It uses Conway's Game of Life to illustrate the concept."
    );
    arguments.getApplicationUsage()->setCommandLineUsage(
        arguments.getApplicationName() + " [options] --startim start_image"
    );
    arguments.getApplicationUsage()->addCommandLineOption( "-h or --help",
                                                           "Display this information" );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--startim",
        "The initial image to seed the game of life with."
    );

    // if user request help write it out to cout.
    if( arguments.read( "-h" ) || arguments.read( "--help" ) )
    {
        arguments.getApplicationUsage()->write( std::cout );
        return 1;
    }

    std::string startName( "" );
    while( arguments.read( "--startim", startName ) )
    {
    }

    if( startName == "" )
    {
        arguments.getApplicationUsage()->write( std::cout );
        return 1;
    }

    // load the image
    osg::ref_ptr<osg::Image> startIm = osgDB::readRefImageFile( startName );

    if( !startIm )
    {
        std::cout << "Could not load start image.\n";
        return ( 1 );
    }

    osg::ref_ptr<osg::Node> scene = createScene( startIm.get() );

    // construct the viewer.
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
    viewer.setThreadingModel( osgViewer::Viewer::SingleThreaded );

    // add the stats handler
    viewer.addEventHandler( new osgViewer::StatsHandler );

    viewer.setSceneData( scene );

    viewer.realize();
    viewer.setCameraManipulator( new osgGA::TrackballManipulator );

    while( !viewer.done() )
    {
        viewer.frame();
        // flip the textures after we've completed a frame
        golpass->flip();
        // attach the proper output to view
        geomss->setTextureAttributeAndModes( 0,
                                             golpass->getOutputTexture().get(),
                                             osg::StateAttribute::ON );
    }

    return 0;
}
