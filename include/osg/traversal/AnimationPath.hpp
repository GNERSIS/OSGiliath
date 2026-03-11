/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Time-varying transform path with keyframed position, rotation, and
 * scale. Used with AnimationPathCallback for scene graph animation.
 */
#pragma once

#include <float.h>
#include <istream>
#include <map>
#include <osg/core/Callback.hpp>
#include <osg/core/Inherit.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/maths/quat.hpp>
#include <osg/maths/transform.hpp>

namespace osg
{

    /** AnimationPath encapsulates a time varying transformation pathway. Can be
     * used for updating camera position and model object position.
     * AnimationPathCallback can be attached directly to Transform nodes to
     * move subgraphs around the scene.
     */
    class OSG_EXPORT AnimationPath : public virtual osg::Object
    {
        public:

            AnimationPath() :
                _loopMode( LOOP )
            {
            }

            AnimationPath( const AnimationPath& ap,
                           const CopyOp&        copyop = CopyOp::SHALLOW_COPY ) :
                Object( ap,
                        copyop ),
                _timeControlPointMap( ap._timeControlPointMap ),
                _loopMode( ap._loopMode )
            {
            }

            virtual osg::Object*
            cloneType() const
            {
                return new AnimationPath();
            }

            virtual osg::Object*
            clone( const osg::CopyOp& copyop ) const
            {
                return new AnimationPath( *this, copyop );
            }

            virtual bool
            isSameKindAs( const osg::Object* obj ) const
            {
                return dynamic_cast<const AnimationPath*>( obj ) != NULL;
            }

            virtual const char*
            libraryName() const
            {
                return "osg";
            }

            virtual const char*
            className() const
            {
                return "AnimationPath";
            }

            class ControlPoint
            {
                public:

                    ControlPoint() :
                        _scale( 1.0,
                                1.0,
                                1.0 )
                    {
                    }

                    ControlPoint( const osg::dvec3& position ) :
                        _position( position ),
                        _rotation(),
                        _scale( 1.0,
                                1.0,
                                1.0 )
                    {
                    }

                    ControlPoint( const osg::dvec3& position,
                                  const osg::quat&  rotation ) :
                        _position( position ),
                        _rotation( rotation ),
                        _scale( 1.0,
                                1.0,
                                1.0 )
                    {
                    }

                    ControlPoint( const osg::dvec3& position,
                                  const osg::quat&  rotation,
                                  const osg::dvec3& scale ) :
                        _position( position ),
                        _rotation( rotation ),
                        _scale( scale )
                    {
                    }

                    void
                    setPosition( const osg::dvec3& position )
                    {
                        _position = position;
                    }

                    const osg::dvec3&
                    getPosition() const
                    {
                        return _position;
                    }

                    void
                    setRotation( const osg::quat& rotation )
                    {
                        _rotation = rotation;
                    }

                    const osg::quat&
                    getRotation() const
                    {
                        return _rotation;
                    }

                    void
                    setScale( const osg::dvec3& scale )
                    {
                        _scale = scale;
                    }

                    const osg::dvec3&
                    getScale() const
                    {
                        return _scale;
                    }

                    inline void
                    interpolate( float               ratio,
                                 const ControlPoint& first,
                                 const ControlPoint& second )
                    {
                        float one_minus_ratio = 1.0F - ratio;
                        _position =
                            first._position * one_minus_ratio + second._position * ratio;
                        _rotation = osg::mix( first._rotation,
                                              second._rotation,
                                              static_cast<float>( ratio ) );
                        _scale = first._scale * one_minus_ratio + second._scale * ratio;
                    }

                    inline void
                    interpolate( double              ratio,
                                 const ControlPoint& first,
                                 const ControlPoint& second )
                    {
                        double one_minus_ratio = 1.0 - ratio;
                        _position =
                            first._position * one_minus_ratio + second._position * ratio;
                        _rotation = osg::mix( first._rotation,
                                              second._rotation,
                                              static_cast<float>( ratio ) );
                        _scale = first._scale * one_minus_ratio + second._scale * ratio;
                    }

                    inline void
                    getMatrix( mat4& matrix ) const
                    {
                        // TRS: translate * rotate * scale
                        matrix = osg::translate( osg::vec3( _position ) ) *
                                 osg::rotate( _rotation ) *
                                 osg::scale( osg::vec3( _scale ) );
                    }

