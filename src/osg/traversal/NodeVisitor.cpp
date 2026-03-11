/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Base class for scene graph traversals. Implements the visitor
 * pattern with pre/post apply callbacks and traversal mode control.
 */
#include <osg/traversal/NodeVisitor.hpp>

#include <osg/geometry/Geometry.hpp>
#include <osg/lighting/LightSource.hpp>
#include <osg/nodes/AutoTransform.hpp>
#include <osg/nodes/Billboard.hpp>
#include <osg/nodes/Camera.hpp>
#include <osg/nodes/CameraView.hpp>
#include <osg/nodes/ClearNode.hpp>
#include <osg/nodes/CoordinateSystemNode.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/LOD.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/nodes/OccluderNode.hpp>
#include <osg/nodes/PagedLOD.hpp>
#include <osg/nodes/PositionAttitudeTransform.hpp>
#include <osg/nodes/Projection.hpp>
#include <osg/nodes/ProxyNode.hpp>
#include <osg/nodes/Sequence.hpp>
#include <osg/nodes/Switch.hpp>
#include <osg/nodes/Transform.hpp>
#include <osg/rendering/OcclusionQueryNode.hpp>
#include <stdlib.h>

using namespace osg;

// ---------------------------------------------------------------------------
// NodeVisitor
// ---------------------------------------------------------------------------

NodeVisitor::NodeVisitor( TraversalMode tm ) :
    Object( true ),
    NodeVisitorBase<Node*>( tm )
{
}

NodeVisitor::NodeVisitor( VisitorType   type,
                          TraversalMode tm ) :
    Object( true ),
    NodeVisitorBase<Node*>( type,
                            tm )
{
}

NodeVisitor::NodeVisitor( const NodeVisitor& nv,
                          const osg::CopyOp& copyop ) :
    Object( nv,
            copyop ),
    NodeVisitorBase<Node*>( nv )
{
}

NodeVisitor::~NodeVisitor()
{
}

void
NodeVisitor::apply( Node& node )
{
    traverse( node );
}

void
NodeVisitor::apply( Drawable& drawable )
{
    apply( static_cast<Node&>( drawable ) );
}

void
NodeVisitor::apply( Geometry& drawable )
{
    apply( static_cast<Drawable&>( drawable ) );
}

void
NodeVisitor::apply( Geode& node )
{
    apply( static_cast<Group&>( node ) );
}

void
NodeVisitor::apply( Billboard& node )
{
    apply( static_cast<Geode&>( node ) );
}

void
NodeVisitor::apply( Group& node )
{
    apply( static_cast<Node&>( node ) );
}

void
NodeVisitor::apply( ProxyNode& node )
{
    apply( static_cast<Group&>( node ) );
}

void
NodeVisitor::apply( Projection& node )
{
    apply( static_cast<Group&>( node ) );
}

void
NodeVisitor::apply( CoordinateSystemNode& node )
{
    apply( static_cast<Group&>( node ) );
}

void
NodeVisitor::apply( LightSource& node )
{
    apply( static_cast<Group&>( node ) );
}

void
NodeVisitor::apply( Transform& node )
{
    apply( static_cast<Group&>( node ) );
}

void
NodeVisitor::apply( Camera& node )
{
    apply( static_cast<Transform&>( node ) );
}

void
NodeVisitor::apply( CameraView& node )
{
    apply( static_cast<Transform&>( node ) );
}

void
NodeVisitor::apply( MatrixTransform& node )
{
    apply( static_cast<Transform&>( node ) );
}

void
NodeVisitor::apply( PositionAttitudeTransform& node )
{
    apply( static_cast<Transform&>( node ) );
}

void
NodeVisitor::apply( AutoTransform& node )
{
    apply( static_cast<Transform&>( node ) );
}

void
NodeVisitor::apply( Switch& node )
{
    apply( static_cast<Group&>( node ) );
}

void
NodeVisitor::apply( Sequence& node )
{
    apply( static_cast<Group&>( node ) );
}

void
NodeVisitor::apply( LOD& node )
{
    apply( static_cast<Group&>( node ) );
}

void
NodeVisitor::apply( PagedLOD& node )
{
    apply( static_cast<LOD&>( node ) );
}

void
NodeVisitor::apply( ClearNode& node )
{
    apply( static_cast<Group&>( node ) );
}

void
NodeVisitor::apply( OccluderNode& node )
{
    apply( static_cast<Group&>( node ) );
}

void
NodeVisitor::apply( OcclusionQueryNode& node )
{
    apply( static_cast<Group&>( node ) );
}

