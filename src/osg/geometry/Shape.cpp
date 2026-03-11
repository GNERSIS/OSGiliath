/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Procedural shape definitions (Sphere, Box, Cylinder, Cone, Capsule,
 * HeightField). Used with ShapeDrawable for simple geometry.
 */
#include <osg/geometry/Shape.hpp>

#include <algorithm>
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/maths/transform.hpp>

using namespace osg;

Shape::~Shape()
{
}

ShapeVisitor::~ShapeVisitor()
{
}

ConstShapeVisitor::~ConstShapeVisitor()
{
}

Sphere::~Sphere()
{
}

Box::~Box()
{
}

Cone::~Cone()
{
}

Cylinder::~Cylinder()
{
}

Capsule::~Capsule()
{
}

InfinitePlane::~InfinitePlane()
{
}

TriangleMesh::~TriangleMesh()
{
}

ConvexHull::~ConvexHull()
{
}

HeightField::HeightField() :
    _columns( 0 ),
    _rows( 0 ),
    _origin( 0.0F,
             0.0F,
             0.0F ),
    _dx( 1.0F ),
    _dy( 1.0F ),
    _skirtHeight( 0.0F ),
    _borderWidth( 0 )
{
    _heights = new FloatArray;
}

HeightField::HeightField( const HeightField& mesh,
                          const CopyOp&      copyop ) :
    Inherit( mesh,
             copyop ),
    _columns( mesh._columns ),
    _rows( mesh._rows ),
    _origin( mesh._origin ),
    _dx( mesh._dx ),
    _dy( mesh._dy ),
    _skirtHeight( mesh._skirtHeight ),
    _borderWidth( mesh._borderWidth ),
    _heights( new FloatArray( *mesh._heights ) )
{
}

HeightField::~HeightField()
{
}

void
HeightField::allocate( unsigned int numColumns,
                       unsigned int numRows )
{
    if( _columns != numColumns || _rows != numRows )
    {
        _heights->resize( numColumns * numRows );
    }
    _columns = numColumns;
    _rows    = numRows;
}

vec3
HeightField::getNormal( unsigned int c,
                        unsigned int r ) const
{
    // four point normal generation.
    float dz_dx;
    if( c == 0 )
    {
        dz_dx = ( getHeight( c + 1, r ) - getHeight( c, r ) ) / getXInterval();
    }
    else if( c == getNumColumns() - 1 )
    {
        dz_dx = ( getHeight( c, r ) - getHeight( c - 1, r ) ) / getXInterval();
    }
    else    // assume 0<c<_numColumns-1
    {
        dz_dx =
            0.5F * ( getHeight( c + 1, r ) - getHeight( c - 1, r ) ) / getXInterval();
    }

    float dz_dy;
    if( r == 0 )
    {
        dz_dy = ( getHeight( c, r + 1 ) - getHeight( c, r ) ) / getYInterval();
    }
    else if( r == getNumRows() - 1 )
    {
        dz_dy = ( getHeight( c, r ) - getHeight( c, r - 1 ) ) / getYInterval();
    }
    else    // assume 0<r<_numRows-1
    {
        dz_dy =
            0.5F * ( getHeight( c, r + 1 ) - getHeight( c, r - 1 ) ) / getYInterval();
    }

    vec3 normal( -dz_dx, -dz_dy, 1.0F );
    normal = osg::normalize( normal );

    return normal;
}

vec2
HeightField::getHeightDelta( unsigned int c,
                             unsigned int r ) const
{
    // four point height generation.
    vec2 heightDelta;
    if( c == 0 )
    {
        heightDelta.x = ( getHeight( c + 1, r ) - getHeight( c, r ) );
    }
    else if( c == getNumColumns() - 1 )
    {
        heightDelta.x = ( getHeight( c, r ) - getHeight( c - 1, r ) );
    }
    else    // assume 0<c<_numColumns-1
    {
        heightDelta.x = 0.5F * ( getHeight( c + 1, r ) - getHeight( c - 1, r ) );
    }

    if( r == 0 )
    {
        heightDelta.y = ( getHeight( c, r + 1 ) - getHeight( c, r ) );
    }
    else if( r == getNumRows() - 1 )
    {
        heightDelta.y = ( getHeight( c, r ) - getHeight( c, r - 1 ) );
    }
    else    // assume 0<r<_numRows-1
    {
        heightDelta.y = 0.5F * ( getHeight( c, r + 1 ) - getHeight( c, r - 1 ) );
    }

    return heightDelta;
}

CompositeShape::~CompositeShape()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
// BuildShapeGeometryVisitor
//

// arbitrary minima for rows & segments
const unsigned int MIN_NUM_ROWS     = 3;
const unsigned int MIN_NUM_SEGMENTS = 5;

