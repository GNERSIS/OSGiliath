/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Root node of a bone hierarchy. Manages the skeleton's update
 * traversal for computing bone world matrices.
 */
#include <osgAnimation/skeletal/Skeleton.hpp>

#include <osg/core/Notify.hpp>
#include <osgAnimation/skeletal/Bone.hpp>

using namespace osgAnimation;

Skeleton::Skeleton()
{
}

Skeleton::Skeleton( const Skeleton&    b,
                    const osg::CopyOp& copyop ) :
    Inherit( b,
             copyop )
{
}

Skeleton::UpdateSkeleton::UpdateSkeleton() :
    _needValidate( true )
{
}

Skeleton::UpdateSkeleton::UpdateSkeleton( const UpdateSkeleton& us,
                                          const osg::CopyOp&    copyop ) :
    Inherit( us,
             copyop )
{
    _needValidate = true;
}

bool
Skeleton::UpdateSkeleton::needToValidate() const
{
    return _needValidate;
}

class ValidateSkeletonVisitor : public osg::DualModeVisitor
{
    public:

        ValidateSkeletonVisitor() :
            osg::DualModeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
        {
        }

        using osg::DualModeVisitor::apply;

        void
        apply( osg::Node& /*node*/ )
        {
            return;
        }

        void
        apply( osg::Transform& node )
        {
            // the idea is to traverse the skeleton or bone but to stop if other node is
            // found
            Bone* bone = dynamic_cast<Bone*>( &node );
            if( !bone )
            {
                return;
            }

            bool foundNonBone = false;

            for( unsigned int i = 0; i < bone->getNumChildren(); ++i )
            {
                if( dynamic_cast<Bone*>( bone->getChild( i ) ) )
                {
                    if( foundNonBone )
                    {
                        OSG_WARN
                            << "Warning: a Bone was found after a non-Bone child "
                               "within a Skeleton. Children of a Bone must be ordered "
                               "with all child Bones first for correct update order."
                            << std::endl;
                        setTraversalMode( TRAVERSE_NONE );
                        return;
                    }
                }
                else
                {
                    foundNonBone = true;
                }
            }
            traverse( node );
        }
};

void
Skeleton::UpdateSkeleton::operator()( osg::Node*        node,
                                      osg::NodeVisitor* nv )
{
    if( nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR )
    {
        Skeleton* skeleton = dynamic_cast<Skeleton*>( node );
        if( _needValidate && skeleton )
        {
            ValidateSkeletonVisitor visitor;
            for( unsigned int i = 0; i < skeleton->getNumChildren(); ++i )
            {
                osg::Node* child = skeleton->getChild( i );
                child->accept( visitor );
            }
            _needValidate = false;
        }
    }
    traverse( node, nv );
}

void
Skeleton::setDefaultUpdateCallback()
{
    setUpdateCallback( new Skeleton::UpdateSkeleton );
}
