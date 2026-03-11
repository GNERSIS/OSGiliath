/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract base for standard camera manipulators. Provides common
 * mouse-button mapping, animation, and home-position logic.
 */
#include <osgGA/manipulators/StandardManipulator.hpp>

#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osgViewer/core/View.hpp>

using namespace osg;
using namespace osgGA;
using namespace osgUtil;

int StandardManipulator::numRelativeFlagsAllocated = 0;

int
StandardManipulator::allocateRelativeFlag()
{
    return numRelativeFlagsAllocated++;
}

/// Constructor.
StandardManipulator::StandardManipulator( int flags ) :
    inherited(),
    _thrown( false ),
    _allowThrow( true ),
    _mouseCenterX( 0.0F ),
    _mouseCenterY( 0.0F ),
    _delta_frame_time( 0.01 ),
    _last_frame_time( 0.0 ),
    _modelSize( 0. ),
    _verticalAxisFixed( true ),
    _flags( flags ),
    _relativeFlags( 0 )
{
}

/// Constructor.
StandardManipulator::StandardManipulator( const StandardManipulator& uim,
                                          const CopyOp&              copyOp ) :
    osg::Object( uim,
                 copyOp ),
    osg::Callback( uim,
                   copyOp ),
    inherited( uim,
               copyOp ),
    _thrown( uim._thrown ),
    _allowThrow( uim._allowThrow ),
    _mouseCenterX( 0.0F ),
    _mouseCenterY( 0.0F ),
    _ga_t1( dynamic_cast<GUIEventAdapter*>( copyOp( uim._ga_t1.get() ) ) ),
    _ga_t0( dynamic_cast<GUIEventAdapter*>( copyOp( uim._ga_t0.get() ) ) ),
    _delta_frame_time( 0.01 ),
    _last_frame_time( 0.0 ),
    _modelSize( uim._modelSize ),
    _verticalAxisFixed( uim._verticalAxisFixed ),
    _flags( uim._flags ),
    _relativeFlags( uim._relativeFlags )
{
}

/** Attach a node to the manipulator.
    Automatically detaches previously attached node.
    setNode(NULL) detaches previously attached nodes.
    Is ignored by manipulators which do not require a reference model.*/
void
StandardManipulator::setNode( Node* node )
{
    _node = node;

    // update model size
    if( _node.get() )
    {
        const sphere& boundingSphere = _node->getBound();
        _modelSize                   = boundingSphere.radius;
    }
    else
    {
        _modelSize = 0.;
    }

    // compute home position
    if( getAutoComputeHomePosition() )
    {
        computeHomePosition( NULL, ( _flags & COMPUTE_HOME_USING_BBOX ) != 0 );
    }
}

/** Return node if attached.*/
const Node*
StandardManipulator::getNode() const
{
    return _node.get();
}

/** Return node if attached.*/
Node*
StandardManipulator::getNode()
{
    return _node.get();
}

/** Makes manipulator to keep camera's "UP" vector.
 *
 *  In general, fixed up vector makes camera control more user friendly.
 *
 *  To change up vector, use CameraManipulator::setCoordinateFrameCallback.*/
void
StandardManipulator::setVerticalAxisFixed( bool value )
{
    _verticalAxisFixed = value;
}

/// Sets manipulator animation time when centering on mouse wheel up is enabled.
void
StandardManipulator::setAnimationTime( const double t )
{
    if( t <= 0. )
    {
        finishAnimation();
        _animationData = NULL;
        return;
    }

    if( !_animationData )
    {
        allocAnimationData();
    }

    _animationData->_animationTime = t;
}

/// Returns manipulator animation time when centering on mouse wheel up is enabled.
double
StandardManipulator::getAnimationTime() const
{
    if( _animationData )
    {
        return _animationData->_animationTime;
    }
    else
    {
        return 0.;
    }
}

/// Returns whether manipulator is performing animation at the moment.
bool
StandardManipulator::isAnimating() const
{
    if( _animationData )
    {
        return _animationData->_isAnimating;
    }
    else
    {
        return false;
    }
}

