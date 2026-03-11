/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract interface for camera manipulators. Defines the contract
 * for setting view matrices from user input and animation.
 */
#pragma once

#include <osg/maths/mat4.hpp>
#include <osg/nodes/CoordinateSystemNode.hpp>
#include <osg/nodes/Node.hpp>
#include <osgGA/events/GUIActionAdapter.hpp>
#include <osgGA/events/GUIEventAdapter.hpp>
#include <osgGA/events/GUIEventHandler.hpp>
#include <osgGA/Export>
#include <osgUtil/culling/SceneView.hpp>

namespace osgGA
{

#define NEW_HOME_POSITION

    /**

    CameraManipulator is an abstract base class defining the interface, and a certain
    amount of default functionality, for classes which wish to control OSG cameras
    in response to GUI events.

    */
    class OSGGA_EXPORT CameraManipulator : public GUIEventHandler
    {
            typedef GUIEventHandler inherited;

        public:

            // We are not using Inherit<>/OSG_REGISTER_TYPE as this is abstract class.
            // Use Inherit<> + OSG_REGISTER_TYPE(osgGA,YourManipulator) in your
            // descendant non-abstract classes.
            virtual const char*
            className() const
            {
                return "CameraManipulator";
            }

            /** callback class to use to allow matrix manipulators to query the
             * application for the local coordinate frame.*/
            class CoordinateFrameCallback : public osg::Referenced
            {
                public:

                    virtual osg::CoordinateFrame
                    getCoordinateFrame( const osg::dvec3& position ) const = 0;

                protected:

                    virtual ~CoordinateFrameCallback()
                    {
                    }
            };

            /** set the coordinate frame which callback tells the manipulator which way
             * is up, east and north.*/
            virtual void
            setCoordinateFrameCallback( CoordinateFrameCallback* cb )
            {
                _coordinateFrameCallback = cb;
            }

            /** get the coordinate frame callback which tells the manipulator which way
             * is up, east and north.*/
            CoordinateFrameCallback*
            getCoordinateFrameCallback()
            {
                return _coordinateFrameCallback.get();
            }

            /** get the coordinate frame callback which tells the manipulator which way
             * is up, east and north.*/
            const CoordinateFrameCallback*
            getCoordinateFrameCallback() const
            {
                return _coordinateFrameCallback.get();
            }

            /** get the coordinate frame.*/
            osg::CoordinateFrame
            getCoordinateFrame( const osg::dvec3& position ) const
            {
                if( _coordinateFrameCallback.valid() )
                {
                    return _coordinateFrameCallback->getCoordinateFrame( position );
                }
                return osg::CoordinateFrame();
            }

            osg::dvec3
            getSideVector( const osg::CoordinateFrame& cf ) const
            {
                return osg::dvec3( cf( 0, 0 ), cf( 0, 1 ), cf( 0, 2 ) );
            }

            osg::dvec3
            getFrontVector( const osg::CoordinateFrame& cf ) const
            {
                return osg::dvec3( cf( 1, 0 ), cf( 1, 1 ), cf( 1, 2 ) );
            }

            osg::dvec3
            getUpVector( const osg::CoordinateFrame& cf ) const
            {
                return osg::dvec3( cf( 2, 0 ), cf( 2, 1 ), cf( 2, 2 ) );
            }

            /** set the position of the matrix manipulator using a 4x4 dmat4.*/
            virtual void
            setByMatrix( const osg::dmat4& matrix ) = 0;

            /** set the position of the matrix manipulator using a 4x4 dmat4.*/
            virtual void
            setByInverseMatrix( const osg::dmat4& matrix ) = 0;

            /** get the position of the manipulator as 4x4 dmat4.*/
            virtual osg::dmat4
            getMatrix() const = 0;

            /** get the position of the manipulator as a inverse matrix of the
             * manipulator, typically used as a model view matrix.*/
            virtual osg::dmat4
            getInverseMatrix() const = 0;

            /** update the camera for the current frame, typically called by the viewer
               classes. Default implementation simply set the camera view matrix. */
            virtual void
            updateCamera( osg::Camera& camera )
            {
                camera.setViewMatrix( getInverseMatrix() );
            }

            /** Get the FusionDistanceMode. Used by SceneView for setting up stereo
             * convergence.*/
            virtual osgUtil::SceneView::FusionDistanceMode
            getFusionDistanceMode() const
            {
                return osgUtil::SceneView::PROPORTIONAL_TO_SCREEN_DISTANCE;
            }

