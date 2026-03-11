/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Ray/line-segment intersection tester. Finds geometry hits along
 * a ray for picking, collision, and line-of-sight queries.
 */
#include <osgUtil/intersection/LineSegmentIntersector.hpp>

#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osg/core/Timer.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/KdTree.hpp>
#include <osg/geometry/TemplatePrimitiveFunctor.hpp>
#include <osg/geometry/TriangleFunctor.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>

using namespace osgUtil;

namespace LineSegmentIntersectorUtils
{

    struct Settings
    {
            Settings() :
                _lineSegIntersector( 0 ),
                _iv( 0 ),
                _drawable( 0 ),
                _limitOneIntersection( false )
            {
            }

            osgUtil::LineSegmentIntersector* _lineSegIntersector;
            osgUtil::IntersectionVisitor*    _iv;
            osg::Drawable*                   _drawable;
            osg::ref_ptr<osg::Vec3Array>     _vertices;
            bool                             _limitOneIntersection;
    };

    template<typename vec3, typename value_type>
    struct IntersectFunctor
    {
            Settings*                     _settings;

            unsigned int                  _primitiveIndex;
            vec3                          _start;
            vec3                          _end;

            typedef std::pair<vec3, vec3> StartEnd;
            typedef std::vector<StartEnd> StartEndStack;

            StartEndStack                 _startEndStack;

            vec3                          _d;
            value_type                    _length;
            value_type                    _inverse_length;

            vec3                          _d_invX;
            vec3                          _d_invY;
            vec3                          _d_invZ;

            bool                          _hit;

            IntersectFunctor() :
                _primitiveIndex( 0 ),
                _length( 0.0 ),
                _inverse_length( 0.0 ),
                _hit( false )
            {
            }

            void
            set( const osg::dvec3& s,
                 const osg::dvec3& e,
                 Settings*         settings )
            {
                _settings = settings;

                _start    = s;
                _end      = e;

                _startEndStack.push_back( StartEnd( _start, _end ) );

                _d               = e - s;
                _length          = osg::length( _d );
                _inverse_length  = ( _length != static_cast<value_type>( 0.0 ) )
                                     ? static_cast<value_type>( 1.0 / _length )
                                     : static_cast<value_type>( 0.0 );
                _d              *= _inverse_length;

                _d_invX          = _d.x != 0.0 ? _d / _d.x : vec3( 0.0, 0.0, 0.0 );
                _d_invY          = _d.y != 0.0 ? _d / _d.y : vec3( 0.0, 0.0, 0.0 );
                _d_invZ          = _d.z != 0.0 ? _d / _d.z : vec3( 0.0, 0.0, 0.0 );
            }

            bool
            enter( const osg::box& bb )
            {
                StartEnd startend = _startEndStack.back();
                vec3&    s        = startend.first;
                vec3&    e        = startend.second;

                // return true;

                // if (!bb.valid()) return true;

                // compare s and e against the xMin to xMax range of bb.
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
                        s = s + _d_invX * ( bb.xMin() - s.x );
                    }

                    if( e.x > bb.xMax() )
                    {
                        // clip e to xMax.
                        e = s + _d_invX * ( bb.xMax() - s.x );
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
                        e = s + _d_invX * ( bb.xMin() - s.x );
                    }

                    if( s.x > bb.xMax() )
                    {
                        // clip e to xMax.
                        s = s + _d_invX * ( bb.xMax() - s.x );
                    }
                }

