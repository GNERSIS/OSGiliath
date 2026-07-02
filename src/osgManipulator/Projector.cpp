/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Projects 2D screen coordinates onto 3D lines, planes, and
 * cylinders for dragger hit calculation.
 */
// osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#include <osgManipulator/Projector.hpp>

#include <osg/maths/Math.hpp>

using namespace osgManipulator;

// When the squared magnitude (length2) of the cross product of 2
// angles is less than this tolerance, they are considered parallel.
// osg::vec3 a, b; (a ^ b).length2()
#define CROSS_PRODUCT_ANGLE_TOLERANCE 1.0E-1

namespace
{

    bool
    computeClosestPoints( const osg::LineSegment& l1,
                          const osg::LineSegment& l2,
                          osg::dvec3&             p1,
                          osg::dvec3&             p2 )
    {
        // Computes the closest points (p1 and p2 on line l1 and l2 respectively) between
        // the two lines An explanation of the algorithm can be found at
        // http://www.geometryalgorithms.com/Archive/algorithm_0106/algorithm_0106.htm

        osg::LineSegment::vec_type u           = l1.end() - l1.start();
        u                                      = osg::normalize( u );
        osg::LineSegment::vec_type v           = l2.end() - l2.start();
        v                                      = osg::normalize( v );

        osg::LineSegment::vec_type w0          = l1.start() - l2.start();

        double                     a           = osg::dot( u, u );
        double                     b           = osg::dot( u, v );
        double                     c           = osg::dot( v, v );
        double                     d           = osg::dot( u, w0 );
        double                     e           = osg::dot( v, w0 );

        double                     denominator = a * c - b * b;

        // Test if lines are parallel
        if( denominator == 0.0 )
        {
            return false;
        }

        double sc = ( b * e - c * d ) / denominator;
        double tc = ( a * e - b * d ) / denominator;

        p1        = l1.start() + u * sc;
        p2        = l2.start() + v * tc;

        return true;
    }

    bool
    computeClosestPointOnLine( const osg::dvec3& lineStart,
                               const osg::dvec3& lineEnd,
                               const osg::dvec3& fromPoint,
                               osg::dvec3&       closestPoint )
    {
        osg::dvec3 v          = lineEnd - lineStart;
        osg::dvec3 w          = fromPoint - lineStart;

        double     c1         = osg::dot( w, v );
        double     c2         = osg::dot( v, v );

        double     almostZero = 0.000001;
        if( c2 < almostZero )
        {
            return false;
        }

        double b     = c1 / c2;
        closestPoint = lineStart + v * b;

        return true;
    }

    bool
    getPlaneLineIntersection( const osg::dvec4& plane,
                              const osg::dvec3& lineStart,
                              const osg::dvec3& lineEnd,
                              osg::dvec3&       isect )
    {
        const double deltaX = lineEnd.x - lineStart.x;
        const double deltaY = lineEnd.y - lineStart.y;
        const double deltaZ = lineEnd.z - lineStart.z;

        const double denominator =
            ( plane[0] * deltaX + plane[1] * deltaY + plane[2] * deltaZ );
        if( denominator == 0.0 )
        {
            return false;
        }

        const double C = ( plane[0] *
                           lineStart.x +
                           plane[1] *
                           lineStart.y +
                           plane[2] *
                           lineStart.z +
                           plane[3] ) /
                         denominator;

        isect.x        = lineStart.x - deltaX * C;
        isect.y        = lineStart.y - deltaY * C;
        isect.z        = lineStart.z - deltaZ * C;

        return true;
    }

    bool
    getSphereLineIntersection( const osg::Sphere& sphere,
                               const osg::dvec3&  lineStart,
                               const osg::dvec3&  lineEnd,
                               osg::dvec3&        frontISect,
                               osg::dvec3&        backISect )
    {
        osg::dvec3 lineDirection = lineEnd - lineStart;
        lineDirection            = osg::normalize( lineDirection );

        osg::dvec3 v             = lineStart - osg::dvec3( sphere.getCenter() );
        double     B             = 2.0 * osg::dot( lineDirection, v );
        double     C = osg::dot( v, v ) - sphere.getRadius() * sphere.getRadius();

        double     discriminant = B * B - 4.0F * C;

        if( discriminant < 0.0F )    // Line and sphere don't intersect.
        {
            return false;
        }

        double discriminantSqroot = sqrt( discriminant );
        double t0                 = ( -B - discriminantSqroot ) * 0.5F;
        frontISect                = lineStart + lineDirection * t0;

        double t1                 = ( -B + discriminantSqroot ) * 0.5F;
        backISect                 = lineStart + lineDirection * t1;

        return true;
    }

