/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract base for 3D manipulation draggers. Provides hit testing,
 * constraint application, and command generation for interactive editing.
 */
// osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/maths/sphere.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osgGA/events/GUIActionAdapter.hpp>
#include <osgGA/events/GUIEventAdapter.hpp>
#include <osgManipulator/Command.hpp>
#include <osgManipulator/Constraint.hpp>
#include <osgUtil/culling/SceneView.hpp>

namespace osgManipulator
{

    class CompositeDragger;
    class MotionCommand;
    class TranslateInLineCommand;
    class TranslateInPlaneCommand;
    class Scale1DCommand;
    class Scale2DCommand;
    class ScaleUniformCommand;
    class Rotate3DCommand;

    /** Computes the nodepath from the given node all the way up to the root. */
    extern OSGMANIPULATOR_EXPORT void
    computeNodePathToRoot( osg::Node&     node,
                           osg::NodePath& np );

    class OSGMANIPULATOR_EXPORT DraggerTransformCallback : public DraggerCallback
    {
        public:

            enum HandleCommandMask
            {
                HANDLE_TRANSLATE_IN_LINE  = 1 << 0,
                HANDLE_TRANSLATE_IN_PLANE = 1 << 1,
                HANDLE_SCALED_1D          = 1 << 2,
                HANDLE_SCALED_2D          = 1 << 3,
                HANDLE_SCALED_UNIFORM     = 1 << 4,
                HANDLE_ROTATE_3D          = 1 << 5,
                HANDLE_ALL                = 0X8'FF'FF'FF
            };

            DraggerTransformCallback( osg::MatrixTransform* transform,
                                      int handleCommandMask = HANDLE_ALL );

            virtual bool
            receive( const MotionCommand& );
            virtual bool
            receive( const TranslateInLineCommand& command );
            virtual bool
            receive( const TranslateInPlaneCommand& command );
            virtual bool
            receive( const Scale1DCommand& command );
            virtual bool
            receive( const Scale2DCommand& command );
            virtual bool
            receive( const ScaleUniformCommand& command );
            virtual bool
            receive( const Rotate3DCommand& command );

            osg::MatrixTransform*
            getTransform()
            {
                return _transform.get();
            }

            const osg::MatrixTransform*
            getTransform() const
            {
                return _transform.get();
            }

        protected:

            unsigned int                            _handleCommandMask;

            osg::observer_ptr<osg::MatrixTransform> _transform;
            osg::dmat4                              _startMotionMatrix;

            osg::dmat4                              _localToWorld;
            osg::dmat4                              _worldToLocal;
    };

    class OSGMANIPULATOR_EXPORT PointerInfo
    {
        public:

            PointerInfo();

            PointerInfo( const PointerInfo& rhs ) :
                _hitList( rhs._hitList ),
                _nearPoint( rhs._nearPoint ),
                _farPoint( rhs._farPoint ),
                _eyeDir( rhs._eyeDir )
            {
                _hitIter = _hitList.begin();
            }

            void
            reset()
            {
                _hitList.clear();
                _hitIter = _hitList.begin();
                setCamera( 0 );
            }

            bool
            completed() const
            {
                return _hitIter == _hitList.end();
            }

            void
            next()
            {
                if( !completed() )
                {
                    ++_hitIter;
                }
            }

            typedef std::pair<osg::NodePath, osg::dvec3> NodePathIntersectionPair;
            typedef std::list<NodePathIntersectionPair>  IntersectionList;

            osg::dvec3
            getLocalIntersectPoint() const
            {
                return _hitIter->second;
            }

            void
            setNearFarPoints( osg::dvec3 nearPoint,
                              osg::dvec3 farPoint )
            {
                _nearPoint = nearPoint;
                _farPoint  = farPoint;
                _eyeDir    = farPoint - nearPoint;
            }

            const osg::dvec3&
            getEyeDir() const
            {
                return _eyeDir;
            }

            void
            getNearFarPoints( osg::dvec3& nearPoint,
                              osg::dvec3& farPoint ) const
            {
                nearPoint = _nearPoint;
                farPoint  = _farPoint;
            }

            bool
            contains( const osg::Node* node ) const;

            void
            setCamera( osg::Camera* camera )
            {

                if( camera )
                {
                    _MVPW = camera->getProjectionMatrix() * camera->getViewMatrix();
                    if( camera->getViewport() )
                    {
                        osg::postMult( _MVPW,
                                       camera->getViewport()->computeWindowMatrix() );
                    }
                    _inverseMVPW = osg::inverse( _MVPW );
                    osg::dvec3 eye, center, up;
                    osg::getLookAt( camera->getViewMatrix(), eye, center, up );
                    _eyeDir = eye - center;
                }
                else
                {
                    _MVPW        = osg::dmat4();
                    _inverseMVPW = osg::dmat4();
                    _eyeDir      = osg::dvec3( 0, 0, 1 );
                }
            }

