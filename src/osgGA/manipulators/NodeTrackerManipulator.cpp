/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Camera manipulator that tracks a scene node. Keeps the camera
 * focused on a moving target with configurable tracking modes.
 */
#include <osgGA/manipulators/NodeTrackerManipulator.hpp>

#include <iterator>
#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/maths/quat.hpp>
#include <osg/nodes/Transform.hpp>

using namespace osg;
using namespace osgGA;

NodeTrackerManipulator::NodeTrackerManipulator( int flags ) :
    Inherit( flags ),
    _trackerMode( NODE_CENTER_AND_ROTATION )
{
    setVerticalAxisFixed( false );
}

NodeTrackerManipulator::NodeTrackerManipulator( const NodeTrackerManipulator& m,
                                                const CopyOp&                 copyOp ) :
    osg::Object( m,
                 copyOp ),
    osg::Callback( m,
                   copyOp ),
    Inherit( m,
             copyOp ),
    _trackNodePath( m._trackNodePath ),
    _trackerMode( m._trackerMode )
{
}

void
NodeTrackerManipulator::setTrackNodePath( const osg::NodePath& nodePath )
{
    _trackNodePath.setNodePath( nodePath );
}

void
NodeTrackerManipulator::setTrackerMode( TrackerMode mode )
{
    _trackerMode = mode;
}

/// Sets rotation mode. \sa setVerticalAxisFixed
void
NodeTrackerManipulator::setRotationMode( RotationMode mode )
{
    setVerticalAxisFixed( mode != TRACKBALL );

    if( getAutoComputeHomePosition() )
    {
        computeHomePosition();
    }
}

/// Gets rotation mode. \sa getVerticalAxisFixed
NodeTrackerManipulator::RotationMode
NodeTrackerManipulator::getRotationMode() const
{
    return getVerticalAxisFixed() ? ELEVATION_AZIM : TRACKBALL;
}

void
NodeTrackerManipulator::setNode( Node* node )
{
    inherited::setNode( node );

    // update model size
    if( _flags & UPDATE_MODEL_SIZE )
    {
        if( _node.valid() )
        {
            setMinimumDistance( clampBetween( _modelSize * 0.001, 0.00001, 1.0 ) );
            OSG_INFO << "NodeTrackerManipulator: setting minimum distance to "
                     << _minimumDistance << std::endl;
        }
    }
}

void
NodeTrackerManipulator::setTrackNode( osg::Node* node )
{
    if( !node )
    {
        OSG_NOTICE << "NodeTrackerManipulator::setTrackNode(Node*):  Unable to set "
                      "tracked node due to null Node*"
                   << std::endl;
        return;
    }

    osg::NodePathList nodePaths = node->getParentalNodePaths();
    if( !nodePaths.empty() )
    {
        if( nodePaths.size() > 1 )
        {
            OSG_NOTICE << "osgGA::NodeTrackerManipualtor::setTrackNode(..) taking first "
                          "parent path, ignoring others."
                       << std::endl;
        }

        for( unsigned int i = 0; i < nodePaths.size(); ++i )
        {
            OSG_NOTICE << "NodePath " << i << std::endl;
            for( NodePath::iterator itr = nodePaths[i].begin();
                 itr != nodePaths[i].end();
                 ++itr )
            {
                OSG_NOTICE << "     " << ( *itr )->className() << std::endl;
            }
        }

        OSG_INFO << "NodeTrackerManipulator::setTrackNode(Node*" << node << " "
                 << node->getName() << "): Path set" << std::endl;
        setTrackNodePath( nodePaths[0] );
    }
    else
    {
        OSG_NOTICE << "NodeTrackerManipulator::setTrackNode(Node*): Unable to set "
                      "tracked node due to empty parental path."
                   << std::endl;
    }
}

void
NodeTrackerManipulator::computeHomePosition()
{
    osg::Node* node = getTrackNode();
    if( node )
    {
        const osg::sphere& boundingSphere = node->getBound();

        setHomePosition( osg::dvec3( boundingSphere.center ) +
                             osg::dvec3( 0.0, -3.5F * boundingSphere.radius, 0.0F ),
                         osg::dvec3( boundingSphere.center ),
                         osg::dvec3( 0.0F, 0.0F, 1.0F ),
                         _autoComputeHomePosition );
    }
}

