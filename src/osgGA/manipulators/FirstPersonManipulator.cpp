/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * First-person camera manipulator. WASD movement with mouse-look
 * for walkthrough-style navigation.
 */
#include <osgGA/manipulators/FirstPersonManipulator.hpp>

#include <cassert>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>

using namespace osg;
using namespace osgGA;

int FirstPersonManipulator::_accelerationFlagIndex  = allocateRelativeFlag();
int FirstPersonManipulator::_maxVelocityFlagIndex   = allocateRelativeFlag();
int FirstPersonManipulator::_wheelMovementFlagIndex = allocateRelativeFlag();

/// Constructor.
FirstPersonManipulator::FirstPersonManipulator( int flags ) :
    Inherit( flags ),
    _velocity( 0. )
{
    setAcceleration( 1.0, true );
    setMaxVelocity( 0.25, true );
    setWheelMovement( 0.05, true );
    if( _flags & SET_CENTER_ON_WHEEL_FORWARD_MOVEMENT )
    {
        setAnimationTime( 0.2 );
    }
}

/// Constructor.
FirstPersonManipulator::FirstPersonManipulator( const FirstPersonManipulator& fpm,
                                                const CopyOp&                 copyOp ) :
    osg::Object( fpm,
                 copyOp ),
    osg::Callback( fpm,
                   copyOp ),
    Inherit( fpm,
             copyOp ),
    _eye( fpm._eye ),
    _rotation( fpm._rotation ),
    _velocity( fpm._velocity ),
    _acceleration( fpm._acceleration ),
    _maxVelocity( fpm._maxVelocity ),
    _wheelMovement( fpm._wheelMovement )
{
}

/** Set the position of the manipulator using a 4x4 matrix.*/
void
FirstPersonManipulator::setByMatrix( const dmat4& matrix )
{
    // set variables
    _eye      = osg::getTrans( matrix );
    _rotation = osg::getRotate( matrix );

    // fix current rotation
    if( getVerticalAxisFixed() )
    {
        fixVerticalAxis( _eye, _rotation, true );
    }
}

/** Set the position of the manipulator using a 4x4 matrix.*/
void
FirstPersonManipulator::setByInverseMatrix( const dmat4& matrix )
{
    setByMatrix( osg::inverse( matrix ) );
}

/** Get the position of the manipulator as 4x4 matrix.*/
dmat4
FirstPersonManipulator::getMatrix() const
{
    return osg::translate( _eye ) * osg::rotate( _rotation );
}

/** Get the position of the manipulator as a inverse matrix of the manipulator,
    typically used as a model view matrix.*/
dmat4
FirstPersonManipulator::getInverseMatrix() const
{
    return osg::rotate( osg::inverse( _rotation ) ) * osg::translate( -_eye );
}

// doc in parent
void
FirstPersonManipulator::setTransformation( const osg::dvec3& eye,
                                           const osg::quat&  rotation )
{
    // set variables
    _eye      = eye;
    _rotation = rotation;

    // fix current rotation
    if( getVerticalAxisFixed() )
    {
        fixVerticalAxis( _eye, _rotation, true );
    }
}

// doc in parent
void
FirstPersonManipulator::getTransformation( osg::dvec3& eye,
                                           osg::quat&  rotation ) const
{
    eye      = _eye;
    rotation = _rotation;
}

// doc in parent
void
FirstPersonManipulator::setTransformation( const osg::dvec3& eye,
                                           const osg::dvec3& center,
                                           const osg::dvec3& up )
{
    // set variables
    osg::dmat4 m( osg::lookAt( eye, center, up ) );
    _eye      = eye;
    _rotation = osg::inverse( osg::getRotate( m ) );

    // fix current rotation
    if( getVerticalAxisFixed() )
    {
        fixVerticalAxis( _eye, _rotation, true );
    }
}

// doc in parent
void
FirstPersonManipulator::getTransformation( osg::dvec3& eye,
                                           osg::dvec3& center,
                                           osg::dvec3& up ) const
{
    center = _eye + _rotation * osg::dvec3( 0., 0., -1. );
    eye    = _eye;
    up     = _rotation * osg::dvec3( 0., 1., 0. );
}

