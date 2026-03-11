/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Ray intersection tester extending LineSegmentIntersector.
 * Provides infinite-ray semantics for shadow and LOS queries.
 */
#include <osgUtil/intersection/RayIntersector.hpp>

#include <cmath>
#include <limits>
#include <osg/core/Notify.hpp>
#include <osg/geometry/KdTree.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osgUtil/intersection/LineSegmentIntersector.hpp>

using namespace osg;
using namespace osgUtil;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  RayIntersector
//

RayIntersector::RayIntersector( CoordinateFrame                cf,
                                RayIntersector*                parent,
                                Intersector::IntersectionLimit intersectionLimit ) :
    Intersector( cf,
                 intersectionLimit ),
    _parent( parent )
{
    if( parent )
    {
        setPrecisionHint( parent->getPrecisionHint() );
    }
}

RayIntersector::RayIntersector( const dvec3& start,
                                const dvec3& direction ) :
    Intersector(),
    _parent( 0 ),
    _start( start ),
    _direction( direction )
{
}

RayIntersector::RayIntersector( CoordinateFrame                cf,
                                const dvec3&                   start,
                                const dvec3&                   direction,
                                RayIntersector*                parent,
                                Intersector::IntersectionLimit intersectionLimit ) :
    Intersector( cf,
                 intersectionLimit ),
    _parent( parent ),
    _start( start ),
    _direction( direction )
{
    if( parent )
    {
        setPrecisionHint( parent->getPrecisionHint() );
    }
}

RayIntersector::RayIntersector( CoordinateFrame cf,
                                double          x,
                                double          y ) :
    Intersector( cf ),
    _parent( 0 )
{
    switch( cf )
    {
        case WINDOW :
            setStart( dvec3( x, y, 0. ) );
            setDirection( dvec3( 0., 0., 1. ) );
            break;
        case PROJECTION :
            setStart( dvec3( x, y, -1. ) );
            setDirection( dvec3( 0., 0., 1. ) );
            break;
        case VIEW :
            setStart( dvec3( x, y, 0. ) );
            setDirection( dvec3( 0., 0., 1. ) );
            break;
        case MODEL :
            setStart( dvec3( x, y, 0. ) );
            setDirection( dvec3( 0., 0., 1. ) );
            break;
    }
}

Intersector*
RayIntersector::clone( IntersectionVisitor& iv )
{
    if( _coordinateFrame == MODEL && iv.getModelMatrix() == 0 )
    {
        return new RayIntersector( MODEL, _start, _direction, this, _intersectionLimit );
    }

    dmat4 matrix( LineSegmentIntersector::getTransformation( iv, _coordinateFrame ) );

    dvec3 newStart = matrix * _start;
    dvec4 tmp      = matrix * dvec4( _start + _direction, 1. );
    dvec3 newEnd   = dvec3( tmp.x, tmp.y, tmp.z ) - ( newStart * tmp.w );
    return new RayIntersector( MODEL, newStart, newEnd, this, _intersectionLimit );
}

bool
RayIntersector::enter( const Node& node )
{
    if( reachedLimit() )
    {
        return false;
    }
    return !node.isCullingActive() || intersects( node.getBound() );
}

void
RayIntersector::leave()
{
    // do nothing
}

void
RayIntersector::reset()
{
    Intersector::reset();

    _intersections.clear();
}

void
RayIntersector::intersect( IntersectionVisitor& iv,
                           Drawable*            drawable )
{
    // did we reached what we wanted as specified by setIntersectionLimit()?
    if( reachedLimit() )
    {
        return;
    }

    // clip ray to finite line segment
    dvec3 s( _start ), e;
    if( !intersectAndClip( s, _direction, e, drawable->getBoundingBox() ) )
    {
        return;
    }

    // dummy traversal
    if( iv.getDoDummyTraversal() )
    {
        return;
    }

    // get intersections using LineSegmentIntersector
    LineSegmentIntersector lsi( MODEL, s, e, NULL, _intersectionLimit );
    lsi.setPrecisionHint( getPrecisionHint() );
    lsi.intersect( iv, drawable, s, e );

    // copy intersections from LineSegmentIntersector
    LineSegmentIntersector::Intersections intersections = lsi.getIntersections();
    if( !intersections.empty() )
    {
        double preLength = osg::length( s - _start );
        double esLength  = osg::length( e - s );

        for( LineSegmentIntersector::Intersections::iterator it = intersections.begin();
             it != intersections.end();
             it++ )
        {
            Intersection hit;
            hit.distance                = preLength + it->ratio * esLength;
            hit.matrix                  = it->matrix;
            hit.nodePath                = it->nodePath;
            hit.drawable                = it->drawable;
            hit.primitiveIndex          = it->primitiveIndex;

            hit.localIntersectionPoint  = it->localIntersectionPoint;
            hit.localIntersectionNormal = it->localIntersectionNormal;

            hit.indexList               = it->indexList;
            hit.ratioList               = it->ratioList;

            insertIntersection( hit );
        }
    }
}

