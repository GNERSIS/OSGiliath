/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Stacked arbitrary matrix element. Contributes a raw 4x4
 * matrix to the composite bone/node transform.
 */
#include <osgAnimation/transform/StackedMatrixElement.hpp>

using namespace osgAnimation;

StackedMatrixElement::StackedMatrixElement()
{
}

StackedMatrixElement::StackedMatrixElement( const std::string& name,
                                            const osg::dmat4&  matrix ) :
    _matrix( matrix )
{
    setName( name );
}

StackedMatrixElement::StackedMatrixElement( const osg::dmat4& matrix ) :
    _matrix( matrix )
{
    setName( "matrix" );
}

StackedMatrixElement::StackedMatrixElement( const StackedMatrixElement& rhs,
                                            const osg::CopyOp&          c ) :
    Inherit( rhs,
             c ),
    _matrix( rhs._matrix )
{
    if( rhs._target.valid() )
    {
        _target = new MatrixTarget( *rhs._target );
    }
}

Target*
StackedMatrixElement::getOrCreateTarget()
{
    if( !_target.valid() )
    {
        _target = new MatrixTarget( osg::mat4( _matrix ) );
    }
    return _target.get();
}

void
StackedMatrixElement::update( float /*t*/ )
{
    if( _target.valid() )
    {
        _matrix = _target->getValue();
    }
}
