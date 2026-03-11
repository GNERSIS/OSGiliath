/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract audio stream interface for synchronized audio playback
 * alongside video ImageStreams or scene events.
 */
#pragma once

#include <osg/images/Image.hpp>
#include <stdlib.h>

namespace osg
{

    /** Pure virtual AudioSink bass class that is used to connect the audio system with
     * AudioStreams. */
    class OSG_EXPORT AudioSink : public osg::Object
    {
        public:

            AudioSink();

            virtual const char*
            libraryName() const
            {
                return "osg";
            }

            virtual const char*
            className() const
            {
                return "AudioSinkInterface";
            }

            virtual void
            play() = 0;
            virtual void
            pause() = 0;
            virtual void
            stop() = 0;

            virtual bool
            playing() const = 0;

            virtual double
            getDelay() const
            {
                return _delay;
            }

            virtual void
            setDelay( const double delay )
            {
                _delay = delay;
            }

            virtual void
            setVolume( float )
            {
            }

            virtual float
            getVolume() const
            {
                return 0.0F;
            }

        private:

            virtual Object*
            cloneType() const
            {
                return 0;
            }

            virtual Object*
            clone( const osg::CopyOp& ) const
            {
                return 0;
            }

            double _delay;
    };

    /** Pure virtual AudioStream base class. Subclasses provide mechanism for
     * reading/generating audio data*/
    class OSG_EXPORT AudioStream : public osg::Object
    {
        public:

            AudioStream();

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            AudioStream( const AudioStream& audio,
                         const CopyOp&      copyop = CopyOp::SHALLOW_COPY );

            virtual bool
            isSameKindAs( const Object* obj ) const
            {
                return dynamic_cast<const AudioStream*>( obj ) != 0;
            }

            virtual const char*
            libraryName() const
            {
                return "osg";
            }

            virtual const char*
            className() const
            {
                return "AudioStream";
            }

            virtual void
            setAudioSink( osg::AudioSink* audio_sink ) = 0;

            virtual void
            consumeAudioBuffer( void* const  buffer,
                                const size_t size ) = 0;

            virtual int
            audioFrequency() const = 0;
            virtual int
            audioNbChannels() const = 0;

            enum SampleFormat
            {
                SAMPLE_FORMAT_U8,
                SAMPLE_FORMAT_S16,
                SAMPLE_FORMAT_S24,
                SAMPLE_FORMAT_S32,
                SAMPLE_FORMAT_F32,
            };

            virtual SampleFormat
            audioSampleFormat() const = 0;
    };

}    // namespace
