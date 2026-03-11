/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * GliderManipulator example application
 */
#include "GliderManipulator.hpp"

#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <stdlib.h>

using namespace osg;
using namespace osgGA;

GliderManipulator::GliderManipulator()
{
    _modelScale = 0.01F;
    _velocity   = 0.2F;
    _yawMode    = YAW_AUTOMATICALLY_WHEN_BANKED;

    _distance   = 1.0F;
}

GliderManipulator::~GliderManipulator()
{
}

void
GliderManipulator::setNode( osg::Node* node )
{
    _node = node;
    if( _node.get() )
    {
        const osg::sphere& boundingSphere = _node->getBound();
        _modelScale                       = boundingSphere.radius;
    }
}

const osg::Node*
GliderManipulator::getNode() const
{
    return _node.get();
}

osg::Node*
GliderManipulator::getNode()
{
    return _node.get();
}

void
GliderManipulator::home( const GUIEventAdapter& ea,
                         GUIActionAdapter&      us )
{
    if( _node.get() )
    {

        const osg::sphere& boundingSphere = _node->getBound();

        osg::vec3          eye =
            boundingSphere.center + osg::vec3( -boundingSphere.radius * 0.25F,
                                               -boundingSphere.radius * 0.25F,
                                               -boundingSphere.radius * 0.03F );

        computePosition( eye,
                         osg::vec3( 1.0F, 1.0F, -0.1F ),
                         osg::vec3( 0.0F, 0.0F, 1.0F ) );

        _velocity = boundingSphere.radius * 0.01F;

        us.requestRedraw();

        us.requestWarpPointer( ( ea.getXmin() + ea.getXmax() ) / 2.0F,
                               ( ea.getYmin() + ea.getYmax() ) / 2.0F );

        flushMouseEventStack();
    }
}

void
GliderManipulator::init( const GUIEventAdapter& ea,
                         GUIActionAdapter&      us )
{
    flushMouseEventStack();

    us.requestContinuousUpdate( false );

    _velocity = 0.2F;

    if( ea.getEventType() != GUIEventAdapter::RESIZE )
    {
        us.requestWarpPointer( ( ea.getXmin() + ea.getXmax() ) / 2.0F,
                               ( ea.getYmin() + ea.getYmax() ) / 2.0F );
    }
}

bool
GliderManipulator::handle( const GUIEventAdapter& ea,
                           GUIActionAdapter&      us )
{
    switch( ea.getEventType() )
    {
#if 0   
        case(GUIEventAdapter::PUSH):
        {

            addMouseEvent(ea);
            us.requestContinuousUpdate(true);
            if (calcMovement()) us.requestRedraw();
            return true;
        }

        case(GUIEventAdapter::RELEASE):
        {

            addMouseEvent(ea);
            us.requestContinuousUpdate(true);
            // if (calcMovement()) us.requestRedraw();
            return true;
        }

        case(GUIEventAdapter::DRAG):
        {

            addMouseEvent(ea);
            us.requestContinuousUpdate(true);
            // if (calcMovement()) us.requestRedraw();
            return true;
        }

        case(GUIEventAdapter::MOVE):
        {

            addMouseEvent(ea);
            us.requestContinuousUpdate(true);
            // if (calcMovement()) us.requestRedraw();

            return true;
        }
#endif
        case( GUIEventAdapter::KEYDOWN ) :
            if( ea.getKey() == ' ' )
            {
                flushMouseEventStack();
                home( ea, us );
                us.requestRedraw();
                us.requestContinuousUpdate( false );
                return true;
            }
            else if( ea.getKey() == 'q' )
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

        case( GUIEventAdapter::FRAME ) :
            addMouseEvent( ea );
            if( calcMovement() )
            {
                us.requestRedraw();
            }
            return true;

        case( GUIEventAdapter::RESIZE ) :
            init( ea, us );
            us.requestRedraw();
            return true;

        default :
            return false;
    }
}

void
GliderManipulator::getUsage( osg::ApplicationUsage& usage ) const
{
    usage.addKeyboardMouseBinding( "Flight: Space",
                                   "Reset the viewing position to home" );
    usage.addKeyboardMouseBinding( "Flight: q",
                                   "Automatically yaw when banked (default)" );
    usage.addKeyboardMouseBinding( "Flight: a", "No yaw when banked" );
}

void
GliderManipulator::flushMouseEventStack()
{
    _ga_t1 = NULL;
    _ga_t0 = NULL;
}

void
GliderManipulator::addMouseEvent( const GUIEventAdapter& ea )
{
    _ga_t1 = _ga_t0;
    _ga_t0 = &ea;
}

