/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Base class for scene graph traversals. Implements the visitor
 * pattern with pre/post apply callbacks and traversal mode control.
 */
#pragma once

#include <osg/core/FrameStamp.hpp>
#include <osg/core/ValueMap.hpp>
#include <osg/core/ValueStack.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/nodes/Node.hpp>
#include <type_traits>

namespace osgUtil
{

    class UpdateVisitor;
    class CullVisitor;
    class IntersectionVisitor;

}

namespace osgGA
{

    class EventVisitor;

}

namespace osg
{

    class Billboard;
    class ClearNode;
    class CoordinateSystemNode;
    class Geode;
    class Group;
    class LightSource;
    class LOD;
    class MatrixTransform;
    class OccluderNode;
    class OcclusionQueryNode;
    class PagedLOD;
    class PositionAttitudeTransform;
    class AutoTransform;
    class MultiViewAutoTransform;
    class Projection;
    class ProxyNode;
    class Sequence;
    class Switch;
    class Transform;
    class Camera;
    class CameraView;
    class Drawable;
    class Geometry;
    class CullStack;

    // Forward declare
    class ConstNodeVisitor;

    const unsigned int UNINITIALIZED_FRAME_NUMBER = 0XFF'FF'FF'FF;

#define META_NodeVisitor( library, name )   \
    virtual const char* libraryName() const \
    {                                       \
        return #library;                    \
    }                                       \
    virtual const char* className() const   \
    {                                       \
        return #name;                       \
    }

    /** Non-template base holding enums shared by NodeVisitor and ConstNodeVisitor.
     * Both NodeVisitorBase<Node*> and NodeVisitorBase<const Node*> inherit from
     * this single definition, so the enum types are identical across both visitors. */
    struct NodeVisitorEnums
    {
            enum TraversalMode
            {
                TRAVERSE_NONE,
                TRAVERSE_PARENTS,
                TRAVERSE_ALL_CHILDREN,
                TRAVERSE_ACTIVE_CHILDREN,
            };

            enum VisitorType
            {
                NODE_VISITOR = 0,
                UPDATE_VISITOR,
                EVENT_VISITOR,
                COLLECT_OCCLUDER_VISITOR,
                CULL_VISITOR,
                INTERSECTION_VISITOR,
            };
    };

    /** Template base class holding shared state for both NodeVisitor and
     * ConstNodeVisitor. NodePtr is either Node* (mutable traversal) or const Node*
     * (const traversal). This is a non-virtual, header-only helper — it does NOT inherit
     * from Object. */
    template<typename NodePtr>
    class NodeVisitorBase : public NodeVisitorEnums
    {
        public:

            using node_pointer   = NodePtr;
            using node_reference = std::remove_pointer_t<NodePtr>&;
            using NodePathType   = std::vector<NodePtr>;

            /** Set the VisitorType, used to distinguish different visitors during
             * traversal of the scene, typically used in the Node::traverse() method
             * to select which behaviour to use for different types of
             * traversal/visitors.*/
            inline void
            setVisitorType( VisitorType type )
            {
                _visitorType = type;
            }

            /** Get the VisitorType.*/
            inline VisitorType
            getVisitorType() const
            {
                return _visitorType;
            }

            /** Set the traversal number. Typically used to denote the frame count.*/
            inline void
            setTraversalNumber( unsigned int fn )
            {
                _traversalNumber = fn;
            }

            /** Get the traversal number. Typically used to denote the frame count.*/
            inline unsigned int
            getTraversalNumber() const
            {
                return _traversalNumber;
            }

            /** Set the FrameStamp that this traversal is associated with.*/
            inline void
            setFrameStamp( FrameStamp* fs )
            {
                _frameStamp = fs;
            }

            /** Get the FrameStamp that this traversal is associated with.*/
            inline const FrameStamp*
            getFrameStamp() const
            {
                return _frameStamp.get();
            }