/// Finishes the animation by performing a step that moves it to its final position.
void
StandardManipulator::finishAnimation()
{
    _thrown = false;

    if( !isAnimating() )
    {
        return;
    }

    applyAnimationStep( 1., _animationData->_phase );
}

/** Move the camera to the default position.

    The user should probably want to use home(GUIEventAdapter&, GUIActionAdapter&)
    instead to set manipulator to the home position. This method does not trigger
    any redraw processing or updates continuous update processing.

    StandardManipulator implementation only updates its internal structures and
    recomputes its home position if autoComputeHomePosition is set.
    Descendant classes are expected to update camera position.*/
void
StandardManipulator::home( double /*currentTime*/ )
{
    if( getAutoComputeHomePosition() )
    {
        computeHomePosition( NULL, ( _flags & COMPUTE_HOME_USING_BBOX ) != 0 );
    }

    _thrown = false;
    setTransformation( _homeEye, _homeCenter, _homeUp );
    flushMouseEventStack();
}

/** Move the camera to the default position.

    If autoComputeHomePosition is on, home position is computed.
    The computation considers camera fov and model size and
    positions camera far enough to fit the model to the screen.

    StandardManipulator implementation only updates its internal data.
    If home position is expected to be supported by the descendant manipulator,
    it has to reimplement the method to update manipulator transformation.*/
void
StandardManipulator::home( const GUIEventAdapter& /*ea*/,
                           GUIActionAdapter& us )
{
    if( getAutoComputeHomePosition() )
    {
        const Camera* camera = us.asView() ? us.asView()->getCamera() : NULL;
        computeHomePosition( camera, ( _flags & COMPUTE_HOME_USING_BBOX ) != 0 );
    }

    _thrown = false;
    setTransformation( _homeEye, _homeCenter, _homeUp );

    us.requestRedraw();
    us.requestContinuousUpdate( false );
    flushMouseEventStack();
}

/** Start/restart the manipulator.*/
void
StandardManipulator::init( const GUIEventAdapter& /*ea*/,
                           GUIActionAdapter& us )
{
    flushMouseEventStack();

    // stop animation
    _thrown = false;
    us.requestContinuousUpdate( false );
}

/** Handles events. Returns true if handled, false otherwise.*/
bool
StandardManipulator::handle( const GUIEventAdapter& ea,
                             GUIActionAdapter&      us )
{
    switch( ea.getEventType() )
    {

        case GUIEventAdapter::FRAME :
            return handleFrame( ea, us );

        case GUIEventAdapter::RESIZE :
            return handleResize( ea, us );

        default :
            break;
    }

    if( ea.getHandled() )
    {
        return false;
    }

    switch( ea.getEventType() )
    {
        case GUIEventAdapter::MOVE :
            return handleMouseMove( ea, us );

        case GUIEventAdapter::DRAG :
            return handleMouseDrag( ea, us );

        case GUIEventAdapter::PUSH :
            return handleMousePush( ea, us );

        case GUIEventAdapter::RELEASE :
            return handleMouseRelease( ea, us );

        case GUIEventAdapter::KEYDOWN :
            return handleKeyDown( ea, us );

        case GUIEventAdapter::KEYUP :
            return handleKeyUp( ea, us );

        case GUIEventAdapter::SCROLL :
            if( _flags & PROCESS_MOUSE_WHEEL )
            {
                return handleMouseWheel( ea, us );
            }
            else
            {
                return false;
            }

        default :
            return false;
    }
}

/// Handles GUIEventAdapter::FRAME event.
bool
StandardManipulator::handleFrame( const GUIEventAdapter& ea,
                                  GUIActionAdapter&      us )
{
    double current_frame_time = ea.getTime();

    _delta_frame_time         = current_frame_time - _last_frame_time;
    _last_frame_time          = current_frame_time;

    if( _thrown && performMovement() )
    {
        us.requestRedraw();
    }

    if( _animationData && _animationData->_isAnimating )
    {
        performAnimationMovement( ea, us );
    }

    return false;
}

