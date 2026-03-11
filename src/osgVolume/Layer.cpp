/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract volume data layer providing 3D image access.
 * Wraps Image/ImageSequence with locator and property binding.
 */
#include <osgVolume/Layer>

#include <osg/core/Endian.hpp>
#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osg/images/ImageUtils.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/textures/ImageStream.hpp>

using namespace osgVolume;

ImageDetails::ImageDetails() :
    _texelOffset( 0.0,
                  0.0,
                  0.0,
                  0.0 ),
    _texelScale( 1.0,
                 1.0,
                 1.0,
                 1.0 )
{
}

ImageDetails::ImageDetails( const ImageDetails& rhs,
                            const osg::CopyOp&  copyop ) :
    Inherit( rhs,
             copyop ),
    _texelOffset( rhs._texelOffset ),
    _texelScale( rhs._texelScale ),
    _matrix( rhs._matrix )
{
}

Layer::Layer() :
    _minFilter( osg::Texture::LINEAR ),
    _magFilter( osg::Texture::LINEAR )
{
}

Layer::Layer( const Layer&       layer,
              const osg::CopyOp& copyop ) :
    Inherit( layer,
             copyop ),
    _filename( layer._filename ),
    _minFilter( layer._minFilter ),
    _magFilter( layer._magFilter )
{
}

Layer::~Layer()
{
}

osg::sphere
Layer::computeBound() const
{
    if( !getLocator() )
    {
        return osg::sphere();
    }

    osg::dvec3 left, right;
    getLocator()->computeLocalBounds( left, right );

    // OSG_NOTICE<<"left = "<<left<<std::endl;
    // OSG_NOTICE<<"right = "<<right<<std::endl;

    return osg::sphere( ( left + right ) * 0.5,
                        static_cast<float>( osg::length( right - left ) * 0.5 ) );
}

void
Layer::addProperty( Property* property )
{
    if( !property )
    {
        return;
    }

    if( !_property )
    {
        _property = property;
        return;
    }

    CompositeProperty* cp = dynamic_cast<CompositeProperty*>( _property.get() );
    if( cp )
    {
        cp->addProperty( property );
    }
    else
    {
        cp = new CompositeProperty;
        cp->addProperty( property );
        cp->addProperty( _property.get() );
        _property = cp;
    }
}

/////////////////////////////////////////////////////////////////////////////
//
// ImageLayer
//
ImageLayer::ImageLayer( osg::Image* image ) :
    _texelOffset( 0.0,
                  0.0,
                  0.0,
                  0.0 ),
    _texelScale( 1.0,
                 1.0,
                 1.0,
                 1.0 ),
    _image( image )
{
}

ImageLayer::ImageLayer( const ImageLayer&  imageLayer,
                        const osg::CopyOp& copyop ) :
    Inherit( imageLayer,
             copyop ),
    _texelOffset( imageLayer._texelOffset ),
    _texelScale( imageLayer._texelScale ),
    _image( imageLayer._image )
{
}

void
ImageLayer::setImage( osg::Image* image )
{
    _image = image;
}

void
ImageLayer::dirty()
{
    if( _image.valid() )
    {
        _image->dirty();
    }
}

void
ImageLayer::setModifiedCount( unsigned int value )
{
    if( !_image )
    {
        return;
    }
    else
    {
        _image->setModifiedCount( value );
    }
}

unsigned int
ImageLayer::getModifiedCount() const
{
    if( !_image )
    {
        return 0;
    }
    else
    {
        return _image->getModifiedCount();
    }
}

bool
ImageLayer::computeMinMax( osg::vec4& minValue,
                           osg::vec4& maxValue )
{
    if( _image.valid() )
    {
        return osg::computeMinMax( _image.get(), minValue, maxValue );
    }
    else
    {
        return false;
    }
}

