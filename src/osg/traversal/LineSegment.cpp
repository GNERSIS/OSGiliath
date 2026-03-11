/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Line segment defined by start and end points. Used for ray-casting,
 * intersection testing, and line-of-sight queries.
 */
#include <osg/traversal/LineSegment.hpp>

#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>

using namespace osg;

LineSegment::~LineSegment()
{
}

bool
LineSegment::intersectAndClip( vec_type&  s,
                               vec_type&  e,
                               const box& bb )
{
    // compate s and e against the xMin to xMax range of bb.
    if( s.x <= e.x )
    {

        // trivial reject of segment wholely outside.
        if( e.x < bb.xMin() )
        {
            return false;
        }
        if( s.x > bb.xMax() )
        {
            return false;
        }

        if( s.x < bb.xMin() )
        {
            // clip s to xMin.
            s = s + ( e - s ) * ( bb.xMin() - s.x ) / ( e.x - s.x );
        }

        if( e.x > bb.xMax() )
        {
            // clip e to xMax.
            e = s + ( e - s ) * ( bb.xMax() - s.x ) / ( e.x - s.x );
        }
    }
    else
    {
        if( s.x < bb.xMin() )
        {
            return false;
        }
        if( e.x > bb.xMax() )
        {
            return false;
        }

        if( e.x < bb.xMin() )
        {
            // clip s to xMin.
            e = s + ( e - s ) * ( bb.xMin() - s.x ) / ( e.x - s.x );
        }

        if( s.x > bb.xMax() )
        {
            // clip e to xMax.
            s = s + ( e - s ) * ( bb.xMax() - s.x ) / ( e.x - s.x );
        }
    }

    // compate s and e against the yMin to yMax range of bb.
    if( s.y <= e.y )
    {

        // trivial reject of segment wholely outside.
        if( e.y < bb.yMin() )
        {
            return false;
        }
        if( s.y > bb.yMax() )
        {
            return false;
        }

        if( s.y < bb.yMin() )
        {
            // clip s to yMin.
            s = s + ( e - s ) * ( bb.yMin() - s.y ) / ( e.y - s.y );
        }

        if( e.y > bb.yMax() )
        {
            // clip e to yMax.
            e = s + ( e - s ) * ( bb.yMax() - s.y ) / ( e.y - s.y );
        }
    }
    else
    {
        if( s.y < bb.yMin() )
        {
            return false;
        }
        if( e.y > bb.yMax() )
        {
            return false;
        }

        if( e.y < bb.yMin() )
        {
            // clip s to yMin.
            e = s + ( e - s ) * ( bb.yMin() - s.y ) / ( e.y - s.y );
        }

        if( s.y > bb.yMax() )
        {
            // clip e to yMax.
            s = s + ( e - s ) * ( bb.yMax() - s.y ) / ( e.y - s.y );
        }
    }

    // compate s and e against the zMin to zMax range of bb.
    if( s.z <= e.z )
    {

        // trivial reject of segment wholely outside.
        if( e.z < bb.zMin() )
        {
            return false;
        }
        if( s.z > bb.zMax() )
        {
            return false;
        }

        if( s.z < bb.zMin() )
        {
            // clip s to zMin.
            s = s + ( e - s ) * ( bb.zMin() - s.z ) / ( e.z - s.z );
        }

        if( e.z > bb.zMax() )
        {
            // clip e to zMax.
            e = s + ( e - s ) * ( bb.zMax() - s.z ) / ( e.z - s.z );
        }
    }
    else
    {
        if( s.z < bb.zMin() )
        {
            return false;
        }
        if( e.z > bb.zMax() )
        {
            return false;
        }

        if( e.z < bb.zMin() )
        {
            // clip s to zMin.
            e = s + ( e - s ) * ( bb.zMin() - s.z ) / ( e.z - s.z );
        }

        if( s.z > bb.zMax() )
        {
            // clip e to zMax.
            s = s + ( e - s ) * ( bb.zMax() - s.z ) / ( e.z - s.z );
        }
    }

    return true;
}

bool
LineSegment::intersect( const box& bb ) const
{
    if( !bb.valid() )
    {
        return false;
    }

    vec_type s = _s, e = _e;
    return intersectAndClip( s, e, bb );
}

bool
LineSegment::intersectAndComputeRatios( const box& bb,
                                        float&     r1,
                                        float&     r2 ) const
{
    if( !bb.valid() )
    {
        return false;
    }

    vec_type s = _s, e = _e;
    bool     result = intersectAndClip( s, e, bb );
    if( result )
    {
        value_type len = osg::length( _e - _s );
        if( len > 0.0F )
        {
            value_type inv_len = 1.0F / len;
            r1                 = ( float )( osg::length( s - _s ) * inv_len );
            r2                 = ( float )( osg::length( e - _s ) * inv_len );
        }
        else
        {
            r1 = 0.0F;
            r2 = 0.0F;
        }
    }
    return result;
}