/// Handles GUIEventAdapter::RESIZE event.
bool
StandardManipulator::handleResize( const GUIEventAdapter& ea,
                                   GUIActionAdapter&      us )
{
    init( ea, us );
    us.requestRedraw();

    return true;
}

/// Handles GUIEventAdapter::MOVE event.
bool
StandardManipulator::handleMouseMove( const GUIEventAdapter& /*ea*/,
                                      GUIActionAdapter& /*us*/ )
{
    return false;
}

/// Handles GUIEventAdapter::DRAG event.
bool
StandardManipulator::handleMouseDrag( const GUIEventAdapter& ea,
                                      GUIActionAdapter&      us )
{
    addMouseEvent( ea );

    if( performMovement() )
    {
        us.requestRedraw();
    }

    us.requestContinuousUpdate( false );
    _thrown = false;

    return true;
}

/// Handles GUIEventAdapter::PUSH event.
bool
StandardManipulator::handleMousePush( const GUIEventAdapter& ea,
                                      GUIActionAdapter&      us )
{
    flushMouseEventStack();
    addMouseEvent( ea );

    if( performMovement() )
    {
        us.requestRedraw();
    }

    us.requestContinuousUpdate( false );
    _thrown = false;

    return true;
}

/// Handles GUIEventAdapter::RELEASE event.
bool
StandardManipulator::handleMouseRelease( const GUIEventAdapter& ea,
                                         GUIActionAdapter&      us )
{
    if( ea.getButtonMask() == 0 )
    {

        double timeSinceLastRecordEvent =
            _ga_t0.valid() ? ( ea.getTime() - _ga_t0->getTime() ) : DBL_MAX;
        if( timeSinceLastRecordEvent > 0.02 )
        {
            flushMouseEventStack();
        }

        if( isMouseMoving() )
        {

            if( performMovement() && _allowThrow )
            {
                us.requestRedraw();
                us.requestContinuousUpdate( true );
                _thrown = true;
            }

            return true;
        }
    }

    flushMouseEventStack();
    addMouseEvent( ea );
    if( performMovement() )
    {
        us.requestRedraw();
    }
    us.requestContinuousUpdate( false );
    _thrown = false;

    return true;
}

/// Handles GUIEventAdapter::KEYDOWN event.
bool
StandardManipulator::handleKeyDown( const GUIEventAdapter& ea,
                                    GUIActionAdapter&      us )
{
    if( ea.getKey() == GUIEventAdapter::KEY_Space )
    {
        flushMouseEventStack();
        _thrown = false;
        home( ea, us );
        return true;
    }

    return false;
}

/// Handles GUIEventAdapter::KEYUP event.
bool
StandardManipulator::handleKeyUp( const GUIEventAdapter& /*ea*/,
                                  GUIActionAdapter& /*us*/ )
{
    return false;
}

/// Handles GUIEventAdapter::SCROLL event.
bool
StandardManipulator::handleMouseWheel( const GUIEventAdapter& /*ea*/,
                                       GUIActionAdapter& /*us*/ )
{
    return false;
}

/** Get the keyboard and mouse usage of the manipulator.*/
void
StandardManipulator::getUsage( ApplicationUsage& usage ) const
{
    usage.addKeyboardMouseBinding( getManipulatorName() + ": Space",
                                   "Reset the viewing position to home" );
}