BuildShapeGeometryVisitor::BuildShapeGeometryVisitor( Geometry*                geometry,
                                                      const TessellationHints* hints ) :
    _geometry( geometry ),
    _hints( hints )
{
    _vertices  = dynamic_cast<Vec3Array*>( geometry->getVertexArray() );
    _normals   = dynamic_cast<Vec3Array*>( geometry->getNormalArray() );
    _texcoords = dynamic_cast<Vec2Array*>( geometry->getTexCoordArray( 0 ) );

    bool requiresClearOfPrimitiveSets = false;

    if( !_vertices || _vertices->getBinding() != Array::BIND_PER_VERTEX )
    {
        requiresClearOfPrimitiveSets = true;
        _vertices                    = new Vec3Array( Array::BIND_PER_VERTEX );
        _geometry->setVertexArray( _vertices.get() );
    }

    if( !_normals || ( _normals->getBinding() !=
                       Array::BIND_PER_VERTEX ||
                       _vertices->size() != _normals->size() ) )
    {
        requiresClearOfPrimitiveSets = true;
        _normals                     = new Vec3Array( Array::BIND_PER_VERTEX );
        _geometry->setNormalArray( _normals.get() );
    }

    if( !_texcoords || ( _texcoords->getBinding() !=
                         Array::BIND_PER_VERTEX ||
                         _vertices->size() != _texcoords->size() ) )
    {
        requiresClearOfPrimitiveSets = true;
        _texcoords                   = new Vec2Array( Array::BIND_PER_VERTEX );
        _geometry->setTexCoordArray( 0, _texcoords.get() );
    }

    if( requiresClearOfPrimitiveSets && !_geometry->getPrimitiveSetList().empty() )
    {
        OSG_NOTICE << "Warning: BuildShapeGeometryVisitor() Geometry contains "
                      "compatible arrays, resetting before shape build."
                   << std::endl;
        _geometry->getPrimitiveSetList().clear();
    }

    _mode        = 0;
    _start_index = 0;
}

void
BuildShapeGeometryVisitor::setMatrix( const dmat4& m )
{
    _matrix  = m;

    _inverse = osg::inverse( m );
    osg::setTrans( _inverse, 0.0, 0.0, 0.0 );
}

void
BuildShapeGeometryVisitor::Vertex( const vec3& v )
{
    _vertices->push_back( v );
    if( _normals.valid() && _normals->size() < _vertices->size() )
    {
        while( _normals->size() < _vertices->size() )
        {
            _normals->push_back( osg::vec3( 0.0F, 0.0F, 1.0F ) );
        }
    }
    if( _texcoords.valid() && _texcoords->size() < _vertices->size() )
    {
        while( _texcoords->size() < _vertices->size() )
        {
            _texcoords->push_back( osg::vec2( 0.0F, 0.0F ) );
        }
    }
}

void
BuildShapeGeometryVisitor::Begin( GLenum mode )
{
    _mode        = mode;
    _start_index = static_cast<unsigned int>( _vertices->size() );
}

void
BuildShapeGeometryVisitor::End()
{
    if( _start_index >= _vertices->size() )
    {
        return;
    }

    bool smallPrimitiveSet = _vertices->size() < 65'536;

    // OSG_NOTICE<<"BuildShapeGeometryVisitor::End() smallPrimitiveSet =
    // "<<smallPrimitiveSet<<std::endl;
    if( _mode == GL_QUADS )
    {
        osg::ref_ptr<osg::DrawElements> primitives =
            smallPrimitiveSet ? static_cast<osg::DrawElements*>(
                                    new osg::DrawElementsUShort( GL_TRIANGLES )
                                )
                              : static_cast<osg::DrawElements*>(
                                    new osg::DrawElementsUInt( GL_TRIANGLES )
                                );

        _geometry->addPrimitiveSet( primitives.get() );

        for( unsigned int i = _start_index; i < _vertices->size(); i += 4 )
        {
            unsigned int p0 = i;
            unsigned int p1 = i + 1;
            unsigned int p2 = i + 2;
            unsigned int p3 = i + 3;

            primitives->addElement( p0 );
            primitives->addElement( p1 );
            primitives->addElement( p3 );

            primitives->addElement( p1 );
            primitives->addElement( p2 );
            primitives->addElement( p3 );
        }
    }
    else if( _mode == GL_QUAD_STRIP )
    {
        osg::ref_ptr<osg::DrawElements> primitives =
            smallPrimitiveSet ? static_cast<osg::DrawElements*>(
                                    new osg::DrawElementsUShort( GL_TRIANGLES )
                                )
                              : static_cast<osg::DrawElements*>(
                                    new osg::DrawElementsUInt( GL_TRIANGLES )
                                );

        _geometry->addPrimitiveSet( primitives.get() );

        for( unsigned int i = _start_index; i < _vertices->size() - 2; i += 2 )
        {
            unsigned int p0 = i;
            unsigned int p1 = i + 1;
            unsigned int p2 = i + 2;
            unsigned int p3 = i + 3;

            primitives->addElement( p0 );
            primitives->addElement( p1 );
            primitives->addElement( p2 );

            primitives->addElement( p1 );
            primitives->addElement( p3 );
            primitives->addElement( p2 );
        }
    }
    else
    {
        _geometry->addPrimitiveSet(
            new DrawArrays( _mode,
                            static_cast<GLint>( _start_index ),
                            static_cast<GLsizei>( _vertices->size() - _start_index ) )
        );
    }

    for( unsigned int i = _start_index; i < _vertices->size(); ++i )
    {
        vec3& v = ( *_vertices )[i];
        v       = vec3( _matrix * dvec3( v ) );

        vec3& n = ( *_normals )[i];
        n       = vec3( _inverse * dvec3( n ) );
        n       = osg::normalize( n );
    }

    _vertices->dirty();
    _normals->dirty();
    _texcoords->dirty();
    _geometry->dirtyGLObjects();

    _start_index = static_cast<unsigned int>( _vertices->size() );
}

