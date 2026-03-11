/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Transform node that positions children using an explicit 4x4 matrix.
 * Used for static transforms, animation callbacks, and scene positioning.
 */
#include <osg/nodes/MatrixTransform.hpp>

#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/maths/transform.hpp>

using namespace osg;

MatrixTransform::MatrixTransform() :
    _inverseDirty( false )
{
}

MatrixTransform::MatrixTransform( const MatrixTransform& transform,
                                  const CopyOp&          copyop ) :
    Inherit<Transform,
            MatrixTransform>( transform,
                              copyop ),
    _matrix( transform._matrix ),
    _inverse( transform._inverse ),
    _inverseDirty( transform._inverseDirty )
{
}

MatrixTransform::MatrixTransform( const dmat4& mat )
{
    _referenceFrame = RELATIVE_RF;

    _matrix         = mat;
    _inverseDirty   = true;
}

MatrixTransform::~MatrixTransform()
{
}

bool
MatrixTransform::computeLocalToWorldMatrix( dmat4& matrix,
                                            NodeVisitor* ) const
{
    if( _referenceFrame == RELATIVE_RF )
    {
        matrix = matrix * _matrix;
    }
    else    // absolute
    {
        matrix = _matrix;
    }
    return true;
}

bool
MatrixTransform::computeWorldToLocalMatrix( dmat4& matrix,
                                            NodeVisitor* ) const
{
    const dmat4& inverse = getInverseMatrix();

    if( _referenceFrame == RELATIVE_RF )
    {
        matrix = inverse * matrix;
    }
    else    // absolute
    {
        matrix = inverse;
    }
    return true;
}
