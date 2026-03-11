/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Stacked scale element. Contributes a scale matrix
 * to the composite bone/node transform.
 */
#include <osgAnimation/transform/StackedScaleElement.hpp>

#include <osg/maths/compat.hpp>
#include <osg/maths/transform.hpp>

using namespace osgAnimation;

StackedScaleElement::StackedScaleElement( const std::string& name,
                                          const osg::vec3&   scale ) :
    _scale( scale )
{
    setName( name );
}

StackedScaleElement::StackedScaleElement( const osg::vec3& scale ) :
    _scale( scale )
{
    setName( "scale" );
}

StackedScaleElement::StackedScaleElement( const StackedScaleElement& rhs,
                                          const osg::CopyOp& ) :
    Inherit( rhs ),
    _scale( rhs._scale )
{
    if( rhs._target.valid() )
    {
        _target = new Vec3Target( *rhs._target );
    }
}

const osg::vec3&
StackedScaleElement::getScale() const
{
    return _scale;
}

void
StackedScaleElement::setScale( const osg::vec3& scale )
{
    _scale = scale;
}

Target*
StackedScaleElement::getTarget()
{
    return _target.get();
}

const Target*
StackedScaleElement::getTarget() const
{
    return _target.get();
}

bool
StackedScaleElement::isIdentity() const
{
    return ( _scale.x == 1 && _scale.y == 1 && _scale.z == 1 );
}

osg::dmat4
StackedScaleElement::getAsMatrix() const
{
    return osg::dmat4( osg::scale( _scale ) );
}

void
StackedScaleElement::applyToMatrix( osg::dmat4& matrix ) const
{
    osg::preMultScale( matrix, osg::dvec3( _scale ) );
}

StackedScaleElement::StackedScaleElement()
{
    _scale = osg::vec3( 1, 1, 1 );
}

void
StackedScaleElement::update( float /*t*/ )
{
    if( _target.valid() )
    {
        _scale = _target->getValue();
    }
}

Target*
StackedScaleElement::getOrCreateTarget()
{
    if( !_target.valid() )
    {
        _target = new Vec3Target( _scale );
    }
    return _target.get();
}