void
BuildShapeGeometryVisitor::drawCylinderBody( unsigned int numSegments,
                                             float        radius,
                                             float        height )
{
    const float angleDelta    = 2.0F * PIf / ( float )numSegments;
    const float texCoordDelta = 1.0F / ( float )numSegments;

    const float r             = radius;
    const float h             = height;

    float       basez         = -h * 0.5F;
    float       topz          = h * 0.5F;

    float       angle         = 0.0F;
    float       texCoord      = 0.0F;

    bool        drawFrontFace = _hints ? _hints->getCreateFrontFace() : true;
    bool        drawBackFace  = _hints ? _hints->getCreateBackFace() : false;

    // The only difference between the font & back face loops is that the
    //  normals are inverted and the order of the vertex pairs is reversed.
    //  The code is mostly duplicated in order to hoist the back/front face
    //  test out of the loop for efficiency

    Begin( GL_QUAD_STRIP );

    if( drawFrontFace )
    {
        for( unsigned int bodyi = 0; bodyi < numSegments;
             ++bodyi, angle += angleDelta, texCoord += texCoordDelta )
        {
            float c = cosf( angle );
            float s = sinf( angle );
            vec3  n( c, s, 0.0F );

            Normal( n );
            TexCoord2f( texCoord, 1.0F );
            Vertex3f( c * r, s * r, topz );

            Normal( n );
            TexCoord2f( texCoord, 0.0F );
            Vertex3f( c * r, s * r, basez );
        }

        // do last point by hand to ensure no round off errors.
        vec3 n( 1.0F, 0.0F, 0.0F );

        Normal( n );
        TexCoord2f( 1.0F, 1.0F );
        Vertex3f( r, 0.0F, topz );

        Normal( n );
        TexCoord2f( 1.0F, 0.0F );
        Vertex3f( r, 0.0F, basez );
    }

    if( drawBackFace )
    {
        for( unsigned int bodyi = 0; bodyi < numSegments;
             ++bodyi, angle += angleDelta, texCoord += texCoordDelta )
        {
            float c = cosf( angle );
            float s = sinf( angle );
            vec3  n( -c, -s, 0.0F );

            Normal( n );
            TexCoord2f( texCoord, 0.0F );
            Vertex3f( c * r, s * r, basez );

            Normal( n );
            TexCoord2f( texCoord, 1.0F );
            Vertex3f( c * r, s * r, topz );
        }

        // do last point by hand to ensure no round off errors.
        vec3 n( -1.0F, 0.0F, 0.0F );

        Normal( n );
        TexCoord2f( 1.0F, 0.0F );
        Vertex3f( r, 0.0F, basez );

        Normal( n );
        TexCoord2f( 1.0F, 1.0F );
        Vertex3f( r, 0.0F, topz );
    }

    End();
}

void
BuildShapeGeometryVisitor::drawHalfSphere( unsigned int numSegments,
                                           unsigned int numRows,
                                           float        radius,
                                           SphereHalf   which,
                                           float        zOffset )
{
    float lDelta            = PIf / ( float )numRows;
    float vDelta            = 1.0F / ( float )numRows;

    bool  top               = ( which == SphereTopHalf );

    bool  drawFrontFace     = _hints ? _hints->getCreateFrontFace() : true;
    bool  drawBackFace      = _hints ? _hints->getCreateBackFace() : false;

    float angleDelta        = PIf * 2.0F / ( float )numSegments;
    float texCoordHorzDelta = 1.0F / ( float )numSegments;

    float lBase =
        -PIf * 0.5F + ( top ? ( lDelta * static_cast<float>( numRows / 2 ) ) : 0.0F );
    float        rBase = ( top ? ( cosf( lBase ) * radius ) : 0.0F );
    float        zBase = ( top ? ( sinf( lBase ) * radius ) : -radius );
    float        vBase = ( top ? ( vDelta * static_cast<float>( numRows / 2 ) ) : 0.0F );
    float        nzBase     = ( top ? ( sinf( lBase ) ) : -1.0F );
    float        nRatioBase = ( top ? ( cosf( lBase ) ) : 0.0F );

    unsigned int rowbegin   = top ? numRows / 2 : 0;
    unsigned int rowend     = top ? numRows : numRows / 2;

    for( unsigned int rowi = rowbegin; rowi < rowend; ++rowi )
    {

        float lTop      = lBase + lDelta;
        float rTop      = cosf( lTop ) * radius;
        float zTop      = sinf( lTop ) * radius;
        float vTop      = vBase + vDelta;
        float nzTop     = sinf( lTop );
        float nRatioTop = cosf( lTop );

        Begin( GL_QUAD_STRIP );

        float angle    = 0.0F;
        float texCoord = 0.0F;

        // The only difference between the font & back face loops is that the
        //  normals are inverted and the order of the vertex pairs is reversed.
        //  The code is mostly duplicated in order to hoist the back/front face
        //  test out of the loop for efficiency

        if( drawFrontFace )
        {
            for( unsigned int topi = 0; topi < numSegments;
                 ++topi, angle += angleDelta, texCoord += texCoordHorzDelta )
            {

                float c = cosf( angle );
                float s = sinf( angle );

                Normal3f( c * nRatioTop, s * nRatioTop, nzTop );
                TexCoord2f( texCoord, vTop );
                Vertex3f( c * rTop, s * rTop, zTop + zOffset );

                Normal3f( c * nRatioBase, s * nRatioBase, nzBase );
                TexCoord2f( texCoord, vBase );
                Vertex3f( c * rBase, s * rBase, zBase + zOffset );
            }

            // do last point by hand to ensure no round off errors.
            Normal3f( nRatioTop, 0.0F, nzTop );
            TexCoord2f( 1.0F, vTop );
            Vertex3f( rTop, 0.0F, zTop + zOffset );

            Normal3f( nRatioBase, 0.0F, nzBase );
            TexCoord2f( 1.0F, vBase );
            Vertex3f( rBase, 0.0F, zBase + zOffset );
        }

        if( drawBackFace )
        {
            for( unsigned int topi = 0; topi < numSegments;
                 ++topi, angle += angleDelta, texCoord += texCoordHorzDelta )
            {

                float c = cosf( angle );
                float s = sinf( angle );

                Normal3f( -c * nRatioBase, -s * nRatioBase, -nzBase );
                TexCoord2f( texCoord, vBase );
                Vertex3f( c * rBase, s * rBase, zBase + zOffset );

                Normal3f( -c * nRatioTop, -s * nRatioTop, -nzTop );
                TexCoord2f( texCoord, vTop );
                Vertex3f( c * rTop, s * rTop, zTop + zOffset );
            }

            // do last point by hand to ensure no round off errors.
            Normal3f( -nRatioBase, 0.0F, -nzBase );
            TexCoord2f( 1.0F, vBase );
            Vertex3f( rBase, 0.0F, zBase + zOffset );

            Normal3f( -nRatioTop, 0.0F, -nzTop );
            TexCoord2f( 1.0F, vTop );
            Vertex3f( rTop, 0.0F, zTop + zOffset );
        }

        End();

        lBase      = lTop;
        rBase      = rTop;
        zBase      = zTop;
        vBase      = vTop;
        nzBase     = nzTop;
        nRatioBase = nRatioTop;
    }
}

