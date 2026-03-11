/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Update callback that tracks a target node. Computes the world
 * position of the tracked node each frame for follower logic.
 */
#include <osg/traversal/NodeTrackerCallback.hpp>

#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/nodes/Camera.hpp>
#include <osg/nodes/CameraView.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/nodes/PositionAttitudeTransform.hpp>
#include <osg/traversal/NodeVisitor.hpp>

using namespace osg;

class ApplyMatrixVisitor : public DualModeVisitor
{
    public:

        using ConstNodeVisitor::apply;
        using NodeVisitor::apply;

        ApplyMatrixVisitor( const osg::dmat4& matrix ) :
            _matrix( matrix )
        {
        }

        virtual void
        apply( Camera& camera )
        {
            camera.setViewMatrix( _matrix );
        }

        virtual void
        apply( CameraView& cv )
        {
            cv.setPosition( osg::getTrans( _matrix ) );
            cv.setAttitude( quat( osg::getRotate( _matrix ) ) );
        }

        virtual void
        apply( MatrixTransform& mt )
        {
            mt.setMatrix( _matrix );
        }

        virtual void
        apply( PositionAttitudeTransform& pat )
        {
            pat.setPosition( osg::getTrans( _matrix ) );
            pat.setAttitude( quat( osg::getRotate( _matrix ) ) );
        }

        osg::dmat4 _matrix;
};

void
NodeTrackerCallback::setTrackNode( osg::Node* node )
{
    if( !node )
    {
        OSG_NOTICE << "NodeTrackerCallback::setTrackNode(Node*):  Unable to set tracked "
                      "node due to null Node*"
                   << std::endl;
        return;
    }

    NodePathList parentNodePaths = node->getParentalNodePaths();

    if( !parentNodePaths.empty() )
    {
        OSG_INFO << "NodeTrackerCallback::setTrackNode(Node*): Path set" << std::endl;
        setTrackNodePath( parentNodePaths[0] );
    }
    else
    {
        OSG_NOTICE << "NodeTrackerCallback::setTrackNode(Node*): Unable to set tracked "
                      "node due to empty parental path."
                   << std::endl;
    }
}

osg::Node*
NodeTrackerCallback::getTrackNode()
{
    osg::NodePath nodePath;
    if( _trackNodePath.getNodePath( nodePath ) )
    {
        return nodePath.back();
    }
    else
    {
        return 0;
    }
}

const osg::Node*
NodeTrackerCallback::getTrackNode() const
{
    osg::NodePath nodePath;
    if( _trackNodePath.getNodePath( nodePath ) )
    {
        return nodePath.back();
    }
    else
    {
        return 0;
    }
}

void
NodeTrackerCallback::operator()( Node*        node,
                                 NodeVisitor* nv )
{
    if( nv->getVisitorType() == NodeVisitor::UPDATE_VISITOR )
    {
        update( *node );
    }

    traverse( node, nv );
}

void
NodeTrackerCallback::update( osg::Node& node )
{
    osg::NodePath nodePath;
    if( _trackNodePath.getNodePath( nodePath ) )
    {
        ApplyMatrixVisitor applyMatrix( computeWorldToLocal( nodePath ) );
        node.accept( applyMatrix );
    }
}