/// Make movement step of manipulator. Returns true if any movement was made.
bool
StandardManipulator::performMovement()
{
    // return if less then two events have been added
    if( _ga_t0.get() == NULL || _ga_t1.get() == NULL )
    {
        return false;
    }

    // get delta time
    double eventTimeDelta = _ga_t0->getTime() - _ga_t1->getTime();
    if( eventTimeDelta < 0. )
    {
        OSG_WARN << "Manipulator warning: eventTimeDelta = " << eventTimeDelta
                 << std::endl;
        eventTimeDelta = 0.;
    }

    // get deltaX and deltaY
    float dx = _ga_t0->getXnormalized() - _ga_t1->getXnormalized();
    float dy = _ga_t0->getYnormalized() - _ga_t1->getYnormalized();

    // return if there is no movement.
    if( dx == 0. && dy == 0. )
    {
        return false;
    }

    // call appropriate methods
    unsigned int buttonMask = static_cast<unsigned int>( _ga_t1->getButtonMask() );
    unsigned int modKeyMask = static_cast<unsigned int>( _ga_t1->getModKeyMask() );
    if( buttonMask == GUIEventAdapter::LEFT_MOUSE_BUTTON )
    {
        return performMovementLeftMouseButton( eventTimeDelta, dx, dy );
    }
    else if( ( buttonMask == GUIEventAdapter::MIDDLE_MOUSE_BUTTON ) ||
             ( buttonMask ==
               GUIEventAdapter::RIGHT_MOUSE_BUTTON &&
               modKeyMask &
               GUIEventAdapter::MODKEY_CTRL ) ||
             ( buttonMask == ( GUIEventAdapter::LEFT_MOUSE_BUTTON |
                               GUIEventAdapter::RIGHT_MOUSE_BUTTON ) ) )
    {
        return performMovementMiddleMouseButton( eventTimeDelta, dx, dy );
    }
    else if( buttonMask == GUIEventAdapter::RIGHT_MOUSE_BUTTON )
    {
        return performMovementRightMouseButton( eventTimeDelta, dx, dy );
    }

    return false;
}

/** Make movement step of manipulator.
    This method implements movement for left mouse button.*/
bool
StandardManipulator::performMovementLeftMouseButton( const double /*eventTimeDelta*/,
                                                     const double /*dx*/,
                                                     const double /*dy*/ )
{
    return false;
}

/** Make movement step of manipulator.
    This method implements movement for middle mouse button
    or combination of left and right mouse button pressed together.*/
bool
StandardManipulator::performMovementMiddleMouseButton( const double /*eventTimeDelta*/,
                                                       const double /*dx*/,
                                                       const double /*dy*/ )
{
    return false;
}

/** Make movement step of manipulator.
    This method implements movement for right mouse button.*/
bool
StandardManipulator::performMovementRightMouseButton( const double /*eventTimeDelta*/,
                                                      const double /*dx*/,
                                                      const double /*dy*/ )
{
    return false;
}

/// The method processes events for manipulation based on relative mouse movement (mouse
/// delta).
bool
StandardManipulator::handleMouseDeltaMovement( const GUIEventAdapter& ea,
                                               GUIActionAdapter&      us )
{
    float dx = ea.getX() - _mouseCenterX;
    float dy = ea.getY() - _mouseCenterY;

    if( dx == 0.F && dy == 0.F )
    {
        return false;
    }

    addMouseEvent( ea );
    centerMousePointer( ea, us );

    return performMouseDeltaMovement( dx, dy );
}

/// The method performs manipulator update based on relative mouse movement (mouse
/// delta).
bool
StandardManipulator::performMouseDeltaMovement( const float /*dx*/,
                                                const float /*dy*/ )
{
    return false;
}

/// Makes the manipulator progress in its current animation.
bool
StandardManipulator::performAnimationMovement( const GUIEventAdapter& ea,
                                               GUIActionAdapter&      us )
{
    double f =
        ( ea.getTime() - _animationData->_startTime ) / _animationData->_animationTime;
    if( f >= 1. )
    {
        f                            = 1.;
        _animationData->_isAnimating = false;
        if( !_thrown )
        {
            us.requestContinuousUpdate( false );
        }
    }

    applyAnimationStep( f, _animationData->_phase );

    _animationData->_phase = f;
    us.requestRedraw();

    return _animationData->_isAnimating;
}

/// Updates manipulator by a single animation step
void
StandardManipulator::applyAnimationStep( const double /*currentProgress*/,
                                         const double /*prevProgress*/ )
{
}

/// Centers mouse pointer
void
StandardManipulator::centerMousePointer( const GUIEventAdapter& ea,
                                         GUIActionAdapter&      us )
{
    _mouseCenterX = ( ea.getXmin() + ea.getXmax() ) / 2.0F;
    _mouseCenterY = ( ea.getYmin() + ea.getYmax() ) / 2.0F;
    us.requestWarpPointer( _mouseCenterX, _mouseCenterY );
}

