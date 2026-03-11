/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * base example application
 */
#include <math.h>
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/state/Depth.hpp>
#include <osg/state/StateSet.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/ReadFile.hpp>

using namespace osg;

Node*
makeBase( void )
{
    int        i, c;
    float      theta;
    float      ir      = 20.0F;

    Vec3Array* coords  = new Vec3Array( 19 );
    Vec2Array* tcoords = new Vec2Array( 19 );
    Vec4Array* colors  = new Vec4Array( 1 );

    ( *colors )[0].set( 1.0F, 1.0F, 1.0F, 1.0F );

    c = 0;
    ( *coords )[c].set( 0.0F, 0.0F, 0.0F );
    ( *tcoords )[c].set( 0.0F, 0.0F );

    for( i = 0; i <= 18; i++ )
    {
        theta = osg::radians( ( float )i * 20.0 );

        ( *coords )[c].set( ir * cosf( theta ), ir * sinf( theta ), 0.0F );
        ( *tcoords )[c].set( ( *coords )[c][0] / 36.0F, ( *coords )[c][1] / 36.0F );

        c++;
    }

    Geometry* geom = new Geometry;

    geom->setVertexArray( coords );

    geom->setTexCoordArray( 0, tcoords );

    geom->setColorArray( colors, Array::BIND_OVERALL );

    geom->addPrimitiveSet( new DrawArrays( PrimitiveSet::TRIANGLE_FAN, 0, 19 ) );

    Texture2D* tex = new Texture2D;

    tex->setImage( osgDB::readRefImageFile( "Images/water.rgb" ) );
    tex->setWrap( Texture2D::WRAP_S, Texture2D::REPEAT );
    tex->setWrap( Texture2D::WRAP_T, Texture2D::REPEAT );

    StateSet* dstate = new StateSet;
    dstate->setTextureAttributeAndModes( 0, tex, StateAttribute::ON );

    // clear the depth to the far plane.
    osg::Depth* depth = new osg::Depth;
    depth->setFunction( osg::Depth::Function::ALWAYS );
    depth->setRange( 1.0, 1.0 );
    dstate->setAttributeAndModes( depth, StateAttribute::ON );

    dstate->setRenderBinDetails( -1, "RenderBin" );

    geom->setStateSet( dstate );

    Geode* geode = new Geode;
    geode->addDrawable( geom );

    return geode;
}