            /** Set the TraversalMask of this NodeVisitor.
             * The TraversalMask is used by the NodeVisitor::validNodeMask() method
             * to determine whether to operate on a node and its subgraph.
             * validNodeMask() is called automatically in the Node::accept() method
             * before any call to NodeVisitor::apply(), apply() is only ever called if
             * validNodeMask returns true. Note, if NodeVisitor::_traversalMask is 0 then
             * all operations will be switched off for all nodes.  Whereas setting both
             * _traversalMask and _nodeMaskOverride to 0xffffffff will allow a visitor to
             * work on all nodes regardless of their own Node::_nodeMask state.*/
            inline void
            setTraversalMask( Node::NodeMask mask )
            {
                _traversalMask = mask;
            }

            /** Get the TraversalMask.*/
            inline Node::NodeMask
            getTraversalMask() const
            {
                return _traversalMask;
            }

            /** Set the NodeMaskOverride mask.
             * Used in validNodeMask() to determine whether to operate on a node or its
             * subgraph, by OR'ing NodeVisitor::_nodeMaskOverride with the Node's own
             * Node::_nodeMask. Typically used to force on nodes which may have been
             * switched off by their own Node::_nodeMask.*/
            inline void
            setNodeMaskOverride( Node::NodeMask mask )
            {
                _nodeMaskOverride = mask;
            }

            /** Get the NodeMaskOverride mask.*/
            inline Node::NodeMask
            getNodeMaskOverride() const
            {
                return _nodeMaskOverride;
            }

            /** Method to called by Node and its subclass' Node::accept() method, if the
             * result is true it is used to cull operations of nodes and their subgraphs.
             * Return true if the result of a bit wise and of the
             * NodeVisitor::_traversalMask with the bit or between
             * NodeVistor::_nodeMaskOverride and the Node::_nodeMask. default values for
             * _traversalMask is 0xffffffff, _nodeMaskOverride is 0x0, and
             * osg::Node::_nodeMask is 0xffffffff. */
            inline bool
            validNodeMask( const osg::Node& node ) const
            {
                return ( getTraversalMask() &
                         ( getNodeMaskOverride() | node.getNodeMask() ) ) != 0;
            }

            /** Set the traversal mode for Node::traverse() to use when
                deciding which children of a node to traverse. If a
                NodeVisitor has been attached via setTraverseVisitor()
                and the new mode is not TRAVERSE_VISITOR then the attached
                visitor is detached. Default mode is TRAVERSE_NONE.*/
            inline void
            setTraversalMode( TraversalMode mode )
            {
                _traversalMode = mode;
            }

            /** Get the traversal mode.*/
            inline TraversalMode
            getTraversalMode() const
            {
                return _traversalMode;
            }

            /** Method called by osg::Node::accept() method before
             * a call to the NodeVisitor::apply(..).  The back of the list will,
             * therefore, be the current node being visited inside the apply(..),
             * and the rest of the list will be the parental sequence of nodes
             * from the top most node applied down the graph to the current node.
             * Note, the user does not typically call pushNodeOnPath() as it
             * will be called automatically by the Node::accept() method.*/
            inline void
            pushOntoNodePath( NodePtr node )
            {
                if( _traversalMode != TRAVERSE_PARENTS )
                {
                    _nodePath.push_back( node );
                }
                else
                {
                    _nodePath.insert( _nodePath.begin(), node );
                }
            }

            /** Method called by osg::Node::accept() method after
             * a call to NodeVisitor::apply(..).
             * Note, the user does not typically call popFromNodePath() as it
             * will be called automatically by the Node::accept() method.*/
            inline void
            popFromNodePath()
            {
                if( _traversalMode != TRAVERSE_PARENTS )
                {
                    _nodePath.pop_back();
                }
                else
                {
                    _nodePath.erase( _nodePath.begin() );
                }
            }

            /** Get the non const NodePath from the top most node applied down
             * to the current Node being visited.*/
            NodePathType&
            getNodePath()
            {
                return _nodePath;
            }

            /** Get the const NodePath from the top most node applied down
             * to the current Node being visited.*/
            const NodePathType&
            getNodePath() const
            {
                return _nodePath;
            }

        protected:

