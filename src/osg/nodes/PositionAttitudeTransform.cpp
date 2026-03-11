/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Transform node using position, attitude (quaternion), scale, and pivot
 * point. Provides intuitive object placement without raw matrix math.
 */
#include <osg/nodes/PositionAttitudeTransform.hpp>

#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/maths/transform.hpp>

using namespace osg;

PositionAttitudeTransform::PositionAttitudeTransform() :
    _scale( 1.0,
            1.0,
            1.0 )
{
}

bool
PositionAttitudeTransform::computeLocalToWorldMatrix( dmat4& matrix,
                                                      NodeVisitor* ) const
{
    if( _referenceFrame == RELATIVE_RF )
    {
        // parent * T(pos) * R(att) * S(scl) * T(-pivot)
        matrix = matrix *
                 osg::translate( _position ) *
                 osg::rotate( dquat( _attitude ) ) *
                 osg::scale( _scale ) *
                 osg::translate( -_pivotPoint );
    }
    else    // absolute
    {
        matrix = osg::translate( _position ) *
                 osg::rotate( dquat( _attitude ) ) *
                 osg::scale( _scale ) *
                 osg::translate( -_pivotPoint );
    }
    return true;
}

bool
PositionAttitudeTransform::computeWorldToLocalMatrix( dmat4& matrix,
                                                      NodeVisitor* ) const
{
    if( _scale.x == 0.0 || _scale.y == 0.0 || _scale.z == 0.0 )
    {
        return false;
    }

    if( _referenceFrame == RELATIVE_RF )
    {
        // T(pivot) * S^-1 * R^-1 * T(-pos) * parent_inv
        matrix = osg::translate( _pivotPoint ) *
                 osg::scale( dvec3( 1.0 / _scale.x, 1.0 / _scale.y, 1.0 / _scale.z ) ) *
                 osg::rotate( osg::inverse( dquat( _attitude ) ) ) *
                 osg::translate( -_position ) *
                 matrix;
    }
    else    // absolute
    {
        matrix = osg::translate( _pivotPoint ) *
                 osg::scale( dvec3( 1.0 / _scale.x, 1.0 / _scale.y, 1.0 / _scale.z ) ) *
                 osg::rotate( osg::inverse( dquat( _attitude ) ) ) *
                 osg::translate( -_position );
    }
    return true;
}
