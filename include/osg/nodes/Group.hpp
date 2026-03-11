/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Scene graph node that manages an ordered list of child nodes.
 * Foundation for all branch nodes (Transform, Switch, LOD, etc.).
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/nodes/Node.hpp>
#include <osg/traversal/NodeVisitor.hpp>

namespace osg
{

    typedef std::vector<ref_ptr<Node>> NodeList;

    /** General group node which maintains a list of children.
     * Children are reference counted. This allows children to be shared
     * with memory management handled automatically via osg::Referenced.
     */
    class OSG_EXPORT Group : public osg::Inherit<Node, Group>
    {
        public:

            OSG_REGISTER_TYPE( osg,
                               Group )

            Group();

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            Group( const Group&,
                   const CopyOp& copyop = CopyOp::SHALLOW_COPY );

            virtual Group*
            asGroup() override
            {
                return this;
            }

            virtual const Group*
            asGroup() const override
            {
                return this;
            }

            virtual void
            traverse( NodeVisitor& nv ) override;
            virtual void
            traverse( ConstNodeVisitor& nv ) const override;

            /** Add Node to Group.
             * If node is not NULL then increment its
             * reference count, add it to the child list and dirty the bounding
             * sphere to force it to recompute on next getBound() and return true for
             * success. Otherwise return false. Scene nodes can't be added as child
             * nodes.
             */
            virtual bool
            addChild( Node* child );

            template<class T>
            bool
            addChild( const ref_ptr<T>& child )
            {
                return addChild( child.get() );
            }

            /** Insert Node to Group at specific location.
             * The new child node is inserted into the child list
             * before the node at the specified index. No nodes
             * are removed from the group with this operation.
             */
            virtual bool
            insertChild( unsigned int index,
                         Node*        child );

            template<class T>
            bool
            insertChild( unsigned int      index,
                         const ref_ptr<T>& child )
            {
                return insertChild( index, child.get() );
            }

            /** Remove Node from Group.
             * If Node is contained in Group then remove it from the child
             * list, decrement its reference count, and dirty the
             * bounding sphere to force it to recompute on next getBound() and
             * return true for success. If Node is not found then return false
             * and do not change the reference count of the Node.
             * Note, do not override, only override removeChildren(,) is required.
             */
            virtual bool
            removeChild( Node* child );

            template<class T>
            bool
            removeChild( const ref_ptr<T>& child )
            {
                return removeChild( child.get() );
            }

            /** Remove Node from Group.
             * If Node is contained in Group then remove it from the child
             * list, decrement its reference count, and dirty the
             * bounding sphere to force it to recompute on next getBound() and
             * return true for success. If Node is not found then return false
             * and do not change the reference count of the Node.
             * Note, do not override, only override removeChildren(,) is required.
             */
            inline bool
            removeChild( unsigned int pos,
                         unsigned int numChildrenToRemove = 1 )
            {
                if( pos < _children.size() )
                {
                    return removeChildren( pos, numChildrenToRemove );
                }
                else
                {
                    return false;
                }
            }

            /** Remove children from Group.
             * Note, must be override by subclasses of Group which add per child
             * attributes.*/
            virtual bool
            removeChildren( unsigned int pos,
                            unsigned int numChildrenToRemove );

            /** Replace specified child Node with another Node.
             * Equivalent to setChild(getChildIndex(orignChild),node)
             * See docs for setChild for further details on implementation.
             */
            virtual bool
            replaceChild( Node* origChild,
                          Node* newChild );

            template<class T,
                     class R>
            bool
            replaceChild( const ref_ptr<T>& origChild,
                          const ref_ptr<R>& newChild )
            {
                return replaceChild( origChild.get(), newChild.get() );
            }

            /** Return the number of children nodes. */
            virtual unsigned int
            getNumChildren() const;

            /** Set child node at position i.
             * Return true if set correctly, false on failure (if node==NULL || i is out
             * of range). When Set can be successful applied, the algorithm is :
             * decrement the reference count origNode and increment the reference count
             * of newNode, and dirty the bounding sphere to force it to recompute on next
             * getBound() and return true. If origNode is not found then return false and
             * do not add newNode. If newNode is NULL then return false and do not remove
             * origNode. Also returns false if newChild is a Scene node.
             */
            virtual bool
            setChild( unsigned int i,
                      Node*        node );

            /** Return child node at position i. */
            inline Node*
            getChild( unsigned int i )
            {
                return _children[i].get();
            }

            /** Return child node at position i. */
            inline const Node*
            getChild( unsigned int i ) const
            {
                return _children[i].get();
            }

            /** Return true if node is contained within Group. */
            inline bool
            containsNode( const Node* node ) const
            {

                for( NodeList::const_iterator itr = _children.begin();
                     itr != _children.end();
                     ++itr )
                {
                    if( itr->get() == node )
                    {
                        return true;
                    }
                }
                return false;
            }

            template<class T>
            bool
            containsNode( const ref_ptr<T>& node ) const
            {
                return containsNode( node.get() );
            }

            /** Get the index number of child, return a value between
             * 0 and _children.size()-1 if found, if not found then
             * return _children.size().
             */
            inline unsigned int
            getChildIndex( const Node* node ) const
            {
                for( unsigned int childNum = 0; childNum < _children.size(); ++childNum )
                {
                    if( _children[childNum] == node )
                    {
                        return childNum;
                    }
                }
                return static_cast<unsigned int>(
                    _children.size()
                );    // node not found.
            }

            /** Set whether to use a mutex to ensure ref() and unref() are thread safe.*/
            virtual void
            setThreadSafeRefUnref( bool threadSafe ) override;

            /** Resize any per context GLObject buffers to specified size. */
            virtual void
            resizeGLObjectBuffers( unsigned int maxSize ) override;

            /** If State is non-zero, this function releases any associated OpenGL
             * objects for the specified graphics context. Otherwise, releases OpenGL
             * objects for all graphics contexts. */
            virtual void
            releaseGLObjects( osg::State* = 0 ) const override;

            virtual sphere
            computeBound() const override;

        protected:

            virtual ~Group();

            virtual void
            childRemoved( unsigned int /*pos*/,
                          unsigned int /*numChildrenToRemove*/ )
            {
            }

            virtual void
            childInserted( unsigned int /*pos*/ )
            {
            }

            NodeList _children;
    };

}
