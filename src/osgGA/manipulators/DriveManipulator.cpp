/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Ground-vehicle camera manipulator. Provides steering and
 * elevation-following for drive-through navigation.
 */
#if defined( _MSC_VER )
    #pragma warning( disable : 4'786 )
#endif

#include <osgGA/manipulators/DriveManipulator.hpp>

#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osgUtil/intersection/LineSegmentIntersector.hpp>
#include <stdlib.h>

using namespace osg;
using namespace osgGA;

#define DRIVER_HEIGHT  15

// #define ABOSULTE_PITCH 1
// #define INCREMENTAL_PITCH 1
#define KEYBOARD_PITCH 1

static double
getHeightOfDriver()
{
    double height = 1.5;
    if( getenv( "OSG_DRIVE_MANIPULATOR_HEIGHT" ) )
    {
        height = osg::asciiToDouble( getenv( "OSG_DRIVE_MANIPULATOR_HEIGHT" ) );
    }
    OSG_INFO << "DriveManipulator::_height set to ==" << height << std::endl;
    return height;
}

DriveManipulator::DriveManipulator()
{
    _modelScale = 1.0;
    _velocity   = 0.0;
    _height     = getHeightOfDriver();
    _buffer     = _height * 2.5;

    //_speedMode = USE_MOUSE_Y_FOR_SPEED;
    _speedMode           = USE_MOUSE_BUTTONS_FOR_SPEED;

    _pitch               = 0.0;
    _distance            = 0.0;

    _pitchUpKeyPressed   = false;
    _pitchDownKeyPressed = false;
}

DriveManipulator::~DriveManipulator()
{
}

void
DriveManipulator::setNode( osg::Node* node )
{
    _node = node;
    if( _node.get() )
    {
        const osg::sphere& boundingSphere = _node->getBound();

        _modelScale                       = boundingSphere.radius;
        //_height = sqrtf(_modelScale)*0.03;
        //_buffer = sqrtf(_modelScale)*0.05;

        _height = getHeightOfDriver();
        _buffer = _height * 2.5;
    }
    if( getAutoComputeHomePosition() )
    {
        computeHomePosition();
    }
}

const osg::Node*
DriveManipulator::getNode() const
{
    return _node.get();
}

osg::Node*
DriveManipulator::getNode()
{
    return _node.get();
}

bool
DriveManipulator::intersect( const osg::dvec3& start,
                             const osg::dvec3& end,
                             osg::dvec3&       intersection,
                             osg::dvec3&       normal ) const
{
    osg::ref_ptr<osgUtil::LineSegmentIntersector> lsi =
        new osgUtil::LineSegmentIntersector( start, end );

    osgUtil::IntersectionVisitor iv( lsi.get() );
    iv.setTraversalMask( _intersectTraversalMask );

    _node->accept( iv );

    if( lsi->containsIntersections() )
    {
        intersection = lsi->getIntersections().begin()->getWorldIntersectPoint();
        normal       = lsi->getIntersections().begin()->getWorldIntersectNormal();
        return true;
    }
    return false;
}

