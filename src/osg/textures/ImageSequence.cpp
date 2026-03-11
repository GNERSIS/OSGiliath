/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Image sequence player that cycles through a list of images.
 * Used for flipbook-style texture animation.
 */
#include <osg/textures/ImageSequence.hpp>

#include <math.h>
#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/nodes/Camera.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osg/traversal/NodeVisitor.hpp>

using namespace osg;

ImageSequence::ImageData::ImageData()
{
}

ImageSequence::ImageData::ImageData( const ImageData& id ) :
    _filename( id._filename ),
    _image( id._image ),
    _imageRequest( id._imageRequest )
{
}

ImageSequence::ImageData&
ImageSequence::ImageData::operator=( const ImageSequence::ImageData& rhs )
{
    if( &rhs != this )
    {
        _filename     = rhs._filename;
        _image        = rhs._image;
        _imageRequest = rhs._imageRequest;
    }
    return *this;
}

ImageSequence::ImageSequence()
{
    _referenceTime             = DBL_MAX;
    _timeMultiplier            = 1.0;

    _mode                      = PRE_LOAD_ALL_IMAGES;
    _length                    = 1.0;
    _timePerImage              = 1.0;

    _seekTime                  = 0.0;
    _seekTimeSet               = false;

    _previousAppliedImageIndex = -1;
}

ImageSequence::ImageSequence( const ImageSequence& is,
                              const CopyOp&        copyop ) :
    osg::ImageStream( is,
                      copyop ),
    _referenceTime( is._referenceTime ),
    _timeMultiplier( is._timeMultiplier ),
    _mode( is._mode ),
    _length( is._length ),
    _timePerImage( is._timePerImage )
{
    _seekTime                  = is._seekTime;
    _seekTimeSet               = is._seekTimeSet;

    _previousAppliedImageIndex = -1;
}

int
ImageSequence::compare( const Image& rhs ) const
{
    return ImageStream::compare( rhs );
}

void
ImageSequence::seek( double time )
{
    _seekTime    = time;
    _seekTimeSet = true;
}

void
ImageSequence::play()
{
    _status = PLAYING;
}

void
ImageSequence::pause()
{
    _status = PAUSED;
}

void
ImageSequence::rewind()
{
    seek( 0.0F );
}

void
ImageSequence::setMode( Mode mode )
{
    _mode = mode;
}

void
ImageSequence::setLength( double length )
{
    if( length <= 0.0 )
    {
        OSG_NOTICE << "ImageSequence::setLength(" << length
                   << ") invalid length value, must be greater than 0." << std::endl;
        return;
    }

    _length = length;
    computeTimePerImage();
}

void
ImageSequence::computeTimePerImage()
{
    if( !_imageDataList.empty() )
    {
        _timePerImage = _length / double( _imageDataList.size() );
    }
    else
    {
        _timePerImage = _length;
    }
}

void
ImageSequence::setImageFile( unsigned int       pos,
                             const std::string& fileName )
{
    std::lock_guard<std::mutex> lock( _mutex );

    if( pos >= _imageDataList.size() )
    {
        _imageDataList.resize( pos );
    }
    _imageDataList[pos]._filename = fileName;
}

std::string
ImageSequence::getImageFile( unsigned int pos ) const
{
    std::lock_guard<std::mutex> lock( _mutex );
    return pos < _imageDataList.size() ? _imageDataList[pos]._filename : std::string();
}

void
ImageSequence::addImageFile( const std::string& fileName )
{
    std::lock_guard<std::mutex> lock( _mutex );
    _imageDataList.push_back( ImageData() );
    _imageDataList.back()._filename = fileName;
    computeTimePerImage();
}

void
ImageSequence::setImage( unsigned int pos,
                         osg::Image*  image )
{
    std::lock_guard<std::mutex> lock( _mutex );

    _setImage( pos, image );
}