    bool
    getUnitCylinderLineIntersection( const osg::dvec3& lineStart,
                                     const osg::dvec3& lineEnd,
                                     osg::dvec3&       isectFront,
                                     osg::dvec3&       isectBack )
    {
        osg::dvec3 dir = lineEnd - lineStart;
        dir            = osg::normalize( dir );

        double a       = dir[0] * dir[0] + dir[1] * dir[1];
        double b       = 2.0F * ( lineStart[0] * dir[0] + lineStart[1] * dir[1] );
        double c       = lineStart[0] * lineStart[0] + lineStart[1] * lineStart[1] - 1;

        double d       = b * b - 4 * a * c;
        if( d < 0.0F )
        {
            return false;
        }

        double dSqroot = sqrt( d );
        double t0, t1;
        if( b > 0.0F )
        {
            t0 = -( 2.0F * c ) / ( dSqroot + b );
            t1 = -( dSqroot + b ) / ( 2.0 * a );
        }
        else
        {
            t0 = ( 2.0F * c ) / ( dSqroot - b );
            t1 = ( dSqroot - b ) / ( 2.0 * a );
        }

        isectFront = lineStart + dir * t0;
        isectBack  = lineStart + dir * t1;
        return true;
    }

    bool
    getCylinderLineIntersection( const osg::Cylinder& cylinder,
                                 const osg::dvec3&    lineStart,
                                 const osg::dvec3&    lineEnd,
                                 osg::dvec3&          isectFront,
                                 osg::dvec3&          isectBack )
    {
        // Compute matrix transformation that takes the cylinder to a unit cylinder with
        // Z-axis as it's axis and (0,0,0) as it's center.
        double     oneOverRadius = 1.0 / cylinder.getRadius();
        osg::dmat4 toUnitCylInZ =
            osg::translate( osg::dvec3( -cylinder.getCenter() ) ) *
            osg::scale( oneOverRadius, oneOverRadius, oneOverRadius ) *
            osg::rotate( osg::dquat( osg::inverse( cylinder.getRotation() ) ) );

        // Transform the lineStart and lineEnd into the unit cylinder space.
        osg::dvec3 unitCylLineStart = lineStart * toUnitCylInZ;
        osg::dvec3 unitCylLineEnd   = lineEnd * toUnitCylInZ;

        // Intersect line with unit cylinder.
        osg::dvec3 unitCylIsectFront, unitCylIsectBack;
        if( !getUnitCylinderLineIntersection( unitCylLineStart,
                                              unitCylLineEnd,
                                              unitCylIsectFront,
                                              unitCylIsectBack ) )
        {
            return false;
        }

        // Transform back from unit cylinder space.
        osg::dmat4 invToUnitCylInZ( osg::inverse( toUnitCylInZ ) );
        isectFront = unitCylIsectFront * invToUnitCylInZ;
        isectBack  = unitCylIsectBack * invToUnitCylInZ;

        return true;
    }

    osg::dvec3
    getLocalEyeDirection( const osg::dvec3& eyeDir,
                          const osg::dmat4& localToWorld )
    {
        // To take a normal from world to local you need to transform it by the transpose
        // of the inverse of the world to local matrix. Pre-multiplying is equivalent to
        // doing a post-multiplication of the transpose.
        osg::dvec3 localEyeDir = localToWorld * eyeDir;
        localEyeDir            = osg::normalize( localEyeDir );
        return localEyeDir;
    }

    osg::Plane
    computePlaneThruPointAndOrientedToEye( const osg::dvec3& eyeDir,
                                           const osg::dmat4& localToWorld,
                                           const osg::dvec3& point,
                                           bool              front )
    {
        osg::dvec3 planeNormal = getLocalEyeDirection( eyeDir, localToWorld );
        if( !front )
        {
            planeNormal = -planeNormal;
        }

        osg::Plane plane;
        plane.set( planeNormal, point );
        return plane;
    }

