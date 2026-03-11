/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract audio stream interface for synchronized audio playback
 * alongside video ImageStreams or scene events.
 */
#include <osg/textures/AudioStream.hpp>

using namespace osg;

AudioSink::AudioSink() :
    _delay( 0.0 )
{
}

AudioStream::AudioStream()
{
}

AudioStream::AudioStream( const AudioStream& audio,
                          const CopyOp&      copyop ) :
    osg::Object( audio,
                 copyop )
{
}