                // compare s and e against the yMin to yMax range of bb.
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
                        s = s + _d_invY * ( bb.yMin() - s.y );
                    }

                    if( e.y > bb.yMax() )
                    {
                        // clip e to yMax.
                        e = s + _d_invY * ( bb.yMax() - s.y );
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
                        e = s + _d_invY * ( bb.yMin() - s.y );
                    }

                    if( s.y > bb.yMax() )
                    {
                        // clip e to yMax.
                        s = s + _d_invY * ( bb.yMax() - s.y );
                    }
                }

                // compare s and e against the zMin to zMax range of bb.
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
                        s = s + _d_invZ * ( bb.zMin() - s.z );
                    }

                    if( e.z > bb.zMax() )
                    {
                        // clip e to zMax.
                        e = s + _d_invZ * ( bb.zMax() - s.z );
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
                        e = s + _d_invZ * ( bb.zMin() - s.z );
                    }

                    if( s.z > bb.zMax() )
                    {
                        // clip e to zMax.
                        s = s + _d_invZ * ( bb.zMax() - s.z );
                    }
                }

                // OSG_NOTICE<<"clampped segment "<<s<<" "<<e<<std::endl;
                _startEndStack.push_back( startend );

                return true;
            }

            void
            leave()
            {
                // OSG_NOTICE<<"leave() "<<_startEndStack.size()<<std::endl;
                _startEndStack.pop_back();
            }

            void
            intersect( const osg::vec3& v0,
                       const osg::vec3& v1,
                       const osg::vec3& v2 )
            {
                if( _settings->_limitOneIntersection && _hit )
                {
                    return;
                }

                // const StartEnd startend = _startEndStack.back();
                // const osg::vec3& ls = startend.first;
                // const osg::vec3& le = startend.second;

                vec3             T   = _start - vec3( static_cast<value_type>( v0.x ),
                                                      static_cast<value_type>( v0.y ),
                                                      static_cast<value_type>( v0.z ) );
                vec3             E2  = vec3( static_cast<value_type>( v2.x - v0.x ),
                                             static_cast<value_type>( v2.y - v0.y ),
                                             static_cast<value_type>( v2.z - v0.z ) );
                vec3             E1  = vec3( static_cast<value_type>( v1.x - v0.x ),
                                             static_cast<value_type>( v1.y - v0.y ),
                                             static_cast<value_type>( v1.z - v0.z ) );

                vec3             P   = osg::cross( _d, E2 );

                value_type       det = osg::dot( P, E1 );

                value_type       r, r0, r1, r2;

                const value_type epsilon = static_cast<value_type>( 1E-10 );
                if( det > epsilon )
                {
                    value_type u = ( osg::dot( vec3( P ), vec3( T ) ) );
                    if( u < 0.0 || u > det )
                    {
                        return;
                    }

                    vec3       Q = osg::cross( T, E1 );
                    value_type v = ( osg::dot( vec3( Q ), vec3( _d ) ) );
                    if( v < 0.0 || v > det )
                    {
                        return;
                    }

                    if( ( u + v ) > det )
                    {
                        return;
                    }

                    value_type inv_det = static_cast<value_type>( 1.0 / det );
                    value_type t       = ( osg::dot( vec3( Q ), vec3( E2 ) ) ) * inv_det;
                    if( t < 0.0 || t > _length )
                    {
                        return;
                    }

                    u  *= inv_det;
                    v  *= inv_det;

                    r0  = static_cast<value_type>( 1.0 ) - u - v;
                    r1  = u;
                    r2  = v;
                    r   = t * _inverse_length;
                }
                else if( det < -epsilon )
                {
                    value_type u = ( osg::dot( vec3( P ), vec3( T ) ) );
                    if( u > 0.0 || u < det )
                    {
                        return;
                    }

                    vec3       Q = osg::cross( T, E1 );
                    value_type v = ( osg::dot( vec3( Q ), vec3( _d ) ) );
                    if( v > 0.0 || v < det )
                    {
                        return;
                    }

                    if( ( u + v ) < det )
                    {
                        return;
                    }

                    value_type inv_det = static_cast<value_type>( 1.0 / det );
                    value_type t       = ( osg::dot( vec3( Q ), vec3( E2 ) ) ) * inv_det;
                    if( t < 0.0 || t > _length )
                    {
                        return;
                    }

                    u  *= inv_det;
                    v  *= inv_det;

                    r0  = static_cast<value_type>( 1.0 ) - u - v;
                    r1  = u;
                    r2  = v;
                    r   = t * _inverse_length;
                }
                else
                {
                    return;
                }

                // Remap ratio into the range of LineSegment
                const osg::dvec3& lsStart = _settings->_lineSegIntersector->getStart();
                const osg::dvec3& lsEnd   = _settings->_lineSegIntersector->getEnd();
                double            remap_ratio =
                    ( osg::length( osg::dvec3( _start ) - lsStart ) + r * _length ) /
                    osg::length( lsEnd - lsStart );

                vec3 in = vec3(
                    lsStart * ( 1.0 - remap_ratio ) + lsEnd * remap_ratio
                );    // == v0*r0 + v1*r1 + v2*r2;
                vec3 normal = osg::cross( E1, E2 );
                normal      = osg::normalize( normal );

                LineSegmentIntersector::Intersection hit;
                hit.ratio                   = remap_ratio;
                hit.matrix                  = _settings->_iv->getModelMatrix();
                hit.nodePath                = _settings->_iv->getNodePath();
                hit.drawable                = _settings->_drawable;
                hit.primitiveIndex          = _primitiveIndex;

                hit.localIntersectionPoint  = in;
                hit.localIntersectionNormal = normal;

                if( _settings->_vertices.valid() )
                {
                    const osg::vec3* first = &( _settings->_vertices->front() );
                    hit.indexList.reserve( 3 );
                    hit.ratioList.reserve( 3 );

                    if( r0 != 0.0F )
                    {
                        hit.indexList.push_back( static_cast<unsigned int>( &v0 -
                                                                            first ) );
                        hit.ratioList.push_back( r0 );
                    }

                    if( r1 != 0.0F )
                    {
                        hit.indexList.push_back( static_cast<unsigned int>( &v1 -
                                                                            first ) );
                        hit.ratioList.push_back( r1 );
                    }

                    if( r2 != 0.0F )
                    {
                        hit.indexList.push_back( static_cast<unsigned int>( &v2 -
                                                                            first ) );
                        hit.ratioList.push_back( r2 );
                    }
                }

                _settings->_lineSegIntersector->insertIntersection( hit );
                _hit = true;
            }

            // handle lines
            void
            operator()( const osg::vec3&,
                        bool /*treatVertexDataAsTemporary*/ )
            {
                ++_primitiveIndex;
            }

            void
            operator()( const osg::vec3&,
                        const osg::vec3&,
                        bool /*treatVertexDataAsTemporary*/ )
            {
                ++_primitiveIndex;
            }

            // handle triangles
            void
            operator()( const osg::vec3& v0,
                        const osg::vec3& v1,
                        const osg::vec3& v2,
                        bool /*treatVertexDataAsTemporary*/ )
            {
                intersect( v0, v1, v2 );
                ++_primitiveIndex;
            }

            void
            operator()( const osg::vec3& v0,
                        const osg::vec3& v1,
                        const osg::vec3& v2,
                        const osg::vec3& v3,
                        bool /*treatVertexDataAsTemporary*/ )
            {
                intersect( v0, v1, v3 );
                intersect( v1, v2, v3 );
                ++_primitiveIndex;
            }

            void
            intersect( const osg::Vec3Array*,
                       int,
                       unsigned int )
            {
            }

            void
            intersect( const osg::Vec3Array*,
                       int,
                       unsigned int,
                       unsigned int )
            {
            }

            void
            intersect( const osg::Vec3Array* vertices,
                       int                   primitiveIndex,
                       unsigned int          p0,
                       unsigned int          p1,
                       unsigned int          p2 )
            {
                if( _settings->_limitOneIntersection && _hit )
                {
                    return;
                }

                _primitiveIndex = static_cast<unsigned int>( primitiveIndex );

                intersect( ( *vertices )[p0], ( *vertices )[p1], ( *vertices )[p2] );
            }

            void
            intersect( const osg::Vec3Array* vertices,
                       int                   primitiveIndex,
                       unsigned int          p0,
                       unsigned int          p1,
                       unsigned int          p2,
                       unsigned int          p3 )
            {
                if( _settings->_limitOneIntersection && _hit )
                {
                    return;
                }

                _primitiveIndex = static_cast<unsigned int>( primitiveIndex );

                intersect( ( *vertices )[p0], ( *vertices )[p1], ( *vertices )[p3] );
                intersect( ( *vertices )[p1], ( *vertices )[p2], ( *vertices )[p3] );
            }
    };

}    // namespace LineSegmentIntersectorUtils

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  LineSegmentIntersector
//