/** Add the current mouse GUIEvent to internal stack.*/
void
StandardManipulator::addMouseEvent( const GUIEventAdapter& ea )
{
    _ga_t1 = _ga_t0;
    _ga_t0 = &ea;
}

/** Reset the internal GUIEvent stack.*/
void
StandardManipulator::flushMouseEventStack()
{
    _ga_t1 = NULL;
    _ga_t0 = NULL;
}

/** Check the speed at which the mouse is moving.
    If speed is below a threshold then return false, otherwise return true.*/
bool
StandardManipulator::isMouseMoving() const
{
    if( _ga_t0.get() == NULL || _ga_t1.get() == NULL )
    {
        return false;
    }

    static const float velocity = 0.1F;

    float              dx       = _ga_t0->getXnormalized() - _ga_t1->getXnormalized();
    float              dy       = _ga_t0->getYnormalized() - _ga_t1->getYnormalized();
    float              len      = sqrtf( dx * dx + dy * dy );
    float              dt = static_cast<float>( _ga_t0->getTime() - _ga_t1->getTime() );

    return ( len > dt * velocity );
}

/** Set the 'allow throw' flag. If it is set to true (default), releasing the mouse
   button while moving the mouse results in a throw. If manipulator was thrown, it
   continues spinning although no mouse button is down at the moment.*/
void
StandardManipulator::setAllowThrow( bool allowThrow )
{
    _allowThrow = allowThrow;
}

/** Returns the scale that should be applied on animation of "thrown" manipulator state
    to avoid its dependency on varying frame rate.

    eventTimeDelta parameter gives the time difference between last two
    events that started the animation.*/
float
StandardManipulator::getThrowScale( const double eventTimeDelta ) const
{
    if( _thrown )
    {
        if( eventTimeDelta == 0.F )
        {
            return 0.F;
        }
        return float( _delta_frame_time / eventTimeDelta );
    }
    else
    {
        return 1.F;
    }
}

/** Update rotation by yaw and pitch.
 *
 *  localUp parameter defines either camera's "UP" vector
 *  that will be preserved during rotation, or it can be zero (0,0,0) to specify
 *  that camera's "UP" vector will be not preserved and free rotation will be made.*/
void
StandardManipulator::rotateYawPitch( quat&        rotation,
                                     const double yaw,
                                     const double pitch,
                                     const dvec3& localUp )
{
    bool verticalAxisFixed = ( localUp != dvec3( 0., 0., 0. ) );

    // fix current rotation
    if( verticalAxisFixed )
    {
        fixVerticalAxis( rotation, localUp, true );
    }

    // rotations
    quat   rotateYaw( quat(
        dquat( -yaw,
               verticalAxisFixed ? localUp : dquat( rotation ) * dvec3( 0., 1., 0. ) )
    ) );
    quat   rotatePitch;
    quat   newRotation;
    dvec3  cameraRight( dquat( rotation ) * dvec3( 1., 0., 0. ) );

    double my_dy = pitch;
    int    i     = 0;

    do
    {

        // rotations
        rotatePitch = quat( dquat( my_dy, cameraRight ) );
        newRotation = rotation * rotateYaw * rotatePitch;

        // update vertical axis
        if( verticalAxisFixed )
        {
            fixVerticalAxis( newRotation, localUp, false );
        }

        // check for viewer's up vector to be more than 90 degrees from "up" axis
        dvec3 newCameraUp = dquat( newRotation ) * dvec3( 0., 1., 0. );
        if( osg::dot( newCameraUp, localUp ) > 0. )
        {

            // apply new rotation
            rotation = newRotation;
            return;
        }

        my_dy /= 2.;
        if( ++i == 20 )
        {
            rotation = rotation * rotateYaw;
            return;
        }

    } while( true );
}

