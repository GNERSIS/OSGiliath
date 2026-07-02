/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Height-above-terrain query. Samples terrain elevation at
 * specified positions for ground-following and clearance checks.
 */
#include <osgSim/HeightAboveTerrain.hpp>

#include <osg/core/Notify.hpp>
#include <osg/nodes/CoordinateSystemNode.hpp>
#include <osgUtil/intersection/LineSegmentIntersector.hpp>

using namespace osgSim;

HeightAboveTerrain::HeightAboveTerrain()
{
    _lowestHeight = -1000.0;

    setDatabaseCacheReadCallback( new DatabaseCacheReadCallback );
}

void
HeightAboveTerrain::clear()
{
    _HATList.clear();
}

unsigned int
HeightAboveTerrain::addPoint( const osg::dvec3& point )
{
    unsigned int index = static_cast<unsigned int>( _HATList.size() );
    _HATList.push_back( HAT( point ) );
    return index;
}

void
HeightAboveTerrain::computeIntersections( osg::Node*          scene,
                                          osg::Node::NodeMask traversalMask )
{
    osg::CoordinateSystemNode* csn = dynamic_cast<osg::CoordinateSystemNode*>( scene );
    osg::EllipsoidModel*       em  = csn ? csn->getEllipsoidModel() : 0;

    osg::ref_ptr<osgUtil::IntersectorGroup> intersectorGroup =
        new osgUtil::IntersectorGroup();

    for( HATList::iterator itr = _HATList.begin(); itr != _HATList.end(); ++itr )
    {
        if( em )
        {

            osg::dvec3 start    = itr->_point;
            osg::dvec3 upVector = em->computeLocalUpVector( start.x, start.y, start.z );

            double     latitude, longitude, height;
            em->convertXYZToLatLongHeight( start.x,
                                           start.y,
                                           start.z,
                                           latitude,
                                           longitude,
                                           height );
            osg::dvec3 end = start - upVector * ( height - _lowestHeight );

            itr->_hat      = height;

            OSG_NOTICE << "lat = " << latitude << " longitude = " << longitude
                       << " height = " << height << std::endl;

            osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector =
                new osgUtil::LineSegmentIntersector( start, end );
            intersectorGroup->addIntersector( intersector.get() );
        }
        else
        {
            osg::dvec3 start = itr->_point;
            osg::dvec3 upVector( 0.0, 0.0, 1.0 );

            double     height = start.z;
            osg::dvec3 end    = start - upVector * ( height - _lowestHeight );

            itr->_hat         = height;

            osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector =
                new osgUtil::LineSegmentIntersector( start, end );
            intersectorGroup->addIntersector( intersector.get() );
        }
    }

    _intersectionVisitor.reset();
    _intersectionVisitor.setTraversalMask( traversalMask );
    _intersectionVisitor.setIntersector( intersectorGroup.get() );

    scene->accept( _intersectionVisitor );

    unsigned int                             index = 0;
    osgUtil::IntersectorGroup::Intersectors& intersectors =
        intersectorGroup->getIntersectors();
    for( osgUtil::IntersectorGroup::Intersectors::iterator intersector_itr =
             intersectors.begin();
         intersector_itr != intersectors.end();
         ++intersector_itr, ++index )
    {
        osgUtil::LineSegmentIntersector* lsi =
            dynamic_cast<osgUtil::LineSegmentIntersector*>( intersector_itr->get() );
        if( lsi )
        {
            osgUtil::LineSegmentIntersector::Intersections& intersections =
                lsi->getIntersections();
            if( !intersections.empty() )
            {
                const osgUtil::LineSegmentIntersector::Intersection& intersection =
                    *intersections.begin();
                osg::dvec3 intersectionPoint =
                    intersection.matrix.valid()
                        ? intersection.localIntersectionPoint * ( *intersection.matrix )
                        : intersection.localIntersectionPoint;
                _HATList[index]._hat =
                    osg::length( _HATList[index]._point - intersectionPoint );
            }
        }
    }
}

double
HeightAboveTerrain::computeHeightAboveTerrain( osg::Node*          scene,
                                               const osg::dvec3&   point,
                                               osg::Node::NodeMask traversalMask )
{
    HeightAboveTerrain hat;
    unsigned int       index = hat.addPoint( point );
    hat.computeIntersections( scene, traversalMask );
    return hat.getHeightAboveTerrain( index );
}

void
HeightAboveTerrain::setDatabaseCacheReadCallback( DatabaseCacheReadCallback* dcrc )
{
    _dcrc = dcrc;
    _intersectionVisitor.setReadCallback( dcrc );
}