void
BuildShapeGeometryVisitor::apply( const Sphere& sphere )
{
    bool drawFrontFace = _hints ? _hints->getCreateFrontFace() : true;
    bool drawBackFace  = _hints ? _hints->getCreateBackFace() : false;

    setMatrix( osg::translate( ( double )sphere.getCenter().x,
                               ( double )sphere.getCenter().y,
                               ( double )sphere.getCenter().z ) );

    unsigned int numSegments = 40;
    unsigned int numRows     = 20;
    float        ratio       = ( _hints ? _hints->getDetailRatio() : 1.0F );
    if( ratio > 0.0F && ratio != 1.0F )
    {
        numRows = static_cast<unsigned int>( static_cast<float>( numRows ) * ratio );
        if( numRows < MIN_NUM_ROWS )
        {
            numRows = MIN_NUM_ROWS;
        }
        numSegments =
            static_cast<unsigned int>( static_cast<float>( numSegments ) * ratio );
        if( numSegments < MIN_NUM_SEGMENTS )
        {
            numSegments = MIN_NUM_SEGMENTS;
        }
    }

    float lDelta            = PIf / ( float )numRows;
    float vDelta            = 1.0F / ( float )numRows;

    float angleDelta        = PIf * 2.0F / ( float )numSegments;
    float texCoordHorzDelta = 1.0F / ( float )numSegments;

    if( drawBackFace )
    {
        float lBase      = -PIf * 0.5F;
        float rBase      = 0.0F;
        float zBase      = -sphere.getRadius();
        float vBase      = 0.0F;
        float nzBase     = -1.0F;
        float nRatioBase = 0.0F;

        for( unsigned int rowi = 0; rowi < numRows; ++rowi )
        {

            float lTop      = lBase + lDelta;
            float rTop      = cosf( lTop ) * sphere.getRadius();
            float zTop      = sinf( lTop ) * sphere.getRadius();
            float vTop      = vBase + vDelta;
            float nzTop     = sinf( lTop );
            float nRatioTop = cosf( lTop );

            Begin( GL_QUAD_STRIP );

            float angle    = 0.0F;
            float texCoord = 0.0F;

            for( unsigned int topi = 0; topi < numSegments;
                 ++topi, angle += angleDelta, texCoord += texCoordHorzDelta )
            {

                float c = cosf( angle );
                float s = sinf( angle );

                Normal3f( -c * nRatioBase, -s * nRatioBase, -nzBase );
                TexCoord2f( texCoord, vBase );
                Vertex3f( c * rBase, s * rBase, zBase );

                Normal3f( -c * nRatioTop, -s * nRatioTop, -nzTop );

                TexCoord2f( texCoord, vTop );
                Vertex3f( c * rTop, s * rTop, zTop );
            }

            // do last point by hand to ensure no round off errors.
            Normal3f( -nRatioBase, 0.0F, -nzBase );
            TexCoord2f( 1.0F, vBase );
            Vertex3f( rBase, 0.0F, zBase );

            Normal3f( -nRatioTop, 0.0F, -nzTop );
            TexCoord2f( 1.0F, vTop );
            Vertex3f( rTop, 0.0F, zTop );

            End();

            lBase      = lTop;
            rBase      = rTop;
            zBase      = zTop;
            vBase      = vTop;
            nzBase     = nzTop;
            nRatioBase = nRatioTop;
        }
    }

    if( drawFrontFace )
    {
        float lBase      = -PIf * 0.5F;
        float rBase      = 0.0F;
        float zBase      = -sphere.getRadius();
        float vBase      = 0.0F;
        float nzBase     = -1.0F;
        float nRatioBase = 0.0F;

        for( unsigned int rowi = 0; rowi < numRows; ++rowi )
        {

            float lTop      = lBase + lDelta;
            float rTop      = cosf( lTop ) * sphere.getRadius();
            float zTop      = sinf( lTop ) * sphere.getRadius();
            float vTop      = vBase + vDelta;
            float nzTop     = sinf( lTop );
            float nRatioTop = cosf( lTop );

            Begin( GL_QUAD_STRIP );

            float angle    = 0.0F;
            float texCoord = 0.0F;

            for( unsigned int topi = 0; topi < numSegments;
                 ++topi, angle += angleDelta, texCoord += texCoordHorzDelta )
            {

                float c = cosf( angle );
                float s = sinf( angle );

                Normal3f( c * nRatioTop, s * nRatioTop, nzTop );
                TexCoord2f( texCoord, vTop );
                Vertex3f( c * rTop, s * rTop, zTop );

                Normal3f( c * nRatioBase, s * nRatioBase, nzBase );
                TexCoord2f( texCoord, vBase );
                Vertex3f( c * rBase, s * rBase, zBase );
            }

            // do last point by hand to ensure no round off errors.
            Normal3f( nRatioTop, 0.0F, nzTop );
            TexCoord2f( 1.0F, vTop );
            Vertex3f( rTop, 0.0F, zTop );

            Normal3f( nRatioBase, 0.0F, nzBase );
            TexCoord2f( 1.0F, vBase );
            Vertex3f( rBase, 0.0F, zBase );

            End();

            lBase      = lTop;
            rBase      = rTop;
            zBase      = zTop;
            vBase      = vTop;
            nzBase     = nzTop;
            nRatioBase = nRatioTop;
        }
    }
}