    // Computes a plane to be used as a basis for determining a displacement.  When
    // eyeDir is close to the cylinder axis, then the plane will be set to be
    // perpendicular to the cylinder axis. Otherwise it will be set to be parallel to the
    // cylinder axis and oriented towards eyeDir.
    osg::Plane
    computeIntersectionPlane( const osg::dvec3&    eyeDir,
                              const osg::dmat4&    localToWorld,
                              const osg::dvec3&    axisDir,
                              const osg::Cylinder& cylinder,
                              osg::dvec3&          planeLineStart,
                              osg::dvec3&          planeLineEnd,
                              bool&                parallelPlane,
                              bool                 front )
    {
        osg::Plane plane;

        osg::dvec3 unitAxisDir = axisDir;
        unitAxisDir            = osg::normalize( unitAxisDir );
        osg::dvec3 perpDir = unitAxisDir ^ getLocalEyeDirection( eyeDir, localToWorld );

        // Check to make sure eye and cylinder axis are not too close
        if( osg::length2( perpDir ) < CROSS_PRODUCT_ANGLE_TOLERANCE )
        {
            // Too close, so instead return plane perpendicular to cylinder axis.
            plane.set( osg::vec3( unitAxisDir ), cylinder.getCenter() );
            parallelPlane = false;
            return plane;
        }

        // Otherwise compute plane along axisDir oriented towards eye
        osg::dvec3 planeDir = perpDir ^ axisDir;
        planeDir            = osg::normalize( planeDir );
        if( !front )
        {
            planeDir = -planeDir;
        }

        osg::dvec3 planePoint = planeDir * cylinder.getRadius() + axisDir;
        plane.set( planeDir, planePoint );

        planeLineStart = planePoint;
        planeLineEnd   = planePoint + axisDir;
        parallelPlane  = true;
        return plane;
    }

}    // namespace

Projector::Projector() :
    _worldToLocalDirty( false )
{
}

Projector::~Projector()
{
}

LineProjector::LineProjector()
{
    _line = new osg::LineSegment( osg::LineSegment::vec_type( 0.0, 0.0, 0.0 ),
                                  osg::LineSegment::vec_type( 1.0, 0.0, 0.0 ) );
}

LineProjector::LineProjector( const osg::LineSegment::vec_type& s,
                              const osg::LineSegment::vec_type& e )
{
    _line = new osg::LineSegment( s, e );
}

LineProjector::~LineProjector()
{
}

bool
LineProjector::project( const PointerInfo& pi,
                        osg::dvec3&        projectedPoint ) const
{
    if( !_line->valid() )
    {
        OSG_WARN << "Warning: Invalid line set. LineProjector::project() failed."
                 << std::endl;
        return false;
    }

    // Transform the line to world/object coordinate space.
    osg::ref_ptr<osg::LineSegment> objectLine = new osg::LineSegment;
    objectLine->mult( *_line, getLocalToWorld() );

    // Get the near and far points for the mouse point.
    osg::dvec3 nearPoint, farPoint;
    pi.getNearFarPoints( nearPoint, farPoint );
    osg::ref_ptr<osg::LineSegment> pointerLine =
        new osg::LineSegment( nearPoint, farPoint );

    osg::dvec3 closestPtLine, closestPtProjWorkingLine;
    if( !computeClosestPoints( *objectLine,
                               *pointerLine,
                               closestPtLine,
                               closestPtProjWorkingLine ) )
    {
        return false;
    }

    osg::dvec3 localClosestPtLine = closestPtLine * getWorldToLocal();

    projectedPoint                = localClosestPtLine;

    return true;
}

PlaneProjector::PlaneProjector()
{
}

PlaneProjector::PlaneProjector( const osg::Plane& plane )
{
    _plane = plane;
}

PlaneProjector::~PlaneProjector()
{
}

bool
PlaneProjector::project( const PointerInfo& pi,
                         osg::dvec3&        projectedPoint ) const
{
    if( !_plane.valid() )
    {
        OSG_WARN << "Warning: Invalid plane set. PlaneProjector::project() failed."
                 << std::endl;
        return false;
    }

    // Get the near and far points for the mouse point.
    osg::dvec3 nearPoint, farPoint;
    pi.getNearFarPoints( nearPoint, farPoint );

    // Transform these points into local coordinates.
    osg::dvec3 objectNearPoint, objectFarPoint;
    objectNearPoint = nearPoint * getWorldToLocal();
    objectFarPoint  = farPoint * getWorldToLocal();

    // Find the intersection of the plane with the line (formed by the near and far
    // points in local coordinates).
    return getPlaneLineIntersection( _plane.asVec4(),
                                     objectNearPoint,
                                     objectFarPoint,
                                     projectedPoint );
}

SphereProjector::SphereProjector() :
    _sphere( new osg::Sphere ),
    _front( true )
{
}

SphereProjector::SphereProjector( osg::Sphere* sphere ) :
    _sphere( sphere ),
    _front( true )
{
}

