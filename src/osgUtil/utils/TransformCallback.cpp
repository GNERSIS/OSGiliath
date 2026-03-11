/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Update callback that applies an AnimationPath transform.
 * Simpler alternative to AnimationPathCallback for nodes.
 */
#include <osgUtil/utils/TransformCallback.hpp>

#include <osg/maths/transform.hpp>
#include <osg/nodes/MatrixTransform.hpp>

using namespace osgUtil;

TransformCallback::TransformCallback( const osg::vec3& pivot,
                                      const osg::vec3& axis,
                                      float            angularVelocity )
{
    _pivot                   = pivot;
    _axis                    = axis;
    _angular_velocity        = angularVelocity;

    _previousTraversalNumber = osg::UNINITIALIZED_FRAME_NUMBER;
    _previousTime            = -1.0;

    _pause                   = false;
}

void
TransformCallback::operator()( osg::Node*        node,
                               osg::NodeVisitor* nv )
{
    osg::MatrixTransform* transform = dynamic_cast<osg::MatrixTransform*>( node );
    if( nv && transform )
    {

        const osg::FrameStamp* fs = nv->getFrameStamp();
        if( !fs )
        {
            return;    // not frame stamp, no handle on the time so can't move.
        }

        double newTime = fs->getSimulationTime();

        // ensure that we do not operate on this node more than
        // once during this traversal.  This is an issue since node
        // can be shared between multiple parents.
        if( !_pause && nv->getTraversalNumber() != _previousTraversalNumber )
        {
            float delta_angle =
                static_cast<float>( _angular_velocity * ( newTime - _previousTime ) );

            osg::dmat4 mat =
                osg::translate( osg::dvec3( -_pivot ) ) *
                osg::rotate( static_cast<double>( delta_angle ), osg::dvec3( _axis ) ) *
                osg::translate( osg::dvec3( _pivot ) );

            // update the specified transform
            transform->preMult( mat );

            _previousTraversalNumber = nv->getTraversalNumber();
        }

        _previousTime = newTime;
    }

    // must call any nested node callbacks and continue subgraph traversal.
    traverse( node, nv );
}
