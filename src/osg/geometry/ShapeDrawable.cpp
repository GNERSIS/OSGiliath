/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Drawable that renders procedural shapes (sphere, box, cylinder,
 * cone, capsule) by tessellating them into triangle geometry.
 */
#include <osg/geometry/ShapeDrawable.hpp>

#include <osg/core/Notify.hpp>
#include <osg/geometry/KdTree.hpp>
#include <osg/GL>

using namespace osg;

ShapeDrawable::ShapeDrawable() :
    _color( 1.0F,
            1.0F,
            1.0F,
            1.0F )
{
}

ShapeDrawable::ShapeDrawable( Shape*             shape,
                              TessellationHints* hints ) :
    _color( 1.0F,
            1.0F,
            1.0F,
            1.0F ),
    _tessellationHints( hints )
{
    setShape( shape );
}

ShapeDrawable::ShapeDrawable( const ShapeDrawable& sd,
                              const CopyOp&        copyop ) :
    Geometry( sd,
              copyop ),
    _color( sd._color ),
    _tessellationHints( sd._tessellationHints )
{
}

ShapeDrawable::~ShapeDrawable()
{
}

void
ShapeDrawable::setShape( Shape* shape )
{
    if( _shape == shape )
    {
        return;
    }

    _shape = shape;

    build();
}

void
ShapeDrawable::setColor( const vec4& color )
{
    _color            = color;

    Vec4Array* colors = dynamic_cast<Vec4Array*>( _colorArray.get() );
    if( !colors || colors->empty() || colors->getBinding() != Array::BIND_OVERALL )
    {
        _colorArray = colors = new Vec4Array( Array::BIND_OVERALL, 1 );
    }

    ( *colors )[0] = color;
    colors->dirty();

    dirtyGLObjects();
}

void
ShapeDrawable::setTessellationHints( TessellationHints* hints )
{
    if( _tessellationHints != hints )
    {
        _tessellationHints = hints;
        build();
    }
}

void
ShapeDrawable::build()
{
    // we can't create a tessellation for a KdTree
    if( dynamic_cast<KdTree*>( _shape.get() ) != 0 )
    {
        return;
    }

    // reset all the properties.
    setVertexArray( 0 );
    setNormalArray( 0 );
    setColorArray( 0 );
    setSecondaryColorArray( 0 );
    setFogCoordArray( 0 );
    getTexCoordArrayList().clear();
    getVertexAttribArrayList().clear();
    getPrimitiveSetList().clear();

    if( _shape )
    {
        BuildShapeGeometryVisitor dsv( this, _tessellationHints.get() );
        _shape->accept( dsv );
    }

    setColor( _color );
}
