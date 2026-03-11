/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Flight-simulator camera manipulator. Provides pitch/yaw/roll
 * with continuous forward motion for fly-through navigation.
 */
#include <osgGA/manipulators/FlightManipulator.hpp>

#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>

using namespace osg;
using namespace osgGA;

/// Constructor.
FlightManipulator::FlightManipulator( int flags ) :
    Inherit( flags ),
    _yawMode( YAW_AUTOMATICALLY_WHEN_BANKED )
{
}

/// Constructor.
FlightManipulator::FlightManipulator( const FlightManipulator& fm,
                                      const CopyOp&            copyOp ) :
    osg::Object( fm,
                 copyOp ),
    osg::Callback( fm,
                   copyOp ),
    Inherit( fm,
             copyOp ),
    _yawMode( fm._yawMode )
{
}

void
FlightManipulator::init( const GUIEventAdapter& ea,
                         GUIActionAdapter&      us )
{
    inherited::init( ea, us );

    // center mouse pointer
    centerMousePointer( ea, us );
}

void
FlightManipulator::home( const GUIEventAdapter& ea,
                         GUIActionAdapter&      us )
{
    inherited::home( ea, us );

    // center mouse pointer
    centerMousePointer( ea, us );
}

// doc in parent
bool
FlightManipulator::handleFrame( const GUIEventAdapter& ea,
                                GUIActionAdapter&      us )
{
    addMouseEvent( ea );
    if( performMovement() )
    {
        us.requestRedraw();
    }

    return false;
}

// doc in parent
bool
FlightManipulator::handleMouseMove( const GUIEventAdapter& ea,
                                    GUIActionAdapter&      us )
{
    return flightHandleEvent( ea, us );
}

// doc in parent
bool
FlightManipulator::handleMouseDrag( const GUIEventAdapter& ea,
                                    GUIActionAdapter&      us )
{
    return flightHandleEvent( ea, us );
}

// doc in parent
bool
FlightManipulator::handleMousePush( const GUIEventAdapter& ea,
                                    GUIActionAdapter&      us )
{
    return flightHandleEvent( ea, us );
}

// doc in parent
bool
FlightManipulator::handleMouseRelease( const GUIEventAdapter& ea,
                                       GUIActionAdapter&      us )
{
    return flightHandleEvent( ea, us );
}

bool
FlightManipulator::handleKeyDown( const GUIEventAdapter& ea,
                                  GUIActionAdapter&      us )
{
    if( inherited::handleKeyDown( ea, us ) )
    {
        return true;
    }

    if( ea.getKey() == 'q' )
    {

        _yawMode = YAW_AUTOMATICALLY_WHEN_BANKED;
        return true;
    }
    else if( ea.getKey() == 'a' )
    {

        _yawMode = NO_AUTOMATIC_YAW;
        return true;
    }

    return false;
}

/// General flight-style event handler
bool
FlightManipulator::flightHandleEvent( const GUIEventAdapter& ea,
                                      GUIActionAdapter&      us )
{
    addMouseEvent( ea );
    us.requestContinuousUpdate( true );
    if( performMovement() )
    {
        us.requestRedraw();
    }

    return true;
}

void
FlightManipulator::getUsage( ApplicationUsage& usage ) const
{
    inherited::getUsage( usage );
    usage.addKeyboardMouseBinding( getManipulatorName() + ": q",
                                   "Automatically yaw when banked (default)" );
    usage.addKeyboardMouseBinding( getManipulatorName() + ": a", "No yaw when banked" );
}

/** Configure the Yaw control for the flight model. */
void
FlightManipulator::setYawControlMode( YawControlMode ycm )
{
    _yawMode = ycm;
}