void
NodeTrackerManipulator::setByMatrix( const osg::dmat4& matrix )
{
    osg::dvec3 eye, center, up;
    osg::getLookAt( matrix, eye, center, up, _distance );
    computePosition( eye, center, up );
}

void
NodeTrackerManipulator::computeNodeWorldToLocal( osg::dmat4& worldToLocal ) const
{
    osg::NodePath nodePath;
    if( _trackNodePath.getNodePath( nodePath ) )
    {
        worldToLocal = osg::computeWorldToLocal( nodePath );
    }
}

void
NodeTrackerManipulator::computeNodeLocalToWorld( osg::dmat4& localToWorld ) const
{
    osg::NodePath nodePath;
    if( _trackNodePath.getNodePath( nodePath ) )
    {
        localToWorld = osg::computeLocalToWorld( nodePath );
    }
}

void
NodeTrackerManipulator::computeNodeCenterAndRotation( osg::dvec3& nodeCenter,
                                                      osg::quat&  nodeRotation ) const
{
    osg::dmat4 localToWorld, worldToLocal;
    computeNodeLocalToWorld( localToWorld );
    computeNodeWorldToLocal( worldToLocal );

    osg::NodePath nodePath;
    if( _trackNodePath.getNodePath( nodePath ) && !nodePath.empty() )
    {
        nodeCenter = osg::dvec3( nodePath.back()->getBound().center ) * localToWorld;
    }
    else
    {
        nodeCenter = osg::dvec3( 0.0F, 0.0F, 0.0F ) * localToWorld;
    }

    switch( _trackerMode )
    {
        case( NODE_CENTER_AND_AZIM ) :
            {
                CoordinateFrame coordinateFrame = getCoordinateFrame( nodeCenter );
                osg::dmat4      localToFrame( localToWorld *
                                              osg::inverse( coordinateFrame ) );

                double     azim = atan2( -localToFrame( 0, 1 ), localToFrame( 0, 0 ) );
                osg::dquat nodeRotationRelToFrame( static_cast<double>( -azim ),
                                                   osg::dvec3( 0.0, 0.0, 1.0 ) );
                osg::dquat rotationOfFrame = osg::getRotate( coordinateFrame );
                osg::dquat nr              = nodeRotationRelToFrame * rotationOfFrame;
                nodeRotation               = osg::quat( static_cast<float>( nr.x ),
                                                        static_cast<float>( nr.y ),
                                                        static_cast<float>( nr.z ),
                                                        static_cast<float>( nr.w ) );
                break;
            }
        case( NODE_CENTER_AND_ROTATION ) :
            {
                // scale the matrix to get rid of any scales before we extract the
                // rotation.
                double sx    = 1.0 / sqrt( localToWorld( 0, 0 ) *
                                           localToWorld( 0, 0 ) +
                                           localToWorld( 1, 0 ) *
                                           localToWorld( 1, 0 ) +
                                           localToWorld( 2, 0 ) *
                                           localToWorld( 2, 0 ) );
                double sy    = 1.0 / sqrt( localToWorld( 0, 1 ) *
                                           localToWorld( 0, 1 ) +
                                           localToWorld( 1, 1 ) *
                                           localToWorld( 1, 1 ) +
                                           localToWorld( 2, 1 ) *
                                           localToWorld( 2, 1 ) );
                double sz    = 1.0 / sqrt( localToWorld( 0, 2 ) *
                                           localToWorld( 0, 2 ) +
                                           localToWorld( 1, 2 ) *
                                           localToWorld( 1, 2 ) +
                                           localToWorld( 2, 2 ) *
                                           localToWorld( 2, 2 ) );
                localToWorld = localToWorld * osg::scale( sx, sy, sz );

                {
                    osg::dquat dr = osg::getRotate( localToWorld );
                    nodeRotation  = osg::quat( static_cast<float>( dr.x ),
                                               static_cast<float>( dr.y ),
                                               static_cast<float>( dr.z ),
                                               static_cast<float>( dr.w ) );
                }
                break;
            }
        case( NODE_CENTER ) :
        default :
            {
                CoordinateFrame coordinateFrame = getCoordinateFrame( nodeCenter );
                osg::dquat      dr              = osg::getRotate( coordinateFrame );
                nodeRotation = osg::quat( static_cast<float>( dr.x ),
                                          static_cast<float>( dr.y ),
                                          static_cast<float>( dr.z ),
                                          static_cast<float>( dr.w ) );
                break;
            }
    }
}