LineSegmentIntersector::LineSegmentIntersector( const osg::dvec3& start,
                                                const osg::dvec3& end ) :
    _parent( 0 ),
    _start( start ),
    _end( end )
{
}

LineSegmentIntersector::LineSegmentIntersector(
    CoordinateFrame                         cf,
    const osg::dvec3&                       start,
    const osg::dvec3&                       end,
    LineSegmentIntersector*                 parent,
    osgUtil::Intersector::IntersectionLimit intersectionLimit
) :
    Intersector( cf,
                 intersectionLimit ),
    _parent( parent ),
    _start( start ),
    _end( end )
{
}

LineSegmentIntersector::LineSegmentIntersector( CoordinateFrame cf,
                                                double          x,
                                                double          y ) :
    Intersector( cf ),
    _parent( 0 )
{
    switch( cf )
    {
        case WINDOW :
            _start.set( x, y, 0.0 );
            _end.set( x, y, 1.0 );
            break;
        case PROJECTION :
            _start.set( x, y, -1.0 );
            _end.set( x, y, 1.0 );
            break;
        case VIEW :
            _start.set( x, y, 0.0 );
            _end.set( x, y, 1.0 );
            break;
        case MODEL :
            _start.set( x, y, 0.0 );
            _end.set( x, y, 1.0 );
            break;
    }
}

