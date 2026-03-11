/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * First-person camera manipulator. WASD movement with mouse-look
 * for walkthrough-style navigation.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osgGA/manipulators/StandardManipulator.hpp>

namespace osgGA
{

    /** FirstPersonManipulator is base class for camera control based on position
        and orientation of camera, like walk, drive, and flight manipulators. */
    class OSGGA_EXPORT FirstPersonManipulator
        : public osg::Inherit<StandardManipulator, FirstPersonManipulator>
    {
            typedef StandardManipulator inherited;

        public:

            FirstPersonManipulator( int flags = DEFAULT_SETTINGS );
            FirstPersonManipulator( const FirstPersonManipulator& fpm,
                                    const osg::CopyOp&            copyOp =
                                        osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgGA,
                               FirstPersonManipulator )

            virtual void
            setByMatrix( const osg::dmat4& matrix );
            virtual void
            setByInverseMatrix( const osg::dmat4& matrix );
            virtual osg::dmat4
            getMatrix() const;
            virtual osg::dmat4
            getInverseMatrix() const;

            virtual void
            setTransformation( const osg::dvec3& eye,
                               const osg::quat&  rotation );
            virtual void
            setTransformation( const osg::dvec3& eye,
                               const osg::dvec3& center,
                               const osg::dvec3& up );
            virtual void
            getTransformation( osg::dvec3& eye,
                               osg::quat&  rotation ) const;
            virtual void
            getTransformation( osg::dvec3& eye,
                               osg::dvec3& center,
                               osg::dvec3& up ) const;

            virtual void
            setVelocity( const double& velocity );
            inline double
            getVelocity() const;
            virtual void
            setAcceleration( const double& acceleration,
                             bool          relativeToModelSize = false );
            double
            getAcceleration( bool* relativeToModelSize = NULL ) const;
            virtual void
            setMaxVelocity( const double& maxVelocity,
                            bool          relativeToModelSize = false );
            double
            getMaxVelocity( bool* relativeToModelSize = NULL ) const;

            virtual void
            setWheelMovement( const double& wheelMovement,
                              bool          relativeToModelSize = false );
            double
            getWheelMovement( bool* relativeToModelSize = NULL ) const;

            virtual void
            home( const osgGA::GUIEventAdapter& ea,
                  osgGA::GUIActionAdapter&      us );
            virtual void
            home( double );

            virtual void
            init( const osgGA::GUIEventAdapter& ea,
                  osgGA::GUIActionAdapter&      us );

        protected:

            virtual bool
            handleMouseWheel( const osgGA::GUIEventAdapter& ea,
                              osgGA::GUIActionAdapter&      us );

            virtual bool
            performMovementLeftMouseButton( const double eventTimeDelta,
                                            const double dx,
                                            const double dy );
            virtual bool
            performMouseDeltaMovement( const float dx,
                                       const float dy );
            virtual void
            applyAnimationStep( const double currentProgress,
                                const double prevProgress );
            virtual bool
            startAnimationByMousePointerIntersection( const osgGA::GUIEventAdapter& ea,
                                                      osgGA::GUIActionAdapter&      us );

            void
            moveForward( const double distance );
            void
            moveForward( const osg::quat& rotation,
                         const double     distance );
            void
            moveRight( const double distance );
            void
                       moveUp( const double distance );

            osg::dvec3 _eye;
            osg::quat  _rotation;
            double     _velocity;

            double     _acceleration;
            static int _accelerationFlagIndex;
            double     _maxVelocity;
            static int _maxVelocityFlagIndex;
            double     _wheelMovement;
            static int _wheelMovementFlagIndex;

            class FirstPersonAnimationData : public AnimationData
            {
                public:

                    osg::quat _startRot;
                    osg::quat _targetRot;
                    void
                    start( const osg::quat& startRotation,
                           const osg::quat& targetRotation,
                           const double     startTime );
            };

            virtual void
            allocAnimationData()
            {
                _animationData = new FirstPersonAnimationData();
            }
    };

    //
    //  inline methods
    //

    /// Returns velocity.
    double
    FirstPersonManipulator::getVelocity() const
    {
        return _velocity;
    }

}