osg::dmat4
NodeTrackerManipulator::getMatrix() const
{
    osg::dvec3 nodeCenter;
    osg::quat  nodeRotation;
    computeNodeCenterAndRotation( nodeCenter, nodeRotation );
    return osg::translate( nodeCenter ) *
           osg::rotate( nodeRotation ) *
           osg::rotate( _rotation ) *
           osg::translate( 0.0, 0.0, _distance );
}

osg::dmat4
NodeTrackerManipulator::getInverseMatrix() const
{
    osg::dvec3 nodeCenter;
    osg::quat  nodeRotation;
    computeNodeCenterAndRotation( nodeCenter, nodeRotation );
    return osg::translate( 0.0, 0.0, -_distance ) *
           osg::rotate( osg::inverse( _rotation ) ) *
           osg::rotate( osg::inverse( nodeRotation ) ) *
           osg::translate( -nodeCenter );
}

void
NodeTrackerManipulator::computePosition( const osg::dvec3& eye,
                                         const osg::dvec3& center,
                                         const osg::dvec3& up )
{
    if( !_node )
    {
        return;
    }

    // compute rotation matrix
    osg::dvec3 lv( center - eye );
    _distance         = osg::length( lv );

    osg::dmat4 lookat = osg::lookAt( eye, center, up );

    osg::dquat dr     = osg::inverse( osg::getRotate( lookat ) );
    _rotation         = osg::quat( static_cast<float>( dr.x ),
                                   static_cast<float>( dr.y ),
                                   static_cast<float>( dr.z ),
                                   static_cast<float>( dr.w ) );
}

// doc in parent
bool
NodeTrackerManipulator::performMovementLeftMouseButton( const double eventTimeDelta,
                                                        const double dx,
                                                        const double dy )
{
    osg::dvec3 nodeCenter;
    osg::quat  nodeRotation;
    computeNodeCenterAndRotation( nodeCenter, nodeRotation );

    // rotate camera
    if( getVerticalAxisFixed() )
    {

        osg::dmat4 rotation_matrix;
        rotation_matrix       = osg::rotate( _rotation );

        osg::dvec3 sideVector = getSideVector( rotation_matrix );
        osg::dvec3 localUp( 0.0F, 0.0F, 1.0F );

        osg::dvec3 forwardVector = localUp ^ sideVector;
        sideVector               = forwardVector ^ localUp;

        forwardVector            = osg::normalize( forwardVector );
        sideVector               = osg::normalize( sideVector );

        osg::quat rotate_elevation;
        rotate_elevation = quat( dquat( static_cast<double>( dy ), sideVector ) );

        osg::quat rotate_azim;
        rotate_azim = quat( dquat( static_cast<double>( -dx ), localUp ) );

        _rotation   = _rotation * rotate_elevation * rotate_azim;
    }
    else
    {
        rotateTrackball( _ga_t0->getXnormalized(),
                         _ga_t0->getYnormalized(),
                         _ga_t1->getXnormalized(),
                         _ga_t1->getYnormalized(),
                         getThrowScale( eventTimeDelta ) );
    }

    return true;
}

// doc in parent
bool
NodeTrackerManipulator::
    performMovementMiddleMouseButton( const double /*eventTimeDelta*/,
                                      const double /*dx*/,
                                      const double /*dy*/ )
{
    osg::dvec3 nodeCenter;
    osg::quat  nodeRotation;
    computeNodeCenterAndRotation( nodeCenter, nodeRotation );

    return true;
}

// doc in parent
bool
NodeTrackerManipulator::performMovementRightMouseButton( const double eventTimeDelta,
                                                         const double dx,
                                                         const double dy )
{
    osg::dvec3 nodeCenter;
    osg::quat  nodeRotation;
    computeNodeCenterAndRotation( nodeCenter, nodeRotation );

    return inherited::performMovementRightMouseButton( eventTimeDelta, dx, dy );
}