Intersector*
LineSegmentIntersector::clone( osgUtil::IntersectionVisitor& iv )
{
    if( _coordinateFrame == MODEL && iv.getModelMatrix() == 0 )
    {
        osg::ref_ptr<LineSegmentIntersector> lsi =
            new LineSegmentIntersector( _start, _end );
        lsi->_parent            = this;
        lsi->_intersectionLimit = this->_intersectionLimit;
        lsi->setPrecisionHint( getPrecisionHint() );
        return lsi.release();
    }

    // compute the matrix that takes this Intersector from its CoordinateFrame into the
    // local MODEL coordinate frame that geometry in the scene graph will always be in.
    osg::dmat4 matrix( getTransformation( iv, _coordinateFrame ) );

    osg::ref_ptr<LineSegmentIntersector> lsi =
        new LineSegmentIntersector( matrix * _start, matrix * _end );
    lsi->_parent            = this;
    lsi->_intersectionLimit = this->_intersectionLimit;
    lsi->setPrecisionHint( getPrecisionHint() );
    return lsi.release();
}

osg::dmat4
LineSegmentIntersector::getTransformation( IntersectionVisitor& iv,
                                           CoordinateFrame      cf )
{
    osg::dmat4 matrix;
    switch( cf )
    {
        case( WINDOW ) :
            if( iv.getModelMatrix() )
            {
                matrix = *iv.getModelMatrix();
            }
            if( iv.getViewMatrix() )
            {
                matrix = *iv.getViewMatrix() * matrix;
            }
            if( iv.getProjectionMatrix() )
            {
                matrix = *iv.getProjectionMatrix() * matrix;
            }
            if( iv.getWindowMatrix() )
            {
                matrix = *iv.getWindowMatrix() * matrix;
            }
            break;
        case( PROJECTION ) :
            if( iv.getModelMatrix() )
            {
                matrix = *iv.getModelMatrix();
            }
            if( iv.getViewMatrix() )
            {
                matrix = *iv.getViewMatrix() * matrix;
            }
            if( iv.getProjectionMatrix() )
            {
                matrix = *iv.getProjectionMatrix() * matrix;
            }
            break;
        case( VIEW ) :
            if( iv.getModelMatrix() )
            {
                matrix = *iv.getModelMatrix();
            }
            if( iv.getViewMatrix() )
            {
                matrix = *iv.getViewMatrix() * matrix;
            }
            break;
        case( MODEL ) :
            if( iv.getModelMatrix() )
            {
                matrix = *iv.getModelMatrix();
            }
            break;
    }

    osg::dmat4 inverse;
    inverse = osg::inverse( matrix );
    return inverse;
}

bool
LineSegmentIntersector::enter( const osg::Node& node )
{
    if( reachedLimit() )
    {
        return false;
    }
    return !node.isCullingActive() || intersects( node.getBound() );
}