void
ImageSequence::_setImage( unsigned int pos,
                          osg::Image*  image )
{
    if( pos >= _imageDataList.size() )
    {
        _imageDataList.resize( pos + 1 );
    }

    _imageDataList[pos]._image    = image;
    _imageDataList[pos]._filename = image->getFileName();
}

Image*
ImageSequence::getImage( unsigned int pos )
{
    std::lock_guard<std::mutex> lock( _mutex );
    return pos < _imageDataList.size() ? _imageDataList[pos]._image.get() : 0;
}

const Image*
ImageSequence::getImage( unsigned int pos ) const
{
    std::lock_guard<std::mutex> lock( _mutex );
    return pos < _imageDataList.size() ? _imageDataList[pos]._image.get() : 0;
}

void
ImageSequence::addImage( osg::Image* image )
{
    if( image == 0 )
    {
        return;
    }

    std::lock_guard<std::mutex> lock( _mutex );

    // OSG_NOTICE<<"merging image in order expected : "<<image->getFileName()<<std::endl;
    _imageDataList.push_back( ImageData() );
    _imageDataList.back()._image = image;

    computeTimePerImage();

    if( data() == 0 )
    {
        setImageToChild( static_cast<int>( _imageDataList.size() ) - 1 );
    }
}

void
ImageSequence::setImageToChild( int pos )
{

    const osg::Image* image =
        ( pos >= 0 && pos < static_cast<int>( _imageDataList.size() ) )
            ? _imageDataList[static_cast<std::size_t>( pos )]._image.get()
            : 0;
    if( image == 0 )
    {
        return;
    }

    // check to see if data is changing, if not don't apply
    if( image->data() == data() )
    {
        return;
    }

    bool discardOldImages = _mode ==
                            PAGE_AND_DISCARD_USED_IMAGES ||
                            _mode == LOAD_AND_DISCARD_IN_UPDATE_TRAVERSAL;
    if( discardOldImages && _previousAppliedImageIndex >= 0 )
    {
        if( _previousAppliedImageIndex < pos )
        {
            OSG_INFO << "Moving forward from " << _previousAppliedImageIndex << " to "
                     << pos << std::endl;
            while( _previousAppliedImageIndex < pos )
            {
                _imageDataList[static_cast<std::size_t>( _previousAppliedImageIndex )]
                    ._image = 0;
                OSG_INFO << "   deleting " << _previousAppliedImageIndex << std::endl;
                ++_previousAppliedImageIndex;
            }
        }
        else if( _previousAppliedImageIndex > pos )
        {
            OSG_INFO << "Moving back from " << _previousAppliedImageIndex << " to "
                     << pos << std::endl;
            while( _previousAppliedImageIndex > pos )
            {
                _imageDataList[static_cast<std::size_t>( _previousAppliedImageIndex )]
                    ._image = 0;
                OSG_INFO << "   deleting " << _previousAppliedImageIndex << std::endl;
                --_previousAppliedImageIndex;
            }
        }
    }

    _previousAppliedImageIndex = pos;

    Image::setImage( image->s(),
                     image->t(),
                     image->r(),
                     image->getInternalTextureFormat(),
                     image->getPixelFormat(),
                     image->getDataType(),
                     const_cast<unsigned char*>( image->data() ),
                     osg::Image::NO_DELETE,
                     static_cast<int>( image->getPacking() ) );

    setMipmapLevels( image->getMipmapLevels() );
}

void
ImageSequence::applyLoopingMode()
{
}

int
ImageSequence::imageIndex( double time )
{
    if( getLoopingMode() == LOOPING )
    {
        double positionRatio = time / _length;
        time                 = ( positionRatio - floor( positionRatio ) ) * _length;
    }

    if( time < 0.0 )
    {
        return 0;
    }
    int index = int( time / _timePerImage );
    if( index >= int( _imageDataList.size() ) )
    {
        return int( _imageDataList.size() ) - 1;
    }
    return index;
}