void
DriveManipulator::computeHomePosition()
{
    if( _node.get() )
    {
        const osg::sphere&   boundingSphere  = _node->getBound();

        osg::dvec3           ep              = osg::dvec3( boundingSphere.center );
        osg::dvec3           bp              = ep;

        osg::CoordinateFrame cf              = getCoordinateFrame( ep );

        ep                                  -= getUpVector( cf ) * _modelScale * 0.0001;
        bp                                  -= getUpVector( cf ) * _modelScale;

        // check to see if any obstruction in front.
        bool       positionSet = false;

        osg::dvec3 ip, np;
        if( intersect( ep, bp, ip, np ) )
        {
            osg::dvec3 uv;
            if( osg::dot( np, getUpVector( cf ) ) > 0.0 )
            {
                uv = np;
            }
            else
            {
                uv = -np;
            }

            ep             = ip;
            ep            += getUpVector( cf ) * _height;
            osg::dvec3 lv  = osg::cross( uv, osg::dvec3( 1.0, 0.0, 0.0 ) );

            setHomePosition( ep, ep + lv, uv );

            positionSet = true;
        }

        if( !positionSet )
        {
            bp  = ep;
            bp += getUpVector( cf ) * _modelScale;

            if( intersect( ep, bp, ip, np ) )
            {

                osg::dvec3 uv;
                if( osg::dot( np, getUpVector( cf ) ) > 0.0 )
                {
                    uv = np;
                }
                else
                {
                    uv = -np;
                }

                ep             = ip;
                ep            += getUpVector( cf ) * _height;
                osg::dvec3 lv  = osg::cross( uv, osg::dvec3( 1.0, 0.0, 0.0 ) );
                setHomePosition( ep, ep + lv, uv );

                positionSet = true;
            }
        }

        if( !positionSet )
        {
            setHomePosition( osg::dvec3( boundingSphere.center ) +
                                 osg::dvec3( 0.0, -2.0 * boundingSphere.radius, 0.0 ),
                             osg::dvec3( boundingSphere.center ) +
                                 osg::dvec3( 0.0, -2.0 * boundingSphere.radius, 0.0 ) +
                                 osg::dvec3( 0.0, 1.0, 0.0 ),
                             osg::dvec3( 0.0, 0.0, 1.0 ) );
        }
    }
}

void
DriveManipulator::home( const GUIEventAdapter& ea,
                        GUIActionAdapter&      us )
{
    if( getAutoComputeHomePosition() )
    {
        computeHomePosition();
    }

    computePosition( _homeEye, _homeCenter, _homeUp );

    _velocity = 0.0;

    _pitch    = 0.0;

    us.requestRedraw();
    us.requestContinuousUpdate( false );

    us.requestWarpPointer( ( ea.getXmin() + ea.getXmax() ) / 2.0F,
                           ( ea.getYmin() + ea.getYmax() ) / 2.0F );

    flushMouseEventStack();
}

void
DriveManipulator::init( const GUIEventAdapter& ea,
                        GUIActionAdapter&      us )
{
    flushMouseEventStack();

    us.requestContinuousUpdate( false );

    _velocity                            = 0.0;

    osg::dvec3           ep              = _eye;

    osg::CoordinateFrame cf              = getCoordinateFrame( ep );

    dmat4                rotation_matrix = osg::rotate( osg::dquat( _rotation ) );
    osg::dvec3           sv              = osg::dvec3( 1.0, 0.0, 0.0 ) * rotation_matrix;
    osg::dvec3           bp              = ep;
    bp                     -= getUpVector( cf ) * _modelScale;

    bool       positionSet  = false;
    osg::dvec3 ip, np;
    if( intersect( ep, bp, ip, np ) )
    {

        osg::dvec3 uv;
        if( osg::dot( np, getUpVector( cf ) ) > 0.0 )
        {
            uv = np;
        }
        else
        {
            uv = -np;
        }

        ep            = ip + uv * _height;
        osg::dvec3 lv = osg::cross( uv, sv );

        computePosition( ep, ep + lv, uv );

        positionSet = true;
    }

    if( !positionSet )
    {
        bp  = ep;
        bp += getUpVector( cf ) * _modelScale;

        if( intersect( ep, bp, ip, np ) )
        {

            osg::dvec3 uv;
            if( osg::dot( np, getUpVector( cf ) ) > 0.0 )
            {
                uv = np;
            }
            else
            {
                uv = -np;
            }

            ep            = ip + uv * _height;
            osg::dvec3 lv = osg::cross( uv, sv );

            computePosition( ep, ep + lv, uv );

            positionSet = true;
        }
    }

    if( ea.getEventType() != GUIEventAdapter::RESIZE )
    {
        us.requestWarpPointer( ( ea.getXmin() + ea.getXmax() ) / 2.0F,
                               ( ea.getYmin() + ea.getYmax() ) / 2.0F );
    }
}