void
BuildShapeGeometryVisitor::apply( const Box& box )
{
    // evaluate hints
    bool  createBody   = ( _hints ? _hints->getCreateBody() : true );
    bool  createTop    = ( _hints ? _hints->getCreateTop() : true );
    bool  createBottom = ( _hints ? _hints->getCreateBottom() : true );

    float dx           = box.getHalfLengths().x;
    float dy           = box.getHalfLengths().y;
    float dz           = box.getHalfLengths().z;

    setMatrix( box.computeRotationMatrix() *
               osg::translate( dvec3( box.getCenter() ) ) );

    Begin( GL_QUADS );

    if( createBody )
    {
        // -ve y plane
        Normal3f( 0.0F, -1.0F, 0.0F );
        TexCoord2f( 0.0F, 1.0F );
        Vertex3f( -dx, -dy, dz );

        Normal3f( 0.0F, -1.0F, 0.0F );
        TexCoord2f( 0.0F, 0.0F );
        Vertex3f( -dx, -dy, -dz );

        Normal3f( 0.0F, -1.0F, 0.0F );
        TexCoord2f( 1.0F, 0.0F );
        Vertex3f( dx, -dy, -dz );

        Normal3f( 0.0F, -1.0F, 0.0F );
        TexCoord2f( 1.0F, 1.0F );
        Vertex3f( dx, -dy, dz );

        // +ve y plane
        Normal3f( 0.0F, 1.0F, 0.0F );
        TexCoord2f( 0.0F, 1.0F );
        Vertex3f( dx, dy, dz );

        Normal3f( 0.0F, 1.0F, 0.0F );
        TexCoord2f( 0.0F, 0.0F );
        Vertex3f( dx, dy, -dz );

        Normal3f( 0.0F, 1.0F, 0.0F );
        TexCoord2f( 1.0F, 0.0F );
        Vertex3f( -dx, dy, -dz );

        Normal3f( 0.0F, 1.0F, 0.0F );
        TexCoord2f( 1.0F, 1.0F );
        Vertex3f( -dx, dy, dz );

        // +ve x plane
        Normal3f( 1.0F, 0.0F, 0.0F );
        TexCoord2f( 0.0F, 1.0F );
        Vertex3f( dx, -dy, dz );

        Normal3f( 1.0F, 0.0F, 0.0F );
        TexCoord2f( 0.0F, 0.0F );
        Vertex3f( dx, -dy, -dz );

        Normal3f( 1.0F, 0.0F, 0.0F );
        TexCoord2f( 1.0F, 0.0F );
        Vertex3f( dx, dy, -dz );

        Normal3f( 1.0F, 0.0F, 0.0F );
        TexCoord2f( 1.0F, 1.0F );
        Vertex3f( dx, dy, dz );

        // -ve x plane
        Normal3f( -1.0F, 0.0F, 0.0F );
        TexCoord2f( 0.0F, 1.0F );
        Vertex3f( -dx, dy, dz );

        Normal3f( -1.0F, 0.0F, 0.0F );
        TexCoord2f( 0.0F, 0.0F );
        Vertex3f( -dx, dy, -dz );

        Normal3f( -1.0F, 0.0F, 0.0F );
        TexCoord2f( 1.0F, 0.0F );
        Vertex3f( -dx, -dy, -dz );

        Normal3f( -1.0F, 0.0F, 0.0F );
        TexCoord2f( 1.0F, 1.0F );
        Vertex3f( -dx, -dy, dz );
    }

    if( createTop )
    {
        // +ve z plane
        Normal3f( 0.0F, 0.0F, 1.0F );
        TexCoord2f( 0.0F, 1.0F );
        Vertex3f( -dx, dy, dz );

        Normal3f( 0.0F, 0.0F, 1.0F );
        TexCoord2f( 0.0F, 0.0F );
        Vertex3f( -dx, -dy, dz );

        Normal3f( 0.0F, 0.0F, 1.0F );
        TexCoord2f( 1.0F, 0.0F );
        Vertex3f( dx, -dy, dz );

        Normal3f( 0.0F, 0.0F, 1.0F );
        TexCoord2f( 1.0F, 1.0F );
        Vertex3f( dx, dy, dz );
    }

    if( createBottom )
    {
        // -ve z plane
        Normal3f( 0.0F, 0.0F, -1.0F );
        TexCoord2f( 0.0F, 1.0F );
        Vertex3f( dx, dy, -dz );

        Normal3f( 0.0F, 0.0F, -1.0F );
        TexCoord2f( 0.0F, 0.0F );
        Vertex3f( dx, -dy, -dz );

        Normal3f( 0.0F, 0.0F, -1.0F );
        TexCoord2f( 1.0F, 0.0F );
        Vertex3f( -dx, -dy, -dz );

        Normal3f( 0.0F, 0.0F, -1.0F );
        TexCoord2f( 1.0F, 1.0F );
        Vertex3f( -dx, dy, -dz );
    }

    End();
}