            NodeVisitorBase( TraversalMode tm = TRAVERSE_NONE ) :
                _visitorType( NODE_VISITOR ),
                _traversalNumber( UNINITIALIZED_FRAME_NUMBER ),
                _traversalMode( tm ),
                _traversalMask( 0XFF'FF'FF'FF ),
                _nodeMaskOverride( 0X0 )
            {
            }

            NodeVisitorBase( VisitorType   type,
                             TraversalMode tm = TRAVERSE_NONE ) :
                _visitorType( type ),
                _traversalNumber( UNINITIALIZED_FRAME_NUMBER ),
                _traversalMode( tm ),
                _traversalMask( 0XFF'FF'FF'FF ),
                _nodeMaskOverride( 0X0 )
            {
            }

            NodeVisitorBase( const NodeVisitorBase& rhs ) :
                _visitorType( rhs._visitorType ),
                _traversalNumber( rhs._traversalNumber ),
                _frameStamp( rhs._frameStamp ),
                _traversalMode( rhs._traversalMode ),
                _traversalMask( rhs._traversalMask ),
                _nodeMaskOverride( rhs._nodeMaskOverride )
            {
            }

            ~NodeVisitorBase() = default;

            VisitorType         _visitorType;
            unsigned int        _traversalNumber;

            ref_ptr<FrameStamp> _frameStamp;

            TraversalMode       _traversalMode;
            Node::NodeMask      _traversalMask;
            Node::NodeMask      _nodeMaskOverride;

            NodePathType        _nodePath;
    };

    /** Visitor for type safe operations on osg::Nodes.
        Based on GOF's Visitor pattern. The NodeVisitor
        is useful for developing type safe operations to nodes
        in the scene graph (as per Visitor pattern), and adds to this
        support for optional scene graph traversal to allow
        operations to be applied to whole scenes at once. The Visitor
        pattern uses a technique of double dispatch as a mechanism to
        call the appropriate apply(..) method of the NodeVisitor.  To
        use this feature one must use the Node::accept(NodeVisitor) which
        is extended in each Node subclass, rather than the NodeVisitor
        apply directly.  So use root->accept(myVisitor); instead of
        myVisitor.apply(*root).  The later method will bypass the double
        dispatch and the appropriate NodeVisitor::apply(..) method will
        not be called. */
    class OSG_EXPORT NodeVisitor : public virtual Object,
                                   public NodeVisitorBase<Node*>
    {
        public:

            NodeVisitor( TraversalMode tm = TRAVERSE_NONE );

            NodeVisitor( VisitorType   type,
                         TraversalMode tm = TRAVERSE_NONE );

            NodeVisitor( const NodeVisitor& nv,
                         const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            virtual ~NodeVisitor();

            OSG_REGISTER_TYPE( osg,
                               NodeVisitor )

            virtual osg::Object*
            cloneType() const override
            {
                return new NodeVisitor();
            }

            virtual osg::Object*
            clone( const osg::CopyOp& copyop ) const override
            {
                return new NodeVisitor( *this, copyop );
            }

            virtual bool
            isSameKindAs( const osg::Object* obj ) const override
            {
                return dynamic_cast<const NodeVisitor*>( obj ) != nullptr;
            }

            virtual const char*
            libraryName() const override
            {
                return _s_libraryName();
            }

            virtual const char*
            className() const override
            {
                return _s_className();
            }

            /** Convert 'this' into a NodeVisitor pointer if Object is a NodeVisitor,
             * otherwise return 0. Equivalent to dynamic_cast<NodeVisitor*>(this).*/
            virtual NodeVisitor*
            asNodeVisitor() override
            {
                return this;
            }

            /** convert 'const this' into a const NodeVisitor pointer if Object is a
             * NodeVisitor, otherwise return 0. Equivalent to dynamic_cast<const
             * NodeVisitor*>(this).*/
            virtual const NodeVisitor*
            asNodeVisitor() const override
            {
                return this;
            }

            /** Convert 'this' into a osgUtil::UpdateVisitor pointer if Object is a
             * osgUtil::UpdateVisitor, otherwise return 0. Equivalent to
             * dynamic_cast<osgUtil::UpdateVisitor*>(this).*/
            virtual osgUtil::UpdateVisitor*
            asUpdateVisitor()
            {
                return 0;
            }

            /** convert 'const this' into a const osgUtil::UpdateVisitor pointer if
             * Object is a osgUtil::UpdateVisitor, otherwise return 0. Equivalent to
             * dynamic_cast<const osgUtil::UpdateVisitor*>(this).*/
            virtual const osgUtil::UpdateVisitor*
            asUpdateVisitor() const
            {
                return 0;
            }

            /** Convert 'this' into a osgUtil::CullVisitor pointer if Object is a
             * osgUtil::CullVisitor, otherwise return 0. Equivalent to
             * dynamic_cast<osgUtil::CullVisitor*>(this).*/
            virtual osgUtil::CullVisitor*
            asCullVisitor()
            {
                return 0;
            }

            /** convert 'const this' into a const osgUtil::CullVisitor pointer if Object
             * is a osgUtil::CullVisitor, otherwise return 0. Equivalent to
             * dynamic_cast<const osgUtil::CullVisitor*>(this).*/
            virtual const osgUtil::CullVisitor*
            asCullVisitor() const
            {
                return 0;
            }

            /** Convert 'this' into a osgGA::EventVisitor pointer if Object is a
             * osgGA::EventVisitor, otherwise return 0. Equivalent to
             * dynamic_cast<osgGA::EventVisitor*>(this).*/
            virtual osgGA::EventVisitor*
            asEventVisitor()
            {
                return 0;
            }

            /** convert 'const this' into a const osgGA::EventVisitor pointer if Object
             * is a osgGA::EventVisitor, otherwise return 0. Equivalent to
             * dynamic_cast<const osgGA::EventVisitor*>(this).*/
            virtual const osgGA::EventVisitor*
            asEventVisitor() const
            {
                return 0;
            }

            /** Convert 'this' into a osgUtil::IntersectionVisitor pointer if Object is a
             * IntersectionVisitor, otherwise return 0. Equivalent to
             * dynamic_cast<osgUtil::IntersectionVisitor*>(this).*/
            virtual osgUtil::IntersectionVisitor*
            asIntersectionVisitor()
            {
                return 0;
            }

            /** convert 'const this' into a const osgUtil::IntersectionVisitor pointer if
             * Object is a IntersectionVisitor, otherwise return 0. Equivalent to
             * dynamic_cast<const osgUtil::IntersectionVisitor*>(this).*/
            virtual const osgUtil::IntersectionVisitor*
            asIntersectionVisitor() const
            {
                return 0;
            }

            /** Convert 'this' into a osg::CullStack pointer if Object is a
             * osg::CullStack, otherwise return 0. Equivalent to
             * dynamic_cast<osg::CullStack*>(this).*/
            virtual osg::CullStack*
            asCullStack()
            {
                return 0;
            }

            /** convert 'const this' into a const osg::CullStack pointer if Object is a
             * osg::CullStack, otherwise return 0. Equivalent to dynamic_cast<const
             * osg::CullStack*>(this).*/
            virtual const osg::CullStack*
            asCullStack() const
            {
                return 0;
            }

            /** Method to call to reset visitor. Useful if your visitor accumulates
                state during a traversal, and you plan to reuse the visitor.
                To flush that state for the next traversal: call reset() prior
                to each traversal.*/
            virtual void
            reset()
            {
            }

            /** Set the ValueMap used to store Values that can be reused over a series of
             * traversals. */
            inline void
            setValueMap( ValueMap* ps )
            {
                _valueMap = ps;
            }

            /** Get the ValueMap. */
            inline ValueMap*
            getValueMap()
            {
                return _valueMap.get();
            }

            /** Get the ValueMap. */
            inline const ValueMap*
            getValueMap() const
            {
                return _valueMap.get();
            }

            /** Get the ValueMap. */
            inline ValueMap*
            getOrCreateValueMap()
            {
                if( !_valueMap )
                {
                    _valueMap = new ValueMap;
                }
                return _valueMap.get();
            }

            /** Set the ValueStack used to stack Values during traversal. */
            inline void
            setValueStack( ValueStack* ps )
            {
                _valueStack = ps;
            }

            /** Get the ValueStack. */
            inline ValueStack*
            getValueStack()
            {
                return _valueStack.get();
            }

            /** Get the const ValueStack. */
            inline const ValueStack*
            getValueStack() const
            {
                return _valueStack.get();
            }

            /** Get the ValueStack. */
            inline ValueStack*
            getOrCreateValueStack()
            {
                if( !_valueStack )
                {
                    _valueStack = new ValueStack;
                }
                return _valueStack.get();
            }

            /** Method for handling traversal of a nodes.
                If you intend to use the visitor for actively traversing
                the scene graph then make sure the accept() methods call
                this method unless they handle traversal directly.*/
            inline void
            traverse( Node& node )
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

            /** Get the eye point in local coordinates.
             * Note, not all NodeVisitor implement this method, it is mainly cull
             * visitors which will implement.*/
            virtual osg::vec3
            getEyePoint() const
            {
                return vec3( 0.0F, 0.0F, 0.0F );
            }

            /** Get the view point in local coordinates.
             * Note, not all NodeVisitor implement this method, it is mainly cull
             * visitors which will implement.*/
            virtual osg::vec3
            getViewPoint() const
            {
                return getEyePoint();
            }

            /** Get the distance from a point to the eye point, distance value in local
             * coordinate system. Note, not all NodeVisitor implement this method, it is
             * mainly cull visitors which will implement. If the
             * getDistanceFromEyePoint(pos) is not implemented then a default value of
             * 0.0 is returned.*/
            virtual float
            getDistanceToEyePoint( const vec3& /*pos*/,
                                   bool /*useLODScale*/ ) const
            {
                return 0.0F;
            }

            /** Get the distance of a point from the eye point, distance value in the eye
             * coordinate system. Note, not all NodeVisitor implement this method, it is
             * mainly cull visitors which will implement. If the
             * getDistanceFromEyePoint(pos) is not implemented than a default value of
             * 0.0 is returned.*/
            virtual float
            getDistanceFromEyePoint( const vec3& /*pos*/,
                                     bool /*useLODScale*/ ) const
            {
                return 0.0F;
            }

            /** Get the distance from a point to the view point, distance value in local
             * coordinate system. Note, not all NodeVisitor implement this method, it is
             * mainly cull visitors which will implement. If the
             * getDistanceToViewPoint(pos) is not implemented then a default value of 0.0
             * is returned.*/
            virtual float
            getDistanceToViewPoint( const vec3& /*pos*/,
                                    bool /*useLODScale*/ ) const
            {
                return 0.0F;
            }

            virtual void
            apply( Drawable& drawable );
            virtual void
            apply( Geometry& geometry );

            virtual void
            apply( Node& node );

            virtual void
            apply( Geode& node );
            virtual void
            apply( Billboard& node );

            virtual void
            apply( Group& node );

            virtual void
            apply( ProxyNode& node );

            virtual void
            apply( Projection& node );

            virtual void
            apply( CoordinateSystemNode& node );

            virtual void
            apply( LightSource& node );

            virtual void
            apply( Transform& node );
            virtual void
            apply( Camera& node );
            virtual void
            apply( CameraView& node );
            virtual void
            apply( MatrixTransform& node );
            virtual void
            apply( PositionAttitudeTransform& node );
            virtual void
            apply( AutoTransform& node );

            virtual void
            apply( Switch& node );
            virtual void
            apply( Sequence& node );
            virtual void
            apply( LOD& node );
            virtual void
            apply( PagedLOD& node );
            virtual void
            apply( ClearNode& node );
            virtual void
            apply( OccluderNode& node );
            virtual void
            apply( OcclusionQueryNode& node );

            /** Callback for managing database paging, such as generated by PagedLOD
             * nodes.*/
            class DatabaseRequestHandler : public osg::Referenced
            {
                public:

                    DatabaseRequestHandler() :
                        Referenced( true )
                    {
                    }

                    virtual void
                    requestNodeFile( const std::string&             fileName,
                                     osg::NodePath&                 nodePath,
                                     float                          priority,
                                     const FrameStamp*              framestamp,
                                     osg::ref_ptr<osg::Referenced>& databaseRequest,
                                     const osg::Referenced*         options = 0 ) = 0;

                protected:

                    virtual ~DatabaseRequestHandler()
                    {
                    }
            };

            /** Set the handler for database requests.*/
            void
            setDatabaseRequestHandler( DatabaseRequestHandler* handler )
            {
                _databaseRequestHandler = handler;
            }

            /** Get the handler for database requests.*/
            DatabaseRequestHandler*
            getDatabaseRequestHandler()
            {
                return _databaseRequestHandler.get();
            }

            /** Get the const handler for database requests.*/
            const DatabaseRequestHandler*
            getDatabaseRequestHandler() const
            {
                return _databaseRequestHandler.get();
            }

            /** Callback for managing image paging, such as generated by PagedLOD
             * nodes.*/
            class ImageRequestHandler : public osg::Referenced
            {
                public:

                    ImageRequestHandler() :
                        Referenced( true )
                    {
                    }

                    virtual double
                    getPreLoadTime() const = 0;

                    virtual osg::ref_ptr<osg::Image>
                    readRefImageFile( const std::string&     fileName,
                                      const osg::Referenced* options = 0 ) = 0;

                    virtual void
                    requestImageFile( const std::string&             fileName,
                                      osg::Object*                   attachmentPoint,
                                      int                            attachmentIndex,
                                      double                         timeToMergeBy,
                                      const FrameStamp*              framestamp,
                                      osg::ref_ptr<osg::Referenced>& imageRequest,
                                      const osg::Referenced*         options = 0 ) = 0;

                protected:

                    virtual ~ImageRequestHandler()
                    {
                    }
            };

            /** Set the handler for image requests.*/
            void
            setImageRequestHandler( ImageRequestHandler* handler )
            {
                _imageRequestHandler = handler;
            }

            /** Get the handler for image requests.*/
            ImageRequestHandler*
            getImageRequestHandler()
            {
                return _imageRequestHandler.get();
            }

            /** Get the const handler for image requests.*/
            const ImageRequestHandler*
            getImageRequestHandler() const
            {
                return _imageRequestHandler.get();
            }

        protected:

            ref_ptr<DatabaseRequestHandler> _databaseRequestHandler;
            ref_ptr<ImageRequestHandler>    _imageRequestHandler;

            osg::ref_ptr<ValueMap>          _valueMap;
            osg::ref_ptr<ValueStack>        _valueStack;
    };

    /** Const-correct visitor for read-only traversal of the scene graph.
     * Mirrors NodeVisitor but all apply() methods take const references.
     * Traversal helpers operate on const Node references/pointers. */
    class OSG_EXPORT ConstNodeVisitor : public virtual Object,
                                        public NodeVisitorBase<const Node*>
    {
        public:

            ConstNodeVisitor( TraversalMode tm = TRAVERSE_NONE );

            ConstNodeVisitor( VisitorType   type,
                              TraversalMode tm = TRAVERSE_NONE );

            ConstNodeVisitor( const ConstNodeVisitor& nv,
                              const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            virtual ~ConstNodeVisitor();

            OSG_REGISTER_TYPE( osg,
                               ConstNodeVisitor )

            virtual osg::Object*
            cloneType() const override
            {
                return new ConstNodeVisitor();
            }

            virtual osg::Object*
            clone( const osg::CopyOp& copyop ) const override
            {
                return new ConstNodeVisitor( *this, copyop );
            }

            virtual bool
            isSameKindAs( const osg::Object* obj ) const override
            {
                return dynamic_cast<const ConstNodeVisitor*>( obj ) != nullptr;
            }

            virtual const char*
            libraryName() const override
            {
                return _s_libraryName();
            }

            virtual const char*
            className() const override
            {
                return _s_className();
            }

            /** Method to call to reset visitor.*/
            virtual void
            reset()
            {
            }

            /** Traverse a const node. Defined out-of-line because Node's const
             * traverse/ascend methods are added in a later task. Until then this
             * is a no-op stub — nothing calls it yet so the build is clean. */
            void
            traverse( const Node& node );

            virtual void
            apply( const Drawable& drawable );
            virtual void
            apply( const Geometry& geometry );

            virtual void
            apply( const Node& node );

            virtual void
            apply( const Geode& node );
            virtual void
            apply( const Billboard& node );

            virtual void
            apply( const Group& node );

            virtual void
            apply( const ProxyNode& node );

            virtual void
            apply( const Projection& node );

            virtual void
            apply( const CoordinateSystemNode& node );

            virtual void
            apply( const LightSource& node );

            virtual void
            apply( const Transform& node );
            virtual void
            apply( const Camera& node );
            virtual void
            apply( const CameraView& node );
            virtual void
            apply( const MatrixTransform& node );
            virtual void
            apply( const PositionAttitudeTransform& node );
            virtual void
            apply( const AutoTransform& node );

            virtual void
            apply( const Switch& node );
            virtual void
            apply( const Sequence& node );
            virtual void
            apply( const LOD& node );
            virtual void
            apply( const PagedLOD& node );
            virtual void
            apply( const ClearNode& node );
            virtual void
            apply( const OccluderNode& node );
            virtual void
            apply( const OcclusionQueryNode& node );
    };

    /** Convenience functor for assisting visiting of arrays of osg::Node's.*/
    class NodeAcceptOp
    {
        public:

            NodeAcceptOp( NodeVisitor& nv ) :
                _nv( nv )
            {
            }

            NodeAcceptOp( const NodeAcceptOp& naop ) :
                _nv( naop._nv )
            {
            }

            void
            operator()( Node* node )
            {
                node->accept( _nv );
            }

            void
            operator()( ref_ptr<Node> node )
            {
                node->accept( _nv );
            }

        protected:

            NodeAcceptOp&
            operator=( const NodeAcceptOp& )
            {
                return *this;
            }

            NodeVisitor& _nv;
    };

    /** Convenience functor for const-correct visiting of arrays of osg::Node's.*/
    class ConstNodeAcceptOp
    {
        public:

            ConstNodeAcceptOp( ConstNodeVisitor& nv ) :
                _nv( nv )
            {
            }

            ConstNodeAcceptOp( const ConstNodeAcceptOp& naop ) :
                _nv( naop._nv )
            {
            }

            void
            operator()( const Node* node )
            {
                node->accept( _nv );
            }

            void
            operator()( const ref_ptr<Node>& node )
            {
                node->accept( _nv );
            }

        protected:

            ConstNodeAcceptOp&
            operator=( const ConstNodeAcceptOp& )
            {
                return *this;
            }

            ConstNodeVisitor& _nv;
    };

    /** Base class for visitors that support both mutable and const traversal.
     * Resolves the virtual Object diamond between NodeVisitor and ConstNodeVisitor
     * by explicitly delegating all Object overrides to NodeVisitor's versions. */
    class OSG_EXPORT DualModeVisitor : public NodeVisitor,
                                       public ConstNodeVisitor
    {
        public:

            // Resolve ambiguous enum type names from the two NodeVisitorEnums bases
            using NodeVisitor::COLLECT_OCCLUDER_VISITOR;
            using NodeVisitor::CULL_VISITOR;
            using NodeVisitor::EVENT_VISITOR;
            using NodeVisitor::INTERSECTION_VISITOR;
            using NodeVisitor::NODE_VISITOR;
            using NodeVisitor::TraversalMode;
            using NodeVisitor::TRAVERSE_ACTIVE_CHILDREN;
            using NodeVisitor::TRAVERSE_ALL_CHILDREN;
            using NodeVisitor::TRAVERSE_NONE;
            using NodeVisitor::TRAVERSE_PARENTS;
            using NodeVisitor::UPDATE_VISITOR;
            using NodeVisitor::VisitorType;

            // Resolve ambiguous `apply` from NodeVisitor and ConstNodeVisitor
            using ConstNodeVisitor::apply;
            using NodeVisitor::apply;

            DualModeVisitor( TraversalMode tm = TRAVERSE_NONE );

            DualModeVisitor( VisitorType   type,
                             TraversalMode tm = TRAVERSE_NONE );

            virtual ~DualModeVisitor();

            // --- Resolve Object diamond by delegating to NodeVisitor ---

            Object*
            cloneType() const override
            {
                return NodeVisitor::cloneType();
            }

            Object*
            clone( const CopyOp& co ) const override
            {
                return NodeVisitor::clone( co );
            }

            bool
            isSameKindAs( const Object* obj ) const override
            {
                return NodeVisitor::isSameKindAs( obj );
            }

            bool
            is_compatible( const std::type_info& t ) const noexcept override
            {
                return NodeVisitor::is_compatible( t );
            }

            const std::type_info&
            type_info() const noexcept override
            {
                return NodeVisitor::type_info();
            }

            std::size_t
            sizeofObject() const noexcept override
            {
                return NodeVisitor::sizeofObject();
            }

            const char*
            libraryName() const override
            {
                return "osg";
            }

            const char*
            className() const override
            {
                return "DualModeVisitor";
            }

            // --- Resolve ambiguous base accessors ---
            // Both NodeVisitorBase<Node*> and NodeVisitorBase<const Node*> have these.
            // For DualModeVisitor, expose the mutable-visitor versions as the primary
            // API, and provide explicit const-visitor accessors via qualified names.

            using ConstNodeVisitor::traverse;
            using NodeVisitor::getFrameStamp;
            using NodeVisitor::getNodeMaskOverride;
            using NodeVisitor::getTraversalMask;
            using NodeVisitor::getTraversalMode;
            using NodeVisitor::getTraversalNumber;
            using NodeVisitor::getVisitorType;
            using NodeVisitor::reset;
            using NodeVisitor::setFrameStamp;
            using NodeVisitor::setNodeMaskOverride;
            using NodeVisitor::setTraversalMask;
            using NodeVisitor::setTraversalMode;
            using NodeVisitor::setTraversalNumber;
            using NodeVisitor::setVisitorType;
            using NodeVisitor::traverse;
            using NodeVisitor::validNodeMask;

            // --- Resolve ambiguous data members and remaining methods ---
            // Both NodeVisitorBase<Node*> and NodeVisitorBase<const Node*> have these.
            // Expose the mutable-visitor versions as the primary members.
            using NodeVisitor::_frameStamp;
            using NodeVisitor::_nodeMaskOverride;
            using NodeVisitor::_nodePath;
            using NodeVisitor::_traversalMask;
            using NodeVisitor::_traversalMode;
            using NodeVisitor::_traversalNumber;
            using NodeVisitor::_visitorType;
            using NodeVisitor::getNodePath;
            using NodeVisitor::NodePathType;
            using NodeVisitor::popFromNodePath;
            using NodeVisitor::pushOntoNodePath;
    };

    class PushPopObject
    {
        public:

            PushPopObject( NodeVisitor*      nv,
                           const Referenced* key,
                           Object*           value ) :
                _valueStack( 0 ),
                _key( 0 )
            {
                if( key )
                {
                    _valueStack = nv->getOrCreateValueStack();
                    _key        = key;
                    _valueStack->push( _key, value );
                }
            }

            PushPopObject( ValueStack*       valueStack,
                           const Referenced* key,
                           Object*           value ) :
                _valueStack( valueStack ),
                _key( 0 )
            {
                if( _valueStack && key )
                {
                    _key = key;
                    _valueStack->push( _key, value );
                }
            }

            inline ~PushPopObject()
            {
                if( _valueStack )
                {
                    _valueStack->pop( _key );
                }
            }

        protected:

            ValueStack*       _valueStack;
            const Referenced* _key;
    };

    class PushPopValue
    {
        public:

            template<typename T>
            PushPopValue( NodeVisitor*      nv,
                          const Referenced* key,
                          const T&          value ) :
                _valueStack( 0 ),
                _key( 0 )
            {
                if( key )
                {
                    _valueStack = nv->getOrCreateValueStack();
                    _key        = key;
                    _valueStack->push( _key, value );
                }
            }

            template<typename T>
            PushPopValue( ValueStack*       valueStack,
                          const Referenced* key,
                          const T&          value ) :
                _valueStack( valueStack ),
                _key( 0 )
            {
                if( _valueStack && key )
                {
                    _key = key;
                    _valueStack->push( _key, value );
                }
            }

            inline ~PushPopValue()
            {
                if( _valueStack )
                {
                    _valueStack->pop( _key );
                }
            }

        protected:

            ValueStack*       _valueStack;
            const Referenced* _key;
    };

    template<>
    inline ValueStack*
    getOrCreateUserObjectOfType<NodeVisitor,
                                ValueStack>( NodeVisitor* nv )
    {
        return nv->getOrCreateValueStack();
    }

    template<>
    inline ValueMap*
    getOrCreateUserObjectOfType<NodeVisitor,
                                ValueMap>( NodeVisitor* nv )
    {
        return nv->getOrCreateValueMap();
    }

}

#include <osg/core/InheritAcceptImpl.hpp>
