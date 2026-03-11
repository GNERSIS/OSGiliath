/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgbillboard example application
 */
#include <osg/core/Notify.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Billboard.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/nodes/Node.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/LineWidth.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/registry/Registry.hpp>
#include <osgGA/manipulators/DriveManipulator.hpp>
#include <osgGA/manipulators/FlightManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgViewer/core/Viewer.hpp>

//
// A simple demo demonstrating different texturing modes,
// including using of texture extensions.
//

typedef std::vector<osg::ref_ptr<osg::Image>> ImageList;

/** create quad at specified position. */
osg::Drawable*
createSquare( const osg::vec3&         corner,
              const osg::vec3&         width,
              const osg::vec3&         height,
              osg::ref_ptr<osg::Image> image )
{
    // set up the Geometry.
    osg::ref_ptr<osg::Geometry> geom   = new osg::Geometry;

    osg::Vec3Array*             coords = new osg::Vec3Array( 4 );
    ( *coords )[0]                     = corner;
    ( *coords )[1]                     = corner + width;
    ( *coords )[2]                     = corner + width + height;
    ( *coords )[3]                     = corner + height;

    geom->setVertexArray( coords );

    osg::Vec3Array* norms = new osg::Vec3Array( 1 );
    ( *norms )[0]         = osg::cross( width, height );
    ( *norms )[0]         = osg::normalize( ( *norms )[0] );

    geom->setNormalArray( norms, osg::Array::BIND_OVERALL );

    osg::Vec2Array* tcoords = new osg::Vec2Array( 4 );
    ( *tcoords )[0].set( 0.0F, 0.0F );
    ( *tcoords )[1].set( 1.0F, 0.0F );
    ( *tcoords )[2].set( 1.0F, 1.0F );
    ( *tcoords )[3].set( 0.0F, 1.0F );
    geom->setTexCoordArray( 0, tcoords );

    {
        osg::DrawElementsUShort* indices = new osg::DrawElementsUShort( GL_TRIANGLES );
        // Quad (0,1,2,3) -> triangles (0,1,3) and (1,2,3)
        indices->push_back( 0 );
        indices->push_back( 1 );
        indices->push_back( 3 );
        indices->push_back( 1 );
        indices->push_back( 2 );
        indices->push_back( 3 );
        geom->addPrimitiveSet( indices );
    }

    if( image )
    {
        osg::StateSet*  stateset = new osg::StateSet;
        osg::Texture2D* texture  = new osg::Texture2D;
        texture->setImage( image );
        stateset->setTextureAttributeAndModes( 0, texture, osg::StateAttribute::ON );
        geom->setStateSet( stateset );
    }

    return geom.release();
}

osg::Drawable*
createAxis( const osg::vec3& corner,
            const osg::vec3& xdir,
            const osg::vec3& ydir,
            const osg::vec3& zdir )
{
    // set up the Geometry.
    osg::ref_ptr<osg::Geometry> geom   = new osg::Geometry;

    osg::Vec3Array*             coords = new osg::Vec3Array( 6 );
    ( *coords )[0]                     = corner;
    ( *coords )[1]                     = corner + xdir;
    ( *coords )[2]                     = corner;
    ( *coords )[3]                     = corner + ydir;
    ( *coords )[4]                     = corner;
    ( *coords )[5]                     = corner + zdir;

    geom->setVertexArray( coords );

    osg::vec4       x_color( 0.0F, 1.0F, 1.0F, 1.0F );
    osg::vec4       y_color( 0.0F, 1.0F, 1.0F, 1.0F );
    osg::vec4       z_color( 1.0F, 0.0F, 0.0F, 1.0F );

    osg::Vec4Array* color = new osg::Vec4Array( 6 );
    ( *color )[0]         = x_color;
    ( *color )[1]         = x_color;
    ( *color )[2]         = y_color;
    ( *color )[3]         = y_color;
    ( *color )[4]         = z_color;
    ( *color )[5]         = z_color;

    geom->setColorArray( color, osg::Array::BIND_PER_VERTEX );

    geom->addPrimitiveSet( new osg::DrawArrays( osg::PrimitiveSet::LINES, 0, 6 ) );

    osg::StateSet*  stateset  = new osg::StateSet;
    osg::LineWidth* linewidth = new osg::LineWidth();
    linewidth->setWidth( 4.0F );
    stateset->setAttributeAndModes( linewidth, osg::StateAttribute::ON );
    geom->setStateSet( stateset );

    return geom.release();
}