/** Sets velocity.
 *
 *  There are no checks for maximum velocity applied.
 */
void
FirstPersonManipulator::setVelocity( const double& velocity )
{
    _velocity = velocity;
}

/** Sets acceleration.
 *
 *  If acceleration effect is unwanted, it can be set to DBL_MAX.
 *  Then, there will be no acceleration and object will reach its
 *  maximum velocity immediately.
 */
void
FirstPersonManipulator::setAcceleration( const double& acceleration,
                                         bool          relativeToModelSize )
{
    _acceleration = acceleration;
    setRelativeFlag( _accelerationFlagIndex, relativeToModelSize );
}

/// Returns acceleration speed.
double
FirstPersonManipulator::getAcceleration( bool* relativeToModelSize ) const
{
    if( relativeToModelSize )
    {
        *relativeToModelSize = getRelativeFlag( _accelerationFlagIndex );
    }

    return _acceleration;
}

/** Sets maximum velocity.
 *
 *  If acceleration is set to DBL_MAX, there is no speeding up.
 *  Instead, maximum velocity is used for velocity at once without acceleration.
 */
void
FirstPersonManipulator::setMaxVelocity( const double& maxVelocity,
                                        bool          relativeToModelSize )
{
    _maxVelocity = maxVelocity;
    setRelativeFlag( _maxVelocityFlagIndex, relativeToModelSize );
}

/// Returns maximum velocity.
double
FirstPersonManipulator::getMaxVelocity( bool* relativeToModelSize ) const
{
    if( relativeToModelSize )
    {
        *relativeToModelSize = getRelativeFlag( _maxVelocityFlagIndex );
    }

    return _maxVelocity;
}

/// Sets movement size on single wheel step.
void
FirstPersonManipulator::setWheelMovement( const double& wheelMovement,
                                          bool          relativeToModelSize )
{
    _wheelMovement = wheelMovement;
    setRelativeFlag( _wheelMovementFlagIndex, relativeToModelSize );
}

/// Returns movement size on single wheel step.
double
FirstPersonManipulator::getWheelMovement( bool* relativeToModelSize ) const
{
    if( relativeToModelSize )
    {
        *relativeToModelSize = getRelativeFlag( _wheelMovementFlagIndex );
    }

    return _wheelMovement;
}

void
FirstPersonManipulator::home( double currentTime )
{
    inherited::home( currentTime );
    _velocity = 0.;
}

void
FirstPersonManipulator::home( const GUIEventAdapter& ea,
                              GUIActionAdapter&      us )
{
    inherited::home( ea, us );
    _velocity = 0.;
}

void
FirstPersonManipulator::init( const GUIEventAdapter& ea,
                              GUIActionAdapter&      us )
{
    inherited::init( ea, us );

    // stop movement
    _velocity = 0.;
}

// doc in parent
bool
FirstPersonManipulator::handleMouseWheel( const GUIEventAdapter& ea,
                                          GUIActionAdapter&      us )
{
    osgGA::GUIEventAdapter::ScrollingMotion sm = ea.getScrollingMotion();

    // handle centering
    if( _flags & SET_CENTER_ON_WHEEL_FORWARD_MOVEMENT )
    {

        if( ( ( sm == GUIEventAdapter::SCROLL_DOWN ) && ( _wheelMovement > 0. ) ) ||
            ( ( sm == GUIEventAdapter::SCROLL_UP ) && ( _wheelMovement < 0. ) ) )
        {

            // stop thrown animation
            _thrown = false;

            if( getAnimationTime() <= 0. )
            {

                // center by mouse intersection (no animation)
                setCenterByMousePointerIntersection( ea, us );
            }

            else
            {

                // start new animation only if there is no animation in progress
                if( !isAnimating() )
                {
                    startAnimationByMousePointerIntersection( ea, us );
                }
            }
        }
    }

    FirstPersonAnimationData* ad =
        dynamic_cast<FirstPersonAnimationData*>( _animationData.get() );
    if( !ad )
    {
        return false;
    }

    switch( sm )
    {

        // mouse scroll up event
        case GUIEventAdapter::SCROLL_UP :
            {
                // move forward
                moveForward(
                    isAnimating() ? ad->_targetRot : _rotation,
                    -_wheelMovement *
                        ( getRelativeFlag( _wheelMovementFlagIndex ) ? _modelSize : 1. )
                );
                us.requestRedraw();
                us.requestContinuousUpdate( isAnimating() || _thrown );
                return true;
            }

        // mouse scroll down event
        case GUIEventAdapter::SCROLL_DOWN :
            {
                // move backward
                moveForward( _wheelMovement *
                             ( getRelativeFlag( _wheelMovementFlagIndex ) ? _modelSize
                                                                          : 1. ) );
                _thrown = false;
                us.requestRedraw();
                us.requestContinuousUpdate( isAnimating() || _thrown );
                return true;
            }

        // unhandled mouse scrolling motion
        default :
            return false;
    }
}