SphereProjector::~SphereProjector()
{
}

bool
SphereProjector::project( const PointerInfo& pi,
                          osg::dvec3&        projectedPoint ) const
{
    if( !_sphere->valid() )
    {
        OSG_WARN << "Warning: Invalid sphere. SphereProjector::project() failed."
                 << std::endl;
        return false;
    }

    // Get the near and far points for the mouse point.
    osg::dvec3 nearPoint, farPoint;
    pi.getNearFarPoints( nearPoint, farPoint );

    // Transform these points into local coordinates.
    osg::dvec3 objectNearPoint, objectFarPoint;
    objectNearPoint = nearPoint * getWorldToLocal();
    objectFarPoint  = farPoint * getWorldToLocal();

    // Find the intersection of the sphere with the line.
    osg::dvec3 dontCare;
    if( _front )
    {
        return getSphereLineIntersection( *_sphere,
                                          objectNearPoint,
                                          objectFarPoint,
                                          projectedPoint,
                                          dontCare );
    }
    return getSphereLineIntersection( *_sphere,
                                      objectNearPoint,
                                      objectFarPoint,
                                      dontCare,
                                      projectedPoint );
}

bool
SphereProjector::isPointInFront( const PointerInfo& pi,
                                 const osg::dmat4&  localToWorld ) const
{
    osg::dvec3 centerToPoint =
        osg::dvec3( getSphere()->getCenter() ) - pi.getLocalIntersectPoint();
    if( osg::dot( centerToPoint, getLocalEyeDirection( pi.getEyeDir(), localToWorld ) ) <
        0.0 )
    {
        return false;
    }
    return true;
}

SpherePlaneProjector::SpherePlaneProjector() :
    _onSphere( false )
{
}

SpherePlaneProjector::SpherePlaneProjector( osg::Sphere* sphere ) :
    SphereProjector( sphere ),
    _onSphere( false )
{
}

SpherePlaneProjector::~SpherePlaneProjector()
{
}

osg::quat
SpherePlaneProjector::getRotation( const osg::dvec3& p1,
                                   bool              p1OnSphere,
                                   const osg::dvec3& p2,
                                   bool              p2OnSphere,
                                   float             radialFactor ) const
{
    osg::dvec3 center = osg::dvec3( getSphere()->getCenter() );
    if( p1OnSphere && p2OnSphere )
    {
        osg::dquat rotation;
        if( _front )
        {
            rotation = osg::dquat( p1 - center, p2 - center );
        }
        else
        {
            rotation = osg::dquat( p2 - center, p1 - center );
        }
        return osg::quat( rotation );
    }
    else if( !p1OnSphere && !p2OnSphere )
    {
        osg::dquat rotation( p1 - center, p2 - center );

        // Extract angle and axis from rotation quaternion
        double     angle        = 2.0 * acos( osg::clampTo( rotation.w, -1.0, 1.0 ) );
        double     sinHalfAngle = sqrt( 1.0 - rotation.w * rotation.w );
        osg::dvec3 axis;
        if( sinHalfAngle > 1E-10 )
        {
            axis = osg::dvec3( rotation.x / sinHalfAngle,
                               rotation.y / sinHalfAngle,
                               rotation.z / sinHalfAngle );
        }
        else
        {
            axis = osg::dvec3( 0.0, 0.0, 1.0 );
        }

        osg::dvec3 realAxis;
        if( osg::dot( axis, _plane.getNormal() ) > 0.0 )
        {
            realAxis = _plane.getNormal();
        }
        else
        {
            realAxis = -_plane.getNormal();
        }

        osg::dquat rollRotation( angle, realAxis );

        osg::dvec3 diff1 = p1 - center;
        osg::dvec3 diff2 = p2 - center;
        double     d     = osg::length( diff2 ) - osg::length( diff1 );

        double     theta = d / getSphere()->getRadius();
        if( fabs( theta ) < 0.000001 || fabs( theta ) > 1.0 )
        {
            return osg::quat( rollRotation );
        }

        diff1               = osg::normalize( diff1 );
        osg::dvec3 pullAxis = diff1 ^ _plane.getNormal();
        pullAxis            = osg::normalize( pullAxis );
        osg::dquat pullRotation( ( double )( radialFactor * theta ), pullAxis );

        osg::dquat totalRotation = pullRotation * rollRotation;
        return osg::quat( totalRotation );
    }
    else
    {
        osg::dvec3 planePoint = center;

        osg::dvec3 intersection, dontCare;
        if( p1OnSphere )
        {
            getSphereLineIntersection( *getSphere(),
                                       p2,
                                       planePoint,
                                       intersection,
                                       dontCare );
        }
        else
        {
            getSphereLineIntersection( *getSphere(),
                                       p1,
                                       planePoint,
                                       intersection,
                                       dontCare );
        }

        osg::dquat rotation;
        if( p1OnSphere )
        {
            rotation = osg::dquat( p1 - center, intersection - center );
        }
        else
        {
            rotation = osg::dquat( intersection - center, p2 - center );
        }
        return osg::quat( rotation );
    }
}

