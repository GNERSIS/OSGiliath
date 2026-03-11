/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Visitor that collects all Bone nodes into a name-indexed map.
 * Used to resolve bone references during skeleton setup.
 */
#include <osgAnimation/skeletal/BoneMapVisitor.hpp>

#include <osgAnimation/skeletal/Skeleton.hpp>

using namespace osgAnimation;

BoneMapVisitor::BoneMapVisitor() :
    osg::DualModeVisitor( osg::DualModeVisitor::TRAVERSE_ALL_CHILDREN )
{
}

void
BoneMapVisitor::apply( osg::Node& )
{
    return;
}

void
BoneMapVisitor::apply( osg::Transform& node )
{
    Bone* bone = dynamic_cast<Bone*>( &node );
    if( bone )
    {
        _map[bone->getName()] = bone;
        traverse( node );
    }
    Skeleton* skeleton = dynamic_cast<Skeleton*>( &node );
    if( skeleton )
    {
        traverse( node );
    }
}

const BoneMap&
BoneMapVisitor::getBoneMap() const
{
    return _map;
}
