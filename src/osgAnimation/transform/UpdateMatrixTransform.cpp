/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Update callback applying blended animation channels to
 * a MatrixTransform's matrix during the update traversal.
 */
#include <osgAnimation/transform/UpdateMatrixTransform.hpp>

#include <osg/nodes/MatrixTransform.hpp>
#include <osg/traversal/NodeVisitor.hpp>

using namespace osgAnimation;

UpdateMatrixTransform::UpdateMatrixTransform( const UpdateMatrixTransform& apc,
                                              const osg::CopyOp&           copyop ) :
    Inherit( apc,
             copyop )
{
    _transforms = StackedTransform( apc.getStackedTransforms(), copyop );
}

UpdateMatrixTransform::UpdateMatrixTransform( const std::string& name ) :
    Inherit( name )
{
}

/** Callback method called by the NodeVisitor when visiting a node.*/
void
UpdateMatrixTransform::operator()( osg::Node*        node,
                                   osg::NodeVisitor* nv )
{
    if( nv && nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR )
    {
        osg::MatrixTransform* matrixTransform =
            dynamic_cast<osg::MatrixTransform*>( node );
        if( matrixTransform )
        {
            // here we would prefer to have a flag inside transform stack in order to
            // avoid update and a dirty state in matrixTransform if it's not require.
            _transforms.update();
            const osg::dmat4& matrix = _transforms.getMatrix();
            matrixTransform->setMatrix( matrix );
        }
    }
    traverse( node, nv );
}

bool
UpdateMatrixTransform::link( osgAnimation::Channel* channel )
{
    const std::string& channelName = channel->getName();

    // check if we can link a StackedTransformElement to the current Channel
    for( StackedTransform::iterator it = _transforms.begin(); it != _transforms.end();
         ++it )
    {
        StackedTransformElement* element = it->get();
        if( element && !element->getName().empty() && channelName == element->getName() )
        {
            Target* target = element->getOrCreateTarget();
            if( target && channel->setTarget( target ) )
            {
                return true;
            }
        }
    }

    OSG_INFO << "UpdateMatrixTransform::link Channel " << channel->getName()
             << " does not contain a symbolic name that can be linked to a "
                "StackedTransformElement."
             << std::endl;

    return false;
}
