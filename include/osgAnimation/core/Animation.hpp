/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Named animation clip with a collection of channels.
 * Manages playback weight, mode (ONCE, LOOP), and duration.
 */
#pragma once

#include <map>
#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>
#include <osg/core/ref_ptr.hpp>
#include <osgAnimation/core/Channel.hpp>
#include <osgAnimation/core/Export.hpp>
#include <vector>

namespace osgAnimation
{

    class OSGANIMATION_EXPORT Animation : public osg::Inherit<osg::Object, Animation>
    {
        public:

            OSG_REGISTER_TYPE(osgAnimation, Animation)Animation() :
            _duration(0.0), _originalDuration(0.0),
            _weight(0), _startTime(0), _playmode(LOOP)
            {
            }

            Animation( const osgAnimation::Animation&,
                       const osg::CopyOp& );

            enum PlayMode
            {
                ONCE,
                STAY,
                LOOP,
                PPONG,
            };

            // addChannel insert the channel and call the computeDuration function
            void
            addChannel( Channel* pChannel );

            // removeChannel remove the channel from channels list and call the
            // computeDuration function
            void
            removeChannel( Channel* pChannel );

            /** Those accessors let you add and remove channels
             *  if you modify something that can change the duration
             *  you are supposed to call computeDuration or setDuration
             */
            ChannelList&
            getChannels();
            const ChannelList&
            getChannels() const;

            /** Change the duration of animation
             *  then evaluate the animation in the range 0-duration
             *  it stretch the animation in time.
             *  see computeDuration too
             */
            void
            setDuration( double duration );

            /** Compute duration from channel and keyframes
             *  if the duration is not specified you should
             *  call this method before using it
             */
            void
            computeDuration();
            double
            getDuration() const;
            double
            computeDurationFromChannels() const;

            void
            setWeight( float weight );
            float
            getWeight() const;

            bool
            update( double time,
                    int    priority = 0 );
            void
            resetTargets();

            void
            setPlayMode( PlayMode mode )
            {
                _playmode = mode;
            }

            PlayMode
            getPlayMode() const
            {
                return _playmode;
            }

            void
            setStartTime( double time )
            {
                _startTime = time;
            }

            double
            getStartTime() const
            {
                return _startTime;
            }

        protected:

            ~Animation()
            {
            }

            double      _duration;
            double      _originalDuration;
            float       _weight;
            double      _startTime;
            PlayMode    _playmode;
            ChannelList _channels;
    };

    typedef std::vector<osg::ref_ptr<osgAnimation::Animation>>           AnimationList;
    typedef std::map<std::string, osg::ref_ptr<osgAnimation::Animation>> AnimationMap;

}