void
LineSegmentIntersector::leave()
{
    // do nothing
}

void
LineSegmentIntersector::intersect( osgUtil::IntersectionVisitor& iv,
                                   osg::Drawable*                drawable )
{
    if( reachedLimit() )
    {
        return;
    }

    osg::dvec3 s( _start ), e( _end );
    if( drawable->isCullingActive() &&
        !intersectAndClip( s, e, drawable->getBoundingBox() ) )
    {
        return;
    }

    if( iv.getDoDummyTraversal() )
    {
        return;
    }

    intersect( iv, drawable, s, e );
}

void
LineSegmentIntersector::intersect( osgUtil::IntersectionVisitor& iv,
                                   osg::Drawable*                drawable,
                                   const osg::dvec3&             s,
                                   const osg::dvec3&             e )
{
    if( reachedLimit() )
    {
        return;
    }

    LineSegmentIntersectorUtils::Settings settings;
    settings._lineSegIntersector   = this;
    settings._iv                   = &iv;
    settings._drawable             = drawable;
    settings._limitOneIntersection = ( _intersectionLimit ==
                                       LIMIT_ONE_PER_DRAWABLE ||
                                       _intersectionLimit == LIMIT_ONE );

    osg::Geometry* geometry        = drawable->asGeometry();
    if( geometry )
    {
        settings._vertices = dynamic_cast<osg::Vec3Array*>( geometry->getVertexArray() );
    }

    osg::KdTree* kdTree = iv.getUseKdTreeWhenAvailable()
                            ? dynamic_cast<osg::KdTree*>( drawable->getShape() )
                            : 0;

    if( getPrecisionHint() == USE_DOUBLE_CALCULATIONS )
    {
        osg::TemplatePrimitiveFunctor<
            LineSegmentIntersectorUtils::IntersectFunctor<osg::dvec3, double>>
            intersector;
        intersector.set( s, e, &settings );

        if( kdTree )
        {
            kdTree->intersect( intersector, kdTree->getNode( 0 ) );
        }
        else
        {
            drawable->accept( intersector );
        }
    }
    else
    {
        osg::TemplatePrimitiveFunctor<
            LineSegmentIntersectorUtils::IntersectFunctor<osg::vec3, float>>
            intersector;
        intersector.set( s, e, &settings );

        if( kdTree )
        {
            kdTree->intersect( intersector, kdTree->getNode( 0 ) );
        }
        else
        {
            drawable->accept( intersector );
        }
    }
}

void
LineSegmentIntersector::reset()
{
    Intersector::reset();

    _intersections.clear();
}

bool
LineSegmentIntersector::intersects( const osg::sphere& bs )
{
    // if bs not valid then return true based on the assumption that an invalid sphere is
    // yet to be defined.
    if( !bs.valid() )
    {
        return true;
    }

    osg::dvec3 sm = _start - osg::dvec3( bs.center );
    double     c  = osg::length2( sm ) - bs.radius * bs.radius;
    if( c < 0.0 )
    {
        return true;
    }

    osg::dvec3 se = _end - _start;
    double     a  = osg::length2( se );
    double     b  = osg::dot( sm, se ) * 2.0;
    double     d  = b * b - 4.0 * a * c;

    if( d < 0.0 )
    {
        return false;
    }

    d          = sqrt( d );

    double div = 1.0 / ( 2.0 * a );

    double r1  = ( -b - d ) * div;
    double r2  = ( -b + d ) * div;

    if( r1 <= 0.0 && r2 <= 0.0 )
    {
        return false;
    }

    if( r1 >= 1.0 && r2 >= 1.0 )
    {
        return false;
    }

    if( _intersectionLimit == LIMIT_NEAREST && !getIntersections().empty() )
    {
        double ratio = ( osg::length( sm ) - bs.radius ) / sqrt( a );
        if( ratio >= getIntersections().begin()->ratio )
        {
            return false;
        }
    }

    // passed all the rejection tests so line must intersect bounding sphere, return
    // true.
    return true;
}