void
ImageLayer::offsetAndScaleImage( const osg::vec4& offset,
                                 const osg::vec4& scale )
{
    if( !_image )
    {
        return;
    }

#if 0
    osg::vec4 minValue, maxValue;
    if (computeMinMax(minValue, maxValue))
    {
        OSG_NOTICE<<"ImageLayer::offsetAndScaleImage("<<offset<<" and "<<scale<<")"<<std::endl;
        OSG_NOTICE<<"     before    _texelOffset "<<_texelOffset<<std::endl;
        OSG_NOTICE<<"     before    _texelScale "<<_texelScale<<std::endl;
        OSG_NOTICE<<"     before    minValue "<<minValue<<std::endl;
        OSG_NOTICE<<"     before    maxValue "<<maxValue<<std::endl;
        OSG_NOTICE<<"     before    minValue transformed "<<minValue[0]*_texelScale[0]+_texelOffset[0]<<std::endl;
        OSG_NOTICE<<"     before    maxValue transformed "<<maxValue[0]*_texelScale[0]+_texelOffset[0]<<std::endl;
    }
#endif

    osg::offsetAndScaleImage( _image.get(), offset, scale );

    _texelScale[0]        /= scale[0];
    _texelScale[1]        /= scale[1];
    _texelScale[2]        /= scale[2];
    _texelScale[3]        /= scale[3];

    _texelOffset[0]       -= offset[0] * _texelScale[0];
    _texelOffset[1]       -= offset[1] * _texelScale[1];
    _texelOffset[2]       -= offset[2] * _texelScale[2];
    _texelOffset[3]       -= offset[3] * _texelScale[3];

    ImageDetails* details  = dynamic_cast<ImageDetails*>( _image->getUserData() );
    if( details )
    {
        details->setTexelOffset( _texelOffset );
        details->setTexelScale( _texelScale );
    }

#if 0
    if (computeMinMax(minValue, maxValue))
    {
        OSG_NOTICE<<"     after     _texelOffset "<<_texelOffset<<std::endl;
        OSG_NOTICE<<"     after     _texelScale "<<_texelScale<<std::endl;
        OSG_NOTICE<<"     after     minValue "<<minValue<<std::endl;
        OSG_NOTICE<<"     after     maxValue "<<maxValue<<std::endl;
        OSG_NOTICE<<"     after     minValue transformed "<<minValue[0]*_texelScale[0]+_texelOffset[0]<<std::endl;
        OSG_NOTICE<<"     after     maxValue transformed "<<maxValue[0]*_texelScale[0]+_texelOffset[0]<<std::endl;
    }
#endif
}

void
ImageLayer::rescaleToZeroToOneRange()
{
    OSG_INFO << "ImageLayer::rescaleToZeroToOneRange()" << std::endl;

    osg::vec4 minValue, maxValue;
    if( computeMinMax( minValue, maxValue ) )
    {
        float minComponent = minValue[0];
        minComponent       = std::min( minComponent, minValue[1] );
        minComponent       = std::min( minComponent, minValue[2] );
        minComponent       = std::min( minComponent, minValue[3] );

        float maxComponent = maxValue[0];
        maxComponent       = std::max( maxComponent, maxValue[1] );
        maxComponent       = std::max( maxComponent, maxValue[2] );
        maxComponent       = std::max( maxComponent, maxValue[3] );

        float scale        = 0.99F / ( maxComponent - minComponent );
        float offset       = -minComponent * scale;

        OSG_INFO << "         scale " << scale << std::endl;
        OSG_INFO << "         offset " << offset << std::endl;

        offsetAndScaleImage( osg::vec4( offset, offset, offset, offset ),
                             osg::vec4( scale, scale, scale, scale ) );
    }
}

void
ImageLayer::translateMinToZero()
{
    osg::vec4 minValue, maxValue;
    if( computeMinMax( minValue, maxValue ) )
    {
        float minComponent = minValue[0];
        minComponent       = std::min( minComponent, minValue[1] );
        minComponent       = std::min( minComponent, minValue[2] );
        minComponent       = std::min( minComponent, minValue[3] );

        float offset       = -minComponent;

        offsetAndScaleImage( osg::vec4( offset, offset, offset, offset ),
                             osg::vec4( 1.0F, 1.0F, 1.0F, 1.0F ) );
    }
}

bool
ImageLayer::requiresUpdateTraversal() const
{
    return dynamic_cast<osg::ImageStream*>( _image.get() ) != 0;
}

void
ImageLayer::update( osg::NodeVisitor& nv )
{
    if( _image.valid() )
    {
        _image->update( &nv );
    }
}

