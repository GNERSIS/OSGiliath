/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * UFO-style camera manipulator. Free 3D movement with no gravity
 * or terrain constraints for unrestricted scene exploration.
 */
#pragma once

#include <iostream>
#include <osg/maths/mat4.hpp>
#include <osg/nodes/Node.hpp>
#include <osgGA/manipulators/CameraManipulator.hpp>

/**
  \class osgGA::UFOManipulator
  \brief A UFO manipulator driven with keybindings.

  The UFOManipulator is better suited for applications that employ
  architectural walk-throughs, or situations where the eyepoint motion
  model must move slowly, deliberately and well controlled.

  The UFO Manipulator allows the following movements with the listed
  Key combinations:
    \param UpArrow          Acceleration forward.
    \param DownArrow        Acceleration backward (or deceleration forward).
    \param LeftArrow        Rotate view and direction of travel to the left.
    \param RightArrow       Rotate view and direction of travel to the right.
    \param SpaceBar         Brake.  Gradually decelerates linear and rotational movement.
    \param Shift/UpArrow    Accelerate up.
    \param Shift/DownArrow  Accelerate down.
    \param Shift/LeftArrow  Accelerate (linearly) left.
    \param Shift/RightArrow Accelerate (linearly) right.
    \param Shift/SpaceBar   Instant brake.  Immediately stop all linear and rotational
movement.

When the Shift key is released, up, down, linear left and/or linear right movement is
decelerated.

    \param Ctrl/UpArrow     Rotate view (but not direction of travel) up.
    \param Ctrl/DownArrow   Rotate view (but not direction of travel) down.
    \param Ctrl/LeftArrow   Rotate view (but not direction of travel) left.
    \param Ctrl/RightArrow  Rotate view (but not direction of travel) right.
    \param Ctrl/Return      Straightens out the view offset.

*/

namespace osgGA
{

    class OSGGA_EXPORT UFOManipulator : public osgGA::CameraManipulator
    {

        public:

            /** Default constructor */
            UFOManipulator();

            /** return className
              \return returns constant "UFO"
              */
            const char*
            className() const override;

            /** Set the current position with a matrix
              \param matrix  A viewpoint matrix.
             */
            void
            setByMatrix( const osg::dmat4& matrix ) override;

            /** Set the current position with the inverse matrix
              \param invmat The inverse of a viewpoint matrix
              */
            void
            setByInverseMatrix( const osg::dmat4& invmat ) override;

            /** Get the current viewmatrix */
            osg::dmat4
            getMatrix() const override;

            /** Get the current inverse view matrix */
            osg::dmat4
            getInverseMatrix() const override;

            /** Set the  subgraph this manipulator is driving the eye through.
                \param node     root of subgraph
             */
            void
            setNode( osg::Node* node ) override;

            /** Get the root node of the subgraph this manipulator is driving the eye
             * through (const)*/
            const osg::Node*
            getNode() const override;

            /** Get the root node of the subgraph this manipulator is driving the eye
             * through */
            osg::Node*
            getNode() override;

            /** Computes the home position based on the extents and scale of the
                scene graph rooted at node */
            virtual void
            computeHomePosition();

            void
            computeHomePosition( const osg::Camera* camera,
                                 bool               useBoundingBox ) override
            {
                CameraManipulator::computeHomePosition( camera, useBoundingBox );
            }

            /** Sets the viewpoint matrix to the home position */
            void
            home( const osgGA::GUIEventAdapter&,
                  osgGA::GUIActionAdapter& ) override;
            void
            home( double ) override;

            void
            init( const GUIEventAdapter&,
                  GUIActionAdapter& ) override;

            /** Handles incoming osgGA events */
            bool
            handle( const osgGA::GUIEventAdapter& ea,
                    osgGA::GUIActionAdapter&      aa ) override;

            /** Reports Usage parameters to the application */
            void
            getUsage( osg::ApplicationUsage& usage ) const override;

            /** Report the current position as LookAt vectors */
            void
            getCurrentPositionAsLookAt( osg::dvec3& eye,
                                        osg::dvec3& center,
                                        osg::dvec3& up );

            void
            setMinHeight( double in_min_height )
            {
                _minHeightAboveGround = in_min_height;
            }

            double
            getMinHeight() const
            {
                return _minHeightAboveGround;
            }

            void
            setMinDistance( double in_min_dist )
            {
                _minDistanceInFront = in_min_dist;
            }

            double
            getMinDistance() const
            {
                return _minDistanceInFront;
            }

            void
            setForwardSpeed( double in_fs )
            {
                _forwardSpeed = in_fs;
            }

            double
            getForwardSpeed() const
            {
                return _forwardSpeed;
            }

            void
            setSideSpeed( double in_ss )
            {
                _sideSpeed = in_ss;
            }

            double
            getSideSpeed() const
            {
                return _sideSpeed;
            }

            void
            setRotationSpeed( double in_rot_speed )
            {
                _directionRotationRate = in_rot_speed;
            }

            double
            getRotationSpeed() const
            {
                return _directionRotationRate;
            }

        protected:

            virtual ~UFOManipulator();

            bool
                                         intersect( const osg::dvec3& start,
                                                    const osg::dvec3& end,
                                                    osg::dvec3&       intersection ) const;

            osg::observer_ptr<osg::Node> _node;
            osg::dmat4                   _matrix;
            osg::dmat4                   _inverseMatrix;
            osg::dmat4                   _offset;

            double                       _minHeightAboveGround;
            double                       _minDistanceInFront;

            double                       _speedEpsilon;
            double                       _forwardSpeed;
            double                       _sideSpeed;
            double                       _upSpeed;
            double                       _speedAccelerationFactor;
            double                       _speedDecelerationFactor;

            bool                         _decelerateUpSideRate;

            double                       _directionRotationEpsilon;
            double                       _directionRotationRate;
            double                       _directionRotationAcceleration;
            double                       _directionRotationDeceleration;

            double                       _viewOffsetDelta;
            double                       _pitchOffsetRate;
            double                       _pitchOffset;
            double                       _yawOffsetRate;
            double                       _yawOffset;

            double                       _t0;
            double                       _dt;
            osg::dvec3                   _direction;
            osg::dvec3                   _position;

            bool                         _shift;
            bool                         _ctrl;
            bool                         _decelerateOffsetRate;

            bool                         _straightenOffset;

            void
            _stop();
            void
            _keyDown( const osgGA::GUIEventAdapter& ea,
                      osgGA::GUIActionAdapter& );
            void
            _keyUp( const osgGA::GUIEventAdapter& ea,
                    osgGA::GUIActionAdapter& );
            void
            _frame( const osgGA::GUIEventAdapter& ea,
                    osgGA::GUIActionAdapter& );

            void
            _adjustPosition();
    };

}