bool
FlightManipulator::performMovement()
{
    // return if less then two events have been added.
    if( _ga_t0.get() == NULL || _ga_t1.get() == NULL )
    {
        return false;
    }

    double eventTimeDelta = _ga_t0->getTime() - _ga_t1->getTime();

    if( eventTimeDelta < 0.0F )
    {
        OSG_WARN << "Manipulator warning: eventTimeDelta = " << eventTimeDelta
                 << std::endl;
        eventTimeDelta = 0.0F;
    }

    unsigned int buttonMask = static_cast<unsigned int>( _ga_t1->getButtonMask() );
    if( buttonMask == GUIEventAdapter::LEFT_MOUSE_BUTTON )
    {
        performMovementLeftMouseButton( eventTimeDelta, 0., 0. );
    }
    else if( buttonMask ==
             GUIEventAdapter::MIDDLE_MOUSE_BUTTON ||
             buttonMask ==
             ( GUIEventAdapter::LEFT_MOUSE_BUTTON |
               GUIEventAdapter::RIGHT_MOUSE_BUTTON ) )
    {
        performMovementMiddleMouseButton( eventTimeDelta, 0., 0. );
    }
    else if( buttonMask == GUIEventAdapter::RIGHT_MOUSE_BUTTON )
    {
        performMovementRightMouseButton( eventTimeDelta, 0., 0. );
    }

    float           dx = _ga_t0->getXnormalized();
    float           dy = _ga_t0->getYnormalized();

    CoordinateFrame cf = getCoordinateFrame( _eye );

    dmat4           rotation_matrix;
    rotation_matrix = osg::rotate( _rotation );

    dvec3 up        = dvec3( 0.0, 1.0, 0.0 ) * rotation_matrix;
    dvec3 lv        = dvec3( 0.0, 0.0, -1.0 ) * rotation_matrix;

    dvec3 sv        = lv ^ up;
    sv              = osg::normalize( sv );

    double pitch    = -inDegrees( dy * 50.0F * eventTimeDelta );
    double roll     = inDegrees( dx * 50.0F * eventTimeDelta );

    quat   delta_rotate;

    quat   roll_rotate;
    quat   pitch_rotate;

    pitch_rotate = quat( static_cast<float>( pitch ),
                         vec3( static_cast<float>( sv.x ),
                               static_cast<float>( sv.y ),
                               static_cast<float>( sv.z ) ) );
    roll_rotate  = quat( static_cast<float>( roll ),
                         vec3( static_cast<float>( lv.x ),
                               static_cast<float>( lv.y ),
                               static_cast<float>( lv.z ) ) );

    delta_rotate = pitch_rotate * roll_rotate;

    if( _yawMode == YAW_AUTOMATICALLY_WHEN_BANKED )
    {
        // float bank = asinf(sv.z);
        double bank = asin( osg::dot( sv, getUpVector( cf ) ) );
        double yaw  = inRadians( bank ) * eventTimeDelta;

        quat   yaw_rotate;
        // yaw_rotate = osg::rotate(yaw,0.0f,0.0f,1.0f);

        {
            osg::dvec3 upv = getUpVector( cf );
            yaw_rotate     = quat( static_cast<float>( yaw ),
                                   vec3( static_cast<float>( upv.x ),
                                         static_cast<float>( upv.y ),
                                         static_cast<float>( upv.z ) ) );
        }

        delta_rotate = delta_rotate * yaw_rotate;
    }

    lv        *= ( _velocity * eventTimeDelta );

    _eye      += lv;
    _rotation  = _rotation * delta_rotate;

    return true;
}

bool
FlightManipulator::performMovementLeftMouseButton( const double eventTimeDelta,
                                                   const double /*dx*/,
                                                   const double /*dy*/ )
{
    // pan model
    _velocity += eventTimeDelta * ( _acceleration + _velocity );
    return true;
}

bool
FlightManipulator::performMovementMiddleMouseButton( const double /*eventTimeDelta*/,
                                                     const double /*dx*/,
                                                     const double /*dy*/ )
{
    _velocity = 0.0F;
    return true;
}

bool
FlightManipulator::performMovementRightMouseButton( const double eventTimeDelta,
                                                    const double /*dx*/,
                                                    const double /*dy*/ )
{
    _velocity -= eventTimeDelta * ( _acceleration + _velocity );
    return true;
}