bool
SpherePlaneProjector::project( const PointerInfo& pi,
                               osg::dvec3&        projectedPoint ) const
{
    if( !_sphere->valid() )
    {
        OSG_WARN << "Warning: Invalid sphere. SpherePlaneProjector::project() failed."
                 << std::endl;
        return false;
    }

    // Get the near and far points for the mouse point.
    osg::dvec3 nearPoint, farPoint;
    pi.getNearFarPoints( nearPoint, farPoint );

    // Transform these points into local coordinates.
    osg::dvec3 objectNearPoint, objectFarPoint;
    objectNearPoint = nearPoint * getWorldToLocal();
    objectFarPoint  = farPoint * getWorldToLocal();

    // Find the intersection of the sphere with the line.
    osg::dvec3 sphereIntersection, dontCare;
    bool       hitSphere = false;
    if( _front )
    {
        hitSphere = getSphereLineIntersection( *_sphere,
                                               objectNearPoint,
                                               objectFarPoint,
                                               sphereIntersection,
                                               dontCare );
    }
    else
    {
        hitSphere = getSphereLineIntersection( *_sphere,
                                               objectNearPoint,
                                               objectFarPoint,
                                               dontCare,
                                               sphereIntersection );
    }

    // Compute plane oriented to the eye.
    _plane =
        computePlaneThruPointAndOrientedToEye( pi.getEyeDir(),
                                               getLocalToWorld(),
                                               osg::dvec3( getSphere()->getCenter() ),
                                               _front );

    // Find the intersection on the plane.
    osg::dvec3 planeIntersection;
    if( hitSphere )
    {
        if( !getPlaneLineIntersection( _plane.asVec4(),
                                       sphereIntersection,
                                       sphereIntersection + _plane.getNormal(),
                                       planeIntersection ) )
        {
            return false;
        }
    }
    else
    {
        if( !getPlaneLineIntersection( _plane.asVec4(),
                                       objectNearPoint,
                                       objectFarPoint,
                                       planeIntersection ) )
        {
            return false;
        }
    }

    // Distance from the plane intersection point to the center of the sphere.
    double dist =
        osg::length( planeIntersection - osg::dvec3( getSphere()->getCenter() ) );

    // If the distance is less that the sphere radius choose the sphere intersection else
    // choose the plane intersection.
    if( dist < getSphere()->getRadius() )
    {
        if( !hitSphere )
        {
            return false;
        }
        projectedPoint = sphereIntersection;
        _onSphere      = true;
    }
    else
    {
        projectedPoint = planeIntersection;
        _onSphere      = false;
    }
    return true;
}

CylinderProjector::CylinderProjector() :
    _cylinder( new osg::Cylinder() ),
    _cylinderAxis( 0.0,
                   0.0,
                   1.0 ),
    _front( true )
{
}

CylinderProjector::CylinderProjector( osg::Cylinder* cylinder ) :
    _front( true )
{
    setCylinder( cylinder );
}

CylinderProjector::~CylinderProjector()
{
}

bool
CylinderProjector::project( const PointerInfo& pi,
                            osg::dvec3&        projectedPoint ) const
{
    if( !_cylinder.valid() )
    {
        OSG_WARN << "Warning: Invalid cylinder. CylinderProjector::project() failed."
                 << std::endl;
        return false;
    }

    // Get the near and far points for the mouse point.
    osg::dvec3 nearPoint, farPoint;
    pi.getNearFarPoints( nearPoint, farPoint );

    // Transform these points into local coordinates.
    osg::dvec3 objectNearPoint, objectFarPoint;
    objectNearPoint = nearPoint * getWorldToLocal();
    objectFarPoint  = farPoint * getWorldToLocal();

    // Find the intersection of the sphere with the line.
    osg::dvec3 dontCare;
    return getCylinderLineIntersection( *_cylinder,
                                        objectNearPoint,
                                        objectFarPoint,
                                        projectedPoint,
                                        dontCare );
}