bool
RayIntersector::intersects( const sphere& bs )
{
    // if bs not valid then return true based on the assumption that an invalid sphere is
    // yet to be defined.
    if( !bs.valid() )
    {
        return true;
    }

    // test for _start inside the bounding sphere
    dvec3  sm = _start - osg::dvec3( bs.center );
    double c  = osg::length2( sm ) - bs.radius * bs.radius;
    if( c < 0.0 )
    {
        return true;
    }

    // solve quadratic equation
    double a = osg::length2( _direction );
    double b = osg::dot( sm, _direction ) * 2.0;
    double d = b * b - 4.0 * a * c;

    // no intersections if d<0
    if( d < 0.0 )
    {
        return false;
    }

    // compute two solutions of quadratic equation
    d          = sqrt( d );
    double div = 1.0 / ( 2.0 * a );
    double r1  = ( -b - d ) * div;
    double r2  = ( -b + d ) * div;

    // return false if both intersections are before the ray start
    if( r1 <= 0.0 && r2 <= 0.0 )
    {
        return false;
    }

    // if LIMIT_NEAREST and closest point of bounding sphere is further than already
    // found intersection, return false
    if( _intersectionLimit == LIMIT_NEAREST && !getIntersections().empty() )
    {
        double minDistance = osg::length( sm ) - bs.radius;
        if( minDistance >= getIntersections().begin()->distance )
        {
            return false;
        }
    }

    // passed all the rejection tests so line must intersect bounding sphere, return
    // true.
    return true;
}

bool
RayIntersector::intersectAndClip( dvec3&       s,
                                  const dvec3& d,
                                  dvec3&       e,
                                  const box&   bbInput )
{
    // bounding box min and max
    dvec3        bb_min( bbInput.min );
    dvec3        bb_max( bbInput.max );

    // Expand the extents of the bounding box by the epsilon to prevent numerical errors
    // resulting in misses.
    const double epsilon = 1E-6;

    // clip s against all three components of the Min to Max range of bb
    for( std::size_t i = 0; i < 3; i++ )
    {
        // test direction
        if( d[i] >= 0. )
        {
            // trivial reject of segment wholly outside
            if( s[i] > bb_max[i] )
            {
                return false;
            }

            if( ( d[i] > epsilon ) && ( s[i] < bb_min[i] ) )
            {
                // clip s to xMin
                double t = ( bb_min[i] - s[i] ) / d[i] - epsilon;
                if( t > 0.0 )
                {
                    s = s + d * t;
                }
            }
        }
        else
        {
            // trivial reject of segment wholly outside
            if( s[i] < bb_min[i] )
            {
                return false;
            }

            if( ( d[i] < -epsilon ) && ( s[i] > bb_max[i] ) )
            {
                // clip s to xMax
                double t = ( bb_max[i] - s[i] ) / d[i] - epsilon;
                if( t > 0.0 )
                {
                    s = s + d * t;
                }
            }
        }
    }

    // t for ending point of clipped ray
    double end_t = std::numeric_limits<double>::infinity();

    // get end point by clipping the ray by bb
    // note: this can not be done in previous loop as start point s is moving
    for( std::size_t i = 0; i < 3; i++ )
    {
        // test direction
        if( d[i] >= epsilon )
        {
            // compute end_t based on xMax
            double t = ( bb_max[i] - s[i] ) / d[i] + epsilon;
            if( t < end_t )
            {
                end_t = t;
            }
        }
        else if( d[i] <= -epsilon )
        {
            // compute end_t based on xMin
            double t = ( bb_min[i] - s[i] ) / d[i] + epsilon;
            if( t < end_t )
            {
                end_t = t;
            }
        }
    }

    // if we failed to clamp the end point return false
    if( end_t == std::numeric_limits<double>::infinity() )
    {
        return false;
    }

    // compute e
    e = s + d * end_t;

    return true;
}