void
BuildShapeGeometryVisitor::apply( const Cone& cone )
{
    setMatrix( cone.computeRotationMatrix() *
               osg::translate( dvec3( cone.getCenter() ) ) );

    // evaluate hints
    bool         createBody   = ( _hints ? _hints->getCreateBody() : true );
    bool         createBottom = ( _hints ? _hints->getCreateBottom() : true );

    unsigned int numSegments  = 40;
    unsigned int numRows      = 10;
    float        ratio        = ( _hints ? _hints->getDetailRatio() : 1.0F );
    if( ratio > 0.0F && ratio != 1.0F )
    {
        numRows = static_cast<unsigned int>( static_cast<float>( numRows ) * ratio );
        if( numRows < MIN_NUM_ROWS )
        {
            numRows = MIN_NUM_ROWS;
        }
        numSegments =
            static_cast<unsigned int>( static_cast<float>( numSegments ) * ratio );
        if( numSegments < MIN_NUM_SEGMENTS )
        {
            numSegments = MIN_NUM_SEGMENTS;
        }
    }

    float r                  = cone.getRadius();
    float h                  = cone.getHeight();

    float normalz            = r / ( sqrtf( r * r + h * h ) );
    float normalRatio        = 1.0F / ( sqrtf( 1.0F + normalz * normalz ) );
    normalz                 *= normalRatio;

    float angleDelta         = 2.0F * PIf / ( float )numSegments;
    float texCoordHorzDelta  = 1.0F / ( float )numSegments;
    float texCoordRowDelta   = 1.0F / ( float )numRows;
    float hDelta             = cone.getHeight() / ( float )numRows;
    float rDelta             = cone.getRadius() / ( float )numRows;

    float topz               = cone.getHeight() + cone.getBaseOffset();
    float topr               = 0.0F;
    float topv               = 1.0F;
    float basez              = topz - hDelta;
    float baser              = rDelta;
    float basev              = topv - texCoordRowDelta;
    float angle;
    float texCoord;

    if( createBody )
    {
        for( unsigned int rowi   = 0; rowi < numRows; ++rowi,
                          topz   = basez,
                          basez -= hDelta,
                          topr   = baser,
                          baser += rDelta,
                          topv   = basev,
                          basev -= texCoordRowDelta )
        {
            // we can't use a fan for the cone top
            // since we need different normals at the top
            // for each face..
            Begin( GL_QUAD_STRIP );

            angle    = 0.0F;
            texCoord = 0.0F;
            for( unsigned int topi = 0; topi < numSegments;
                 ++topi, angle += angleDelta, texCoord += texCoordHorzDelta )
            {

                float c = cosf( angle );
                float s = sinf( angle );

                Normal3f( c * normalRatio, s * normalRatio, normalz );
                TexCoord2f( texCoord, topv );
                Vertex3f( c * topr, s * topr, topz );

                Normal3f( c * normalRatio, s * normalRatio, normalz );
                TexCoord2f( texCoord, basev );
                Vertex3f( c * baser, s * baser, basez );
            }

            // do last point by hand to ensure no round off errors.
            Normal3f( normalRatio, 0.0F, normalz );
            TexCoord2f( 1.0F, topv );
            Vertex3f( topr, 0.0F, topz );

            Normal3f( normalRatio, 0.0F, normalz );
            TexCoord2f( 1.0F, basev );
            Vertex3f( baser, 0.0F, basez );

            End();
        }
    }

    if( createBottom )
    {
        Begin( GL_TRIANGLE_FAN );

        angle    = PIf * 2.0F;
        texCoord = 1.0F;
        basez    = cone.getBaseOffset();

        Normal3f( 0.0F, 0.0F, -1.0F );
        TexCoord2f( 0.5F, 0.5F );
        Vertex3f( 0.0F, 0.0F, basez );

        for( unsigned int bottomi = 0; bottomi < numSegments;
             ++bottomi, angle -= angleDelta, texCoord -= texCoordHorzDelta )
        {

            float c = cosf( angle );
            float s = sinf( angle );

            Normal3f( 0.0F, 0.0F, -1.0F );
            TexCoord2f( c * 0.5F + 0.5F, s * 0.5F + 0.5F );
            Vertex3f( c * r, s * r, basez );
        }

        Normal3f( 0.0F, 0.0F, -1.0F );
        TexCoord2f( 1.0F, 0.0F );
        Vertex3f( r, 0.0F, basez );

        End();
    }
}