                    inline void
                    getMatrix( dmat4& matrix ) const
                    {
                        // TRS: translate * rotate * scale
                        matrix = osg::translate( _position ) *
                                 osg::rotate( osg::dquat( _rotation ) ) *
                                 osg::scale( _scale );
                    }

                    inline void
                    getInverse( mat4& matrix ) const
                    {
                        // Inverse TRS: inv(S) * inv(R) * inv(T)
                        matrix = osg::scale(
                                     osg::vec3( static_cast<float>( 1.0 / _scale.x ),
                                                static_cast<float>( 1.0 / _scale.y ),
                                                static_cast<float>( 1.0 / _scale.z ) )
                                 ) *
                                 osg::rotate( osg::inverse( _rotation ) ) *
                                 osg::translate( osg::vec3( -_position ) );
                    }

                    inline void
                    getInverse( dmat4& matrix ) const
                    {
                        // Inverse TRS: inv(S) * inv(R) * inv(T)
                        matrix = osg::scale( osg::dvec3( 1.0 / _scale.x,
                                                         1.0 / _scale.y,
                                                         1.0 / _scale.z ) ) *
                                 osg::rotate( osg::inverse( osg::dquat( _rotation ) ) ) *
                                 osg::translate( -_position );
                    }

                protected:

                    osg::dvec3 _position;
                    osg::quat  _rotation;
                    osg::dvec3 _scale;
            };

            /** Given a specific time, return the transformation matrix for a point. */
            bool
            getMatrix( double time,
                       mat4&  matrix ) const
            {
                ControlPoint cp;
                if( !getInterpolatedControlPoint( time, cp ) )
                {
                    return false;
                }
                cp.getMatrix( matrix );
                return true;
            }

            /** Given a specific time, return the transformation matrix for a point. */
            bool
            getMatrix( double time,
                       dmat4& matrix ) const
            {
                ControlPoint cp;
                if( !getInterpolatedControlPoint( time, cp ) )
                {
                    return false;
                }
                cp.getMatrix( matrix );
                return true;
            }

            /** Given a specific time, return the inverse transformation matrix for a
             * point. */
            bool
            getInverse( double time,
                        mat4&  matrix ) const
            {
                ControlPoint cp;
                if( !getInterpolatedControlPoint( time, cp ) )
                {
                    return false;
                }
                cp.getInverse( matrix );
                return true;
            }

            bool
            getInverse( double time,
                        dmat4& matrix ) const
            {
                ControlPoint cp;
                if( !getInterpolatedControlPoint( time, cp ) )
                {
                    return false;
                }
                cp.getInverse( matrix );
                return true;
            }

            /** Given a specific time, return the local ControlPoint frame for a point.
             */
            virtual bool
            getInterpolatedControlPoint( double        time,
                                         ControlPoint& controlPoint ) const;

            /** Insert a control point into the AnimationPath.*/
            void
            insert( double              time,
                    const ControlPoint& controlPoint );

            double
            getFirstTime() const
            {
                if( !_timeControlPointMap.empty() )
                {
                    return _timeControlPointMap.begin()->first;
                }
                else
                {
                    return 0.0;
                }
            }

            double
            getLastTime() const
            {
                if( !_timeControlPointMap.empty() )
                {
                    return _timeControlPointMap.rbegin()->first;
                }
                else
                {
                    return 0.0;
                }
            }

            double
            getPeriod() const
            {
                return getLastTime() - getFirstTime();
            }

            enum LoopMode
            {
                SWING,
                LOOP,
                NO_LOOPING,
            };

            void
            setLoopMode( LoopMode lm )
            {
                _loopMode = lm;
            }

            LoopMode
            getLoopMode() const
            {
                return _loopMode;
            }

            typedef std::map<double, ControlPoint> TimeControlPointMap;

            void
            setTimeControlPointMap( TimeControlPointMap& tcpm )
            {
                _timeControlPointMap = tcpm;
            }

            TimeControlPointMap&
            getTimeControlPointMap()
            {
                return _timeControlPointMap;
            }

            const TimeControlPointMap&
            getTimeControlPointMap() const
            {
                return _timeControlPointMap;
            }

            bool
            empty() const
            {
                return _timeControlPointMap.empty();
            }

            void
            clear()
            {
                _timeControlPointMap.clear();
            }

            /** Read the animation path from a flat ASCII file stream. */
            void
            read( std::istream& in );

            /** Write the animation path to a flat ASCII file stream. */
            void
            write( std::ostream& out ) const;

