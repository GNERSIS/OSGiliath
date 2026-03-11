/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Streaming image source for video textures. Extends Image with
 * play/pause/seek controls and frame update callbacks.
 */
#include <osg/textures/ImageStream.hpp>

using namespace osg;

ImageStream::ImageStream() :
    _status( INVALID ),
    _loopingMode( LOOPING )
{
    setDataVariance( DataVariance::DYNAMIC );

    // #ifndef __APPLE__
    //  disabled under OSX for time being while we resolve why PBO
    //  doesn't function properly under OSX.
    setPixelBufferObject( new PixelBufferObject( this ) );
    // #endif
}

ImageStream::ImageStream( const ImageStream& image,
                          const CopyOp&      copyop ) :
    Image( image,
           copyop ),
    _status( image._status ),
    _loopingMode( image._loopingMode ),
    _audioStreams( image._audioStreams )
{
}

int
ImageStream::compare( const Image& rhs ) const
{
    return Image::compare( rhs );
}