void
GliderManipulator::setByMatrix( const osg::dmat4& matrix )
{
    _eye      = osg::vec3( osg::getTrans( matrix ) );
    _rotation = osg::quat( osg::getRotate( matrix ) );
    _distance = 1.0F;
}

osg::dmat4
GliderManipulator::getMatrix() const
{
    return osg::dmat4( osg::rotate( _rotation ) ) * osg::dmat4( osg::translate( _eye ) );
}

osg::dmat4
GliderManipulator::getInverseMatrix() const
{
    return osg::dmat4( osg::translate( -_eye ) ) *
           osg::dmat4( osg::rotate( osg::inverse( _rotation ) ) );
}

void
GliderManipulator::computePosition( const osg::vec3& eye,
                                    const osg::vec3& lv,
                                    const osg::vec3& up )
{
    osg::vec3 f( lv );
    f = osg::normalize( f );
    osg::vec3 s( f ^ up );
    s = osg::normalize( s );
    osg::vec3 u( s ^ f );
    u = osg::normalize( u );

    osg::dmat4 rotation_matrix( s[0],
                                u[0],
                                -f[0],
                                0.0F,
                                s[1],
                                u[1],
                                -f[1],
                                0.0F,
                                s[2],
                                u[2],
                                -f[2],
                                0.0F,
                                0.0F,
                                0.0F,
                                0.0F,
                                1.0F );

    _eye      = eye;
    _distance = osg::length( lv );
    _rotation = osg::inverse( osg::quat( osg::getRotate( rotation_matrix ) ) );
}

bool
GliderManipulator::calcMovement()
{
    //_camera->setFusionDistanceMode(osg::Camera::PROPORTIONAL_TO_SCREEN_DISTANCE);

    // return if less then two events have been added.
    if( _ga_t0.get() == NULL || _ga_t1.get() == NULL )
    {
        return false;
    }

    double dt = _ga_t0->getTime() - _ga_t1->getTime();

    if( dt < 0.0F )
    {
        notify( INFO ) << "warning dt = " << dt << std::endl;
        dt = 0.0F;
    }

    unsigned int buttonMask = _ga_t1->getButtonMask();
    if( buttonMask == GUIEventAdapter::LEFT_MOUSE_BUTTON )
    {
        // pan model.

        _velocity += dt * _modelScale * 0.05F;
    }
    else if( buttonMask ==
             GUIEventAdapter::MIDDLE_MOUSE_BUTTON ||
             buttonMask ==
             ( GUIEventAdapter::LEFT_MOUSE_BUTTON |
               GUIEventAdapter::RIGHT_MOUSE_BUTTON ) )
    {

        _velocity = 0.0F;
    }
    else if( buttonMask == GUIEventAdapter::RIGHT_MOUSE_BUTTON )
    {

        _velocity -= dt * _modelScale * 0.05F;
    }

    float dx = _ga_t0->getXnormalized();
    float dy = _ga_t0->getYnormalized();

    // osg::notify(osg::NOTICE)<<"dx = "<<dx<<" dy = "<<dy<<"dt = "<<dt<<std::endl;

    // mew - flag to reverse mouse-control mapping
    if( getenv( "OSGHANGGLIDE_REVERSE_CONTROLS" ) )
    {
        dx = -dx;
        dy = -dy;
    }

    osg::dmat4 rotation_matrix;
    rotation_matrix = osg::dmat4( osg::rotate( _rotation ) );

    osg::vec3 up    = osg::vec3( osg::vec3( 0.0F, 1.0F, 0.0F ) * rotation_matrix );
    osg::vec3 lv    = osg::vec3( osg::vec3( 0.0F, 0.0F, -1.0F ) * rotation_matrix );

    osg::vec3 sv    = lv ^ up;
    sv              = osg::normalize( sv );

    float     pitch = -inDegrees( dy * 75.0F * dt );
    float     roll  = inDegrees( dx * 50.0F * dt );

    osg::quat delta_rotate;

    osg::quat roll_rotate;
    osg::quat pitch_rotate;

    pitch_rotate = osg::quat( pitch, osg::vec3( sv.x, sv.y, sv.z ) );
    roll_rotate  = osg::quat( roll, osg::vec3( lv.x, lv.y, lv.z ) );

    delta_rotate = pitch_rotate * roll_rotate;

    if( _yawMode == YAW_AUTOMATICALLY_WHEN_BANKED )
    {
        float     bank = asinf( sv.z );
        float     yaw  = inRadians( bank ) * dt;

        osg::quat yaw_rotate;
        yaw_rotate   = osg::quat( yaw, osg::vec3( 0.0F, 0.0F, 1.0F ) );

        delta_rotate = delta_rotate * yaw_rotate;
    }

    lv        *= ( _velocity * dt );

    _eye      += lv;
    _rotation  = _rotation * delta_rotate;

    return true;
}
