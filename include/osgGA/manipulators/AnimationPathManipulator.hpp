/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Camera manipulator that follows an AnimationPath.
 * Plays back pre-recorded camera trajectories.
 */
#pragma once

#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/traversal/AnimationPath.hpp>
#include <osgGA/manipulators/CameraManipulator.hpp>

namespace osgGA
{

    //
    // The AnimationPathManipulator is a dmat4 Manipulator that reads an
    // animation path from a file and plays it back.  The file is expected
    // to be ascii and a succession of lines with 8 floating point values
    // per line.  The succession of values are:
    // time  px py pz ax ay az aw
    // where:
    //    time = elapsed time in seconds from the beginning of the animation
    //    px py pz = World position in cartesian coordinates
    //    ax ay az aw = Orientation (attitude) defined as a quaternion

    class OSGGA_EXPORT AnimationPathManipulator : public CameraManipulator
    {
        public:

            AnimationPathManipulator( osg::AnimationPath* animationPath = 0 );

            AnimationPathManipulator( const std::string& filename );

            virtual const char*
            className() const
            {
                return "AnimationPath";
            }

            void
            setTimeScale( double s )
            {
                _timeScale = s;
            }

            double
            getTimeScale() const
            {
                return _timeScale;
            }

            void
            setTimeOffset( double o )
            {
                _timeOffset = o;
            }

            double
            getTimeOffset() const
            {
                return _timeOffset;
            }

            struct AnimationCompletedCallback : public virtual osg::Referenced
            {
                    virtual void
                    completed( const AnimationPathManipulator* apm ) = 0;
            };

            void
            setAnimationCompletedCallback( AnimationCompletedCallback* acc )
            {
                _animationCompletedCallback = acc;
            }

            AnimationCompletedCallback*
            getAnimationCompletedCallback()
            {
                return _animationCompletedCallback.get();
            }

            const AnimationCompletedCallback*
            getAnimationCompletedCallback() const
            {
                return _animationCompletedCallback.get();
            }

            void
            setPrintOutTimingInfo( bool printOutTimingInfo )
            {
                _printOutTimingInfo = printOutTimingInfo;
            }

            bool
            getPrintOutTimingInfo() const
            {
                return _printOutTimingInfo;
            }

            /** set the position of the matrix manipulator using a 4x4 dmat4.*/
            virtual void
            setByMatrix( const osg::dmat4& matrix )
            {
                _matrix = matrix;
            }

            /** set the position of the matrix manipulator using a 4x4 dmat4.*/
            virtual void
            setByInverseMatrix( const osg::dmat4& matrix )
            {
                _matrix = osg::inverse( matrix );
            }

            /** get the position of the manipulator as 4x4 dmat4.*/
            virtual osg::dmat4
            getMatrix() const
            {
                return _matrix;
            }

            /** get the position of the manipulator as a inverse matrix of the
             * manipulator, typically used as a model view matrix.*/
            virtual osg::dmat4
            getInverseMatrix() const
            {
                return osg::inverse( _matrix );
            }

            void
            setAnimationPath( osg::AnimationPath* animationPath )
            {
                _animationPath = animationPath;
            }

            osg::AnimationPath*
            getAnimationPath()
            {
                return _animationPath.get();
            }

            const osg::AnimationPath*
            getAnimationPath() const
            {
                return _animationPath.get();
            }

            bool
            valid() const
            {
                return _animationPath.valid();
            }

            void
            init( const GUIEventAdapter& ea,
                  GUIActionAdapter&      us );

            void
            home( const GUIEventAdapter& ea,
                  GUIActionAdapter&      us );
            void
            home( double currentTime );

            virtual bool
            handle( const GUIEventAdapter& ea,
                    GUIActionAdapter&      us );

            /** Get the keyboard and mouse usage of this manipulator.*/
            virtual void
            getUsage( osg::ApplicationUsage& usage ) const;

        protected:

            bool _valid;

            bool _printOutTimingInfo;

            void
                                                     handleFrame( double time );

            osg::ref_ptr<osg::AnimationPath>         _animationPath;

            double                                   _timeOffset;
            double                                   _timeScale;

            osg::ref_ptr<AnimationCompletedCallback> _animationCompletedCallback;

            double                                   _pauseTime;
            bool                                     _isPaused;

            double                                   _realStartOfTimedPeriod;
            double                                   _animStartOfTimedPeriod;
            int                                      _numOfFramesSinceStartOfTimedPeriod;

            osg::dmat4                               _matrix;
    };

}
