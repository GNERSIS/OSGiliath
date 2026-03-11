/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Flight-simulator camera manipulator. Provides pitch/yaw/roll
 * with continuous forward motion for fly-through navigation.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osgGA/manipulators/FirstPersonManipulator.hpp>

namespace osgGA
{

    /** FlightManipulator is a CameraManipulator which provides flight simulator-like
     *  updating of the camera position & orientation. By default, the left mouse
     *  button accelerates, the right mouse button decelerates, and the middle mouse
     *  button (or left and right simultaneously) stops dead.
     */
    class OSGGA_EXPORT FlightManipulator
        : public osg::Inherit<FirstPersonManipulator, FlightManipulator>
    {
            typedef FirstPersonManipulator inherited;

        public:

            enum YawControlMode
            {
                YAW_AUTOMATICALLY_WHEN_BANKED,
                NO_AUTOMATIC_YAW,
            };

            FlightManipulator( int flags = UPDATE_MODEL_SIZE | COMPUTE_HOME_USING_BBOX );
            FlightManipulator( const FlightManipulator& fpm,
                               const osg::CopyOp& copyOp = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgGA,
                               FlightManipulator )

            virtual void
            setYawControlMode( YawControlMode ycm );
            inline YawControlMode
            getYawControlMode() const;

            virtual void
            home( const osgGA::GUIEventAdapter& ea,
                  osgGA::GUIActionAdapter&      us );
            virtual void
            init( const osgGA::GUIEventAdapter& ea,
                  osgGA::GUIActionAdapter&      us );
            virtual void
            getUsage( osg::ApplicationUsage& usage ) const;

        protected:

            virtual bool
            handleFrame( const osgGA::GUIEventAdapter& ea,
                         osgGA::GUIActionAdapter&      us );
            virtual bool
            handleMouseMove( const osgGA::GUIEventAdapter& ea,
                             osgGA::GUIActionAdapter&      us );
            virtual bool
            handleMouseDrag( const osgGA::GUIEventAdapter& ea,
                             osgGA::GUIActionAdapter&      us );
            virtual bool
            handleMousePush( const osgGA::GUIEventAdapter& ea,
                             osgGA::GUIActionAdapter&      us );
            virtual bool
            handleMouseRelease( const osgGA::GUIEventAdapter& ea,
                                osgGA::GUIActionAdapter&      us );
            virtual bool
            handleKeyDown( const osgGA::GUIEventAdapter& ea,
                           osgGA::GUIActionAdapter&      us );
            virtual bool
            flightHandleEvent( const osgGA::GUIEventAdapter& ea,
                               osgGA::GUIActionAdapter&      us );

            virtual bool
            performMovement();
            virtual bool
            performMovementLeftMouseButton( const double eventTimeDelta,
                                            const double dx,
                                            const double dy );
            virtual bool
            performMovementMiddleMouseButton( const double eventTimeDelta,
                                              const double dx,
                                              const double dy );
            virtual bool
                           performMovementRightMouseButton( const double eventTimeDelta,
                                                            const double dx,
                                                            const double dy );

            YawControlMode _yawMode;
    };

    //
    //  inline methods
    //

    /// Returns the Yaw control for the flight model.
    inline FlightManipulator::YawControlMode
    FlightManipulator::getYawControlMode() const
    {
        return _yawMode;
    }

}
