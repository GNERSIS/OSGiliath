/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract base for standard camera manipulators. Provides common
 * mouse-button mapping, animation, and home-position logic.
 */
#pragma once

#include <osgGA/manipulators/CameraManipulator.hpp>

namespace osgGA
{

    /** StandardManipulator class provides basic functionality
        for user controlled manipulation.*/
    class OSGGA_EXPORT StandardManipulator : public CameraManipulator
    {
            typedef CameraManipulator inherited;

        public:

            // flags
            enum UserInteractionFlags
            {
                UPDATE_MODEL_SIZE                    = 0X01,
                COMPUTE_HOME_USING_BBOX              = 0X02,
                PROCESS_MOUSE_WHEEL                  = 0X04,
                SET_CENTER_ON_WHEEL_FORWARD_MOVEMENT = 0X08,
                DEFAULT_SETTINGS = UPDATE_MODEL_SIZE /*| COMPUTE_HOME_USING_BBOX*/ |
                    PROCESS_MOUSE_WHEEL,
            };

            StandardManipulator( int flags = DEFAULT_SETTINGS );
            StandardManipulator( const StandardManipulator& m,
                                 const osg::CopyOp& copyOp = osg::CopyOp::SHALLOW_COPY );

            // We are not using Inherit<>/OSG_REGISTER_TYPE as this is abstract class.
            // Use Inherit<> + OSG_REGISTER_TYPE(osgGA,YourManipulator) in your
            // descendant non-abstract classes.
            virtual const char*
            className() const
            {
                return "StandardManipulator";
            }

            /** Sets manipulator by eye position and eye orientation.*/
            virtual void
            setTransformation( const osg::dvec3& eye,
                               const osg::quat&  rotation ) = 0;

            /** Sets manipulator by eye position, center of rotation, and up vector.*/
            virtual void
            setTransformation( const osg::dvec3& eye,
                               const osg::dvec3& center,
                               const osg::dvec3& up ) = 0;

            /** Gets manipulator's eye position and eye orientation.*/
            virtual void
            getTransformation( osg::dvec3& eye,
                               osg::quat&  rotation ) const = 0;

            /** Gets manipulator's focal center, eye position, and up vector.*/
            virtual void
            getTransformation( osg::dvec3& eye,
                               osg::dvec3& center,
                               osg::dvec3& up ) const = 0;

            virtual void
            setNode( osg::Node* );
            virtual const osg::Node*
            getNode() const;
            virtual osg::Node*
            getNode();

            virtual void
            setVerticalAxisFixed( bool value );
            inline bool
            getVerticalAxisFixed() const;
            inline bool
            getAllowThrow() const;
            virtual void
            setAllowThrow( bool allowThrow );

            virtual void
            setAnimationTime( const double t );
            double
            getAnimationTime() const;
            bool
            isAnimating() const;
            virtual void
            finishAnimation();

            virtual void
            home( const osgGA::GUIEventAdapter& ea,
                  osgGA::GUIActionAdapter&      us );
            virtual void
            home( double );

            virtual void
            init( const osgGA::GUIEventAdapter& ea,
                  osgGA::GUIActionAdapter&      us );
            virtual bool
            handle( const osgGA::GUIEventAdapter& ea,
                    osgGA::GUIActionAdapter&      us );
            virtual void
            getUsage( osg::ApplicationUsage& usage ) const;

        protected:

            virtual bool
            handleFrame( const osgGA::GUIEventAdapter& ea,
                         osgGA::GUIActionAdapter&      us );
            virtual bool
            handleResize( const osgGA::GUIEventAdapter& ea,
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
            handleKeyUp( const osgGA::GUIEventAdapter& ea,
                         osgGA::GUIActionAdapter&      us );
            virtual bool
            handleMouseWheel( const osgGA::GUIEventAdapter& ea,
                              osgGA::GUIActionAdapter&      us );
            virtual bool
            handleMouseDeltaMovement( const osgGA::GUIEventAdapter& ea,
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
            virtual bool
            performMouseDeltaMovement( const float dx,
                                       const float dy );
            virtual bool
            performAnimationMovement( const osgGA::GUIEventAdapter& ea,
                                      osgGA::GUIActionAdapter&      us );
            virtual void
            applyAnimationStep( const double currentProgress,
                                const double prevProgress );

            void
            addMouseEvent( const osgGA::GUIEventAdapter& ea );
            void
            flushMouseEventStack();
            virtual bool
            isMouseMoving() const;
            float
            getThrowScale( const double eventTimeDelta ) const;
            virtual void
            centerMousePointer( const osgGA::GUIEventAdapter& ea,
                                osgGA::GUIActionAdapter&      us );

            static void
            rotateYawPitch( osg::quat&        rotation,
                            const double      yaw,
                            const double      pitch,
                            const osg::dvec3& localUp = osg::dvec3( 0.,
                                                                    0.,
                                                                    0. ) );
            static void
            fixVerticalAxis( osg::quat&        rotation,
                             const osg::dvec3& localUp,
                             bool              disallowFlipOver );
            void
            fixVerticalAxis( osg::dvec3& eye,
                             osg::quat&  rotation,
                             bool        disallowFlipOver );
            static void
            fixVerticalAxis( const osg::dvec3& forward,
                             const osg::dvec3& up,
                             osg::dvec3&       newUp,
                             const osg::dvec3& localUp,
                             bool              disallowFlipOver );
            virtual bool
            setCenterByMousePointerIntersection( const osgGA::GUIEventAdapter& ea,
                                                 osgGA::GUIActionAdapter&      us );
            virtual bool
            startAnimationByMousePointerIntersection( const osgGA::GUIEventAdapter& ea,
                                                      osgGA::GUIActionAdapter&      us );

            // mouse state
            bool                                       _thrown;
            bool                                       _allowThrow;
            float                                      _mouseCenterX, _mouseCenterY;

            // internal event stack comprising last two mouse events.
            osg::ref_ptr<const osgGA::GUIEventAdapter> _ga_t1;
            osg::ref_ptr<const osgGA::GUIEventAdapter> _ga_t0;

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

            // scene data
            osg::ref_ptr<osg::Node>                    _node;
            double                                     _modelSize;
            bool                                       _verticalAxisFixed;

            // animation stuff
            class OSGGA_EXPORT AnimationData : public osg::Referenced
            {
                public:

                    double _animationTime;
                    bool   _isAnimating;
                    double _startTime;
                    double _phase;

                    AnimationData();
                    void
                    start( const double startTime );
            };

            osg::ref_ptr<AnimationData> _animationData;

            virtual void
            allocAnimationData()
            {
                _animationData = new AnimationData();
            }

            // flags
            int _flags;

            // flags indicating that a value is relative to model size
            int _relativeFlags;
            inline bool
            getRelativeFlag( int index ) const;
            inline void
                       setRelativeFlag( int  index,
                                        bool value );
            static int numRelativeFlagsAllocated;
            static int
            allocateRelativeFlag();
    };

    //
    //  inline methods
    //

    inline bool
    StandardManipulator::getRelativeFlag( int index ) const
    {
        return ( _relativeFlags & ( 0X01 << index ) ) != 0;
    }

    inline void
    StandardManipulator::setRelativeFlag( int  index,
                                          bool value )
    {
        if( value )
        {
            _relativeFlags |= ( 0X01 << index );
        }
        else
        {
            _relativeFlags &= ~( 0X01 << index );
        }
    }

    /// Returns whether manipulator preserves camera's "UP" vector.
    inline bool
    StandardManipulator::getVerticalAxisFixed() const
    {
        return _verticalAxisFixed;
    }

    /// Returns true if the camera can be thrown, false otherwise. It defaults to true.
    inline bool
    StandardManipulator::getAllowThrow() const
    {
        return _allowThrow;
    }

}
