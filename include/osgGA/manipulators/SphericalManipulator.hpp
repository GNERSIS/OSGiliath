/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Spherical-coordinate camera manipulator. Orbits using azimuth
 * and elevation angles for globe-style navigation.
 */
#pragma once

#include <osg/maths/Math.hpp>
#include <osg/maths/Matrix.hpp>
#include <osg/maths/quat.hpp>
#include <osgGA/manipulators/CameraManipulator.hpp>

namespace osgGA
{

    class OSGGA_EXPORT SphericalManipulator : public CameraManipulator
    {
        public:

            SphericalManipulator();

            const char*
            className() const override
            {
                return "Spherical Manipulator";
            }

            /** set the position of the matrix manipulator using a 4x4 dmat4.*/
            void
            setByMatrix( const osg::dmat4& matrix ) override;

            /** set the position of the matrix manipulator using a 4x4 dmat4.*/
            void
            setByInverseMatrix( const osg::dmat4& matrix ) override
            {
                setByMatrix( osg::inverse( matrix ) );
            }

            /** get the position of the manipulator as 4x4 dmat4.*/
            osg::dmat4
            getMatrix() const override;

            /** get the position of the manipulator as a inverse matrix of the
             * manipulator, typically used as a model view matrix.*/
            osg::dmat4
            getInverseMatrix() const override;

            /** Get the FusionDistanceMode. Used by SceneView for setting up stereo
             * convergence.*/
            osgUtil::SceneView::FusionDistanceMode
            getFusionDistanceMode() const override
            {
                return osgUtil::SceneView::USE_FUSION_DISTANCE_VALUE;
            }

            /** Get the FusionDistanceValue. Used by SceneView for setting up stereo
             * convergence.*/
            float
            getFusionDistanceValue() const override
            {
                return static_cast<float>( _distance );
            }

            /** Attach a node to the manipulator.
            Automatically detaches previously attached node.
            setNode(NULL) detaches previously nodes.
            Is ignored by manipulators which do not require a reference model.*/
            void
            setNode( osg::Node* ) override;

            /** Return node if attached.*/
            const osg::Node*
            getNode() const override;

            /** Return node if attached.*/
            osg::Node*
            getNode() override;

            /** Move the camera to the default position.
            May be ignored by manipulators if home functionality is not appropriate.*/
            void
            home( const osgGA::GUIEventAdapter& ea,
                  osgGA::GUIActionAdapter&      us ) override;
            void
            home( double ) override;

            /** Start/restart the manipulator.*/
            void
            init( const osgGA::GUIEventAdapter& ea,
                  osgGA::GUIActionAdapter&      us ) override;

            void
            zoomOn( const osg::sphere& bound );

            /** handle events, return true if handled, false otherwise.*/
            bool
            handle( const osgGA::GUIEventAdapter& ea,
                    osgGA::GUIActionAdapter&      us ) override;

            /** Compute the home position.*/
            virtual void
            computeHomePosition();

            void
            computeHomePosition( const osg::Camera* camera,
                                 bool               useBoundingBox ) override
            {
                CameraManipulator::computeHomePosition( camera, useBoundingBox );
            }

            void
            computeViewPosition( const osg::sphere& bound,
                                 double&            scale,
                                 double&            distance,
                                 osg::dvec3&        center );

            void
            setCenter( const osg::dvec3& center )
            {
                _center = center;
            }

            const osg::dvec3&
            getCenter() const
            {
                return _center;
            }

            bool
            setDistance( double distance );

            double
            getDistance() const
            {
                return _distance;
            }

            double
            getHomeDistance() const
            {
                return _homeDistance;
            }

            void
            setHeading( double azimuth )
            {
                _heading = azimuth;
            }

            double
            getHeading() const
            {
                return _heading;
            }

            void
            setElevation( double elevation )
            {
                _elevation = elevation;
            }

            double
            getElevtion() const
            {
                return _elevation;
            }

            /** get the minimum distance (as ratio) the eye point can be zoomed in */
            double
            getMinimumZoomScale() const
            {
                return _minimumZoomScale;
            }

            /** set the minimum distance (as ratio) the eye point can be zoomed in
               towards the center before the center is pushed forward.*/
            void
            setMinimumZoomScale( double minimumZoomScale )
            {
                _minimumZoomScale = minimumZoomScale;
            }

            /** set the mouse scroll wheel zoom delta.
             * Range -1.0 to +1.0,  -ve value inverts wheel direction and zero switches
             * off scroll wheel. */
            void
            setScroolWheelZoomDelta( double zoomDelta )
            {
                _zoomDelta = zoomDelta;
            }

            /** get the mouse scroll wheel zoom delta. */
            double
            getScroolWheelZoomDelta() const
            {
                return _zoomDelta;
            }

            /** Get the keyboard and mouse usage of this manipulator.*/
            void
            getUsage( osg::ApplicationUsage& usage ) const override;

            enum RotationMode
            {
                ELEVATION_HEADING = 0,
                HEADING,
                ELEVATION,
                MAP,
            };

            RotationMode
            getRotationMode() const
            {
                return _rotationMode;
            }

            void
            setRotationMode( RotationMode mode );

            /** Returns true if the camera can be thrown, false otherwise. This defaults
             * to true. */
            bool
            getAllowThrow() const
            {
                return _allowThrow;
            }

            /** Set the 'allow throw' flag. Releasing the mouse button while moving the
             * camera results in a throw. */
            void
            setAllowThrow( bool allowThrow )
            {
                _allowThrow = allowThrow;
            }

        protected:

            virtual ~SphericalManipulator();

            /** Reset the internal GUIEvent stack.*/
            void
            flushMouseEventStack();
            /** Add the current mouse GUIEvent to internal stack.*/
            void
            addMouseEvent( const osgGA::GUIEventAdapter& ea );

            /** For the give mouse movement calculate the movement of the camera.
            Return true is camera has moved and a redraw is required.*/
            bool
            calcMovement();

            /** Check the speed at which the mouse is moving.
            If speed is below a threshold then return false, otherwise return true.*/
            bool
                                                       isMouseMoving();

            // Internal event stack comprising last two mouse events.
            osg::ref_ptr<const osgGA::GUIEventAdapter> _ga_t1;
            osg::ref_ptr<const osgGA::GUIEventAdapter> _ga_t0;

            osg::observer_ptr<osg::Node>               _node;

            double                                     _modelScale;
            double                                     _minimumZoomScale;

            bool                                       _thrown;
            bool                                       _allowThrow;

            /** The approximate amount of time it is currently taking to draw a frame.
             * This is used to compute the delta in translation/rotation during a thrown
             * display update. It allows us to match an delta in position/rotation
             * independent of the rendering frame rate.
             */
            double                                     _delta_frame_time;

            /** The time the last frame started.
             * Used when _rate_sensitive is true so that we can match display update rate
             * to rotation/translation rate.
             */
            double                                     _last_frame_time;

            RotationMode                               _rotationMode;
            osg::dvec3                                 _center;
            double                                     _distance;
            double _heading;      // angle from x axis in xy plane
            double _elevation;    // angle from xy plane, positive upwards towards the z
                                  // axis
            double _homeDistance;
            double _zoomDelta;
    };

}