/////////////////////////////////////////////////////////////////////////////
//
// CompositeLayer
//
CompositeLayer::CompositeLayer()
{
}

CompositeLayer::CompositeLayer( const CompositeLayer& compositeLayer,
                                const osg::CopyOp&    copyop ) :
    Inherit( compositeLayer,
             copyop )
{
}

void
CompositeLayer::clear()
{
    _layers.clear();
}

bool
CompositeLayer::requiresUpdateTraversal() const
{
    for( Layers::const_iterator itr = _layers.begin(); itr != _layers.end(); ++itr )
    {
        if( itr->layer->requiresUpdateTraversal() )
        {
            return true;
        }
    }

    return false;
}

void
CompositeLayer::update( osg::NodeVisitor& nv )
{
    for( Layers::const_iterator itr = _layers.begin(); itr != _layers.end(); ++itr )
    {
        itr->layer->update( nv );
    }
}

/////////////////////////////////////////////////////////////////////////////
//
// createNormalMapTexture
//
osg::Image*
osgVolume::createNormalMapTexture( osg::Image* image_3d )
{
    OSG_INFO << "Computing NormalMapTexture" << std::endl;

    GLenum       dataType             = image_3d->getDataType();

    unsigned int sourcePixelIncrement = 1;
    unsigned int alphaOffset          = 0;
    switch( image_3d->getPixelFormat() )
    {
        case( GL_RED ) :
            sourcePixelIncrement = 1;
            alphaOffset          = 0;
            break;
        case( GL_RG ) :
            sourcePixelIncrement = 2;
            alphaOffset          = 1;
            break;
        case( GL_RGB ) :
            sourcePixelIncrement = 3;
            alphaOffset          = 0;
            break;
        case( GL_RGBA ) :
            sourcePixelIncrement = 4;
            alphaOffset          = 3;
            break;
        default :
            OSG_NOTICE << "Source pixel format not support for normal map generation."
                       << std::endl;
            return 0;
    }

    osg::ref_ptr<osg::Image> normalmap_3d = new osg::Image;
    normalmap_3d->allocateImage( image_3d->s(),
                                 image_3d->t(),
                                 image_3d->r(),
                                 GL_RGBA,
                                 GL_UNSIGNED_BYTE );

    if( osg::getCpuByteOrder() == osg::LittleEndian )
    {
        alphaOffset = sourcePixelIncrement - alphaOffset - 1;
    }

    for( unsigned int r = 1; r < static_cast<unsigned int>( image_3d->r() - 1 ); ++r )
    {
        for( unsigned int t = 1; t < static_cast<unsigned int>( image_3d->t() - 1 );
             ++t )
        {

            if( dataType == GL_UNSIGNED_BYTE )
            {
                unsigned char* ptr   = image_3d->data( 1, t, r ) + alphaOffset;
                unsigned char* left  = image_3d->data( 0, t, r ) + alphaOffset;
                unsigned char* right = image_3d->data( 2, t, r ) + alphaOffset;
                unsigned char* above = image_3d->data( 1, t + 1, r ) + alphaOffset;
                unsigned char* below = image_3d->data( 1, t - 1, r ) + alphaOffset;
                unsigned char* in    = image_3d->data( 1, t, r + 1 ) + alphaOffset;
                unsigned char* out   = image_3d->data( 1, t, r - 1 ) + alphaOffset;

                unsigned char* destination =
                    ( unsigned char* )normalmap_3d->data( 1, t, r );

                for( int s = 1; s < image_3d->s() - 1; ++s )
                {

                    osg::vec3 grad( ( float )( *left ) - ( float )( *right ),
                                    ( float )( *below ) - ( float )( *above ),
                                    ( float )( *out ) - ( float )( *in ) );

                    grad = osg::normalize( grad );

                    if( grad.x == 0.0F && grad.y == 0.0F && grad.z == 0.0F )
                    {
                        grad.set( 128.0F, 128.0F, 128.0F );
                    }
                    else
                    {
                        grad.x = osg::clampBetween( ( grad.x + 1.0F ) * 128.0F,
                                                    0.0F,
                                                    255.0F );
                        grad.y = osg::clampBetween( ( grad.y + 1.0F ) * 128.0F,
                                                    0.0F,
                                                    255.0F );
                        grad.z = osg::clampBetween( ( grad.z + 1.0F ) * 128.0F,
                                                    0.0F,
                                                    255.0F );
                    }

                    *( destination++ ) =
                        ( unsigned char )( grad.x );    // scale and bias X.
                    *( destination++ ) =
                        ( unsigned char )( grad.y );    // scale and bias Y.
                    *( destination++ ) =
                        ( unsigned char )( grad.z );    // scale and bias Z.

                    *destination++  = *ptr;

                    ptr            += sourcePixelIncrement;
                    left           += sourcePixelIncrement;
                    right          += sourcePixelIncrement;
                    above          += sourcePixelIncrement;
                    below          += sourcePixelIncrement;
                    in             += sourcePixelIncrement;
                    out            += sourcePixelIncrement;
                }
            }
            else if( dataType == GL_SHORT )
            {
                short* ptr   = ( short* )( image_3d->data( 1, t, r ) + alphaOffset );
                short* left  = ( short* )( image_3d->data( 0, t, r ) + alphaOffset );
                short* right = ( short* )( image_3d->data( 2, t, r ) + alphaOffset );
                short* above = ( short* )( image_3d->data( 1, t + 1, r ) + alphaOffset );
                short* below = ( short* )( image_3d->data( 1, t - 1, r ) + alphaOffset );
                short* in    = ( short* )( image_3d->data( 1, t, r + 1 ) + alphaOffset );
                short* out   = ( short* )( image_3d->data( 1, t, r - 1 ) + alphaOffset );

                unsigned char* destination =
                    ( unsigned char* )normalmap_3d->data( 1, t, r );

                for( int s = 1; s < image_3d->s() - 1; ++s )
                {

                    osg::vec3 grad( ( float )( *left ) - ( float )( *right ),
                                    ( float )( *below ) - ( float )( *above ),
                                    ( float )( *out ) - ( float )( *in ) );

                    grad = osg::normalize( grad );

                    // OSG_NOTICE<<"normal "<<grad<<std::endl;

                    if( grad.x == 0.0F && grad.y == 0.0F && grad.z == 0.0F )
                    {
                        grad.set( 128.0F, 128.0F, 128.0F );
                    }
                    else
                    {
                        grad.x = osg::clampBetween( ( grad.x + 1.0F ) * 128.0F,
                                                    0.0F,
                                                    255.0F );
                        grad.y = osg::clampBetween( ( grad.y + 1.0F ) * 128.0F,
                                                    0.0F,
                                                    255.0F );
                        grad.z = osg::clampBetween( ( grad.z + 1.0F ) * 128.0F,
                                                    0.0F,
                                                    255.0F );
                    }

                    *( destination++ ) =
                        ( unsigned char )( grad.x );    // scale and bias X.
                    *( destination++ ) =
                        ( unsigned char )( grad.y );    // scale and bias Y.
                    *( destination++ ) =
                        ( unsigned char )( grad.z );    // scale and bias Z.

                    *destination++  = static_cast<unsigned char>( *ptr / 128 );

                    ptr            += sourcePixelIncrement;
                    left           += sourcePixelIncrement;
                    right          += sourcePixelIncrement;
                    above          += sourcePixelIncrement;
                    below          += sourcePixelIncrement;
                    in             += sourcePixelIncrement;
                    out            += sourcePixelIncrement;
                }
            }
            else if( dataType == GL_UNSIGNED_SHORT )
            {
                unsigned short* ptr =
                    ( unsigned short* )( image_3d->data( 1, t, r ) + alphaOffset );
                unsigned short* left =
                    ( unsigned short* )( image_3d->data( 0, t, r ) + alphaOffset );
                unsigned short* right =
                    ( unsigned short* )( image_3d->data( 2, t, r ) + alphaOffset );
                unsigned short* above =
                    ( unsigned short* )( image_3d->data( 1, t + 1, r ) + alphaOffset );
                unsigned short* below =
                    ( unsigned short* )( image_3d->data( 1, t - 1, r ) + alphaOffset );
                unsigned short* in =
                    ( unsigned short* )( image_3d->data( 1, t, r + 1 ) + alphaOffset );
                unsigned short* out =
                    ( unsigned short* )( image_3d->data( 1, t, r - 1 ) + alphaOffset );

                unsigned char* destination =
                    ( unsigned char* )normalmap_3d->data( 1, t, r );

                for( int s = 1; s < image_3d->s() - 1; ++s )
                {

                    osg::vec3 grad( ( float )( *left ) - ( float )( *right ),
                                    ( float )( *below ) - ( float )( *above ),
                                    ( float )( *out ) - ( float )( *in ) );

                    grad = osg::normalize( grad );

                    if( grad.x == 0.0F && grad.y == 0.0F && grad.z == 0.0F )
                    {
                        grad.set( 128.0F, 128.0F, 128.0F );
                    }
                    else
                    {
                        grad.x = osg::clampBetween( ( grad.x + 1.0F ) * 128.0F,
                                                    0.0F,
                                                    255.0F );
                        grad.y = osg::clampBetween( ( grad.y + 1.0F ) * 128.0F,
                                                    0.0F,
                                                    255.0F );
                        grad.z = osg::clampBetween( ( grad.z + 1.0F ) * 128.0F,
                                                    0.0F,
                                                    255.0F );
                    }

                    *( destination++ ) =
                        ( unsigned char )( grad.x );    // scale and bias X.
                    *( destination++ ) =
                        ( unsigned char )( grad.y );    // scale and bias Y.
                    *( destination++ ) =
                        ( unsigned char )( grad.z );    // scale and bias Z.

                    *destination++  = *ptr / 256;

                    ptr            += sourcePixelIncrement;
                    left           += sourcePixelIncrement;
                    right          += sourcePixelIncrement;
                    above          += sourcePixelIncrement;
                    below          += sourcePixelIncrement;
                    in             += sourcePixelIncrement;
                    out            += sourcePixelIncrement;
                }
            }
        }
    }

    OSG_INFO << "Created NormalMapTexture" << std::endl;

    return normalmap_3d.release();
}

