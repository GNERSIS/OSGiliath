/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Update callback that tracks a target node. Computes the world
 * position of the tracked node each frame for follower logic.
 */
#pragma once

#include <iterator>
#include <osg/core/ObserverNodePath.hpp>
#include <osg/nodes/Node.hpp>

namespace osg
{

    class OSG_EXPORT NodeTrackerCallback : public NodeCallback
    {
        public:

            void
            setTrackNodePath( const osg::NodePath& nodePath )
            {
                _trackNodePath.setNodePath( nodePath );
            }

            void
            setTrackNodePath( const ObserverNodePath& nodePath )
            {
                _trackNodePath = nodePath;
            }

            ObserverNodePath&
            getTrackNodePath()
            {
                return _trackNodePath;
            }

            void
            setTrackNode( osg::Node* node );
            osg::Node*
            getTrackNode();
            const osg::Node*
            getTrackNode() const;

            /** Implements the callback. */
            virtual void
            operator()( Node*        node,
                        NodeVisitor* nv );

            /** Update the node to track the nodepath.*/
            void
            update( osg::Node& node );

        protected:

            ObserverNodePath _trackNodePath;
    };

}