osg::ref_ptr<osg::Node>
createModel()
{

    // create the root node which will hold the model.
    osg::ref_ptr<osg::Group> root = new osg::Group();

    // add the drawable into a single geode to be shared...
    osg::Billboard*          center = new osg::Billboard();
    center->setMode( osg::Billboard::POINT_ROT_EYE );
    center->addDrawable( createSquare( osg::vec3( -0.5F, 0.0F, -0.5F ),
                                       osg::vec3( 1.0F, 0.0F, 0.0F ),
                                       osg::vec3( 0.0F, 0.0F, 1.0F ),
                                       osgDB::readRefImageFile( "Images/reflect.rgb" ) ),
                         osg::vec3( 0.0F, 0.0F, 0.0F ) );

    osg::Billboard* x_arrow = new osg::Billboard();
    x_arrow->setMode( osg::Billboard::AXIAL_ROT );
    x_arrow->setAxis( osg::vec3( 1.0F, 0.0F, 0.0F ) );
    x_arrow->setNormal( osg::vec3( 0.0F, -1.0F, 0.0F ) );
    x_arrow->addDrawable(
        createSquare( osg::vec3( -0.5F, 0.0F, -0.5F ),
                      osg::vec3( 1.0F, 0.0F, 0.0F ),
                      osg::vec3( 0.0F, 0.0F, 1.0F ),
                      osgDB::readRefImageFile( "Cubemap_axis/posx.png" ) ),
        osg::vec3( 5.0F, 0.0F, 0.0F )
    );

    osg::Billboard* y_arrow = new osg::Billboard();
    y_arrow->setMode( osg::Billboard::AXIAL_ROT );
    y_arrow->setAxis( osg::vec3( 0.0F, 1.0F, 0.0F ) );
    y_arrow->setNormal( osg::vec3( 1.0F, 0.0F, 0.0F ) );
    y_arrow->addDrawable(
        createSquare( osg::vec3( 0.0F, -0.5F, -0.5F ),
                      osg::vec3( 0.0F, 1.0F, 0.0F ),
                      osg::vec3( 0.0F, 0.0F, 1.0F ),
                      osgDB::readRefImageFile( "Cubemap_axis/posy.png" ) ),
        osg::vec3( 0.0F, 5.0F, 0.0F )
    );

    osg::Billboard* z_arrow = new osg::Billboard();
    z_arrow->setMode( osg::Billboard::AXIAL_ROT );
    z_arrow->setAxis( osg::vec3( 0.0F, 0.0F, 1.0F ) );
    z_arrow->setNormal( osg::vec3( 0.0F, -1.0F, 0.0F ) );
    z_arrow->addDrawable(
        createSquare( osg::vec3( -0.5F, 0.0F, -0.5F ),
                      osg::vec3( 1.0F, 0.0F, 0.0F ),
                      osg::vec3( 0.0F, 0.0F, 1.0F ),
                      osgDB::readRefImageFile( "Cubemap_axis/posz.png" ) ),
        osg::vec3( 0.0F, 0.0F, 5.0F )
    );

    osg::Geode* axis = new osg::Geode();
    axis->addDrawable( createAxis( osg::vec3( 0.0F, 0.0F, 0.0F ),
                                   osg::vec3( 5.0F, 0.0F, 0.0F ),
                                   osg::vec3( 0.0F, 5.0F, 0.0F ),
                                   osg::vec3( 0.0F, 0.0F, 5.0F ) ) );

    root->addChild( center );
    root->addChild( x_arrow );
    root->addChild( y_arrow );
    root->addChild( z_arrow );
    root->addChild( axis );

    return root;
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

    osgViewer::Viewer viewer;

    // set the scene to render
    viewer.setSceneData( createModel() );

    // run the viewers frame loop
    return viewer.run();
}