bool
DriveManipulator::handle( const GUIEventAdapter& ea,
                          GUIActionAdapter&      us )
{
    switch( ea.getEventType() )
    {
        case( GUIEventAdapter::FRAME ) :
            addMouseEvent( ea );
            if( calcMovement() )
            {
                us.requestRedraw();
            }
            return false;

        case( GUIEventAdapter::RESIZE ) :
            init( ea, us );
            us.requestRedraw();
            return true;
        default :
            break;
    }

    if( ea.getHandled() )
    {
        return false;
    }

    switch( ea.getEventType() )
    {
        case( GUIEventAdapter::PUSH ) :
            {

                addMouseEvent( ea );
                us.requestContinuousUpdate( true );
                if( calcMovement() )
                {
                    us.requestRedraw();
                }
                return true;
            }

        case( GUIEventAdapter::RELEASE ) :
            {

                addMouseEvent( ea );
                us.requestContinuousUpdate( true );
                if( calcMovement() )
                {
                    us.requestRedraw();
                }
                return true;
            }

        case( GUIEventAdapter::DRAG ) :
            {

                addMouseEvent( ea );
                us.requestContinuousUpdate( true );
                if( calcMovement() )
                {
                    us.requestRedraw();
                }
                return true;
            }

        case( GUIEventAdapter::MOVE ) :
            {

                addMouseEvent( ea );
                us.requestContinuousUpdate( true );
                if( calcMovement() )
                {
                    us.requestRedraw();
                }
                return true;
            }

        case( GUIEventAdapter::KEYDOWN ) :
            {
                if( ea.getKey() == GUIEventAdapter::KEY_Space )
                {
                    flushMouseEventStack();
                    home( ea, us );
                    return true;
                }
                else if( ea.getKey() == 'q' )
                {
                    _speedMode = USE_MOUSE_Y_FOR_SPEED;
                    return true;
                }
                else if( ea.getKey() == 'a' )
                {
                    _speedMode = USE_MOUSE_BUTTONS_FOR_SPEED;
                    return true;
                }
#ifdef KEYBOARD_PITCH
                else if( ea.getKey() ==
                         osgGA::GUIEventAdapter::KEY_Up ||
                         ea.getKey() ==
                         osgGA::GUIEventAdapter::KEY_KP_Up ||
                         ea.getKey() == '9' )
                {
                    _pitchUpKeyPressed = true;
                    return true;
                }
                else if( ea.getKey() ==
                         osgGA::GUIEventAdapter::KEY_Down ||
                         ea.getKey() ==
                         osgGA::GUIEventAdapter::KEY_KP_Down ||
                         ea.getKey() == '6' )
                {
                    _pitchDownKeyPressed = true;
                    return true;
                }
#endif
                return false;
            }

        case( GUIEventAdapter::KEYUP ) :
            {
#ifdef KEYBOARD_PITCH
                if( ea.getKey() ==
                    osgGA::GUIEventAdapter::KEY_Up ||
                    ea.getKey() ==
                    osgGA::GUIEventAdapter::KEY_KP_Up ||
                    ea.getKey() == '9' )
                {
                    _pitchUpKeyPressed = false;
                    return true;
                }
                else if( ea.getKey() ==
                         osgGA::GUIEventAdapter::KEY_Down ||
                         ea.getKey() ==
                         osgGA::GUIEventAdapter::KEY_KP_Down ||
                         ea.getKey() == '6' )
                {
                    _pitchDownKeyPressed = false;
                    return true;
                }
#endif
                return false;
            }

        default :
            return false;
    }
}

