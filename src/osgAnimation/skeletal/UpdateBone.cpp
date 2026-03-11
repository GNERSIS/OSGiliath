/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Update callback applying blended animation channels to
 * a Bone's local transform during the update traversal.
 */
#include <osgAnimation/skeletal/UpdateBone.hpp>

#include <osg/traversal/NodeVisitor.hpp>
#include <osgAnimation/skeletal/Bone.hpp>

using namespace osgAnimation;

UpdateBone::UpdateBone( const std::string& name ) :
    Inherit( name )
{
}

UpdateBone::UpdateBone( const UpdateBone&  apc,
                        const osg::CopyOp& copyop ) :
    Inherit( apc,
             copyop )
{
}

/** Callback method called by the NodeVisitor when visiting a node.*/
void
UpdateBone::operator()( osg::Node*        node,
                        osg::NodeVisitor* nv )
{
    if( nv && nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR )
    {
        Bone* b = dynamic_cast<Bone*>( node );
        if( !b )
        {
            OSG_WARN << "Warning: UpdateBone set on non-Bone object." << std::endl;
            return;
        }

        // here we would prefer to have a flag inside transform stack in order to avoid
        // update and a dirty state in matrixTransform if it's not require.
        _transforms.update();
        const osg::dmat4& matrix = _transforms.getMatrix();
        b->setMatrix( matrix );

        Bone* parent = b->getBoneParent();
        if( parent )
        {
            b->setMatrixInSkeletonSpace( parent->getMatrixInSkeletonSpace() * matrix );
        }
        else
        {
            b->setMatrixInSkeletonSpace( matrix );
        }
    }
    traverse( node, nv );
}