void
BuildShapeGeometryVisitor::apply( const Cylinder& cylinder )
{
    setMatrix( cylinder.computeRotationMatrix() *
               osg::translate( dvec3( cylinder.getCenter() ) ) );

    // evaluate hints
    bool         createBody   = ( _hints ? _hints->getCreateBody() : true );
    bool         createTop    = ( _hints ? _hints->getCreateTop() : true );
    bool         createBottom = ( _hints ? _hints->getCreateBottom() : true );

    unsigned int numSegments  = 40;
    float        ratio        = ( _hints ? _hints->getDetailRatio() : 1.0F );
    if( ratio > 0.0F && ratio != 1.0F )
    {
        numSegments =
            static_cast<unsigned int>( static_cast<float>( numSegments ) * ratio );
        if( numSegments < MIN_NUM_SEGMENTS )
        {
            numSegments = MIN_NUM_SEGMENTS;
        }
    }

    // cylinder body
    if( createBody )
    {
        drawCylinderBody( numSegments, cylinder.getRadius(), cylinder.getHeight() );
    }

    float angleDelta    = 2.0F * PIf / ( float )numSegments;
    float texCoordDelta = 1.0F / ( float )numSegments;

    float r             = cylinder.getRadius();
    float h             = cylinder.getHeight();

    float basez         = -h * 0.5F;
    float topz          = h * 0.5F;

    float angle         = 0.0F;
    float texCoord      = 0.0F;

    // cylinder top
    if( createTop )
    {

        Begin( GL_TRIANGLE_FAN );

        Normal3f( 0.0F, 0.0F, 1.0F );
        TexCoord2f( 0.5F, 0.5F );
        Vertex3f( 0.0F, 0.0F, topz );

        angle    = 0.0F;
        texCoord = 0.0F;
        for( unsigned int topi = 0; topi < numSegments;
             ++topi, angle += angleDelta, texCoord += texCoordDelta )
        {
            float c = cosf( angle );
            float s = sinf( angle );

            Normal3f( 0.0F, 0.0F, 1.0F );
            TexCoord2f( c * 0.5F + 0.5F, s * 0.5F + 0.5F );
            Vertex3f( c * r, s * r, topz );
        }

        Normal3f( 0.0F, 0.0F, 1.0F );
        TexCoord2f( 1.0F, 0.5F );
        Vertex3f( r, 0.0F, topz );

        End();
    }

    // cylinder bottom
    if( createBottom )
    {
        Begin( GL_TRIANGLE_FAN );

        Normal3f( 0.0F, 0.0F, -1.0F );
        TexCoord2f( 0.5F, 0.5F );
        Vertex3f( 0.0F, 0.0F, basez );

        angle    = PIf * 2.0F;
        texCoord = 1.0F;
        for( unsigned int bottomi = 0; bottomi < numSegments;
             ++bottomi, angle -= angleDelta, texCoord -= texCoordDelta )
        {
            float c = cosf( angle );
            float s = sinf( angle );

            Normal3f( 0.0F, 0.0F, -1.0F );
            TexCoord2f( c * 0.5F + 0.5F, s * 0.5F + 0.5F );
            Vertex3f( c * r, s * r, basez );
        }

        Normal3f( 0.0F, 0.0F, -1.0F );
        TexCoord2f( 1.0F, 0.5F );
        Vertex3f( r, 0.0F, basez );

        End();
    }
}

void
BuildShapeGeometryVisitor::apply( const Capsule& capsule )
{
    setMatrix( capsule.computeRotationMatrix() *
               osg::translate( dvec3( capsule.getCenter() ) ) );

    // evaluate hints
    bool         createBody   = ( _hints ? _hints->getCreateBody() : true );
    bool         createTop    = ( _hints ? _hints->getCreateTop() : true );
    bool         createBottom = ( _hints ? _hints->getCreateBottom() : true );

    unsigned int numSegments  = 40;
    unsigned int numRows      = 20;
    float        ratio        = ( _hints ? _hints->getDetailRatio() : 1.0F );
    if( ratio > 0.0F && ratio != 1.0F )
    {
        numSegments =
            static_cast<unsigned int>( static_cast<float>( numSegments ) * ratio );
        if( numSegments < MIN_NUM_SEGMENTS )
        {
            numSegments = MIN_NUM_SEGMENTS;
        }
        numRows = static_cast<unsigned int>( static_cast<float>( numRows ) * ratio );
        if( numRows < MIN_NUM_ROWS )
        {
            numRows = MIN_NUM_ROWS;
        }
    }

    // if numRows is odd the top and bottom halves of sphere won't match, so bump up to
    // the next event numRows
    if( ( numRows % 2 ) != 0 )
    {
        ++numRows;
    }

    // capsule cylindrical body
    if( createBody )
    {
        drawCylinderBody( numSegments, capsule.getRadius(), capsule.getHeight() );
    }

    // capsule top cap
    if( createTop )
    {
        drawHalfSphere( numSegments,
                        numRows,
                        capsule.getRadius(),
                        SphereTopHalf,
                        capsule.getHeight() / 2.0F );
    }

    // capsule bottom cap
    if( createBottom )
    {
        drawHalfSphere( numSegments,
                        numRows,
                        capsule.getRadius(),
                        SphereBottomHalf,
                        -capsule.getHeight() / 2.0F );
    }
}

void
BuildShapeGeometryVisitor::apply( const InfinitePlane& )
{
    OSG_NOTICE << "Warning: BuildShapeGeometryVisitor::apply(const InfinitePlane& "
                  "plane) not yet implemented. "
               << std::endl;
}

void
BuildShapeGeometryVisitor::apply( const TriangleMesh& mesh )
{
    const Vec3Array*  vertices = mesh.getVertices();
    const IndexArray* indices  = mesh.getIndices();

    if( vertices && indices )
    {
        Begin( GL_TRIANGLES );

        for( unsigned int i = 0; i + 2 < indices->getNumElements(); i += 3 )
        {
            const vec3& v1     = ( *vertices )[indices->index( i )];
            const vec3& v2     = ( *vertices )[indices->index( i + 1 )];
            const vec3& v3     = ( *vertices )[indices->index( i + 2 )];
            vec3        normal = osg::cross( v2 - v1, v3 - v2 );
            normal             = osg::normalize( normal );

            Normal( normal );
            Vertex( v1 );

            Normal( normal );
            Vertex( v2 );

            Normal( normal );
            Vertex( v3 );
        }

        End();
    }
}

void
BuildShapeGeometryVisitor::apply( const ConvexHull& hull )
{
    apply( ( const TriangleMesh& )hull );
}