void
ImageSequence::update( osg::NodeVisitor* nv )
{
    std::lock_guard<std::mutex> lock( _mutex );

    // if imageDataList is empty then there is nothing update can do.
    if( _imageDataList.empty() )
    {
        return;
    }

    osg::NodeVisitor::ImageRequestHandler* irh = nv->getImageRequestHandler();
    const osg::FrameStamp*                 fs  = nv->getFrameStamp();

    // OSG_NOTICE<<"ImageSequence::update("<<fs<<", "<<irh<<")"<<std::endl;

    if( _referenceTime == DBL_MAX )
    {
        _referenceTime = fs->getSimulationTime();
    }

    bool   looping = getLoopingMode() == LOOPING;
    double time    = ( fs->getSimulationTime() - _referenceTime ) * _timeMultiplier;
    bool   useDirectTimeRequest = _seekTimeSet;

    if( _seekTimeSet || _status == PAUSED || _status == INVALID )
    {
        time                 = _seekTime;
        useDirectTimeRequest = true;
        _referenceTime       = fs->getSimulationTime() - time / _timeMultiplier;
    }
    else
    {
        if( looping )
        {
            while( time > _length )
            {
                _referenceTime += _length / _timeMultiplier;
                time           -= _length;
            }
        }
        else
        {
            if( time > _length )
            {
                _referenceTime = fs->getSimulationTime() - _length / _timeMultiplier;
                time           = _length;
            }
        }
    }

    _seekTime    = time;
    _seekTimeSet = false;

    if( irh && _mode == PRE_LOAD_ALL_IMAGES )
    {
        for( ImageDataList::iterator itr = _imageDataList.begin();
             itr != _imageDataList.end();
             ++itr )
        {
            if( !( itr->_image ) && !( itr->_filename.empty() ) )
            {
                itr->_image =
                    irh->readRefImageFile( itr->_filename, _readOptions.get() );
            }
        }
    }

    int index = int( time / _timePerImage );
    // OSG_NOTICE<<"time= "<<time<<" _timePerImage="<<_timePerImage<<" index="<<index<<"
    // _length="<<_length<<std::endl;

    if( index >= int( _imageDataList.size() ) )
    {
        index = int( _imageDataList.size() ) - 1;
    }

    if( index >= 0 && index < int( _imageDataList.size() ) )
    {
        // need to find the nearest relevant change.
        if( !_imageDataList[static_cast<std::size_t>( index )]._image )
        {
            if( _previousAppliedImageIndex < index )
            {
                OSG_DEBUG << "ImageSequence::update(..) Moving forward by "
                          << index - _previousAppliedImageIndex << std::endl;
                while(
                    index >=
                    0 &&
                    !_imageDataList[static_cast<std::size_t>( index )]._image.valid()
                )
                {
                    --index;
                }
            }
            else if( _previousAppliedImageIndex > index )
            {
                OSG_DEBUG << "ImageSequence::update(..) Moving back by "
                          << _previousAppliedImageIndex - index << std::endl;
                while(
                    index <
                    static_cast<int>( _imageDataList.size() ) &&
                    !_imageDataList[static_cast<std::size_t>( index )]._image.valid()
                )
                {
                    ++index;
                }
            }
        }

        if( index >= 0 && index != _previousAppliedImageIndex )
        {
            setImageToChild( index );
        }
    }

    // OSG_NOTICE<<"time = "<<time<<std::endl;

    if( !irh )
    {
        return;
    }

    bool loadDirectly = ( _mode == LOAD_AND_RETAIN_IN_UPDATE_TRAVERSAL ) ||
                        ( _mode == LOAD_AND_DISCARD_IN_UPDATE_TRAVERSAL );

    if( useDirectTimeRequest )
    {
        int i = osg::maximum<int>( 0, int( time / _timePerImage ) );
        if( i >=
            int( _imageDataList.size() ) ||
            !_imageDataList[static_cast<std::size_t>( i )]._image )
        {
            i = osg::minimum<int>( i, static_cast<int>( _imageDataList.size() ) - 1 );

            if( loadDirectly )
            {
                OSG_NOTICE << "Reading file, entry=" << i << " : _fileNames[i]="
                           << _imageDataList[static_cast<std::size_t>( i )]._filename
                           << std::endl;
                osg::ref_ptr<osg::Image> image = irh->readRefImageFile(
                    _imageDataList[static_cast<std::size_t>( i )]._filename,
                    _readOptions.get()
                );
                if( image.valid() )
                {
                    OSG_NOTICE << "   Assigning image "
                               << _imageDataList[static_cast<std::size_t>( i )]._filename
                               << std::endl;
                    _setImage( static_cast<unsigned int>( i ), image.get() );
                    setImageToChild( i );
                }
                else
                {
                    OSG_NOTICE << "   Failed to read file "
                               << _imageDataList[static_cast<std::size_t>( i )]._filename
                               << std::endl;
                }
            }
            else
            {
                OSG_NOTICE << "Requesting file, entry=" << i << " : _fileNames[i]="
                           << _imageDataList[static_cast<std::size_t>( i )]._filename
                           << std::endl;
                irh->requestImageFile(
                    _imageDataList[static_cast<std::size_t>( i )]._filename,
                    this,
                    i,
                    time,
                    fs,
                    _imageDataList[static_cast<std::size_t>( i )]._imageRequest,
                    _readOptions.get()
                );
            }
        }
    }
    else
    {
        double preLoadTime =
            time + std::min( irh->getPreLoadTime() * _timeMultiplier, _length );

        int startLoadIndex = int( time / _timePerImage );
        if( startLoadIndex >= int( _imageDataList.size() ) )
        {
            startLoadIndex = int( _imageDataList.size() ) - 1;
        }
        if( startLoadIndex < 0 )
        {
            startLoadIndex = 0;
        }

        int endLoadIndex = int( preLoadTime / _timePerImage );
        if( looping && ( endLoadIndex >= int( _imageDataList.size() ) ) )
        {
            endLoadIndex -= int( _imageDataList.size() );
        }
        if( endLoadIndex >= int( _imageDataList.size() ) )
        {
            endLoadIndex = int( _imageDataList.size() ) - 1;
        }
        if( endLoadIndex < 0 )
        {
            endLoadIndex = 0;
        }

        double requestTime = time;

        if( endLoadIndex < startLoadIndex )
        {
            for( int i = startLoadIndex; i < int( _imageDataList.size() ); ++i )
            {
                if( !_imageDataList[static_cast<std::size_t>( i )]._image )
                {
                    irh->requestImageFile(
                        _imageDataList[static_cast<std::size_t>( i )]._filename,
                        this,
                        i,
                        requestTime,
                        fs,
                        _imageDataList[static_cast<std::size_t>( i )]._imageRequest,
                        _readOptions.get()
                    );
                }
                requestTime += _timePerImage;
            }

            for( int i = 0; i <= endLoadIndex; ++i )
            {
                if( !_imageDataList[static_cast<std::size_t>( i )]._image )
                {
                    irh->requestImageFile(
                        _imageDataList[static_cast<std::size_t>( i )]._filename,
                        this,
                        i,
                        requestTime,
                        fs,
                        _imageDataList[static_cast<std::size_t>( i )]._imageRequest,
                        _readOptions.get()
                    );
                }
                requestTime += _timePerImage;
            }
        }
        else
        {
            for( int i = startLoadIndex; i <= endLoadIndex; ++i )
            {
                if( !_imageDataList[static_cast<std::size_t>( i )]._image )
                {
                    irh->requestImageFile(
                        _imageDataList[static_cast<std::size_t>( i )]._filename,
                        this,
                        i,
                        requestTime,
                        fs,
                        _imageDataList[static_cast<std::size_t>( i )]._imageRequest,
                        _readOptions.get()
                    );
                }
                requestTime += _timePerImage;
            }
        }
    }
}