bool
LineSegment::intersectAndComputeRatios( const box& bb,
                                        double&    r1,
                                        double&    r2 ) const
{
    if( !bb.valid() )
    {
        return false;
    }

    vec_type s = _s, e = _e;
    bool     result = intersectAndClip( s, e, bb );
    if( result )
    {
        double len = osg::length( _e - _s );
        if( len > 0.0 )
        {
            double inv_len = 1.0 / len;
            r1             = ( osg::length( s - _s ) * inv_len );
            r2             = ( osg::length( e - _s ) * inv_len );

            OSG_NOTICE << "s = (" << s << "), e = (" << e << ")" << std::endl;
        }
        else
        {
            r1 = 0.0;
            r2 = 0.0;
        }
    }
    return result;
}

bool
LineSegment::intersectAndComputeRatios( const sphere& bs,
                                        float&        r1,
                                        float&        r2 ) const
{
    vec_type   sm = _s - dvec3( bs.center );
    value_type c  = osg::length2( sm ) - bs.radius * bs.radius;

    vec_type   se = _e - _s;
    value_type a  = osg::length2( se );

    // check for zero length segment.
    if( a == 0.0 )
    {
        // check if start point outside sphere radius
        if( c > 0.0 )
        {
            return false;
        }

        // length segment within radius of bounding sphere but zero length
        // so return true, and set the ratio so the start point is the one
        // to be used.
        r1 = 1.0F;
        r2 = 0.0F;
        return true;
    }

    value_type b = osg::dot( sm, se ) * 2.0F;

    value_type d = b * b - 4.0F * a * c;

    if( d < 0.0F )
    {
        return false;
    }

    d              = ( value_type )sqrt( d );

    value_type div = 1.0F / ( 2.0F * a );

    r1             = ( float )( ( -b - d ) * div );
    r2             = ( float )( ( -b + d ) * div );

    if( r1 <= 0.0F && r2 <= 0.0F )
    {
        return false;
    }

    if( r1 >= 1.0F && r2 >= 1.0F )
    {
        return false;
    }

    return true;
}

bool
LineSegment::intersectAndComputeRatios( const sphere& bs,
                                        double&       r1,
                                        double&       r2 ) const
{
    vec_type   sm = _s - dvec3( bs.center );
    value_type c  = osg::length2( sm ) - bs.radius * bs.radius;

    vec_type   se = _e - _s;
    value_type a  = osg::length2( se );

    // check for zero length segment.
    if( a == 0.0 )
    {
        // check if start point outside sphere radius
        if( c > 0.0 )
        {
            return false;
        }

        // length segment within radius of bounding sphere but zero length
        // so return true, and set the ratio so the start point is the one
        // to be used.
        r1 = 1.0F;
        r2 = 0.0F;
        return true;
    }

    value_type b = osg::dot( sm, se ) * 2.0;

    value_type d = b * b - 4.0F * a * c;

    if( d < 0.0F )
    {
        return false;
    }

    d              = ( value_type )sqrt( d );

    value_type div = 1.0F / ( 2.0 * a );

    r1             = ( ( -b - d ) * div );
    r2             = ( ( -b + d ) * div );

    if( r1 <= 0.0 && r2 <= 0.0 )
    {
        return false;
    }

    if( r1 >= 1.0 && r2 >= 1.0 )
    {
        return false;
    }

    return true;
}

bool
LineSegment::intersect( const sphere& bs ) const
{
    vec_type   sm = _s - dvec3( bs.center );
    value_type c  = osg::length2( sm ) - bs.radius * bs.radius;
    if( c < 0.0F )
    {
        return true;
    }

    vec_type   se = _e - _s;
    value_type a  = osg::length2( se );

    value_type b  = ( osg::dot( sm, se ) ) * 2.0F;

    value_type d  = b * b - 4.0F * a * c;

    if( d < 0.0F )
    {
        return false;
    }

    d              = ( value_type )sqrt( d );

    value_type div = 1.0F / ( 2.0F * a );

    value_type r1  = ( -b - d ) * div;
    value_type r2  = ( -b + d ) * div;

    if( r1 <= 0.0F && r2 <= 0.0F )
    {
        return false;
    }

    if( r1 >= 1.0F && r2 >= 1.0F )
    {
        return false;
    }

    return true;
}