// doc in parent
bool
FirstPersonManipulator::performMovementLeftMouseButton( const double /*eventTimeDelta*/,
                                                        const double dx,
                                                        const double dy )
{
    // world up vector
    CoordinateFrame coordinateFrame = getCoordinateFrame( _eye );
    dvec3           localUp         = getUpVector( coordinateFrame );

    rotateYawPitch( _rotation, dx, dy, localUp );

    return true;
}

bool
FirstPersonManipulator::performMouseDeltaMovement( const float dx,
                                                   const float dy )
{
    // rotate camera
    if( getVerticalAxisFixed() )
    {

        // world up vector
        CoordinateFrame coordinateFrame = getCoordinateFrame( _eye );
        dvec3           localUp         = getUpVector( coordinateFrame );

        rotateYawPitch( _rotation, dx, dy, localUp );
    }
    else
    {

        rotateYawPitch( _rotation, dx, dy );
    }

    return true;
}

/// Move camera forward by distance parameter.
void
FirstPersonManipulator::moveForward( const double distance )
{
    moveForward( _rotation, distance );
}

/// Move camera forward by distance parameter.
void
FirstPersonManipulator::moveForward( const quat&  rotation,
                                     const double distance )
{
    _eye += rotation * dvec3( 0., 0., -distance );
}

/// Move camera right by distance parameter.
void
FirstPersonManipulator::moveRight( const double distance )
{
    _eye += _rotation * dvec3( distance, 0., 0. );
}

/// Move camera up by distance parameter.
void
FirstPersonManipulator::moveUp( const double distance )
{
    _eye += _rotation * dvec3( 0., distance, 0. );
}

void
FirstPersonManipulator::applyAnimationStep( const double currentProgress,
                                            const double /*prevProgress*/ )
{
    FirstPersonAnimationData* ad =
        dynamic_cast<FirstPersonAnimationData*>( _animationData.get() );
    if( !ad )
    {
        return;
    }

    // compute new rotation
    _rotation =
        osg::mix( ad->_startRot, ad->_targetRot, static_cast<float>( currentProgress ) );

    // fix vertical axis
    if( getVerticalAxisFixed() )
    {
        fixVerticalAxis( _eye, _rotation, false );
    }
}

// doc in parent
bool
FirstPersonManipulator::startAnimationByMousePointerIntersection(
    const osgGA::GUIEventAdapter& ea,
    osgGA::GUIActionAdapter&      us
)
{
    // get current transformation
    osg::dvec3 prevEye;
    osg::quat  prevRot;
    getTransformation( prevEye, prevRot );

    // center by mouse intersection
    if( !setCenterByMousePointerIntersection( ea, us ) )
    {
        return false;
    }

    FirstPersonAnimationData* ad =
        dynamic_cast<FirstPersonAnimationData*>( _animationData.get() );
    if( !ad )
    {
        return false;
    }

    // setup animation data and restore original transformation
    ad->start( prevRot, _rotation, ea.getTime() );
    setTransformation( _eye, prevRot );

    return true;
}

void
FirstPersonManipulator::FirstPersonAnimationData::start( const quat&  startRotation,
                                                         const quat&  targetRotation,
                                                         const double startTime )
{
    AnimationData::start( startTime );

    _startRot  = startRotation;
    _targetRot = targetRotation;
}