void
BuildShapeGeometryVisitor::apply( const HeightField& field )
{
    if( field.getNumColumns() == 0 || field.getNumRows() == 0 )
    {
        return;
    }

    setMatrix( field.computeRotationMatrix() *
               osg::translate( dvec3( field.getOrigin() ) ) );

    float dx    = field.getXInterval();
    float dy    = field.getYInterval();

    float du    = 1.0F / ( ( float )field.getNumColumns() - 1.0F );
    float dv    = 1.0F / ( ( float )field.getNumRows() - 1.0F );

    float vBase = 0.0F;

    vec3  vertTop;
    vec3  normTop;

    vec3  vertBase;
    vec3  normBase;

    if( field.getSkirtHeight() != 0.0F )
    {
        Begin( GL_QUAD_STRIP );

        float        u = 0.0F;

        // draw bottom skirt
        unsigned int col;
        vertTop.y = 0.0F;
        for( col = 0; col < field.getNumColumns(); ++col, u += du )
        {
            vertTop.x = dx * ( float )col;
            vertTop.z = field.getHeight( col, 0 );
            normTop   = field.getNormal( col, 0 );

            TexCoord2f( u, 0.0F );
            Normal( normTop );
            Vertex( vertTop );

            vertTop.z -= field.getSkirtHeight();

            TexCoord2f( u, 0.0F );
            Normal( normTop );
            Vertex( vertTop );
        }

        End();

        // draw top skirt
        Begin( GL_QUAD_STRIP );

        unsigned int row = field.getNumRows() - 1;

        u                = 0.0F;
        vertTop.y        = dy * ( float )( row );
        for( col = 0; col < field.getNumColumns(); ++col, u += du )
        {
            vertTop.x = dx * ( float )col;
            vertTop.z = field.getHeight( col, row );
            normTop   = field.getNormal( col, row );

            TexCoord2f( u, 1.0F );
            Normal( normTop );
            Vertex3f( vertTop.x, vertTop.y, vertTop.z - field.getSkirtHeight() );

            // vertTop.z-=field.getSkirtHeight();

            TexCoord2f( u, 1.0F );
            Normal( normTop );
            Vertex( vertTop );
        }

        End();
    }

    // draw each row of HeightField
    for( unsigned int row = 0; row < field.getNumRows() - 1; ++row, vBase += dv )
    {

        float vTop = vBase + dv;
        float u    = 0.0F;

        Begin( GL_QUAD_STRIP );

        // draw skirt at beginning of this row if required.
        if( field.getSkirtHeight() != 0.0F )
        {
            vertTop.set( 0.0F,
                         dy * ( float )( row + 1 ),
                         field.getHeight( 0, row + 1 ) - field.getSkirtHeight() );
            normTop = field.getNormal( 0, row + 1 );

            vertBase.set( 0.0F,
                          dy * ( float )row,
                          field.getHeight( 0, row ) - field.getSkirtHeight() );
            normBase = field.getNormal( 0, row );

            TexCoord2f( u, vTop );
            Normal( normTop );
            Vertex( vertTop );

            TexCoord2f( u, vBase );
            Normal( normBase );
            Vertex( vertBase );
        }

        // draw the actual row
        for( unsigned int col = 0; col < field.getNumColumns(); ++col, u += du )
        {
            vertTop.set( dx * ( float )col,
                         dy * ( float )( row + 1 ),
                         field.getHeight( col, row + 1 ) );
            normTop = field.getNormal( col, row + 1 );

            vertBase.set( dx * ( float )col,
                          dy * ( float )row,
                          field.getHeight( col, row ) );
            normBase = field.getNormal( col, row );

            TexCoord2f( u, vTop );
            Normal( normTop );
            Vertex( vertTop );

            TexCoord2f( u, vBase );
            Normal( normBase );
            Vertex( vertBase );
        }

        // draw skirt at end of this row if required.
        if( field.getSkirtHeight() != 0.0F )
        {

            vertBase.z -= field.getSkirtHeight();
            vertTop.z  -= field.getSkirtHeight();

            TexCoord2f( u, vTop );
            Normal( normTop );
            Vertex( vertTop );

            TexCoord2f( u, vBase );
            Normal( normBase );
            Vertex( vertBase );
        }

        End();
    }
}

void
BuildShapeGeometryVisitor::apply( const CompositeShape& group )
{
    for( unsigned int i = 0; i < group.getNumChildren(); ++i )
    {
        group.getChild( i )->accept( *this );
    }
}

Geometry*
osg::convertShapeToGeometry( const Shape&             shape,
                             const TessellationHints* hints )
{
    ref_ptr<Geometry>         geometry = new Geometry;

    BuildShapeGeometryVisitor buildGeometry( geometry.get(), hints );
    shape.accept( buildGeometry );

    return geometry.release();
}

Geometry*
osg::convertShapeToGeometry( const Shape&             shape,
                             const TessellationHints* hints,
                             const vec4&              color,
                             Array::Binding           colorBinding )
{
    ref_ptr<Geometry> geometry  = convertShapeToGeometry( shape, hints );

    unsigned int      numColors = 0;
    switch( colorBinding )
    {
        case( Array::BIND_OVERALL ) :
            numColors = 1;
            break;
        case( Array::BIND_PER_VERTEX ) :
            numColors = geometry->getVertexArray()->getNumElements();
            break;
        case( Array::BIND_PER_PRIMITIVE_SET ) :
            numColors =
                static_cast<unsigned int>( geometry->getPrimitiveSetList().size() );
            break;
        default :
            break;
    }

    if( numColors > 0 )
    {
        ref_ptr<Vec4Array> colors = new Vec4Array( colorBinding );
        geometry->setColorArray( colors.get() );

        for( unsigned int i = 0; i < numColors; ++i )
        {
            colors->push_back( color );
        }
    }

    return geometry.release();
}
