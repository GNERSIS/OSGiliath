/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Stacked quaternion rotation element. Contributes a quaternion
 * rotation to the composite bone/node transform.
 */
#include <osgAnimation/transform/StackedQuaternionElement.hpp>

#include <osg/maths/compat.hpp>
#include <osg/maths/transform.hpp>

using namespace osgAnimation;

StackedQuaternionElement::StackedQuaternionElement( const std::string& name,
                                                    const osg::quat&   quaternion ) :
    _quaternion( quaternion )
{
    setName( name );
}

StackedQuaternionElement::StackedQuaternionElement( const StackedQuaternionElement& rhs,
                                                    const osg::CopyOp& ) :
    Inherit( rhs ),
    _quaternion( rhs._quaternion )
{
    if( rhs._target.valid() )
    {
        _target = new QuatTarget( *rhs._target );
    }
}

StackedQuaternionElement::StackedQuaternionElement( const osg::quat& quat ) :
    _quaternion( quat )
{
    setName( "quaternion" );
}

StackedQuaternionElement::StackedQuaternionElement()
{
}

const osg::quat&
StackedQuaternionElement::getQuaternion() const
{
    return _quaternion;
}

void
StackedQuaternionElement::setQuaternion( const osg::quat& q )
{
    _quaternion = q;
}

void
StackedQuaternionElement::applyToMatrix( osg::dmat4& matrix ) const
{
    osg::preMultRotate( matrix, osg::dquat( _quaternion ) );
}

osg::dmat4
StackedQuaternionElement::getAsMatrix() const
{
    return osg::rotate( osg::dquat( _quaternion ) );
}

bool
StackedQuaternionElement::isIdentity() const
{
    return ( _quaternion[0] ==
             0 &&
             _quaternion[1] ==
             0 &&
             _quaternion[2] ==
             0 &&
             _quaternion[3] == 1.0 );
}

void
StackedQuaternionElement::update( float /*t*/ )
{
    if( _target.valid() )
    {
        _quaternion = _target->getValue();
    }
}

Target*
StackedQuaternionElement::getOrCreateTarget()
{
    if( !_target.valid() )
    {
        _target = new QuatTarget( _quaternion );
    }
    return _target.get();
}

Target*
StackedQuaternionElement::getTarget()
{
    return _target.get();
}

const Target*
StackedQuaternionElement::getTarget() const
{
    return _target.get();
}
