/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgshape example application
 */
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/state/Material.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/io/WriteFile.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgUtil/utils/ShaderGen.hpp>
#include <osgViewer/core/Viewer.hpp>

// for the grid data..
#include "../osghangglide/terrain_coords.hpp"

#include <osg/rendering/HeadlessCapture.hpp>

osg::Geode*
createShapes( osg::ArgumentParser& arguments )
{
    osg::Geode*              geode = new osg::Geode();

    // ---------------------------------------
    // Set up a StateSet to texture the objects
    // ---------------------------------------
    osg::StateSet*           stateset = new osg::StateSet();

    osg::ref_ptr<osg::Image> image    = osgDB::readRefImageFile( "Images/lz.rgb" );
    if( image )
    {
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage( image );
        texture->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR );
        stateset->setTextureAttributeAndModes( 0, texture, osg::StateAttribute::ON );
    }

    geode->setStateSet( stateset );

    float                   radius = 0.8F;
    float                   height = 1.0F;

    osg::TessellationHints* hints  = new osg::TessellationHints;
    hints->setDetailRatio( 0.5F );

    geode->addDrawable(
        new osg::ShapeDrawable( new osg::Sphere( osg::vec3( 0.0F, 0.0F, 0.0F ), radius ),
                                hints )
    );
    geode->addDrawable( new osg::ShapeDrawable(
        new osg::Box( osg::vec3( 2.0F, 0.0F, 0.0F ), 2 * radius ),
        hints
    ) );
    geode->addDrawable( new osg::ShapeDrawable(
        new osg::Cone( osg::vec3( 4.0F, 0.0F, 0.0F ), radius, height ),
        hints
    ) );
    geode->addDrawable( new osg::ShapeDrawable(
        new osg::Cylinder( osg::vec3( 6.0F, 0.0F, 0.0F ), radius, height ),
        hints
    ) );
    geode->addDrawable( new osg::ShapeDrawable(
        new osg::Capsule( osg::vec3( 8.0F, 0.0F, 0.0F ), radius, height ),
        hints
    ) );

    osg::HeightField* grid = new osg::HeightField;
    if( arguments.read( "--large" ) )
    {
        unsigned int numX  = 512;
        unsigned int numY  = 512;
        double       sizeX = 10.0;
        double       sizeY = 10.0;
        grid->allocate( numX, numY );
        grid->setXInterval( sizeX / float( numX ) );
        grid->setYInterval( sizeY / float( numY ) );

        for( unsigned int r = 0; r < numY; ++r )
        {
            for( unsigned int c = 0; c < numX; ++c )
            {
                double rx = double( c ) / double( numX - 1 );
                double ry = double( r ) / double( numY - 1 );

                grid->setHeight( c, r, 2.0 * sin( rx * ry * 4.0 * osg::PI ) );
            }
        }
    }
    else
    {
        grid->allocate( 38, 39 );
        grid->setXInterval( 0.28F );
        grid->setYInterval( 0.28F );

        for( unsigned int r = 0; r < 39; ++r )
        {
            for( unsigned int c = 0; c < 38; ++c )
            {
                grid->setHeight( c, r, vertex[r + c * 39][2] );
            }
        }
    }

    geode->addDrawable( new osg::ShapeDrawable( grid ) );

    osg::ConvexHull* mesh     = new osg::ConvexHull;
    osg::Vec3Array*  vertices = new osg::Vec3Array( 4 );
    ( *vertices )[0].set( 9.0 + 0.0F, -1.0F + 2.0F, -1.0F + 0.0F );
    ( *vertices )[1].set( 9.0 + 1.0F, -1.0F + 0.0F, -1.0F + 0.0F );
    ( *vertices )[2].set( 9.0 + 2.0F, -1.0F + 2.0F, -1.0F + 0.0F );
    ( *vertices )[3].set( 9.0 + 1.0F, -1.0F + 1.0F, -1.0F + 2.0F );
    osg::UByteArray* indices = new osg::UByteArray( 12 );
    ( *indices )[0]          = 0;
    ( *indices )[1]          = 2;
    ( *indices )[2]          = 1;
    ( *indices )[3]          = 0;
    ( *indices )[4]          = 1;
    ( *indices )[5]          = 3;
    ( *indices )[6]          = 1;
    ( *indices )[7]          = 2;
    ( *indices )[8]          = 3;
    ( *indices )[9]          = 2;
    ( *indices )[10]         = 0;
    ( *indices )[11]         = 3;
    mesh->setVertices( vertices );
    mesh->setIndices( indices );
    geode->addDrawable( new osg::ShapeDrawable( mesh ) );

    return geode;
}

int
main( int    argc,
      char** argv )
{
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

    // add model to viewer.
    viewer.setSceneData( createShapes( arguments ) );

    // add the state manipulator
    viewer.addEventHandler(
        new osgGA::StateSetManipulator( viewer.getCamera()->getOrCreateStateSet() )
    );
    return viewer.run();
}