            void
            addIntersection( const osg::NodePath& nodePath,
                             const osg::dvec3&    intersectionPoint )
            {
                bool needToResetHitIter = _hitList.empty();
                _hitList.push_back( NodePathIntersectionPair( nodePath,
                                                              intersectionPoint ) );
                if( needToResetHitIter )
                {
                    _hitIter = _hitList.begin();
                }
            }

            void
            setMousePosition( float pixel_x,
                              float pixel_y )
            {
                projectWindowXYIntoObject( osg::dvec2( pixel_x, pixel_y ),
                                           _nearPoint,
                                           _farPoint );
            }

        protected:

            bool
            projectWindowXYIntoObject( const osg::dvec2& windowCoord,
                                       osg::dvec3&       nearPoint,
                                       osg::dvec3&       farPoint ) const;

        public:

            IntersectionList                 _hitList;
            IntersectionList::const_iterator _hitIter;

        protected:

            osg::dvec3 _nearPoint, _farPoint;
            osg::dvec3 _eyeDir;

            osg::dmat4 _MVPW;
            osg::dmat4 _inverseMVPW;
    };

    /**
     * Base class for draggers. Concrete draggers implement the pick event handler
     * and generate motion commands (translate, rotate, ...) and sends these
     * command to all the DraggerCallbacks & Transforms that are connected to the Dragger
     * that generates the commands.
     */
    class OSGMANIPULATOR_EXPORT Dragger
        : public osg::Inherit<osg::MatrixTransform, Dragger>
    {
        public:

            OSG_REGISTER_TYPE( osgManipulator,
                               Dragger )

            /**
             * Set/Get parent dragger. For simple draggers parent points to itself.
             * For composite draggers parent points to the parent dragger that uses
             * this dragger.
             */
            virtual void
            setParentDragger( Dragger* parent )
            {
                _parentDragger = parent;
            }

            Dragger*
            getParentDragger()
            {
                return _parentDragger;
            }

            const Dragger*
            getParentDragger() const
            {
                return _parentDragger;
            }

            /** Returns 0 if this Dragger is not a CompositeDragger. */
            virtual const CompositeDragger*
            getComposite() const
            {
                return 0;
            }

            /** Returns 0 if this Dragger is not a CompositeDragger. */
            virtual CompositeDragger*
            getComposite()
            {
                return 0;
            }

            void
            setHandleEvents( bool flag );

            bool
            getHandleEvents() const
            {
                return _handleEvents;
            }

            void
            setActivationModKeyMask( unsigned int mask )
            {
                _activationModKeyMask = mask;
            }

            unsigned int
            getActivationModKeyMask() const
            {
                return _activationModKeyMask;
            }

            void
            setActivationMouseButtonMask( unsigned int mask )
            {
                _activationMouseButtonMask = mask;
            }

            unsigned int
            getActivationMouseButtonMask() const
            {
                return _activationMouseButtonMask;
            }

            void
            setActivationKeyEvent( int key )
            {
                _activationKeyEvent = key;
            }

            int
            getActivationKeyEvent() const
            {
                return _activationKeyEvent;
            }

            virtual void
            traverse( osg::NodeVisitor& nv );

            virtual bool
            handle( const osgGA::GUIEventAdapter& ea,
                    osgGA::GUIActionAdapter&      aa );

            virtual bool
            handle( const PointerInfo&,
                    const osgGA::GUIEventAdapter&,
                    osgGA::GUIActionAdapter& )
            {
                return false;
            }

            typedef std::vector<osg::ref_ptr<Constraint>> Constraints;

            void
            addConstraint( Constraint* constraint );

            template<class T>
            void
            addConstraint( const osg::ref_ptr<T>& c )
            {
                addConstraint( c.get() );
            }

            void
            removeConstraint( Constraint* constraint );

            template<class T>
            void
            removeConstraint( const osg::ref_ptr<T>& c )
            {
                removeConstraint( c.get() );
            }

            Constraints&
            getConstraints()
            {
                return _constraints;
            }

            const Constraints&
            getConstraints() const
            {
                return _constraints;
            }

            typedef std::vector<osg::ref_ptr<DraggerCallback>> DraggerCallbacks;

            void
            addDraggerCallback( DraggerCallback* dc );

            template<class T>
            void
            addDraggerCallback( const osg::ref_ptr<T>& dc )
            {
                addDraggerCallback( dc.get() );
            }

            void
            removeDraggerCallback( DraggerCallback* dc );

            template<class T>
            void
            removeDraggerCallback( const osg::ref_ptr<T>& dc )
            {
                removeDraggerCallback( dc.get() );
            }

            DraggerCallbacks&
            getDraggerCallbacks()
            {
                return _draggerCallbacks;
            }

            const DraggerCallbacks&
            getDraggerCallbacks() const
            {
                return _draggerCallbacks;
            }

            void
            addTransformUpdating( MatrixTransform* transform,
                                  int              handleCommandMask =
                                      DraggerTransformCallback::HANDLE_ALL );
            void
            removeTransformUpdating( MatrixTransform* transform );

            /** Setup default geometry for dragger. */
            virtual void
            setupDefaultGeometry()
            {
            }

            virtual bool
            receive( const MotionCommand& command );
            virtual void
            dispatch( MotionCommand& command );

            void
            setDraggerActive( bool active )
            {
                _draggerActive = active;
            }

            bool
            getDraggerActive() const
            {
                return _draggerActive;
            }

            /**
             * Set/Get the traversal mask used by this dragger when looking for
             * intersections during event handling. This is useful to "hide" some
             * geometry during event handling.
             */
            virtual void
            setIntersectionMask( osg::Node::NodeMask intersectionMask )
            {
                _intersectionMask = intersectionMask;
            }

            osg::Node::NodeMask
            getIntersectionMask() const
            {
                return _intersectionMask;
            }

            /** Return true if the axis of the Locator are inverted requiring the faces
             * of any cubes used from rendering to be flipped to ensure the correct
             * front/back face is used.*/
            bool
            inverted() const;

            /** apply the appropriate FrontFace setting to provided StateSet to ensure
             * that the rendering of hull of the volume is the correct orientation.*/
            void
            applyAppropriateFrontFace( osg::StateSet* ss ) const;

            Dragger();
            Dragger( const Dragger&     rhs,
                     const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

        protected:

            virtual ~Dragger();

            bool                          _handleEvents;
            bool                          _draggerActive;

            unsigned int                  _activationModKeyMask;
            unsigned int                  _activationMouseButtonMask;
            int                           _activationKeyEvent;
            bool                          _activationPermittedByModKeyMask;
            bool                          _activationPermittedByMouseButtonMask;
            bool                          _activationPermittedByKeyEvent;

            osgManipulator::PointerInfo   _pointer;

            Dragger*                      _parentDragger;

            osg::ref_ptr<DraggerCallback> _selfUpdater;
            Constraints                   _constraints;
            DraggerCallbacks              _draggerCallbacks;
            osg::Node::NodeMask           _intersectionMask;
    };

    /**
     * CompositeDragger allows to create complex draggers that are composed of a
     * hierarchy of Draggers.
     */
    class OSGMANIPULATOR_EXPORT CompositeDragger
        : public osg::Inherit<Dragger, CompositeDragger>
    {
        public:

            OSG_REGISTER_TYPE( osgManipulator,
                               CompositeDragger )

            typedef std::vector<osg::ref_ptr<Dragger>> DraggerList;

            virtual const CompositeDragger*
            getComposite() const
            {
                return this;
            }

            virtual CompositeDragger*
            getComposite()
            {
                return this;
            }

            virtual void
            setParentDragger( Dragger* parent );

            virtual bool
            handle( const PointerInfo&            pi,
                    const osgGA::GUIEventAdapter& ea,
                    osgGA::GUIActionAdapter&      aa );

            // Composite-specific methods below
            virtual bool
            addDragger( Dragger* dragger );

            template<class T>
            bool
            addDragger( const osg::ref_ptr<T>& dc )
            {
                return addDragger( dc.get() );
            }

            virtual bool
            removeDragger( Dragger* dragger );

            template<class T>
            bool
            removeDragger( const osg::ref_ptr<T>& dc )
            {
                return removeDragger( dc.get() );
            }

            unsigned int
            getNumDraggers() const
            {
                return static_cast<unsigned int>( _draggerList.size() );
            }

            Dragger*
            getDragger( unsigned int i )
            {
                return _draggerList[i].get();
            }

            const Dragger*
            getDragger( unsigned int i ) const
            {
                return _draggerList[i].get();
            }

            bool
            containsDragger( const Dragger* dragger ) const;

            template<class T>
            bool
            containsDragger( const osg::ref_ptr<T>& dc ) const
            {
                return containsDragger( dc.get() );
            }

            DraggerList::iterator
            findDragger( const Dragger* dragger );

            virtual void
            setIntersectionMask( osg::Node::NodeMask intersectionMask );

            CompositeDragger()
            {
            }

            CompositeDragger( const CompositeDragger& rhs,
                              const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

        protected:

            virtual ~CompositeDragger()
            {
            }

            DraggerList _draggerList;
    };

    /**
     * Culls the drawable all the time. Used by draggers to have invisible geometry
     * around lines and points so that they can be picked. For example, a dragger
     * could have a line with an invisible cylinder around it to enable picking on
     * that line.
     */
    void OSGMANIPULATOR_EXPORT
    setDrawableToAlwaysCull( osg::Drawable& drawable );

    /**
     * Convenience function for setting the material color on a node.
     */
    void OSGMANIPULATOR_EXPORT
    setMaterialColor( const osg::vec4& color,
                      osg::Node&       node );

}