void
DriveManipulator::getUsage( osg::ApplicationUsage& usage ) const
{
    usage.addKeyboardMouseBinding( "Drive: Space",
                                   "Reset the viewing position to home" );
    usage.addKeyboardMouseBinding( "Drive: q", "Use mouse y for controlling speed" );
    usage.addKeyboardMouseBinding( "Drive: a",
                                   "Use mouse middle,right mouse buttons for speed" );
    usage.addKeyboardMouseBinding( "Drive: Down", "Cursor down key to look downwards" );
    usage.addKeyboardMouseBinding( "Drive: Up", "Cursor up key to look upwards" );
}

void
DriveManipulator::flushMouseEventStack()
{
    _ga_t1 = NULL;
    _ga_t0 = NULL;
}

void
DriveManipulator::addMouseEvent( const GUIEventAdapter& ea )
{
    _ga_t1 = _ga_t0;
    _ga_t0 = &ea;
}

void
DriveManipulator::setByMatrix( const osg::dmat4& matrix )
{
    _eye      = osg::getTrans( matrix );
    _rotation = osg::quat( osg::getRotate( matrix ) );
}

osg::dmat4
DriveManipulator::getMatrix() const
{
    return osg::translate( _eye ) *
           osg::rotate( osg::dquat( _rotation ) ) *
           osg::rotate( _pitch, 1.0, 0.0, 0.0 );
}

osg::dmat4
DriveManipulator::getInverseMatrix() const
{
    return osg::rotate( -_pitch, 1.0, 0.0, 0.0 ) *
           osg::rotate( osg::inverse( osg::dquat( _rotation ) ) ) *
           osg::translate( -_eye );
}

void
DriveManipulator::computePosition( const osg::dvec3& eye,
                                   const osg::dvec3& center,
                                   const osg::dvec3& up )
{
    osg::dvec3 lv = center - eye;

    osg::dvec3 f  = osg::normalize( lv );
    osg::dvec3 s  = osg::normalize( osg::cross( f, up ) );
    osg::dvec3 u  = osg::normalize( osg::cross( s, f ) );

    // Basis vectors as rows for column-vector convention
    osg::dmat4 rotation_matrix( s[0],
                                u[0],
                                -f[0],
                                0.0,
                                s[1],
                                u[1],
                                -f[1],
                                0.0,
                                s[2],
                                u[2],
                                -f[2],
                                0.0,
                                0.0,
                                0.0,
                                0.0,
                                1.0 );

    _eye      = eye;
    _rotation = osg::quat( osg::inverse( osg::getRotate( rotation_matrix ) ) );
}

