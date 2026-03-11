/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Ground-vehicle camera manipulator. Provides steering and
 * elevation-following for drive-through navigation.
 */
#pragma once

#include <osg/maths/compat.hpp>
#include <osg/maths/quat.hpp>
#include <osgGA/manipulators/CameraManipulator.hpp>

namespace osgGA
{

    /**
    DriveManipulator is a camera manipulator which provides drive-like
    functionality. By default, the left mouse button accelerates, the right
    mouse button decelerates, and the middle mouse button (or left and
    right simultaneously) stops dead.
    */

    class OSGGA_EXPORT DriveManipulator : public CameraManipulator
    {
        public:

            DriveManipulator();

            const char*
            className() const override
            {
                return "Drive";
            }

            /** Get the position of the matrix manipulator using a 4x4 dmat4.*/
            void
            setByMatrix( const osg::dmat4& matrix ) override;

            /** Set the position of the matrix manipulator using a 4x4 dmat4.*/
            void
            setByInverseMatrix( const osg::dmat4& matrix ) override
            {
                setByMatrix( osg::inverse( matrix ) );
            }

            /** Get the position of the manipulator as 4x4 dmat4.*/
            osg::dmat4
            getMatrix() const override;

            /** Get the position of the manipulator as a inverse matrix of the
             * manipulator, typically used as a model view matrix.*/
            osg::dmat4
            getInverseMatrix() const override;

            void
            setNode( osg::Node* ) override;

            const osg::Node*
            getNode() const override;

            osg::Node*
            getNode() override;

            virtual void
            computeHomePosition();

            void
            computeHomePosition( const osg::Camera* camera,
                                 bool               useBoundingBox ) override
            {
                CameraManipulator::computeHomePosition( camera, useBoundingBox );
            }

            void
            home( const GUIEventAdapter& ea,
                  GUIActionAdapter&      us ) override;

            void
            init( const GUIEventAdapter& ea,
                  GUIActionAdapter&      us ) override;

            bool
            handle( const GUIEventAdapter& ea,
                    GUIActionAdapter&      us ) override;

            /** Get the keyboard and mouse usage of this manipulator.*/
            void
            getUsage( osg::ApplicationUsage& usage ) const override;

            void
            setModelScale( double in_ms )
            {
                _modelScale = in_ms;
            }

            double
            getModelScale() const
            {
                return _modelScale;
            }

            void
            setVelocity( double in_vel )
            {
                _velocity = in_vel;
            }

            double
            getVelocity() const
            {
                return _velocity;
            }

            void
            setHeight( double in_h )
            {
                _height = in_h;
            }

            double
            getHeight() const
            {
                return _height;
            }

        protected:

            virtual ~DriveManipulator();

            bool
            intersect( const osg::dvec3& start,
                       const osg::dvec3& end,
                       osg::dvec3&       intersection,
                       osg::dvec3&       normal ) const;

            /** Reset the internal GUIEvent stack.*/
            void
            flushMouseEventStack();

            /** Add the current mouse GUIEvent to internal stack.*/
            void
            addMouseEvent( const GUIEventAdapter& ea );

            void
            computePosition( const osg::dvec3& eye,
                             const osg::dvec3& lv,
                             const osg::dvec3& up );

            /** For the given mouse movement calculate the movement of the camera.
             * Return true if camera has moved and a redraw is required.
             */
            bool
                                                calcMovement();

            // Internal event stack comprising last two mouse events.
            osg::ref_ptr<const GUIEventAdapter> _ga_t1;
            osg::ref_ptr<const GUIEventAdapter> _ga_t0;

            osg::observer_ptr<osg::Node>        _node;

            double                              _modelScale;
            double                              _velocity;
            double                              _height;
            double                              _buffer;

            enum SpeedControlMode
            {
                USE_MOUSE_Y_FOR_SPEED,
                USE_MOUSE_BUTTONS_FOR_SPEED,
            };

            SpeedControlMode _speedMode;

            osg::dvec3       _eye;
            osg::quat        _rotation;
            double           _pitch;
            double           _distance;

            bool             _pitchUpKeyPressed;
            bool             _pitchDownKeyPressed;
    };

}
