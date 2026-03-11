/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Skeleton bone node carrying bind-pose and inverse-bind matrices.
 * Forms the bone hierarchy for skeletal animation.
 */
#include <osgAnimation/skeletal/Bone.hpp>

#include <osgAnimation/skeletal/Skeleton.hpp>
#include <osgAnimation/skeletal/UpdateBone.hpp>

using namespace osgAnimation;

Bone::Bone( const Bone&        b,
            const osg::CopyOp& copyop ) :
    Inherit( b,
             copyop ),
    _invBindInSkeletonSpace( b._invBindInSkeletonSpace ),
    _boneInSkeletonSpace( b._boneInSkeletonSpace )
{
}

Bone::Bone( const std::string& name )
{
    if( !name.empty() )
    {
        setName( name );
    }
}

void
Bone::setDefaultUpdateCallback( const std::string& name )
{
    std::string cbName = name;
    if( cbName.empty() )
    {
        cbName = getName();
    }
    setUpdateCallback( new UpdateBone( cbName ) );
}

Bone*
Bone::getBoneParent()
{
    if( getParents().empty() )
    {
        return 0;
    }
    osg::Node::ParentList parents = getParents();
    for( osg::Node::ParentList::iterator it = parents.begin(); it != parents.end();
         ++it )
    {
        Bone* pb = dynamic_cast<Bone*>( *it );
        if( pb )
        {
            return pb;
        }
    }
    return 0;
}

const Bone*
Bone::getBoneParent() const
{
    if( getParents().empty() )
    {
        return 0;
    }
    const osg::Node::ParentList& parents = getParents();
    for( osg::Node::ParentList::const_iterator it = parents.begin(); it != parents.end();
         ++it )
    {
        const Bone* pb = dynamic_cast<const Bone*>( *it );
        if( pb )
        {
            return pb;
        }
    }
    return 0;
}