// ---------------------------------------------------------------------------
// ConstNodeVisitor
// ---------------------------------------------------------------------------

ConstNodeVisitor::ConstNodeVisitor( TraversalMode tm ) :
    Object( true ),
    NodeVisitorBase<const Node*>( tm )
{
}

ConstNodeVisitor::ConstNodeVisitor( VisitorType   type,
                                    TraversalMode tm ) :
    Object( true ),
    NodeVisitorBase<const Node*>( type,
                                  tm )
{
}

ConstNodeVisitor::ConstNodeVisitor( const ConstNodeVisitor& nv,
                                    const osg::CopyOp&      copyop ) :
    Object( nv,
            copyop ),
    NodeVisitorBase<const Node*>( nv )
{
}

ConstNodeVisitor::~ConstNodeVisitor()
{
}

void
ConstNodeVisitor::traverse( const Node& node )
{
    if( _traversalMode == TRAVERSE_PARENTS )
    {
        node.ascend( *this );
    }
    else if( _traversalMode != TRAVERSE_NONE )
    {
        node.traverse( *this );
    }
}

void
ConstNodeVisitor::apply( const Node& node )
{
    traverse( node );
}

void
ConstNodeVisitor::apply( const Drawable& drawable )
{
    apply( static_cast<const Node&>( drawable ) );
}

void
ConstNodeVisitor::apply( const Geometry& drawable )
{
    apply( static_cast<const Drawable&>( drawable ) );
}

void
ConstNodeVisitor::apply( const Geode& node )
{
    apply( static_cast<const Group&>( node ) );
}

void
ConstNodeVisitor::apply( const Billboard& node )
{
    apply( static_cast<const Geode&>( node ) );
}

void
ConstNodeVisitor::apply( const Group& node )
{
    apply( static_cast<const Node&>( node ) );
}

void
ConstNodeVisitor::apply( const ProxyNode& node )
{
    apply( static_cast<const Group&>( node ) );
}

void
ConstNodeVisitor::apply( const Projection& node )
{
    apply( static_cast<const Group&>( node ) );
}

void
ConstNodeVisitor::apply( const CoordinateSystemNode& node )
{
    apply( static_cast<const Group&>( node ) );
}

void
ConstNodeVisitor::apply( const LightSource& node )
{
    apply( static_cast<const Group&>( node ) );
}

void
ConstNodeVisitor::apply( const Transform& node )
{
    apply( static_cast<const Group&>( node ) );
}

void
ConstNodeVisitor::apply( const Camera& node )
{
    apply( static_cast<const Transform&>( node ) );
}

void
ConstNodeVisitor::apply( const CameraView& node )
{
    apply( static_cast<const Transform&>( node ) );
}

void
ConstNodeVisitor::apply( const MatrixTransform& node )
{
    apply( static_cast<const Transform&>( node ) );
}

void
ConstNodeVisitor::apply( const PositionAttitudeTransform& node )
{
    apply( static_cast<const Transform&>( node ) );
}

void
ConstNodeVisitor::apply( const AutoTransform& node )
{
    apply( static_cast<const Transform&>( node ) );
}

void
ConstNodeVisitor::apply( const Switch& node )
{
    apply( static_cast<const Group&>( node ) );
}

void
ConstNodeVisitor::apply( const Sequence& node )
{
    apply( static_cast<const Group&>( node ) );
}

void
ConstNodeVisitor::apply( const LOD& node )
{
    apply( static_cast<const Group&>( node ) );
}

void
ConstNodeVisitor::apply( const PagedLOD& node )
{
    apply( static_cast<const LOD&>( node ) );
}

void
ConstNodeVisitor::apply( const ClearNode& node )
{
    apply( static_cast<const Group&>( node ) );
}

void
ConstNodeVisitor::apply( const OccluderNode& node )
{
    apply( static_cast<const Group&>( node ) );
}

void
ConstNodeVisitor::apply( const OcclusionQueryNode& node )
{
    apply( static_cast<const Group&>( node ) );
}

// ---------------------------------------------------------------------------
// DualModeVisitor
// ---------------------------------------------------------------------------

DualModeVisitor::DualModeVisitor( TraversalMode tm ) :
    Object( true ),
    NodeVisitor( tm ),
    ConstNodeVisitor( tm )
{
}

DualModeVisitor::DualModeVisitor( VisitorType   type,
                                  TraversalMode tm ) :
    Object( true ),
    NodeVisitor( type,
                 tm ),
    ConstNodeVisitor( type,
                      tm )
{
}

DualModeVisitor::~DualModeVisitor()
{
}
