#pragma once

#include "FFmpegDecoder.hpp"

#include <osg/core/Inherit.hpp>
#include <osg/textures/AudioStream.hpp>

namespace osgFFmpeg
{

    class FFmpegAudioStream : public osg::Inherit<osg::AudioStream, FFmpegAudioStream>
    {
        public:

            OSG_REGISTER_TYPE( osgFFmpeg,
                               FFmpegAudioStream )

            FFmpegAudioStream( FFmpegDecoder* decoder = 0 );
            FFmpegAudioStream( const FFmpegAudioStream& audio,
                               const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            virtual void
            setAudioSink( osg::AudioSink* audio_sink );

            void
            consumeAudioBuffer( void* const  buffer,
                                const size_t size );

            int
            audioFrequency() const;
            int
            audioNbChannels() const;
            osg::AudioStream::SampleFormat
            audioSampleFormat() const;

            double
            duration() const;

        private:

            virtual ~FFmpegAudioStream();

            osg::ref_ptr<FFmpegDecoder> m_decoder;
    };

}