            /** Get the FusionDistanceValue. Used by SceneView for setting up stereo
             * convergence.*/
            virtual float
            getFusionDistanceValue() const
            {
                return 1.0F;
            }

            /** Set the mask to use when set up intersection traversal such as used in
             * manNEW_HOME_POSITIONipulators that follow terrain or have collision
             * detection. The intersection traversal mask is useful for controlling what
             * parts of the scene graph should be used for intersection purposes.*/
            void
            setIntersectTraversalMask( unsigned int mask )
            {
                _intersectTraversalMask = mask;
            }

            /** Get the mask to use when set up intersection traversal such as used in
             * manipulators that follow terrain or have collision detection.*/
            unsigned int
            getIntersectTraversalMask() const
            {
                return _intersectTraversalMask;
            }

            /**
            Attach a node to the manipulator, automatically detaching any previously
            attached node. setNode(NULL) detaches previous nodes. May be ignored by
            manipulators which do not require a reference model.
            */
            virtual void
            setNode( osg::Node* )
            {
            }

            /** Return const node if attached.*/
            virtual const osg::Node*
            getNode() const
            {
                return NULL;
            }

            /** Return node if attached.*/
            virtual osg::Node*
            getNode()
            {
                return NULL;
            }

            /** Manually set the home position, and set the automatic compute of home
             * position. */
            virtual void
            setHomePosition( const osg::dvec3& eye,
                             const osg::dvec3& center,
                             const osg::dvec3& up,
                             bool              autoComputeHomePosition = false )
            {
                setAutoComputeHomePosition( autoComputeHomePosition );
                _homeEye    = eye;
                _homeCenter = center;
                _homeUp     = up;
            }

            /** Get the manually set home position. */
            virtual void
            getHomePosition( osg::dvec3& eye,
                             osg::dvec3& center,
                             osg::dvec3& up ) const
            {
                eye    = _homeEye;
                center = _homeCenter;
                up     = _homeUp;
            }

            /** Set whether the automatic compute of the home position is enabled.*/
            virtual void
            setAutoComputeHomePosition( bool flag )
            {
                _autoComputeHomePosition = flag;
            }

            /** Get whether the automatic compute of the home position is enabled.*/
            bool
            getAutoComputeHomePosition() const
            {
                return _autoComputeHomePosition;
            }

            /** Compute the home position.*/
            virtual void
            computeHomePosition( const osg::Camera* camera         = NULL,
                                 bool               useBoundingBox = false );

            /** finish any active manipulator animations.*/
            virtual void
            finishAnimation()
            {
            }

            /**
            Move the camera to the default position.
            May be ignored by manipulators if home functionality is not appropriate.
            */
            virtual void
            home( const GUIEventAdapter&,
                  GUIActionAdapter& )
            {
            }

            /**
            Move the camera to the default position.
            This version does not require GUIEventAdapter and GUIActionAdapter so may be
            called from somewhere other than a handle() method in GUIEventHandler.
            Application must be aware of implications.
            */
            virtual void
            home( double /*currentTime*/ )
            {
            }

            /**
            Start/restart the manipulator.
            */
            virtual void
            init( const GUIEventAdapter&,
                  GUIActionAdapter& )
            {
            }

            /** Handle event. Override the handle(..) method in your event handlers to
             * respond to events. */
            virtual bool
            handle( osgGA::Event*     event,
                    osg::Object*      object,
                    osg::NodeVisitor* nv )
            {
                return GUIEventHandler::handle( event, object, nv );
            }

            /** Handle events, return true if handled, false otherwise. */
            virtual bool
            handle( const GUIEventAdapter& ea,
                    GUIActionAdapter&      us );

        protected:

            CameraManipulator();
            CameraManipulator( const CameraManipulator& mm,
                               const osg::CopyOp& copyOp = osg::CopyOp::SHALLOW_COPY );

            virtual ~CameraManipulator();

            std::string
                                                  getManipulatorName() const;

            unsigned int                          _intersectTraversalMask;

            bool                                  _autoComputeHomePosition;

            osg::dvec3                            _homeEye;
            osg::dvec3                            _homeCenter;
            osg::dvec3                            _homeUp;

            osg::ref_ptr<CoordinateFrameCallback> _coordinateFrameCallback;
    };

}
