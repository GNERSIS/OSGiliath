/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Group with distance-based visibility switching.
 * Hides children beyond a configured visibility range.
 */
#include <osgSim/VisibilityGroup>

#include <osgUtil/culling/CullVisitor.hpp>
#include <osgUtil/intersection/LineSegmentIntersector.hpp>

using namespace osgSim;
using namespace osg;

VisibilityGroup::VisibilityGroup() :
    _volumeIntersectionMask( 0XFF'FF'FF'FF ),
    _segmentLength( 0.F )
{
}

VisibilityGroup::VisibilityGroup( const VisibilityGroup& sw,
                                  const osg::CopyOp&     copyop ) :
    Inherit( sw,
             copyop ),
    _volumeIntersectionMask( 0XFF'FF'FF'FF ),
    _segmentLength( 0.F )
{
}

void
VisibilityGroup::traverse( osg::NodeVisitor& nv )
{
    if( nv.getTraversalMode() ==
        osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN &&
        nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR )
    {
        // cast to cullvisitor
        osgUtil::CullVisitor& cv = ( osgUtil::CullVisitor& )nv;

        // here we test if we are inside the visibilityvolume

        // first get the eyepoint and in local coordinates
        osg::vec3             eye  = cv.getEyeLocal();
        osg::vec3             look = cv.getLookVectorLocal();

        // now scale the segment to the segment length - if 0 use the group bounding
        // sphere radius
        float                 length = _segmentLength;
        if( length == 0.F )
        {
            length = 2.0F * getBound().radius;
        }
        look                                                 *= length;
        osg::vec3                                     center  = eye + look;

        osg::vec3                                     seg     = center - eye;

        // perform the intersection using the given mask
        osg::ref_ptr<osgUtil::LineSegmentIntersector> lineseg =
            new osgUtil::LineSegmentIntersector( osg::dvec3( eye ),
                                                 osg::dvec3( center ) );
        osgUtil::IntersectionVisitor iv( lineseg.get() );
        iv.setTraversalMask( _volumeIntersectionMask );

        if( _visibilityVolume.valid() )
        {
            _visibilityVolume->accept( iv );
        }

        // now examine the hit record
        if( lineseg->containsIntersections() )
        {
            osgUtil::LineSegmentIntersector::Intersection intersection =
                lineseg->getFirstIntersection();
            osg::vec3 normal = intersection.getWorldIntersectNormal();

            if( osg::dot( normal, seg ) > 0.F )    // we are inside
            {
                Group::traverse( nv );
            }
        }
    }
    else
    {
        Group::traverse( nv );
    }
}