Texture*
RayIntersector::Intersection::getTextureLookUp( vec3& tc ) const
{
    Geometry*  geometry = drawable.valid() ? drawable->asGeometry() : 0;
    Vec3Array* vertices =
        geometry ? dynamic_cast<Vec3Array*>( geometry->getVertexArray() ) : 0;

    if( vertices )
    {
        if( indexList.size() == 3 && ratioList.size() == 3 )
        {
            unsigned int i1                   = indexList[0];
            unsigned int i2                   = indexList[1];
            unsigned int i3                   = indexList[2];

            float        r1                   = static_cast<float>( ratioList[0] );
            float        r2                   = static_cast<float>( ratioList[1] );
            float        r3                   = static_cast<float>( ratioList[2] );

            Array*       texcoords            = ( geometry->getNumTexCoordArrays() > 0 )
                                                  ? geometry->getTexCoordArray( 0 )
                                                  : 0;
            FloatArray*  texcoords_FloatArray = dynamic_cast<FloatArray*>( texcoords );
            Vec2Array*   texcoords_Vec2Array  = dynamic_cast<Vec2Array*>( texcoords );
            Vec3Array*   texcoords_Vec3Array  = dynamic_cast<Vec3Array*>( texcoords );
            if( texcoords_FloatArray )
            {
                // we have tex coord array so now we can compute the final tex coord at
                // the point of intersection.
                float tc1 = ( *texcoords_FloatArray )[i1];
                float tc2 = ( *texcoords_FloatArray )[i2];
                float tc3 = ( *texcoords_FloatArray )[i3];
                tc.x      = tc1 * r1 + tc2 * r2 + tc3 * r3;
            }
            else if( texcoords_Vec2Array )
            {
                // we have tex coord array so now we can compute the final tex coord at
                // the point of intersection.
                const vec2& tc1 = ( *texcoords_Vec2Array )[i1];
                const vec2& tc2 = ( *texcoords_Vec2Array )[i2];
                const vec2& tc3 = ( *texcoords_Vec2Array )[i3];
                tc.x            = tc1.x * r1 + tc2.x * r2 + tc3.x * r3;
                tc.y            = tc1.y * r1 + tc2.y * r2 + tc3.y * r3;
            }
            else if( texcoords_Vec3Array )
            {
                // we have tex coord array so now we can compute the final tex coord at
                // the point of intersection.
                const vec3& tc1 = ( *texcoords_Vec3Array )[i1];
                const vec3& tc2 = ( *texcoords_Vec3Array )[i2];
                const vec3& tc3 = ( *texcoords_Vec3Array )[i3];
                tc.x            = tc1.x * r1 + tc2.x * r2 + tc3.x * r3;
                tc.y            = tc1.y * r1 + tc2.y * r2 + tc3.y * r3;
                tc.z            = tc1.z * r1 + tc2.z * r2 + tc3.z * r3;
            }
            else
            {
                return 0;
            }
        }

        const Texture* activeTexture = 0;

        if( drawable->getStateSet() )
        {
            const Texture* texture =
                dynamic_cast<Texture*>( drawable->getStateSet()->getTextureAttribute(
                    0,
                    StateAttribute::Type::TEXTURE
                ) );
            if( texture )
            {
                activeTexture = texture;
            }
        }

        for( NodePath::const_reverse_iterator itr = nodePath.rbegin();
             itr != nodePath.rend() && !activeTexture;
             ++itr )
        {
            const Node* node = *itr;
            if( node->getStateSet() )
            {
                if( !activeTexture )
                {
                    const Texture* texture = dynamic_cast<const Texture*>(
                        node->getStateSet()
                            ->getTextureAttribute( 0, StateAttribute::Type::TEXTURE )
                    );
                    if( texture )
                    {
                        activeTexture = texture;
                    }
                }
            }
        }

        return const_cast<Texture*>( activeTexture );
    }
    return 0;
}