            /** Write the control point to a flat ASCII file stream. */
            void
            write( TimeControlPointMap::const_iterator itr,
                   std::ostream&                       out ) const;

        protected:

            virtual ~AnimationPath()
            {
            }

            TimeControlPointMap _timeControlPointMap;
            LoopMode            _loopMode;
    };

    class OSG_EXPORT AnimationPathCallback
        : public osg::Inherit<NodeCallback, AnimationPathCallback>
    {
        public:

            AnimationPathCallback() :
                _pivotPoint( 0.0,
                             0.0,
                             0.0 ),
                _useInverseMatrix( false ),
                _timeOffset( 0.0 ),
                _timeMultiplier( 1.0 ),
                _firstTime( DBL_MAX ),
                _latestTime( 0.0 ),
                _pause( false ),
                _pauseTime( 0.0 )
            {
            }

            AnimationPathCallback( const AnimationPathCallback& apc,
                                   const CopyOp&                copyop ) :
                Object( apc,
                        copyop ),
                Callback( apc,
                          copyop ),
                osg::Inherit<NodeCallback,
                             AnimationPathCallback>( apc,
                                                     copyop ),
                _animationPath( apc._animationPath ),
                _pivotPoint( apc._pivotPoint ),
                _useInverseMatrix( apc._useInverseMatrix ),
                _timeOffset( apc._timeOffset ),
                _timeMultiplier( apc._timeMultiplier ),
                _firstTime( apc._firstTime ),
                _latestTime( apc._latestTime ),
                _pause( apc._pause ),
                _pauseTime( apc._pauseTime )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               AnimationPathCallback )

            /** Construct an AnimationPathCallback with a specified animation path.*/
            AnimationPathCallback( AnimationPath* ap,
                                   double         timeOffset     = 0.0,
                                   double         timeMultiplier = 1.0 ) :
                _animationPath( ap ),
                _pivotPoint( 0.0,
                             0.0,
                             0.0 ),
                _useInverseMatrix( false ),
                _timeOffset( timeOffset ),
                _timeMultiplier( timeMultiplier ),
                _firstTime( DBL_MAX ),
                _latestTime( 0.0 ),
                _pause( false ),
                _pauseTime( 0.0 )
            {
            }

            /** Construct an AnimationPathCallback and automatically create an animation
             * path to produce a rotation about a point.*/
            AnimationPathCallback( const osg::dvec3& pivot,
                                   const osg::dvec3& axis,
                                   float             angularVelocity );

            void
            setAnimationPath( AnimationPath* path )
            {
                _animationPath = path;
            }

            AnimationPath*
            getAnimationPath()
            {
                return _animationPath.get();
            }

            const AnimationPath*
            getAnimationPath() const
            {
                return _animationPath.get();
            }

            inline void
            setPivotPoint( const dvec3& pivot )
            {
                _pivotPoint = pivot;
            }

            inline const dvec3&
            getPivotPoint() const
            {
                return _pivotPoint;
            }

            void
            setUseInverseMatrix( bool useInverseMatrix )
            {
                _useInverseMatrix = useInverseMatrix;
            }

            bool
            getUseInverseMatrix() const
            {
                return _useInverseMatrix;
            }

            void
            setTimeOffset( double offset )
            {
                _timeOffset = offset;
            }

            double
            getTimeOffset() const
            {
                return _timeOffset;
            }

            void
            setTimeMultiplier( double multiplier )
            {
                _timeMultiplier = multiplier;
            }

            double
            getTimeMultiplier() const
            {
                return _timeMultiplier;
            }

            virtual void
            reset();

            void
            setPause( bool pause );

            bool
            getPause() const
            {
                return _pause;
            }

            /** Get the animation time that is used to specify the position along
             * the AnimationPath. Animation time is computed from the formula:
             *   ((_latestTime-_firstTime)-_timeOffset)*_timeMultiplier.*/
            virtual double
            getAnimationTime() const;

            /** Implements the callback. */
            virtual void
            operator()( Node*        node,
                        NodeVisitor* nv );

            void
            update( osg::Node& node );

        public:

            ref_ptr<AnimationPath> _animationPath;
            osg::dvec3             _pivotPoint;
            bool                   _useInverseMatrix;
            double                 _timeOffset;
            double                 _timeMultiplier;
            double                 _firstTime;
            double                 _latestTime;
            bool                   _pause;
            double                 _pauseTime;

        protected:

            ~AnimationPathCallback()
            {
            }
    };

}