bool
LineSegment::intersect( const vec3& v1,
                        const vec3& v2,
                        const vec3& v3,
                        float&      r )
{
    if( v1 == v2 || v2 == v3 || v1 == v3 )
    {
        return false;
    }

    vec_type   vse  = _e - _s;

    vec_type   v12  = dvec3( v2 ) - dvec3( v1 );
    vec_type   n12  = osg::cross( v12, vse );
    value_type ds12 = osg::dot( _s - dvec3( v1 ), n12 );
    value_type d312 = osg::dot( dvec3( v3 ) - dvec3( v1 ), n12 );
    if( d312 >= 0.0 )
    {
        if( ds12 < 0.0 )
        {
            return false;
        }
        if( ds12 > d312 )
        {
            return false;
        }
    }
    else    // d312 < 0
    {
        if( ds12 > 0.0 )
        {
            return false;
        }
        if( ds12 < d312 )
        {
            return false;
        }
    }

    vec_type   v23  = dvec3( v3 ) - dvec3( v2 );
    vec_type   n23  = osg::cross( v23, vse );
    value_type ds23 = osg::dot( _s - dvec3( v2 ), n23 );
    value_type d123 = osg::dot( dvec3( v1 ) - dvec3( v2 ), n23 );
    if( d123 >= 0.0 )
    {
        if( ds23 < 0.0 )
        {
            return false;
        }
        if( ds23 > d123 )
        {
            return false;
        }
    }
    else    // d123 < 0
    {
        if( ds23 > 0.0 )
        {
            return false;
        }
        if( ds23 < d123 )
        {
            return false;
        }
    }

    vec_type   v31  = dvec3( v1 ) - dvec3( v3 );
    vec_type   n31  = osg::cross( v31, vse );
    value_type ds31 = osg::dot( _s - dvec3( v3 ), n31 );
    value_type d231 = osg::dot( dvec3( v2 ) - dvec3( v3 ), n31 );
    if( d231 >= 0.0 )
    {
        if( ds31 < 0.0 )
        {
            return false;
        }
        if( ds31 > d231 )
        {
            return false;
        }
    }
    else    // d231 < 0
    {
        if( ds31 > 0.0 )
        {
            return false;
        }
        if( ds31 < d231 )
        {
            return false;
        }
    }

    value_type r3 = ds12 / d312;
    value_type r1 = ds23 / d123;
    value_type r2 = ds31 / d231;

    // value_type rt = r1+r2+r3;

    vec_type   in      = dvec3( v1 ) * r1 + dvec3( v2 ) * r2 + dvec3( v3 ) * r3;

    value_type length  = osg::length( vse );
    vse               /= length;
    value_type d       = osg::dot( in - _s, vse );

    if( d < 0.0 )
    {
        return false;
    }
    if( d > length )
    {
        return false;
    }

    r = static_cast<float>( d / length );

    return true;
}

bool
LineSegment::intersect( const dvec3& v1,
                        const dvec3& v2,
                        const dvec3& v3,
                        double&      r )
{
    if( v1 == v2 || v2 == v3 || v1 == v3 )
    {
        return false;
    }

    vec_type   vse  = _e - _s;

    vec_type   v12  = v2 - v1;
    vec_type   n12  = osg::cross( v12, vse );
    value_type ds12 = osg::dot( _s - v1, n12 );
    value_type d312 = osg::dot( v3 - v1, n12 );
    if( d312 >= 0.0 )
    {
        if( ds12 < 0.0 )
        {
            return false;
        }
        if( ds12 > d312 )
        {
            return false;
        }
    }
    else    // d312 < 0
    {
        if( ds12 > 0.0 )
        {
            return false;
        }
        if( ds12 < d312 )
        {
            return false;
        }
    }

    vec_type   v23  = v3 - v2;
    vec_type   n23  = osg::cross( v23, vse );
    value_type ds23 = osg::dot( _s - v2, n23 );
    value_type d123 = osg::dot( v1 - v2, n23 );
    if( d123 >= 0.0 )
    {
        if( ds23 < 0.0 )
        {
            return false;
        }
        if( ds23 > d123 )
        {
            return false;
        }
    }
    else    // d123 < 0
    {
        if( ds23 > 0.0 )
        {
            return false;
        }
        if( ds23 < d123 )
        {
            return false;
        }
    }

    vec_type   v31  = v1 - v3;
    vec_type   n31  = osg::cross( v31, vse );
    value_type ds31 = osg::dot( _s - v3, n31 );
    value_type d231 = osg::dot( v2 - v3, n31 );
    if( d231 >= 0.0 )
    {
        if( ds31 < 0.0 )
        {
            return false;
        }
        if( ds31 > d231 )
        {
            return false;
        }
    }
    else    // d231 < 0
    {
        if( ds31 > 0.0 )
        {
            return false;
        }
        if( ds31 < d231 )
        {
            return false;
        }
    }

    value_type r3 = ds12 / d312;
    value_type r1 = ds23 / d123;
    value_type r2 = ds31 / d231;

    // value_type rt = r1+r2+r3;

    vec_type   in      = v1 * r1 + v2 * r2 + v3 * r3;

    value_type length  = osg::length( vse );
    vse               /= length;
    value_type d       = osg::dot( in - _s, vse );

    if( d < 0.0 )
    {
        return false;
    }
    if( d > length )
    {
        return false;
    }

    r = d / length;

    return true;
}
