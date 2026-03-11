/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * TestManipulator example application
 */
#include "TestManipulator.hpp"

#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>

using namespace osg;
using namespace osgGA;

TestManipulator::TestManipulator()
{
    _modelScale       = 0.01F;
    _minimumZoomScale = 0.05F;
    _thrown           = false;

    _distance         = 1.0F;
}

TestManipulator::~TestManipulator()
{
}

void
TestManipulator::setNode( osg::Node* node )
{
    _node = node;
    if( _node.get() )
    {
        const osg::sphere& boundingSphere = _node->getBound();
        _modelScale                       = boundingSphere.radius;
    }
}

const osg::Node*
TestManipulator::getNode() const
{
    return _node.get();
}

osg::Node*
TestManipulator::getNode()
{
    return _node.get();
}

/*ea*/
void
TestManipulator::home( const GUIEventAdapter&,
                       GUIActionAdapter& us )
{
    if( _node.get() )
    {

        const osg::sphere& boundingSphere = _node->getBound();

        computePosition( boundingSphere.center + osg::vec3( 0.0F, 0.0F, 20.0F ),
                         osg::vec3( 0.0F, 1.0F, 0.0F ),
                         osg::vec3( 0.0F, 0.0F, 1.0F ) );

        us.requestRedraw();
    }
}

void
TestManipulator::init( const GUIEventAdapter&,
                       GUIActionAdapter& )
{
    flushMouseEventStack();
}

bool
TestManipulator::handle( const GUIEventAdapter& ea,
                         GUIActionAdapter&      us )
{
    switch( ea.getEventType() )
    {
        case( GUIEventAdapter::PUSH ) :
            {
                flushMouseEventStack();
                addMouseEvent( ea );
                if( calcMovement() )
                {
                    us.requestRedraw();
                }
                us.requestContinuousUpdate( false );
                _thrown = false;
                return true;
            }

        case( GUIEventAdapter::RELEASE ) :
            {
                if( ea.getButtonMask() == 0 )
                {

                    if( isMouseMoving() )
                    {
                        if( calcMovement() )
                        {
                            us.requestRedraw();
                            us.requestContinuousUpdate( true );
                            _thrown = true;
                        }
                    }
                    else
                    {
                        flushMouseEventStack();
                        addMouseEvent( ea );
                        if( calcMovement() )
                        {
                            us.requestRedraw();
                        }
                        us.requestContinuousUpdate( false );
                        _thrown = false;
                    }
                }
                else
                {
                    flushMouseEventStack();
                    addMouseEvent( ea );
                    if( calcMovement() )
                    {
                        us.requestRedraw();
                    }
                    us.requestContinuousUpdate( false );
                    _thrown = false;
                }
                return true;
            }

        case( GUIEventAdapter::DRAG ) :
            {
                addMouseEvent( ea );
                if( calcMovement() )
                {
                    us.requestRedraw();
                }
                us.requestContinuousUpdate( false );
                _thrown = false;
                return true;
            }

        case( GUIEventAdapter::MOVE ) :
            {
                return false;
            }

        case( GUIEventAdapter::KEYDOWN ) :
            if( ea.getKey() == ' ' )
            {
                flushMouseEventStack();
                _thrown = false;
                home( ea, us );
                us.requestRedraw();
                us.requestContinuousUpdate( false );
                return true;
            }
            return false;
        case( GUIEventAdapter::FRAME ) :
            if( _thrown )
            {
                if( calcMovement() )
                {
                    us.requestRedraw();
                }
                return true;
            }
            return false;
        default :
            return false;
    }
}

bool
TestManipulator::isMouseMoving()
{
    if( _ga_t0.get() == NULL || _ga_t1.get() == NULL )
    {
        return false;
    }

    static const float velocity = 0.1F;

    float              dx       = _ga_t0->getXnormalized() - _ga_t1->getXnormalized();
    float              dy       = _ga_t0->getYnormalized() - _ga_t1->getYnormalized();
    float              len      = sqrtf( dx * dx + dy * dy );
    float              dt       = _ga_t0->getTime() - _ga_t1->getTime();

    return ( len > dt * velocity );
}

void
TestManipulator::flushMouseEventStack()
{
    _ga_t1 = NULL;
    _ga_t0 = NULL;
}

void
TestManipulator::addMouseEvent( const GUIEventAdapter& ea )
{
    _ga_t1 = _ga_t0;
    _ga_t0 = &ea;
}

void
TestManipulator::setByMatrix( const osg::dmat4& matrix )
{
    _center   = osg::vec3( osg::getTrans( matrix ) );
    _rotation = osg::quat( osg::getRotate( matrix ) );
    _distance = 1.0F;
}

osg::dmat4
TestManipulator::getMatrix() const
{
    return osg::dmat4( osg::rotate( _rotation ) ) *
           osg::dmat4( osg::translate( _center ) );
}

osg::dmat4
TestManipulator::getInverseMatrix() const
{
    return osg::dmat4( osg::translate( -_center ) ) *
           osg::dmat4( osg::rotate( osg::inverse( _rotation ) ) );
}

void
TestManipulator::computePosition( const osg::vec3& eye,
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

    _center   = eye + lv;
    _distance = osg::length( lv );
    _rotation = osg::inverse( osg::quat( osg::getRotate( rotation_matrix ) ) );
}

bool
TestManipulator::calcMovement()
{

    // return if less then two events have been added.
    if( _ga_t0.get() == NULL || _ga_t1.get() == NULL )
    {
        return false;
    }

    float dx = _ga_t0->getXnormalized() - _ga_t1->getXnormalized();
    float dy = _ga_t0->getYnormalized() - _ga_t1->getYnormalized();

    // return if there is no movement.
    if( dx == 0 && dy == 0 )
    {
        return false;
    }

    unsigned int buttonMask = _ga_t1->getButtonMask();
    if( buttonMask == GUIEventAdapter::LEFT_MOUSE_BUTTON )
    {

        // rotate camera.

        osg::quat new_rotate;
        new_rotate = osg::quat( dx / 3.0F, osg::vec3( osg::vec3( 0.0F, 0.0F, 1.0F ) ) );

        _rotation  = _rotation * new_rotate;

        return true;
    }
    else if( buttonMask == GUIEventAdapter::MIDDLE_MOUSE_BUTTON )
    {

        // pan model.

        osg::vec3 dv  = osg::vec3( 0.0F, 0.0F, -500.0F ) * dy;

        _center      += dv;

        return true;
    }
    else if( buttonMask == GUIEventAdapter::RIGHT_MOUSE_BUTTON )
    {
        osg::dmat4 rotation_matrix = osg::dmat4( osg::rotate( _rotation ) );

        osg::vec3  uv  = osg::vec3( osg::vec3( 0.0F, 1.0F, 0.0F ) * rotation_matrix );
        osg::vec3  sv  = osg::vec3( osg::vec3( 1.0F, 0.0F, 0.0F ) * rotation_matrix );
        osg::vec3  fv  = uv ^ sv;
        osg::vec3  dv  = fv * ( dy * -500.0F ) - sv * ( dx * 500.0F );

        _center       += dv;

        return true;
    }

    return false;
}
