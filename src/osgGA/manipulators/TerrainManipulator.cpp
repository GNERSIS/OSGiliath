/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Terrain-aware orbit manipulator. Constrains the camera above
 * the terrain surface for landscape viewing.
 */
#include <osgGA/manipulators/TerrainManipulator.hpp>

#include <osg/core/io_utils.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osgUtil/intersection/LineSegmentIntersector.hpp>

using namespace osg;
using namespace osgGA;

/// Constructor.
TerrainManipulator::TerrainManipulator( int flags ) :
    Inherit( flags )
{
}

/// Constructor.
TerrainManipulator::TerrainManipulator( const TerrainManipulator& tm,
                                        const CopyOp&             copyOp ) :
    osg::Object( tm,
                 copyOp ),
    osg::Callback( tm,
                   copyOp ),
    Inherit( tm,
             copyOp ),
    _previousUp( tm._previousUp )
{
}

/** Sets the manipulator rotation mode. RotationMode is now deprecated by
    osgGA::StandardManipulator::setVerticalAxisFixed() functionality,
    that is used across StandardManipulator derived classes.*/
void
TerrainManipulator::setRotationMode( TerrainManipulator::RotationMode mode )
{
    setVerticalAxisFixed( mode == ELEVATION_AZIM );
}

/** Returns the manipulator rotation mode.*/
TerrainManipulator::RotationMode
TerrainManipulator::getRotationMode() const
{
    return getVerticalAxisFixed() ? ELEVATION_AZIM : ELEVATION_AZIM_ROLL;
}

void
TerrainManipulator::setNode( Node* node )
{
    inherited::setNode( node );

    // update model size
    if( _flags & UPDATE_MODEL_SIZE )
    {
        if( _node.valid() )
        {
            setMinimumDistance( clampBetween( _modelSize * 0.001, 0.00001, 1.0 ) );
            OSG_INFO << "TerrainManipulator: setting _minimumDistance to "
                     << _minimumDistance << std::endl;
        }
    }
}

void
TerrainManipulator::setByMatrix( const dmat4& matrix )
{

    dvec3 lookVector( -matrix( 2, 0 ), -matrix( 2, 1 ), -matrix( 2, 2 ) );
    dvec3 eye( matrix( 3, 0 ), matrix( 3, 1 ), matrix( 3, 2 ) );

    OSG_INFO << "eye point " << eye << std::endl;
    OSG_INFO << "lookVector " << lookVector << std::endl;

    if( !_node )
    {
        _center   = eye + lookVector;
        _distance = osg::length( lookVector );
        _rotation = osg::getRotate( matrix );
        return;
    }

    // need to reintersect with the terrain
    const sphere& bs    = _node->getBound();
    float distance      = static_cast<float>( osg::length( dvec3( eye - bs.center ) ) +
                                              _node->getBound().radius );
    dvec3 start_segment = eye;
    dvec3 end_segment   = eye + lookVector * distance;

    dvec3 ip;
    bool  hitFound = false;
    if( intersect( start_segment, end_segment, ip ) )
    {
        OSG_INFO << "Hit terrain ok A" << std::endl;
        _center   = ip;

        _distance = osg::length( eye - ip );

        dmat4 rotation_matrix =
            osg::translate( -_center ) * matrix * osg::translate( 0.0, 0.0, -_distance );

        _rotation = osg::getRotate( rotation_matrix );

        hitFound  = true;
    }

    if( !hitFound )
    {
        CoordinateFrame eyePointCoordFrame = getCoordinateFrame( eye );

        if( intersect( eye + getUpVector( eyePointCoordFrame ) * distance,
                       eye - getUpVector( eyePointCoordFrame ) * distance,
                       ip ) )
        {
            _center   = ip;

            _distance = osg::length( eye - ip );

            _rotation.set( 0, 0, 0, 1 );

            hitFound = true;
        }
    }

    CoordinateFrame coordinateFrame = getCoordinateFrame( _center );
    _previousUp                     = getUpVector( coordinateFrame );

    clampOrientation();
}

void
TerrainManipulator::setTransformation( const osg::dvec3& eye,
                                       const osg::dvec3& center,
                                       const osg::dvec3& up )
{
    if( !_node )
    {
        return;
    }

    // compute rotation matrix
    dvec3 lv( center - eye );
    _distance = osg::length( lv );
    _center   = center;

    OSG_INFO << "In compute" << std::endl;

    if( _node.valid() )
    {
        bool   hitFound = false;

        double distance = osg::length( lv );
        double maxDistance =
            distance + 2 * osg::length( dvec3( eye - _node->getBound().center ) );
        dvec3 farPosition = eye + lv * ( maxDistance / distance );
        dvec3 endPoint    = center;
        for( int i = 0; !hitFound && i < 2; ++i, endPoint = farPosition )
        {
            // compute the intersection with the scene.

            dvec3 ip;
            if( intersect( eye, endPoint, ip ) )
            {
                _center   = ip;
                _distance = osg::length( ip - eye );

                hitFound  = true;
            }
        }
    }

    // note LookAt = inv(CF)*inv(RM)*inv(T) which is equivalent to:
    // inv(R) = CF*LookAt.

    dmat4 rotation_matrix           = osg::lookAt( eye, center, up );

    _rotation                       = osg::inverse( osg::getRotate( rotation_matrix ) );

    CoordinateFrame coordinateFrame = getCoordinateFrame( _center );
    _previousUp                     = getUpVector( coordinateFrame );

    clampOrientation();
}