/** The method corrects the rotation to make impression of fixed up direction.
 *  Technically said, it makes the roll component of the rotation equal to zero.
 *
 *  Up vector is given by CoordinateFrame and it is +z by default.
 *  It can be changed by osgGA::CameraManipulator::setCoordinateFrameCallback().
 *
 *  Eye parameter is user position, rotation is the rotation to be fixed, and
 *  disallowFlipOver, when set on true, avoids pitch rotation component to grow
 *  over +/- 90 degrees. If this happens and disallowFlipOver is true,
 *  manipulator is rotated by 180 degrees. More precisely, roll rotation component is
 * changed by 180 degrees, making pitch once again between -90..+90 degrees limits.*/
void
StandardManipulator::fixVerticalAxis( dvec3& eye,
                                      quat&  rotation,
                                      bool   disallowFlipOver )
{
    CoordinateFrame coordinateFrame = getCoordinateFrame( eye );
    dvec3           localUp         = getUpVector( coordinateFrame );

    fixVerticalAxis( rotation, localUp, disallowFlipOver );
}

/** The method corrects the rotation to make impression of fixed up direction.
 *  Technically said, it makes the roll component of the rotation equal to zero.
 *
 *  rotation parameter is the rotation to be fixed.
 *  localUp is UP vector and must not be zero length.
 *  disallowFlipOver, when set on true, avoids pitch rotation component to grow
 *  over +/- 90 degrees. If this happens and disallowFlipOver is true,
 *  manipulator is rotated by 180 degrees. More precisely, roll rotation component is
 * changed by 180 degrees, making pitch once again between -90..+90 degrees limits.*/
void
StandardManipulator::fixVerticalAxis( quat&        rotation,
                                      const dvec3& localUp,
                                      bool         disallowFlipOver )
{
    // camera direction vectors
    dvec3 cameraUp      = dquat( rotation ) * dvec3( 0., 1., 0. );
    dvec3 cameraRight   = dquat( rotation ) * dvec3( 1., 0., 0. );
    dvec3 cameraForward = dquat( rotation ) * dvec3( 0., 0., -1. );

    // computed directions
    dvec3 newCameraRight1 = cameraForward ^ localUp;
    dvec3 newCameraRight2 = cameraUp ^ localUp;
    dvec3 newCameraRight =
        ( osg::length2( newCameraRight1 ) > osg::length2( newCameraRight2 ) )
            ? newCameraRight1
            : newCameraRight2;
    if( osg::dot( newCameraRight, cameraRight ) < 0. )
    {
        newCameraRight = -newCameraRight;
    }

    // vertical axis correction
    quat rotationVerticalAxisCorrection;
    rotationVerticalAxisCorrection = quat( dquat( cameraRight, newCameraRight ) );

    // rotate camera
    rotation = rotation * rotationVerticalAxisCorrection;

    if( disallowFlipOver )
    {

        // make viewer's up vector to be always less than 90 degrees from "up" axis
        dvec3 newCameraUp = newCameraRight ^ cameraForward;
        if( osg::dot( newCameraUp, localUp ) < 0. )
        {
            rotation = quat( dquat( PI, dvec3( 0., 0., 1. ) ) ) * rotation;
        }
    }
}

/** The method corrects the rotation to make impression of fixed up direction.
 *  Technically said, it makes the roll component of the rotation equal to zero.
 *
 *  forward and up parameters are the forward and up vectors of the manipulator.
 *  newUp will receive corrected UP manipulator vector. localUp is UP vector
 *  that is used for vertical correction.
 *  disallowFlipOver when set on true avoids pitch rotation component to grow
 *  over +/- 90 degrees. If this happens and disallowFlipOver is true,
 *  right and up camera vectors are negated (changing roll by 180 degrees),
 *  making pitch once again between -90..+90 degrees limits.*/