/////////////////////////////////////////////////////////////////////////////
//
// applyTransferFunction
//
struct ApplyTransferFunctionOperator : public osg::CastAndScaleToFloatOperation
{
        ApplyTransferFunctionOperator( osg::TransferFunction1D* tf,
                                       unsigned char*           data ) :
            _tf( tf ),
            _data( data )
        {
        }

        inline void
        luminance( float l ) const
        {
            osg::vec4 c = _tf->getColor( l );
            // std::cout<<"l = "<<l<<" c="<<c<<std::endl;
            *( _data++ ) = ( unsigned char )( c[0] * 255.0F + 0.5F );
            *( _data++ ) = ( unsigned char )( c[1] * 255.0F + 0.5F );
            *( _data++ ) = ( unsigned char )( c[2] * 255.0F + 0.5F );
            *( _data++ ) = ( unsigned char )( c[3] * 255.0F + 0.5F );
        }

        inline void
        alpha( float a ) const
        {
            luminance( a );
        }

        inline void
        luminance_alpha( float l,
                         float /*a*/ ) const
        {
            luminance( l );
        }

        inline void
        rgb( float r,
             float g,
             float b ) const
        {
            luminance( ( r + g + b ) * 0.3333333F );
        }

        inline void
        rgba( float /*r*/,
              float /*g*/,
              float /*b*/,
              float a ) const
        {
            luminance( a );
        }

        mutable osg::ref_ptr<osg::TransferFunction1D> _tf;
        mutable unsigned char*                        _data;
};

osg::Image*
osgVolume::applyTransferFunction( osg::Image*              image,
                                  osg::TransferFunction1D* transferFunction )
{
    OSG_INFO << "Applying transfer function" << std::endl;

    osg::Image* output_image = new osg::Image;
    output_image->allocateImage( image->s(),
                                 image->t(),
                                 image->r(),
                                 GL_RGBA,
                                 GL_UNSIGNED_BYTE );

    ApplyTransferFunctionOperator op( transferFunction, output_image->data() );
    osg::readImage( image, op );

    return output_image;
}
