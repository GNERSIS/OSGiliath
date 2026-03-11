/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Base orbit manipulator with rotate/zoom/pan. TrackballManipulator
 * extends this with unconstrained rotation.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osgGA/manipulators/StandardManipulator.hpp>

namespace osgGA
{

    /** OrbitManipulator is base class for camera control based on focal center,
        distance from the center, and orientation of distance vector to the eye.
        This is the base class for trackball style manipulators.*/
    class OSGGA_EXPORT OrbitManipulator
        : public osg::Inherit<StandardManipulator, OrbitManipulator>
    {
            typedef StandardManipulator inherited;

        public:

            OrbitManipulator( int flags = DEFAULT_SETTINGS );
            OrbitManipulator( const OrbitManipulator& om,
                              const osg::CopyOp& copyOp = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgGA,
                               OrbitManipulator )

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

            void
            setHeading( double azimuth );
            double
            getHeading() const;
            void
            setElevation( double elevation );
            double
            getElevation() const;

            virtual void
            setCenter( const osg::dvec3& center );
            const osg::dvec3&
            getCenter() const;
            virtual void
            setRotation( const osg::quat& rotation );
            const osg::quat&
            getRotation() const;
            virtual void
            setDistance( double distance );
            double
            getDistance() const;

            virtual void
            setTrackballSize( const double& size );
            inline double
            getTrackballSize() const;
            virtual void
            setWheelZoomFactor( double wheelZoomFactor );
            inline double
            getWheelZoomFactor() const;

            virtual void
            setMinimumDistance( const double& minimumDistance,
                                bool          relativeToModelSize = false );
            double
            getMinimumDistance( bool* relativeToModelSize = NULL ) const;

            virtual osgUtil::SceneView::FusionDistanceMode
            getFusionDistanceMode() const;
            virtual float
            getFusionDistanceValue() const;

        protected:

            virtual bool
            handleMouseWheel( const osgGA::GUIEventAdapter& ea,
                              osgGA::GUIActionAdapter&      us );

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
            virtual void
            applyAnimationStep( const double currentProgress,
                                const double prevProgress );

            virtual void
            rotateTrackball( const float px0,
                             const float py0,
                             const float px1,
                             const float py1,
                             const float scale );
            virtual void
            rotateWithFixedVertical( const float dx,
                                     const float dy );
            virtual void
            rotateWithFixedVertical( const float      dx,
                                     const float      dy,
                                     const osg::vec3& up );
            virtual void
            panModel( const float dx,
                      const float dy,
                      const float dz = 0.F );
            virtual void
            zoomModel( const float dy,
                       bool        pushForwardIfNeeded = true );
            void
            trackball( osg::dvec3& axis,
                       float&      angle,
                       float       p1x,
                       float       p1y,
                       float       p2x,
                       float       p2y );
            float
            tb_project_to_sphere( float r,
                                  float x,
                                  float y );
            virtual bool
            startAnimationByMousePointerIntersection( const osgGA::GUIEventAdapter& ea,
                                                      osgGA::GUIActionAdapter&      us );

            osg::dvec3 _center;
            osg::quat  _rotation;
            double     _distance;

            double     _trackballSize;
            double     _wheelZoomFactor;

            double     _minimumDistance;
            static int _minimumDistanceFlagIndex;

            class OrbitAnimationData : public AnimationData
            {
                public:

                    osg::dvec3 _movement;
                    void
                    start( const osg::dvec3& movement,
                           const double      startTime );
            };

            virtual void
            allocAnimationData()
            {
                _animationData = new OrbitAnimationData();
            }
    };

    //
    //  inline functions
    //

    /** Get the size of the trackball relative to the model size. */
    inline double
    OrbitManipulator::getTrackballSize() const
    {
        return _trackballSize;
    }

    /** Get the mouse wheel zoom factor.*/
    inline double
    OrbitManipulator::getWheelZoomFactor() const
    {
        return _wheelZoomFactor;
    }

}
