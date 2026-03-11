/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Streaming image source for video textures. Extends Image with
 * play/pause/seek controls and frame update callbacks.
 */
#pragma once

#include <osg/images/Image.hpp>
#include <osg/textures/AudioStream.hpp>

namespace osg
{

    // forward declare of osg::Texture
    class Texture;

    /**
     * Image Stream class.
     */
    class OSG_EXPORT ImageStream : public Image
    {
        public:

            ImageStream();

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            ImageStream( const ImageStream& image,
                         const CopyOp&      copyop = CopyOp::SHALLOW_COPY );

            virtual Object*
            cloneType() const
            {
                return new ImageStream();
            }

            virtual Object*
            clone( const CopyOp& copyop ) const
            {
                return new ImageStream( *this, copyop );
            }

            virtual bool
            isSameKindAs( const Object* obj ) const
            {
                return dynamic_cast<const ImageStream*>( obj ) != 0;
            }

            virtual const char*
            libraryName() const
            {
                return "osg";
            }

            virtual const char*
            className() const
            {
                return "ImageStream";
            }

            /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            virtual int
            compare( const Image& rhs ) const;

            enum StreamStatus
            {
                INVALID,
                PLAYING,
                PAUSED,
                REWINDING,
            };

            virtual void
            seek( double /*time*/ )
            {
            }

            virtual void
            play()
            {
                _status = PLAYING;
            }

            virtual void
            pause()
            {
                _status = PAUSED;
            }

            virtual void
            rewind()
            {
                _status = REWINDING;
            }

            virtual void
            quit( bool /*waitForThreadToExit*/ = true )
            {
            }

            StreamStatus
            getStatus() const
            {
                return _status;
            }

            enum LoopingMode
            {
                NO_LOOPING,
                LOOPING,
            };

            void
            setLoopingMode( LoopingMode mode )
            {
                if( _loopingMode == mode )
                {
                    return;
                }

                _loopingMode = mode;
                applyLoopingMode();
            }

            LoopingMode
            getLoopingMode() const
            {
                return _loopingMode;
            }

            virtual double
            getCreationTime() const
            {
                return HUGE_VAL;
            }

            virtual double
            getLength() const
            {
                return 0.0;
            }

            virtual double
            getFrameRate() const
            {
                return 0.0;
            }

            virtual double
            getCurrentTime() const
            {
                return 0.0;
            }

            virtual void
            setReferenceTime( double )
            {
            }

            virtual double
            getReferenceTime() const
            {
                return 0.0;
            }

            virtual void
            setTimeMultiplier( double )
            {
            }

            virtual double
            getTimeMultiplier() const
            {
                return 0.0;
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

            /// set the balance of the audio: -1 = left, 0 = center,  1 = right
            virtual float
            getAudioBalance()
            {
                return 0.0F;
            }

            virtual void
            setAudioBalance( float /*b*/ )
            {
            }

            typedef std::vector<osg::ref_ptr<osg::AudioStream>> AudioStreams;

            void
            setAudioStreams( const AudioStreams& asl )
            {
                _audioStreams = asl;
            }

            AudioStreams&
            getAudioStreams()
            {
                return _audioStreams;
            }

            const AudioStreams&
            getAudioStreams() const
            {
                return _audioStreams;
            }

            /** create a suitable texture for this imagestream, return NULL, if not
             * supported implement this method in subclasses to use special technologies
             * like CoreVideo or similar.
             */
            virtual osg::Texture*
            createSuitableTexture()
            {
                return NULL;
            }

        protected:

            virtual void
            applyLoopingMode()
            {
            }

            virtual ~ImageStream()
            {
            }

            StreamStatus _status;
            LoopingMode  _loopingMode;

            AudioStreams _audioStreams;
    };

}    // namespace
