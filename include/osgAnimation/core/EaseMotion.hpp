/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Easing function library for animation timing curves.
 * Provides ease-in, ease-out, and ease-in-out functions.
 */
#pragma once

#include <osg/core/Notify.hpp>
#include <osg/core/ref_ptr.hpp>
#include <osg/core/Referenced.hpp>
#include <osg/maths/Math.hpp>
#include <vector>

namespace osgAnimation
{

    struct OutBounceFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                if( ( t ) < ( 1.0F / 2.75F ) )
                {
                    result = 7.5625F * t * t;
                }
                else if( t < ( 2.0F / 2.75F ) )
                {
                    t      = t - ( 1.5F / 2.75F );
                    result = 7.5625F * t * t + .75F;
                }
                else if( t < ( 2.5F / 2.75F ) )
                {
                    t      = t - ( 2.25F / 2.75F );
                    result = 7.5625F * t * t + .9375F;
                }
                else
                {
                    t      = t - ( 2.625F / 2.75F );
                    result = 7.5625F * t * t + .984375F;
                }
            }
    };

    struct InBounceFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                OutBounceFunction::getValueAt( 1 - t, result );
                result = 1 - result;
            }
    };

    struct InOutBounceFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                if( t < 0.5 )
                {
                    InBounceFunction::getValueAt( t * 2, result );
                    result *= 0.5F;
                }
                else
                {
                    OutBounceFunction::getValueAt( t * 2 - 1, result );
                    result = result * 0.5F + 0.5F;
                }
            }
    };

    /// Linear function
    struct LinearFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                result = t;
            }
    };

    /// Quad function
    struct OutQuadFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                result = -( t * ( t - 2.0F ) );
            }
    };

    struct InQuadFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                result = t * t;
            }
    };

    struct InOutQuadFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                t *= 2.0F;
                if( t < 1.0F )
                {
                    result = 0.5F * t * t;
                }
                else
                {
                    t      -= 1.0F;
                    result  = -0.5F * ( t * ( t - 2 ) - 1 );
                }
            }
    };

    /// Cubic function
    struct OutCubicFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                t      = t - 1.0F;
                result = t * t * t + 1;
            }
    };

    struct InCubicFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                result = t * t * t;
            }
    };

    struct InOutCubicFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                t *= 2.0F;
                if( t < 1.0F )
                {
                    result = 0.5F * t * t * t;
                }
                else
                {
                    t      -= 2.0F;
                    result  = 0.5F * ( t * t * t + 2.0F );
                }
            }
    };

    /// Quart function
    struct InQuartFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                result = t * t * t * t * t;
            }
    };

    struct OutQuartFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                t      = t - 1;
                result = -( t * t * t * t - 1 );
            }
    };

    struct InOutQuartFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                t = t * 2.0F;
                if( t < 1 )
                {
                    result = 0.5F * t * t * t * t;
                }
                else
                {
                    t      -= 2.0F;
                    result  = -0.5F * ( t * t * t * t - 2 );
                }
            }
    };

    /// Elastic function
    struct OutElasticFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                result = static_cast<float>(
                    pow( 2.0F, -10.0F * t ) *
                    sin( ( t - 0.3F / 4.0F ) * ( 2.0 * osg::PI ) / 0.3 ) +
                    1.0
                );
            }
    };

    struct InElasticFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                OutElasticFunction::getValueAt( 1.0F - t, result );
                result = 1.0F - result;
            }
    };

    struct InOutElasticFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                t *= 2.0F;
                if( t < 1.0F )
                {
                    t      -= 1.0F;
                    result  = static_cast<float>(
                        -0.5 * ( 1.0 *
                                 pow( 2.0, 10.0 * t ) *
                                 sin( ( t - 0.45 / 4.0 ) * ( 2.0 * osg::PI ) / 0.45 ) )
                    );
                }
                else
                {
                    t      -= 1.0F;
                    result  = static_cast<float>(
                        pow( 2.0, -10.0 * t ) *
                        sin( ( t - 0.45 / 4.0 ) * ( 2.0 * osg::PI ) / 0.45 ) *
                        0.5 +
                        1.0
                    );
                }
            }
    };

    // Sine function
    struct OutSineFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                result = static_cast<float>( sin( t * ( osg::PI / 2.0 ) ) );
            }
    };

    struct InSineFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                result = static_cast<float>( -cos( t * ( osg::PI / 2.0 ) ) + 1.0 );
            }
    };

    struct InOutSineFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                result = static_cast<float>( -0.5 * ( cos( osg::PI * t ) - 1.0 ) );
            }
    };

    // Back function
    struct OutBackFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                t      -= 1.0F;
                result  = t * t * ( ( 1.70158F + 1.0F ) * t + 1.70158F ) + 1.0F;
            }
    };

    struct InBackFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                result = t * t * ( ( 1.70158F + 1.0F ) * t - 1.70158F );
            }
    };

    struct InOutBackFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                float s  = 1.70158F * 1.525F;
                t       *= 2.0F;
                if( t < 1.0F )
                {
                    result = 0.5F * ( t * t * ( ( s + 1.0F ) * t - s ) );
                }
                else
                {
                    float p = t -= 2.0F;
                    result       = 0.5F * ( ( p )*t * ( ( s + 1.0F ) * t + s ) + 2.0F );
                }
            }
    };

    // Circ function
    struct OutCircFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                t      -= 1.0F;
                result  = sqrt( 1.0F - t * t );
            }
    };

    struct InCircFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                result = -( sqrt( 1.0F - ( t * t ) ) - 1.0F );
            }
    };

    struct InOutCircFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                t *= 2.0F;
                if( t < 1.0F )
                {
                    result = -0.5F * ( sqrt( 1.0F - t * t ) - 1.0F );
                }
                else
                {
                    t      -= 2.0F;
                    result  = 0.5F * ( sqrt( 1 - t * t ) + 1.0F );
                }
            }
    };

    // Expo function
    struct OutExpoFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                if( t == 1.0F )
                {
                    result = 0.0F;
                }
                else
                {
                    result = -powf( 2.0F, -10.0F * t ) + 1.0F;
                }
            }
    };

    struct InExpoFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                if( t == 0.0F )
                {
                    result = 0.0F;
                }
                else
                {
                    result = powf( 2.0F, 10.0F * ( t - 1.0F ) );
                }
            }
    };

    struct InOutExpoFunction
    {
            inline static void
            getValueAt( float  t,
                        float& result )
            {
                if( t == 0.0F || t == 1.0F )
                {
                    result = 0.0F;
                }
                else
                {
                    t *= 2.0F;
                    if( t < 1.0F )
                    {
                        result = 0.5F * powf( 2.0F, 10.0F * ( t - 1.0F ) );
                    }
                    else
                    {
                        result = 0.5F * ( -powf( 2.0F, -10.0F * ( t - 1.0F ) ) + 2.0F );
                    }
                }
            }
    };

    class Motion : public osg::Referenced
    {
        public:

            typedef float value_type;

            enum TimeBehaviour
            {
                CLAMP,
                LOOP,
            };

            Motion( float         startValue  = 0,
                    float         duration    = 1,
                    float         changeValue = 1,
                    TimeBehaviour tb          = CLAMP ) :
                _time( 0 ),
                _startValue( startValue ),
                _changeValue( changeValue ),
                _duration( duration ),
                _behaviour( tb )
            {
            }

            virtual ~Motion()
            {
            }

            void
            reset()
            {
                setTime( 0 );
            }

            float
            getTime() const
            {
                return _time;
            }

            float
            evaluateTime( float time ) const
            {
                switch( _behaviour )
                {
                    case CLAMP :
                        if( time > _duration )
                        {
                            time = _duration;
                        }
                        else if( time < 0.0 )
                        {
                            time = 0.0;
                        }
                        break;
                    case LOOP :
                        if( time <= 0 )
                        {
                            time = 0;
                        }
                        else
                        {
                            time = fmodf( time, _duration );
                        }
                        break;
                }
                return time;
            }

            void
            update( float dt )
            {
                _time = evaluateTime( _time + dt );
            }

            void
            setTime( float time )
            {
                _time = evaluateTime( time );
            }

            void
            getValue( value_type& result ) const
            {
                getValueAt( _time, result );
            }

            value_type
            getValue() const
            {
                value_type result;
                getValueAt( _time, result );
                return result;
            }

            void
            getValueAt( float       time,
                        value_type& result ) const
            {
                getValueInNormalizedRange( evaluateTime( time ) / _duration, result );
                result = result * _changeValue + _startValue;
            }

            value_type
            getValueAt( float time ) const
            {
                value_type result;
                getValueAt( evaluateTime( time ), result );
                return result;
            }

            virtual void
            getValueInNormalizedRange( float       t,
                                       value_type& result ) const = 0;

            float
            getDuration() const
            {
                return _duration;
            }

        protected:

            float         _time;
            float         _startValue;
            float         _changeValue;
            float         _duration;
            TimeBehaviour _behaviour;
    };

    template<typename T>
    struct MathMotionTemplate : public Motion
    {
            MathMotionTemplate( float         startValue  = 0,
                                float         duration    = 1,
                                float         changeValue = 1,
                                TimeBehaviour tb          = CLAMP ) :
                Motion( startValue,
                        duration,
                        changeValue,
                        tb )
            {
            }

            virtual void
            getValueInNormalizedRange( float       t,
                                       value_type& result ) const
            {
                T::getValueAt( t, result );
            }
    };

    template<class T>
    struct SamplerMotionTemplate : public Motion
    {
            T _sampler;

            SamplerMotionTemplate( float         startValue  = 0,
                                   float         duration    = 1,
                                   float         changeValue = 1,
                                   TimeBehaviour tb          = CLAMP ) :
                Motion( startValue,
                        duration,
                        changeValue,
                        tb )
            {
            }

            T&
            getSampler()
            {
                return _sampler;
            }

            const T&
            getSampler() const
            {
                return _sampler;
            }

            virtual void
            getValueInNormalizedRange( float       t,
                                       value_type& result ) const
            {
                if( !_sampler.getKeyframeContainer() )
                {
                    result = 0;
                    return;
                }
                float size = _sampler.getEndTime() - _sampler.getStartTime();
                t          = t * size + _sampler.getStartTime();
                _sampler.getValueAt( t, result );
            }
    };

    struct CompositeMotion : public Motion
    {
            typedef std::vector<osg::ref_ptr<Motion>> MotionList;
            MotionList                                _motions;

            MotionList&
            getMotionList()
            {
                return _motions;
            }

            const MotionList&
            getMotionList() const
            {
                return _motions;
            }

            CompositeMotion( float         startValue  = 0,
                             float         duration    = 1,
                             float         changeValue = 1,
                             TimeBehaviour tb          = CLAMP ) :
                Motion( startValue,
                        duration,
                        changeValue,
                        tb )
            {
            }

            virtual void
            getValueInNormalizedRange( float       t,
                                       value_type& result ) const
            {
                if( _motions.empty() )
                {
                    result = 0;
                    osg::notify( osg::WARN )
                        << "CompositeMotion::getValueInNormalizedRange no Motion in the "
                           "CompositeMotion, add motion to have result"
                        << std::endl;
                    return;
                }
                for( MotionList::const_iterator it = _motions.begin();
                     it != _motions.end();
                     ++it )
                {
                    const Motion* motion  = static_cast<const Motion*>( it->get() );
                    float durationInRange = motion->getDuration() / getDuration();
                    if( t < durationInRange )
                    {
                        float tInRange = t / durationInRange * motion->getDuration();
                        motion->getValueAt( tInRange, result );
                        return;
                    }
                    else
                    {
                        t = t - durationInRange;
                    }
                }
                osg::notify( osg::WARN )
                    << "CompositeMotion::getValueInNormalizedRange did find the value "
                       "in range, something wrong"
                    << std::endl;
                result = 0;
            }
    };

    // linear
    typedef MathMotionTemplate<LinearFunction>       LinearMotion;

    // quad
    typedef MathMotionTemplate<OutQuadFunction>      OutQuadMotion;
    typedef MathMotionTemplate<InQuadFunction>       InQuadMotion;
    typedef MathMotionTemplate<InOutQuadFunction>    InOutQuadMotion;

    // cubic
    typedef MathMotionTemplate<OutCubicFunction>     OutCubicMotion;
    typedef MathMotionTemplate<InCubicFunction>      InCubicMotion;
    typedef MathMotionTemplate<InOutCubicFunction>   InOutCubicMotion;

    // quart
    typedef MathMotionTemplate<OutQuartFunction>     OutQuartMotion;
    typedef MathMotionTemplate<InQuartFunction>      InQuartMotion;
    typedef MathMotionTemplate<InOutQuartFunction>   InOutQuartMotion;

    // bounce
    typedef MathMotionTemplate<OutBounceFunction>    OutBounceMotion;
    typedef MathMotionTemplate<InBounceFunction>     InBounceMotion;
    typedef MathMotionTemplate<InOutBounceFunction>  InOutBounceMotion;

    // elastic
    typedef MathMotionTemplate<OutElasticFunction>   OutElasticMotion;
    typedef MathMotionTemplate<InElasticFunction>    InElasticMotion;
    typedef MathMotionTemplate<InOutElasticFunction> InOutElasticMotion;

    // sine
    typedef MathMotionTemplate<OutSineFunction>      OutSineMotion;
    typedef MathMotionTemplate<InSineFunction>       InSineMotion;
    typedef MathMotionTemplate<InOutSineFunction>    InOutSineMotion;

    // back
    typedef MathMotionTemplate<OutBackFunction>      OutBackMotion;
    typedef MathMotionTemplate<InBackFunction>       InBackMotion;
    typedef MathMotionTemplate<InOutBackFunction>    InOutBackMotion;

    // circ
    typedef MathMotionTemplate<OutCircFunction>      OutCircMotion;
    typedef MathMotionTemplate<InCircFunction>       InCircMotion;
    typedef MathMotionTemplate<InOutCircFunction>    InOutCircMotion;

    // expo
    typedef MathMotionTemplate<OutExpoFunction>      OutExpoMotion;
    typedef MathMotionTemplate<InExpoFunction>       InExpoMotion;
    typedef MathMotionTemplate<InOutExpoFunction>    InOutExpoMotion;

}
