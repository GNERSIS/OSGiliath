/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Thread-safe weak reference to a path of nodes. Automatically
 * invalidated when any node in the path is deleted.
 */
#pragma once

#include <osg/core/observer_ptr.hpp>
#include <osg/nodes/Node.hpp>
#include <vector>

namespace osg
{

    typedef std::vector<osg::ref_ptr<osg::Node>> RefNodePath;

    /** ObserverNodePath is an observer class for tracking changes to a NodePath,
     * that automatically invalidates it when nodes are deleted.*/
    class OSG_EXPORT ObserverNodePath
    {
        public:

            ObserverNodePath();

            ObserverNodePath( const ObserverNodePath& rhs );

            ObserverNodePath( const osg::NodePath& nodePath );

            ~ObserverNodePath();

            ObserverNodePath&
            operator=( const ObserverNodePath& rhs );

            /** get the NodePath from the first parental chain back to root, plus the
             * specified node.*/
            void
            setNodePathTo( osg::Node* node );

            void
            setNodePath( const osg::RefNodePath& nodePath );

            void
            setNodePath( const osg::NodePath& nodePath );

            void
            clearNodePath();

            /** Get a thread safe RefNodePath, return true if NodePath is valid.*/
            bool
            getRefNodePath( RefNodePath& refNodePath ) const;

            /** Get a lightweight NodePath that isn't thread safe but
             * may be safely used in single threaded applications, or when
             * its known that the NodePath won't be invalidated during usage
             * of the NodePath. return true if NodePath is valid.*/
            bool
            getNodePath( NodePath& nodePath ) const;

            bool
            empty() const
            {
                std::lock_guard<std::mutex> lock( _mutex );
                return _nodePath.empty();
            }

        protected:

            void
            _setNodePath( const osg::NodePath& nodePath );
            void
                                                              _clearNodePath();

            typedef std::vector<osg::observer_ptr<osg::Node>> ObsNodePath;
            mutable std::mutex                                _mutex;
            ObsNodePath                                       _nodePath;
    };

}