bool
TerrainManipulator::intersect( const dvec3& start,
                               const dvec3& end,
                               dvec3&       intersection ) const
{
    ref_ptr<osgUtil::LineSegmentIntersector> lsi =
        new osgUtil::LineSegmentIntersector( start, end );

    osgUtil::IntersectionVisitor iv( lsi.get() );
    iv.setTraversalMask( _intersectTraversalMask );

    _node->accept( iv );

    if( lsi->containsIntersections() )
    {
        intersection = lsi->getIntersections().begin()->getWorldIntersectPoint();
        return true;
    }
    return false;
}

bool
TerrainManipulator::performMovementMiddleMouseButton( const double eventTimeDelta,
                                                      const double dx,
                                                      const double dy )
{
    // pan model.
    double scale = -0.3F * _distance * getThrowScale( eventTimeDelta );

    dmat4  rotation_matrix;
    rotation_matrix = osg::rotate( _rotation );

    // compute look vector.
    dvec3 sideVector = getSideVector( rotation_matrix );

    // CoordinateFrame coordinateFrame = getCoordinateFrame(_center);
    // dvec3 localUp = getUpVector(coordinateFrame);
    dvec3 localUp        = _previousUp;

    dvec3 forwardVector  = localUp ^ sideVector;
    sideVector           = forwardVector ^ localUp;

    forwardVector        = osg::normalize( forwardVector );
    sideVector           = osg::normalize( sideVector );

    dvec3 dv             = forwardVector * ( dy * scale ) + sideVector * ( dx * scale );

    _center             += dv;

    // need to recompute the intersection point along the look vector.

    bool hitFound = false;

    if( _node.valid() )
    {
        // now reorientate the coordinate frame to the frame coords.
        CoordinateFrame coordinateFrame = getCoordinateFrame( _center );

        // need to reintersect with the terrain
        double          distance = _node->getBound().radius * 0.25F;

        dvec3           ip1;
        dvec3           ip2;
        bool hit_ip1 = intersect( _center,
                                  _center + getUpVector( coordinateFrame ) * distance,
                                  ip1 );
        bool hit_ip2 = intersect( _center,
                                  _center - getUpVector( coordinateFrame ) * distance,
                                  ip2 );
        if( hit_ip1 )
        {
            if( hit_ip2 )
            {
                _center  = osg::length2( _center - ip1 ) < osg::length2( _center - ip2 )
                             ? ip1
                             : ip2;

                hitFound = true;
            }
            else
            {
                _center  = ip1;
                hitFound = true;
            }
        }
        else if( hit_ip2 )
        {
            _center  = ip2;
            hitFound = true;
        }

        if( !hitFound )
        {
            // ??
            OSG_INFO << "TerrainManipulator unable to intersect with terrain."
                     << std::endl;
        }

        coordinateFrame   = getCoordinateFrame( _center );
        dvec3 new_localUp = getUpVector( coordinateFrame );

        quat  pan_rotation;
        pan_rotation = quat( dquat( localUp, new_localUp ) );

        if( !osg::zeroRotation( pan_rotation ) )
        {
            _rotation   = _rotation * pan_rotation;
            _previousUp = new_localUp;
            // OSG_NOTICE<<"Rotating from "<<localUp<<" to "<<new_localUp<<"  angle =
            // "<<acos(localUp*new_localUp/(osg::length(localUp)*osg::length(new_localUp)))<<std::endl;

            // clampOrientation();
        }
        else
        {
            OSG_INFO << "New up orientation nearly inline - no need to rotate"
                     << std::endl;
        }
    }

    return true;
}

bool
TerrainManipulator::performMovementRightMouseButton( const double eventTimeDelta,
                                                     const double /*dx*/,
                                                     const double dy )
{
    // zoom model
    zoomModel( static_cast<float>( dy * getThrowScale( eventTimeDelta ) ), false );
    return true;
}

void
TerrainManipulator::clampOrientation()
{
    if( !getVerticalAxisFixed() )
    {
        dmat4 rotation_matrix;
        rotation_matrix                 = osg::rotate( dquat( _rotation ) );

        dvec3           lookVector      = -getUpVector( rotation_matrix );
        dvec3           upVector        = getFrontVector( rotation_matrix );

        CoordinateFrame coordinateFrame = getCoordinateFrame( _center );
        dvec3           localUp         = getUpVector( coordinateFrame );
        // dvec3 localUp = _previousUp;

        dvec3           sideVector = lookVector ^ localUp;

        if( osg::length( sideVector ) < 0.1 )
        {
            OSG_INFO << "Side vector short " << osg::length( sideVector ) << std::endl;

            sideVector = upVector ^ localUp;
            sideVector = osg::normalize( sideVector );
        }

        dvec3 newUpVector = sideVector ^ lookVector;
        newUpVector       = osg::normalize( newUpVector );

        quat rotate_roll;
        rotate_roll = quat( dquat( upVector, newUpVector ) );

        if( !osg::zeroRotation( rotate_roll ) )
        {
            _rotation = _rotation * rotate_roll;
        }
    }
}