bool
LineSegmentIntersector::intersectAndClip( osg::dvec3&     s,
                                          osg::dvec3&     e,
                                          const osg::box& bbInput )
{
    osg::dvec3 bb_min( bbInput.min );
    osg::dvec3 bb_max( bbInput.max );

    double     epsilon = 1E-5;

    // compare s and e against the xMin to xMax range of bb.
    if( s.x <= e.x )
    {
        // trivial reject of segment wholely outside.
        if( e.x < bb_min.x )
        {
            return false;
        }
        if( s.x > bb_max.x )
        {
            return false;
        }

        if( s.x < bb_min.x )
        {
            // clip s to xMin.
            double r = ( bb_min.x - s.x ) / ( e.x - s.x ) - epsilon;
            if( r > 0.0 )
            {
                s = s + ( e - s ) * r;
            }
        }

        if( e.x > bb_max.x )
        {
            // clip e to xMax.
            double r = ( bb_max.x - s.x ) / ( e.x - s.x ) + epsilon;
            if( r < 1.0 )
            {
                e = s + ( e - s ) * r;
            }
        }
    }
    else
    {
        if( s.x < bb_min.x )
        {
            return false;
        }
        if( e.x > bb_max.x )
        {
            return false;
        }

        if( e.x < bb_min.x )
        {
            // clip e to xMin.
            double r = ( bb_min.x - e.x ) / ( s.x - e.x ) - epsilon;
            if( r > 0.0 )
            {
                e = e + ( s - e ) * r;
            }
        }

        if( s.x > bb_max.x )
        {
            // clip s to xMax.
            double r = ( bb_max.x - e.x ) / ( s.x - e.x ) + epsilon;
            if( r < 1.0 )
            {
                s = e + ( s - e ) * r;
            }
        }
    }

    // compare s and e against the yMin to yMax range of bb.
    if( s.y <= e.y )
    {
        // trivial reject of segment wholely outside.
        if( e.y < bb_min.y )
        {
            return false;
        }
        if( s.y > bb_max.y )
        {
            return false;
        }

        if( s.y < bb_min.y )
        {
            // clip s to yMin.
            double r = ( bb_min.y - s.y ) / ( e.y - s.y ) - epsilon;
            if( r > 0.0 )
            {
                s = s + ( e - s ) * r;
            }
        }

        if( e.y > bb_max.y )
        {
            // clip e to yMax.
            double r = ( bb_max.y - s.y ) / ( e.y - s.y ) + epsilon;
            if( r < 1.0 )
            {
                e = s + ( e - s ) * r;
            }
        }
    }
    else
    {
        if( s.y < bb_min.y )
        {
            return false;
        }
        if( e.y > bb_max.y )
        {
            return false;
        }

        if( e.y < bb_min.y )
        {
            // clip e to yMin.
            double r = ( bb_min.y - e.y ) / ( s.y - e.y ) - epsilon;
            if( r > 0.0 )
            {
                e = e + ( s - e ) * r;
            }
        }

        if( s.y > bb_max.y )
        {
            // clip s to yMax.
            double r = ( bb_max.y - e.y ) / ( s.y - e.y ) + epsilon;
            if( r < 1.0 )
            {
                s = e + ( s - e ) * r;
            }
        }
    }

    // compare s and e against the zMin to zMax range of bb.
    if( s.z <= e.z )
    {
        // trivial reject of segment wholely outside.
        if( e.z < bb_min.z )
        {
            return false;
        }
        if( s.z > bb_max.z )
        {
            return false;
        }

        if( s.z < bb_min.z )
        {
            // clip s to zMin.
            double r = ( bb_min.z - s.z ) / ( e.z - s.z ) - epsilon;
            if( r > 0.0 )
            {
                s = s + ( e - s ) * r;
            }
        }

        if( e.z > bb_max.z )
        {
            // clip e to zMax.
            double r = ( bb_max.z - s.z ) / ( e.z - s.z ) + epsilon;
            if( r < 1.0 )
            {
                e = s + ( e - s ) * r;
            }
        }
    }
    else
    {
        if( s.z < bb_min.z )
        {
            return false;
        }
        if( e.z > bb_max.z )
        {
            return false;
        }

        if( e.z < bb_min.z )
        {
            // clip e to zMin.
            double r = ( bb_min.z - e.z ) / ( s.z - e.z ) - epsilon;
            if( r > 0.0 )
            {
                e = e + ( s - e ) * r;
            }
        }

        if( s.z > bb_max.z )
        {
            // clip s to zMax.
            double r = ( bb_max.z - e.z ) / ( s.z - e.z ) + epsilon;
            if( r < 1.0 )
            {
                s = e + ( s - e ) * r;
            }
        }
    }

    // OSG_NOTICE<<"clampped segment "<<s<<" "<<e<<std::endl;

    // if (s==e) return false;

    return true;
}