bool
CylinderProjector::isPointInFront( const PointerInfo& pi,
                                   const osg::dmat4&  localToWorld ) const
{
    osg::dvec3 closestPointOnAxis;
    computeClosestPointOnLine( osg::dvec3( getCylinder()->getCenter() ),
                               osg::dvec3( getCylinder()->getCenter() ) + _cylinderAxis,
                               pi.getLocalIntersectPoint(),
                               closestPointOnAxis );

    osg::dvec3 perpPoint = pi.getLocalIntersectPoint() - closestPointOnAxis;
    if( osg::dot( perpPoint, getLocalEyeDirection( pi.getEyeDir(), localToWorld ) ) <
        0.0 )
    {
        return false;
    }
    return true;
}

CylinderPlaneProjector::CylinderPlaneProjector() :
    _parallelPlane( false )
{
}

CylinderPlaneProjector::CylinderPlaneProjector( osg::Cylinder* cylinder ) :
    CylinderProjector( cylinder ),
    _parallelPlane( false )
{
}

CylinderPlaneProjector::~CylinderPlaneProjector()
{
}

bool
CylinderPlaneProjector::project( const PointerInfo& pi,
                                 osg::dvec3&        projectedPoint ) const
{
    if( !_cylinder.valid() )
    {
        OSG_WARN << "Warning: Invalid cylinder. CylinderProjector::project() failed."
                 << std::endl;
        return false;
    }

    // Get the near and far points for the mouse point.
    osg::dvec3 nearPoint, farPoint;
    pi.getNearFarPoints( nearPoint, farPoint );

    // Transform these points into local coordinates.
    osg::dvec3 objectNearPoint, objectFarPoint;
    objectNearPoint = nearPoint * getWorldToLocal();
    objectFarPoint  = farPoint * getWorldToLocal();

    // Computes either a plane parallel to cylinder axis oriented to the eye or the plane
    // perpendicular to the cylinder axis if the eye-cylinder angle is close.
    _plane = computeIntersectionPlane( pi.getEyeDir(),
                                       getLocalToWorld(),
                                       _cylinderAxis,
                                       *_cylinder,
                                       _planeLineStart,
                                       _planeLineEnd,
                                       _parallelPlane,
                                       _front );

    // Now find the point of intersection on our newly-calculated plane.
    getPlaneLineIntersection( _plane.asVec4(),
                              objectNearPoint,
                              objectFarPoint,
                              projectedPoint );
    return true;
}

osg::quat
CylinderPlaneProjector::getRotation( const osg::dvec3& p1,
                                     const osg::dvec3& p2 ) const
{
    if( _parallelPlane )
    {
        osg::dvec3 closestPointToPlaneLine1, closestPointToPlaneLine2;
        computeClosestPointOnLine( _planeLineStart,
                                   _planeLineEnd,
                                   p1,
                                   closestPointToPlaneLine1 );
        computeClosestPointOnLine( _planeLineStart,
                                   _planeLineEnd,
                                   p2,
                                   closestPointToPlaneLine2 );

        osg::dvec3 v1   = p1 - closestPointToPlaneLine1;
        osg::dvec3 v2   = p2 - closestPointToPlaneLine2;

        osg::dvec3 diff = v2 - v1;
        double     d    = osg::length( diff );

        // The amount of rotation is inversely proportional to the size of the cylinder
        double     angle   = ( getCylinder()->getRadius() == 0.0 )
                               ? 0.0
                               : ( d / getCylinder()->getRadius() );
        osg::dvec3 rotAxis = _plane.getNormal() ^ v1;

        if( osg::length( v2 ) > osg::length( v1 ) )
        {
            return osg::quat( osg::dquat( angle, rotAxis ) );
        }
        else
        {
            return osg::quat( osg::dquat( -angle, rotAxis ) );
        }
    }
    else
    {
        osg::dvec3 v1   = p1 - osg::dvec3( getCylinder()->getCenter() );
        osg::dvec3 v2   = p2 - osg::dvec3( getCylinder()->getCenter() );

        double cosAngle = osg::dot( v1, v2 ) / ( osg::length( v1 ) * osg::length( v2 ) );

        if( cosAngle > 1.0 || cosAngle < -1.0 )
        {
            return osg::quat();
        }

        double     angle   = acos( cosAngle );
        osg::dvec3 rotAxis = v1 ^ v2;

        return osg::quat( osg::dquat( angle, rotAxis ) );
    }
}
