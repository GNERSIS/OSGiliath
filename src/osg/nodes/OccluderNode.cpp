/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Scene graph node holding an occluder for occlusion culling.
 * Placed in the scene to define occluding geometry.
 */
#include <osg/nodes/OccluderNode.hpp>

using namespace osg;

OccluderNode::OccluderNode()
{
}

OccluderNode::OccluderNode( const OccluderNode& node,
                            const CopyOp&       copyop ) :
    Inherit( node,
             copyop ),
    _occluder( dynamic_cast<ConvexPlanarOccluder*>( copyop( node._occluder.get() ) ) )
{
}

sphere
OccluderNode::computeBound() const
{
    sphere bsphere( Group::computeBound() );

    if( getOccluder() )
    {
        box                                    bb;
        const ConvexPlanarPolygon::VertexList& vertexList =
            getOccluder()->getOccluder().getVertexList();
        for( ConvexPlanarPolygon::VertexList::const_iterator itr = vertexList.begin();
             itr != vertexList.end();
             ++itr )
        {
            bb.expandBy( *itr );
        }
        if( bb.valid() )
        {
            bsphere.expandBy( bb );
        }
    }
    return bsphere;
}