bool
DriveManipulator::calcMovement()
{
    // return if less then two events have been added.
    if( _ga_t0.get() == NULL || _ga_t1.get() == NULL )
    {
        return false;
    }

    double dt = _ga_t0->getTime() - _ga_t1->getTime();

    if( dt < 0.0F )
    {
        OSG_INFO << "warning dt = " << dt << std::endl;
        dt = 0.0;
    }

    double accelerationFactor = _height * 10.0;

    switch( _speedMode )
    {
        case( USE_MOUSE_Y_FOR_SPEED ) :
            {
                double dy = _ga_t0->getYnormalized();
                _velocity = _height * dy;
                break;
            }
        case( USE_MOUSE_BUTTONS_FOR_SPEED ) :
            {
                unsigned int buttonMask =
                    static_cast<unsigned int>( _ga_t1->getButtonMask() );
                if( buttonMask == GUIEventAdapter::LEFT_MOUSE_BUTTON )
                {
                    // pan model.

                    _velocity += dt * accelerationFactor;
                }
                else if( buttonMask ==
                         GUIEventAdapter::MIDDLE_MOUSE_BUTTON ||
                         buttonMask ==
                         ( GUIEventAdapter::LEFT_MOUSE_BUTTON |
                           GUIEventAdapter::RIGHT_MOUSE_BUTTON ) )
                {

                    _velocity = 0.0;
                }
                else if( buttonMask == GUIEventAdapter::RIGHT_MOUSE_BUTTON )
                {

                    _velocity -= dt * accelerationFactor;
                }
                break;
            }
    }

    osg::CoordinateFrame cf              = getCoordinateFrame( _eye );

    osg::dmat4           rotation_matrix = osg::rotate( osg::dquat( _rotation ) );

    osg::dvec3           up              = osg::dvec3( 0.0, 1.0, 0.0 ) * rotation_matrix;
    osg::dvec3           lv = osg::dvec3( 0.0, 0.0, -1.0 ) * rotation_matrix;
    osg::dvec3           sv = osg::dvec3( 1.0, 0.0, 0.0 ) * rotation_matrix;

    // rotate the camera.
    double               dx  = _ga_t0->getXnormalized();

    double               yaw = -osg::radians( dx * 50.0 * dt );

#ifdef KEYBOARD_PITCH
    double pitch_delta = 0.5;
    if( _pitchUpKeyPressed )
    {
        _pitch += pitch_delta * dt;
    }
    if( _pitchDownKeyPressed )
    {
        _pitch -= pitch_delta * dt;
    }
#endif

#if defined( ABOSULTE_PITCH )
    // absolute pitch
    double dy = _ga_t0->getYnormalized();
    _pitch    = -dy * 0.5;
#elif defined( INCREMENTAL_PITCH )
    // incremental pitch
    double dy  = _ga_t0->getYnormalized();
    _pitch    += dy * dt;
#endif

    osg::quat yaw_rotation( static_cast<float>( yaw ),
                            osg::vec3( static_cast<float>( up.x ),
                                       static_cast<float>( up.y ),
                                       static_cast<float>( up.z ) ) );

    _rotation       = _rotation * yaw_rotation;

    rotation_matrix = osg::rotate( osg::dquat( _rotation ) );

    sv              = osg::dvec3( 1.0, 0.0, 0.0 ) * rotation_matrix;

    // movement is big enough the move the eye point along the look vector.
    if( fabs( _velocity * dt ) > 1E-8 )
    {
        double distanceToMove = _velocity * dt;

        double signedBuffer;
        if( distanceToMove >= 0.0 )
        {
            signedBuffer = _buffer;
        }
        else
        {
            signedBuffer = -_buffer;
        }

        // check to see if any obstruction in front.
        osg::dvec3 ip, np;
        if( intersect( _eye, _eye + lv * ( signedBuffer + distanceToMove ), ip, np ) )
        {
            if( distanceToMove >= 0.0 )
            {
                distanceToMove = osg::length( ip - _eye ) - _buffer;
            }
            else
            {
                distanceToMove = _buffer - osg::length( ip - _eye );
            }

            _velocity = 0.0;
        }

        // check to see if forward point is correct height above terrain.
        osg::dvec3 fp  = _eye + lv * distanceToMove;
        osg::dvec3 lfp = fp - up * ( _height * 5.0 );

        if( intersect( fp, lfp, ip, np ) )
        {
            if( osg::dot( up, np ) > 0.0 )
            {
                up = np;
            }
            else
            {
                up = -np;
            }

            _eye = ip + up * _height;

            lv   = osg::cross( up, sv );

            computePosition( _eye, _eye + lv, up );

            return true;
        }

        // no hit on the terrain found therefore resort to a fall under
        // under the influence of gravity.
        osg::dvec3 dp  = lfp;
        dp            -= getUpVector( cf ) * ( 2.0 * _modelScale );

        if( intersect( lfp, dp, ip, np ) )
        {

            if( osg::dot( up, np ) > 0.0 )
            {
                up = np;
            }
            else
            {
                up = -np;
            }

            _eye = ip + up * _height;

            lv   = osg::cross( up, sv );

            computePosition( _eye, _eye + lv, up );

            return true;
        }

        // no collision with terrain has been found therefore track horizontally.

        lv   *= ( _velocity * dt );

        _eye += lv;
    }

    return true;
}
