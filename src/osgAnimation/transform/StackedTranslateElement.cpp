/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Stacked translation element. Contributes a translation
 * matrix to the composite bone/node transform.
 */
#include <osgAnimation/transform/StackedTranslateElement.hpp>

#include <osg/maths/compat.hpp>
#include <osg/maths/transform.hpp>

using namespace osgAnimation;

StackedTranslateElement::StackedTranslateElement( const std::string& name,
                                                  const osg::vec3&   translate ) :
    _translate( translate )
{
    setName( name );
}

StackedTranslateElement::StackedTranslateElement( const osg::vec3& translate ) :
    _translate( translate )
{
    setName( "translate" );
}

StackedTranslateElement::StackedTranslateElement()
{
}

StackedTranslateElement::StackedTranslateElement( const StackedTranslateElement& rhs,
                                                  const osg::CopyOp& ) :
    Inherit( rhs ),
    _translate( rhs._translate )
{
    if( rhs._target.valid() )
    {
        _target = new Vec3Target( *rhs._target );
    }
}

void
StackedTranslateElement::applyToMatrix( osg::dmat4& matrix ) const
{
    osg::preMultTranslate( matrix, osg::dvec3( _translate ) );
}

osg::dmat4
StackedTranslateElement::getAsMatrix() const
{
    return osg::dmat4( osg::translate( _translate ) );
}

bool
StackedTranslateElement::isIdentity() const
{
    return ( _translate[0] == 0 && _translate[1] == 0 && _translate[2] == 0 );
}

const osg::vec3&
StackedTranslateElement::getTranslate() const
{
    return _translate;
}

void
StackedTranslateElement::setTranslate( const osg::vec3& value )
{
    _translate = value;
}

Target*
StackedTranslateElement::getOrCreateTarget()
{
    if( !_target.valid() )
    {
        _target = new Vec3Target( _translate );
    }
    return _target.get();
}

Target*
StackedTranslateElement::getTarget()
{
    return _target.get();
}

const Target*
StackedTranslateElement::getTarget() const
{
    return _target.get();
}

void
StackedTranslateElement::update( float /*t*/ )
{
    if( _target.valid() )
    {
        _translate = _target->getValue();
    }
}