osg::Texture*
LineSegmentIntersector::Intersection::getTextureLookUp( osg::vec3& tc ) const
{
    osg::Geometry*  geometry = drawable.valid() ? drawable->asGeometry() : 0;
    osg::Vec3Array* vertices =
        geometry ? dynamic_cast<osg::Vec3Array*>( geometry->getVertexArray() ) : 0;

    if( vertices )
    {
        if( indexList.size() == 3 && ratioList.size() == 3 )
        {
            unsigned int     i1        = indexList[0];
            unsigned int     i2        = indexList[1];
            unsigned int     i3        = indexList[2];

            float            r1        = static_cast<float>( ratioList[0] );
            float            r2        = static_cast<float>( ratioList[1] );
            float            r3        = static_cast<float>( ratioList[2] );

            osg::Array*      texcoords = ( geometry->getNumTexCoordArrays() > 0 )
                                           ? geometry->getTexCoordArray( 0 )
                                           : 0;
            osg::FloatArray* texcoords_FloatArray =
                dynamic_cast<osg::FloatArray*>( texcoords );
            osg::Vec2Array* texcoords_Vec2Array =
                dynamic_cast<osg::Vec2Array*>( texcoords );
            osg::Vec3Array* texcoords_Vec3Array =
                dynamic_cast<osg::Vec3Array*>( texcoords );
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
                const osg::vec2& tc1 = ( *texcoords_Vec2Array )[i1];
                const osg::vec2& tc2 = ( *texcoords_Vec2Array )[i2];
                const osg::vec2& tc3 = ( *texcoords_Vec2Array )[i3];
                tc.x                 = tc1.x * r1 + tc2.x * r2 + tc3.x * r3;
                tc.y                 = tc1.y * r1 + tc2.y * r2 + tc3.y * r3;
            }
            else if( texcoords_Vec3Array )
            {
                // we have tex coord array so now we can compute the final tex coord at
                // the point of intersection.
                const osg::vec3& tc1 = ( *texcoords_Vec3Array )[i1];
                const osg::vec3& tc2 = ( *texcoords_Vec3Array )[i2];
                const osg::vec3& tc3 = ( *texcoords_Vec3Array )[i3];
                tc.x                 = tc1.x * r1 + tc2.x * r2 + tc3.x * r3;
                tc.y                 = tc1.y * r1 + tc2.y * r2 + tc3.y * r3;
                tc.z                 = tc1.z * r1 + tc2.z * r2 + tc3.z * r3;
            }
            else
            {
                return 0;
            }
        }

        const osg::Texture* activeTexture = 0;

        if( drawable->getStateSet() )
        {
            const osg::Texture* texture = dynamic_cast<osg::Texture*>(
                drawable->getStateSet()
                    ->getTextureAttribute( 0, osg::StateAttribute::Type::TEXTURE )
            );
            if( texture )
            {
                activeTexture = texture;
            }
        }

        for( osg::NodePath::const_reverse_iterator itr = nodePath.rbegin();
             itr != nodePath.rend() && !activeTexture;
             ++itr )
        {
            const osg::Node* node = *itr;
            if( node->getStateSet() )
            {
                if( !activeTexture )
                {
                    const osg::Texture* texture = dynamic_cast<const osg::Texture*>(
                        node->getStateSet()->getTextureAttribute(
                            0,
                            osg::StateAttribute::Type::TEXTURE
                        )
                    );
                    if( texture )
                    {
                        activeTexture = texture;
                    }
                }
            }
        }

        return const_cast<osg::Texture*>( activeTexture );
    }
    return 0;
}
