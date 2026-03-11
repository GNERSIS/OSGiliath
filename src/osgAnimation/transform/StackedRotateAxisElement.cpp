/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Stacked axis-angle rotation element. Contributes a rotation
 * around a fixed axis to the composite transform.
 */
#include <osgAnimation/transform/StackedRotateAxisElement.hpp>

#include <osg/maths/compat.hpp>
#include <osg/maths/transform.hpp>

using namespace osgAnimation;

StackedRotateAxisElement::StackedRotateAxisElement( const std::string& name,
                                                    const osg::vec3&   axis,
                                                    double             angle ) :
    _axis( axis ),
    _angle( angle )
{
    setName( name );
}

StackedRotateAxisElement::StackedRotateAxisElement( const osg::vec3& axis,
                                                    double           angle ) :
    _axis( axis ),
    _angle( angle )
{
    setName( "rotateaxis" );
}

StackedRotateAxisElement::StackedRotateAxisElement() :
    _axis( osg::vec3( 1,
                      0,
                      0 ) ),
    _angle( 0 )
{
}

StackedRotateAxisElement::StackedRotateAxisElement( const StackedRotateAxisElement& rhs,
                                                    const osg::CopyOp& ) :
    Inherit( rhs ),
    _axis( rhs._axis ),
    _angle( rhs._angle )
{
    if( rhs._target.valid() )
    {
        _target = new FloatTarget( *rhs._target );
    }
}

osg::dmat4
StackedRotateAxisElement::getAsMatrix() const
{
    return osg::dmat4( osg::rotate( osg::quat( static_cast<float>( _angle ), _axis ) ) );
}

void
StackedRotateAxisElement::update( float /*t*/ )
{
    if( _target.valid() )
    {
        _angle = _target->getValue();
    }
}

const osg::vec3&
StackedRotateAxisElement::getAxis() const
{
    return _axis;
}

double
StackedRotateAxisElement::getAngle() const
{
    return _angle;
}

void
StackedRotateAxisElement::setAxis( const osg::vec3& axis )
{
    _axis = axis;
}

void
StackedRotateAxisElement::setAngle( double angle )
{
    _angle = angle;
}

Target*
StackedRotateAxisElement::getOrCreateTarget()
{
    if( !_target.valid() )
    {
        _target = new FloatTarget( static_cast<float>( _angle ) );
    }
    return _target.get();
}

void
StackedRotateAxisElement::applyToMatrix( osg::dmat4& matrix ) const
{
    osg::preMultRotate( matrix,
                        osg::dquat( osg::quat( static_cast<float>( _angle ), _axis ) ) );
}