void
StandardManipulator::fixVerticalAxis( const osg::dvec3& forward,
                                      const osg::dvec3& up,
                                      osg::dvec3&       newUp,
                                      const osg::dvec3& localUp,
                                      bool /*disallowFlipOver*/ )
{
    // right direction
    osg::dvec3 right1 = forward ^ localUp;
    osg::dvec3 right2 = up ^ localUp;
    osg::dvec3 right =
        ( osg::length2( right1 ) > osg::length2( right2 ) ) ? right1 : right2;

    // updatedUp
    osg::dvec3 updatedUp = right ^ forward;
    updatedUp            = osg::normalize( updatedUp );
    if( osg::length2( updatedUp ) > 0. )
    {

        // return updatedUp
        newUp = updatedUp;
    }

    else
    {

        // return original up
        OSG_WARN << "StandardManipulator::fixVerticalAxis warning: Can not update "
                    "vertical axis."
                 << std::endl;
        newUp = up;
    }
}

/** The method sends a ray into the scene and the point of the closest intersection
    is used to set a new center for the manipulator. For Orbit-style manipulators,
    the orbiting center is set. For FirstPerson-style manipulators, view is pointed
    towards the center.*/
bool
StandardManipulator::setCenterByMousePointerIntersection( const GUIEventAdapter& ea,
                                                          GUIActionAdapter&      us )
{
    osg::View* view = us.asView();
    if( !view )
    {
        return false;
    }

    Camera* camera = view->getCamera();
    if( !camera )
    {
        return false;
    }

    // prepare variables
    float x = ( ea.getX() - ea.getXmin() ) / ( ea.getXmax() - ea.getXmin() );
    float y = ( ea.getY() - ea.getYmin() ) / ( ea.getYmax() - ea.getYmin() );
    LineSegmentIntersector::CoordinateFrame cf;
    Viewport*                               vp = camera->getViewport();
    if( vp )
    {
        cf  = Intersector::WINDOW;
        x  *= static_cast<float>( vp->width() );
        y  *= static_cast<float>( vp->height() );
    }
    else
    {
        cf = Intersector::PROJECTION;
    }

    // perform intersection computation
    ref_ptr<LineSegmentIntersector> picker = new LineSegmentIntersector( cf, x, y );
    IntersectionVisitor             iv( picker.get() );
    camera->accept( iv );

    // return on no intersections
    if( !picker->containsIntersections() )
    {
        return false;
    }

    // get all intersections
    LineSegmentIntersector::Intersections& intersections = picker->getIntersections();

    // get current transformation
    osg::dvec3                             eye, oldCenter, up;
    getTransformation( eye, oldCenter, up );

    // new center
    osg::dvec3 newCenter = ( *intersections.begin() ).getWorldIntersectPoint();

    // make vertical axis correction
    if( getVerticalAxisFixed() )
    {

        CoordinateFrame coordinateFrame = getCoordinateFrame( newCenter );
        dvec3           localUp         = getUpVector( coordinateFrame );

        fixVerticalAxis( newCenter - eye, up, up, localUp, true );
    }

    // set the new center
    setTransformation( eye, newCenter, up );

    // warp pointer
    // note: this works for me on standard camera on GraphicsWindowEmbedded and Qt,
    //       while it was necessary to implement requestWarpPointer like follows:
    //
    // void QOSGWidget::requestWarpPointer( float x, float y )
    // {
    //    osgViewer::Viewer::requestWarpPointer( x, y );
    //    QCursor::setPos( this->mapToGlobal( QPoint( int( x+.5f ), int( y+.5f ) ) ) );
    // }
    //
    // Additions of .5f are just for the purpose of rounding.
    centerMousePointer( ea, us );

    return true;
}

/** Makes mouse pointer intersection test with the geometry below the pointer
    and starts animation to center camera to look at the closest hit below the mouse
   pointer.

    If there is a hit, animation is started and true is returned.
    Otherwise, the method returns false.*/
bool
StandardManipulator::
    startAnimationByMousePointerIntersection( const osgGA::GUIEventAdapter& /*ea*/,
                                              osgGA::GUIActionAdapter& /*us*/ )
{
    return false;
}

StandardManipulator::AnimationData::AnimationData() :
    _animationTime( 0.0 ),
    _isAnimating( false ),
    _startTime( 0.0 ),
    _phase( 0.0 )
{
}

void
StandardManipulator::AnimationData::start( const double startTime )
{
    _isAnimating = true;
    _startTime   = startTime;
    _phase       = 0.;
}
