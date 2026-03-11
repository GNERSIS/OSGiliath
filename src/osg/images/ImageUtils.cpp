/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Image manipulation utilities: sub-image copy, format conversion,
 * mipmap generation, and pixel-data access helpers.
 */
#include <osg/images/ImageUtils.hpp>

#include "dxtctool.hpp"

#include <float.h>
#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/textures/Texture.hpp>
#include <string.h>

namespace osg
{

    struct FindRangeOperator : public CastAndScaleToFloatOperation
    {
            FindRangeOperator() :
                _rmin( FLT_MAX ),
                _rmax( -FLT_MAX ),
                _gmin( FLT_MAX ),
                _gmax( -FLT_MAX ),
                _bmin( FLT_MAX ),
                _bmax( -FLT_MAX ),
                _amin( FLT_MAX ),
                _amax( -FLT_MAX )
            {
            }

            float _rmin, _rmax, _gmin, _gmax, _bmin, _bmax, _amin, _amax;

            inline void
            luminance( float l )
            {
                rgba( l, l, l, l );
            }

            inline void
            alpha( float a )
            {
                rgba( 1.0F, 1.0F, 1.0F, a );
            }

            inline void
            luminance_alpha( float l,
                             float a )
            {
                rgba( l, l, l, a );
            }

            inline void
            rgb( float r,
                 float g,
                 float b )
            {
                rgba( r, g, b, 1.0F );
            }

            inline void
            rgba( float r,
                  float g,
                  float b,
                  float a )
            {
                _rmin = std::min( r, _rmin );
                _rmax = std::max( r, _rmax );
                _gmin = std::min( g, _gmin );
                _gmax = std::max( g, _gmax );
                _bmin = std::min( b, _bmin );
                _bmax = std::max( b, _bmax );
                _amin = std::min( a, _amin );
                _amax = std::max( a, _amax );
            }
    };

    struct OffsetAndScaleOperator
    {
            OffsetAndScaleOperator( const osg::vec4& offset,
                                    const osg::vec4& scale ) :
                _offset( offset ),
                _scale( scale )
            {
            }

            osg::vec4 _offset;
            osg::vec4 _scale;

            inline void
            luminance( float& l ) const
            {
                l = _offset.r + l * _scale.r;
            }

            inline void
            alpha( float& a ) const
            {
                a = _offset.a + a * _scale.a;
            }

            inline void
            luminance_alpha( float& l,
                             float& a ) const
            {
                l = _offset.r + l * _scale.r;
                a = _offset.a + a * _scale.a;
            }

            inline void
            rgb( float& r,
                 float& g,
                 float& b ) const
            {
                r = _offset.r + r * _scale.r;
                g = _offset.g + g * _scale.g;
                b = _offset.b + b * _scale.b;
            }

            inline void
            rgba( float& r,
                  float& g,
                  float& b,
                  float& a ) const
            {
                r = _offset.r + r * _scale.r;
                g = _offset.g + g * _scale.g;
                b = _offset.b + b * _scale.b;
                a = _offset.a + a * _scale.a;
            }
    };

    bool
    computeMinMax( const osg::Image* image,
                   osg::vec4&        minValue,
                   osg::vec4&        maxValue )
    {
        if( !image )
        {
            return false;
        }

        osg::FindRangeOperator rangeOp;
        readImage( image, rangeOp );
        minValue.r = rangeOp._rmin;
        minValue.g = rangeOp._gmin;
        minValue.b = rangeOp._bmin;
        minValue.a = rangeOp._amin;

        maxValue.r = rangeOp._rmax;
        maxValue.g = rangeOp._gmax;
        maxValue.b = rangeOp._bmax;
        maxValue.a = rangeOp._amax;

        return minValue.r <=
               maxValue.r &&
               minValue.g <=
               maxValue.g &&
               minValue.b <=
               maxValue.b &&
               minValue.a <= maxValue.a;
    }

    bool
    offsetAndScaleImage( osg::Image*      image,
                         const osg::vec4& offset,
                         const osg::vec4& scale )
    {
        if( !image )
        {
            return false;
        }

        modifyImage( image, OffsetAndScaleOperator( offset, scale ) );

        return true;
    }

    template<typename SRC,
             typename DEST>
    void
    _copyRowAndScale( const SRC* src,
                      DEST*      dest,
                      int        num,
                      float      scale )
    {
        if( scale == 1.0 )
        {
            for( int i = 0; i < num; ++i )
            {
                *dest = DEST( *src );
                ++dest;
                ++src;
            }
        }
        else
        {
            for( int i = 0; i < num; ++i )
            {
                *dest = DEST( float( *src ) * scale );
                ++dest;
                ++src;
            }
        }
    }

    template<typename DEST>
    void
    _copyRowAndScale( const unsigned char* src,
                      GLenum               srcDataType,
                      DEST*                dest,
                      int                  num,
                      float                scale )
    {
        switch( srcDataType )
        {
            case( GL_BYTE ) :
                _copyRowAndScale( ( char* )src, dest, num, scale );
                break;
            case( GL_UNSIGNED_BYTE ) :
                _copyRowAndScale( ( unsigned char* )src, dest, num, scale );
                break;
            case( GL_SHORT ) :
                _copyRowAndScale( ( short* )src, dest, num, scale );
                break;
            case( GL_UNSIGNED_SHORT ) :
                _copyRowAndScale( ( unsigned short* )src, dest, num, scale );
                break;
            case( GL_INT ) :
                _copyRowAndScale( ( int* )src, dest, num, scale );
                break;
            case( GL_UNSIGNED_INT ) :
                _copyRowAndScale( ( unsigned int* )src, dest, num, scale );
                break;
            case( GL_FLOAT ) :
                _copyRowAndScale( ( float* )src, dest, num, scale );
                break;
        }
    }

    void
    _copyRowAndScale( const unsigned char* src,
                      GLenum               srcDataType,
                      unsigned char*       dest,
                      GLenum               dstDataType,
                      int                  num,
                      float                scale )
    {
        switch( dstDataType )
        {
            case( GL_BYTE ) :
                _copyRowAndScale( src, srcDataType, ( char* )dest, num, scale );
                break;
            case( GL_UNSIGNED_BYTE ) :
                _copyRowAndScale( src, srcDataType, ( unsigned char* )dest, num, scale );
                break;
            case( GL_SHORT ) :
                _copyRowAndScale( src, srcDataType, ( short* )dest, num, scale );
                break;
            case( GL_UNSIGNED_SHORT ) :
                _copyRowAndScale( src,
                                  srcDataType,
                                  ( unsigned short* )dest,
                                  num,
                                  scale );
                break;
            case( GL_INT ) :
                _copyRowAndScale( src, srcDataType, ( int* )dest, num, scale );
                break;
            case( GL_UNSIGNED_INT ) :
                _copyRowAndScale( src, srcDataType, ( unsigned int* )dest, num, scale );
                break;
            case( GL_FLOAT ) :
                _copyRowAndScale( src, srcDataType, ( float* )dest, num, scale );
                break;
        }
    }

    struct RecordRowOperator : public CastAndScaleToFloatOperation
    {
            RecordRowOperator( unsigned int num ) :
                _colours( num ),
                _pos( 0 )
            {
            }

            mutable std::vector<osg::vec4> _colours;
            mutable unsigned int           _pos;

            inline void
            luminance( float l ) const
            {
                rgba( l, l, l, 1.0F );
            }

            inline void
            alpha( float a ) const
            {
                rgba( 1.0F, 1.0F, 1.0F, a );
            }

            inline void
            luminance_alpha( float l,
                             float a ) const
            {
                rgba( l, l, l, a );
            }

            inline void
            rgb( float r,
                 float g,
                 float b ) const
            {
                rgba( r, g, b, 1.0F );
            }

            inline void
            rgba( float r,
                  float g,
                  float b,
                  float a ) const
            {
                _colours[_pos++].set( r, g, b, a );
            }
    };

    struct WriteRowOperator
    {
            WriteRowOperator() :
                _pos( 0 )
            {
            }

            WriteRowOperator( unsigned int num ) :
                _colours( num ),
                _pos( 0 )
            {
            }

            std::vector<osg::vec4> _colours;
            mutable unsigned int   _pos;

            inline void
            luminance( float& l ) const
            {
                l = _colours[_pos++].r;
            }

            inline void
            alpha( float& a ) const
            {
                a = _colours[_pos++].a;
            }

            inline void
            luminance_alpha( float& l,
                             float& a ) const
            {
                l = _colours[_pos].r;
                a = _colours[_pos++].a;
            }

            inline void
            rgb( float& r,
                 float& g,
                 float& b ) const
            {
                r = _colours[_pos].r;
                g = _colours[_pos].g;
                b = _colours[_pos].b;
            }

            inline void
            rgba( float& r,
                  float& g,
                  float& b,
                  float& a ) const
            {
                r = _colours[_pos].r;
                g = _colours[_pos].g;
                b = _colours[_pos].b;
                a = _colours[_pos++].a;
            }
    };

    bool
    copyImage( const osg::Image* srcImage,
               int               src_s,
               int               src_t,
               int               src_r,
               int               width,
               int               height,
               int               depth,
               osg::Image*       destImage,
               int               dest_s,
               int               dest_t,
               int               dest_r,
               bool              doRescale )
    {
        if( ( dest_s + width ) > ( destImage->s() ) )
        {
            OSG_NOTICE << "copyImage(" << srcImage << ", " << src_s << ", " << src_t
                       << ", " << src_r << ", " << width << ", " << height << ", "
                       << depth << std::endl;
            OSG_NOTICE << "          " << destImage << ", " << dest_s << ", " << dest_t
                       << ", " << dest_r << ", " << doRescale << ")" << std::endl;
            OSG_NOTICE << "   input width too large." << std::endl;
            return false;
        }

        if( ( dest_t + height ) > ( destImage->t() ) )
        {
            OSG_NOTICE << "copyImage(" << srcImage << ", " << src_s << ", " << src_t
                       << ", " << src_r << ", " << width << ", " << height << ", "
                       << depth << std::endl;
            OSG_NOTICE << "          " << destImage << ", " << dest_s << ", " << dest_t
                       << ", " << dest_r << ", " << doRescale << ")" << std::endl;
            OSG_NOTICE << "   input height too large." << std::endl;
            return false;
        }

        if( ( dest_r + depth ) > ( destImage->r() ) )
        {
            OSG_NOTICE << "copyImage(" << srcImage << ", " << src_s << ", " << src_t
                       << ", " << src_r << ", " << width << ", " << height << ", "
                       << depth << std::endl;
            OSG_NOTICE << "          " << destImage << ", " << dest_s << ", " << dest_t
                       << ", " << dest_r << ", " << doRescale << ")" << std::endl;
            OSG_NOTICE << "   input depth too large." << std::endl;
            return false;
        }

        float scale = 1.0F;
        if( doRescale && srcImage->getDataType() != destImage->getDataType() )
        {
            switch( srcImage->getDataType() )
            {
                case( GL_BYTE ) :
                    scale = 1.0F / 128.0F;
                    break;
                case( GL_UNSIGNED_BYTE ) :
                    scale = 1.0F / 255.0F;
                    break;
                case( GL_SHORT ) :
                    scale = 1.0F / 32768.0F;
                    break;
                case( GL_UNSIGNED_SHORT ) :
                    scale = 1.0F / 65535.0F;
                    break;
                case( GL_INT ) :
                    scale = 1.0F / 2147483648.0F;
                    break;
                case( GL_UNSIGNED_INT ) :
                    scale = 1.0F / 4294967295.0F;
                    break;
                case( GL_FLOAT ) :
                    scale = 1.0F;
                    break;
            }
            switch( destImage->getDataType() )
            {
                case( GL_BYTE ) :
                    scale *= 128.0F;
                    break;
                case( GL_UNSIGNED_BYTE ) :
                    scale *= 255.0F;
                    break;
                case( GL_SHORT ) :
                    scale *= 32768.0F;
                    break;
                case( GL_UNSIGNED_SHORT ) :
                    scale *= 65535.0F;
                    break;
                case( GL_INT ) :
                    scale *= 2147483648.0F;
                    break;
                case( GL_UNSIGNED_INT ) :
                    scale *= 4294967295.0F;
                    break;
                case( GL_FLOAT ) :
                    scale *= 1.0F;
                    break;
            }
        }

        if( srcImage->getPixelFormat() == destImage->getPixelFormat() )
        {
            // OSG_NOTICE<<"copyImage("<<srcImage<<", "<<src_s<<", "<< src_t<<",
            // "<<src_r<<", "<<width<<", "<<height<<", "<<depth<<std::endl; OSG_NOTICE<<"
            // "<<destImage<<", "<<dest_s<<", "<< dest_t<<", "<<dest_r<<",
            // "<<doRescale<<")"<<std::endl;

            if( srcImage->getDataType() == destImage->getDataType() && !doRescale )
            {
                // OSG_NOTICE<<"   Compatible pixelFormat and dataType."<<std::endl;
                for( int slice = 0; slice < depth; ++slice )
                {
                    for( int row = 0; row < height; ++row )
                    {
                        const unsigned char* srcData =
                            srcImage->data( static_cast<unsigned int>( src_s ),
                                            static_cast<unsigned int>( src_t + row ),
                                            static_cast<unsigned int>( src_r + slice ) );
                        unsigned char* destData = destImage->data(
                            static_cast<unsigned int>( dest_s ),
                            static_cast<unsigned int>( dest_t + row ),
                            static_cast<unsigned int>( dest_r + slice )
                        );
                        memcpy( destData,
                                srcData,
                                ( static_cast<unsigned int>( width ) *
                                  destImage->getPixelSizeInBits() ) /
                                    8 );
                    }
                }
                return true;
            }
            else
            {
                // OSG_NOTICE<<"   Compatible pixelFormat and incompatible
                // dataType."<<std::endl;
                for( int slice = 0; slice < depth; ++slice )
                {
                    for( int row = 0; row < height; ++row )
                    {
                        const unsigned char* srcData =
                            srcImage->data( static_cast<unsigned int>( src_s ),
                                            static_cast<unsigned int>( src_t + row ),
                                            static_cast<unsigned int>( src_r + slice ) );
                        unsigned char* destData = destImage->data(
                            static_cast<unsigned int>( dest_s ),
                            static_cast<unsigned int>( dest_t + row ),
                            static_cast<unsigned int>( dest_r + slice )
                        );
                        unsigned int numComponents = osg::Image::computeNumComponents(
                            destImage->getPixelFormat()
                        );

                        _copyRowAndScale(
                            srcData,
                            srcImage->getDataType(),
                            destData,
                            destImage->getDataType(),
                            static_cast<int>( static_cast<unsigned int>( width ) *
                                              numComponents ),
                            scale
                        );
                    }
                }

                return true;
            }
        }
        else
        {
            // OSG_NOTICE<<"copyImage("<<srcImage<<", "<<src_s<<", "<< src_t<<",
            // "<<src_r<<", "<<width<<", "<<height<<", "<<depth<<std::endl; OSG_NOTICE<<"
            // "<<destImage<<", "<<dest_s<<", "<< dest_t<<", "<<dest_r<<",
            // "<<doRescale<<")"<<std::endl;

            RecordRowOperator readOp( static_cast<unsigned int>( width ) );
            WriteRowOperator  writeOp;

            for( int slice = 0; slice < depth; ++slice )
            {
                for( int row = 0; row < height; ++row )
                {

                    // reset the indices to beginning
                    readOp._pos  = 0;
                    writeOp._pos = 0;

                    // read the pixels into readOp's _colour array
                    readRow( static_cast<unsigned int>( width ),
                             srcImage->getPixelFormat(),
                             srcImage->getDataType(),
                             srcImage->data( static_cast<unsigned int>( src_s ),
                                             static_cast<unsigned int>( src_t + row ),
                                             static_cast<unsigned int>( src_r +
                                                                        slice ) ),
                             readOp );

                    // pass readOp's _colour array contents over to writeOp (note this is
                    // just a pointer swap).
                    writeOp._colours.swap( readOp._colours );

                    modifyRow(
                        static_cast<unsigned int>( width ),
                        destImage->getPixelFormat(),
                        destImage->getDataType(),
                        destImage->data( static_cast<unsigned int>( dest_s ),
                                         static_cast<unsigned int>( dest_t + row ),
                                         static_cast<unsigned int>( dest_r + slice ) ),
                        writeOp
                    );

                    // return readOp's _colour array contents back to its rightful owner.
                    writeOp._colours.swap( readOp._colours );
                }
            }

            return true;
        }
    }

    struct SetToColourOperator
    {
            SetToColourOperator( const osg::vec4& colour ) :
                _colour( colour )
            {
            }

            inline void
            luminance( float& l ) const
            {
                l = ( _colour.r + _colour.g + _colour.b ) * 0.333333F;
            }

            inline void
            alpha( float& a ) const
            {
                a = _colour.a;
            }

            inline void
            luminance_alpha( float& l,
                             float& a ) const
            {
                l = ( _colour.r + _colour.g + _colour.b ) * 0.333333F;
                a = _colour.a;
            }

            inline void
            rgb( float& r,
                 float& g,
                 float& b ) const
            {
                r = _colour.r;
                g = _colour.g;
                b = _colour.b;
            }

            inline void
            rgba( float& r,
                  float& g,
                  float& b,
                  float& a ) const
            {
                r = _colour.r;
                g = _colour.g;
                b = _colour.b;
                a = _colour.a;
            }

            osg::vec4 _colour;
    };

    bool
    clearImageToColor( osg::Image*      image,
                       const osg::vec4& colour )
    {
        if( !image )
        {
            return false;
        }

        modifyImage( image, SetToColourOperator( colour ) );

        return true;
    }

    /** Search through the list of Images and find the maximum number of components used
     * among the images.*/
    unsigned int
    maximimNumOfComponents( const ImageList& imageList )
    {
        unsigned int max_components = 0;
        for( osg::ImageList::const_iterator itr = imageList.begin();
             itr != imageList.end();
             ++itr )
        {
            osg::Image* image       = itr->get();
            GLenum      pixelFormat = image->getPixelFormat();
            if( pixelFormat ==
                GL_RED ||
                pixelFormat ==
                GL_RG ||
                pixelFormat ==
                GL_RGB ||
                pixelFormat ==
                GL_RGBA ||
                pixelFormat ==
                GL_BGR ||
                pixelFormat == GL_BGRA )
            {
                max_components = maximum( Image::computeNumComponents( pixelFormat ),
                                          max_components );
            }
        }
        return max_components;
    }

    osg::Image*
    createImage3D( const ImageList& imageList,
                   GLenum           desiredPixelFormat,
                   int              s_maximumImageSize,
                   int              t_maximumImageSize,
                   int              r_maximumImageSize,
                   bool             resizeToPowerOfTwo )
    {
        OSG_INFO << "createImage3D(..)" << std::endl;
        int max_s   = 0;
        int max_t   = 0;
        int total_r = 0;
        for( osg::ImageList::const_iterator itr = imageList.begin();
             itr != imageList.end();
             ++itr )
        {
            osg::Image* image       = itr->get();
            GLenum      pixelFormat = image->getPixelFormat();
            if( pixelFormat ==
                GL_RED ||
                pixelFormat ==
                GL_RG ||
                pixelFormat ==
                GL_RGB ||
                pixelFormat ==
                GL_RGBA ||
                pixelFormat ==
                GL_BGR ||
                pixelFormat == GL_BGRA )
            {
                max_s    = maximum( image->s(), max_s );
                max_t    = maximum( image->t(), max_t );
                total_r += image->r();
            }
            else
            {
                OSG_INFO << "Image " << image->getFileName()
                         << " has unsuitable pixel format 0x" << std::hex << pixelFormat
                         << std::dec << std::endl;
            }
        }

        // bool remapRGBtoLuminance;
        // bool remapRGBtoRGBA;

        if( desiredPixelFormat == 0 )
        {
            unsigned int max_components = maximimNumOfComponents( imageList );
            switch( max_components )
            {
                case( 1 ) :
                    OSG_INFO << "desiredPixelFormat = GL_RED" << std::endl;
                    desiredPixelFormat = GL_RED;
                    break;
                case( 2 ) :
                    OSG_INFO << "desiredPixelFormat = GL_RG" << std::endl;
                    desiredPixelFormat = GL_RG;
                    break;
                case( 3 ) :
                    OSG_INFO << "desiredPixelFormat = GL_RGB" << std::endl;
                    desiredPixelFormat = GL_RGB;
                    break;
                case( 4 ) :
                    OSG_INFO << "desiredPixelFormat = GL_RGBA" << std::endl;
                    desiredPixelFormat = GL_RGBA;
                    break;
            }
        }
        if( desiredPixelFormat == 0 )
        {
            return 0;
        }

        // compute nearest powers of two for each axis.

        int isize_s = 1;
        int isize_t = 1;
        int isize_r = 1;

        if( resizeToPowerOfTwo )
        {
            while( isize_s < max_s && isize_s < s_maximumImageSize )
            {
                isize_s *= 2;
            }
            while( isize_t < max_t && isize_t < t_maximumImageSize )
            {
                isize_t *= 2;
            }
            while( isize_r < total_r && isize_r < r_maximumImageSize )
            {
                isize_r *= 2;
            }
        }
        else
        {
            isize_s = max_s;
            isize_t = max_t;
            isize_r = total_r;
        }

        // now allocate the 3d texture;
        osg::ref_ptr<osg::Image> image_3d = new osg::Image;
        image_3d->allocateImage( isize_s,
                                 isize_t,
                                 isize_r,
                                 desiredPixelFormat,
                                 GL_UNSIGNED_BYTE );

        int r_offset    = ( total_r < isize_r ) ? ( isize_r - total_r ) / 2 : 0;

        int curr_dest_r = r_offset;

        // copy across the values from the source images into the image_3d.
        for( osg::ImageList::const_iterator itr = imageList.begin();
             itr != imageList.end();
             ++itr )
        {
            osg::Image* image       = itr->get();
            GLenum      pixelFormat = image->getPixelFormat();
            if( pixelFormat ==
                GL_RED ||
                pixelFormat ==
                GL_RG ||
                pixelFormat ==
                GL_RGB ||
                pixelFormat ==
                GL_RGBA ||
                pixelFormat ==
                GL_BGR ||
                pixelFormat == GL_BGRA )
            {

                int num_s = minimum( image->s(), image_3d->s() );
                int num_t = minimum( image->t(), image_3d->t() );
                int num_r = minimum( image->r(), image_3d->r() - curr_dest_r );

                int s_offset_dest =
                    ( image->s() < isize_s ) ? ( isize_s - image->s() ) / 2 : 0;
                int t_offset_dest =
                    ( image->t() < isize_t ) ? ( isize_t - image->t() ) / 2 : 0;

                copyImage( image,
                           0,
                           0,
                           0,
                           num_s,
                           num_t,
                           num_r,
                           image_3d.get(),
                           s_offset_dest,
                           t_offset_dest,
                           curr_dest_r,
                           false );

                curr_dest_r += num_r;
            }
        }

        return image_3d.release();
    }

    struct ModulateAlphaByLuminanceOperator
    {
            ModulateAlphaByLuminanceOperator()
            {
            }

            inline void
            luminance( float& ) const
            {
            }

            inline void
            alpha( float& ) const
            {
            }

            inline void
            luminance_alpha( float& l,
                             float& a ) const
            {
                a *= l;
            }

            inline void
            rgb( float&,
                 float&,
                 float& ) const
            {
            }

            inline void
            rgba( float& r,
                  float& g,
                  float& b,
                  float& a ) const
            {
                float l  = ( r + g + b ) * 0.3333333F;
                a       *= l;
            }
    };

    osg::Image*
    createImage3DWithAlpha( const ImageList& imageList,
                            int              s_maximumImageSize,
                            int              t_maximumImageSize,
                            int              r_maximumImageSize,
                            bool             resizeToPowerOfTwo )
    {
        GLenum       desiredPixelFormat       = 0;
        bool         modulateAlphaByLuminance = false;

        unsigned int maxNumComponents         = maximimNumOfComponents( imageList );
        if( maxNumComponents == 3 )
        {
            desiredPixelFormat       = GL_RGBA;
            modulateAlphaByLuminance = true;
        }

        osg::ref_ptr<osg::Image> image = createImage3D( imageList,
                                                        desiredPixelFormat,
                                                        s_maximumImageSize,
                                                        t_maximumImageSize,
                                                        r_maximumImageSize,
                                                        resizeToPowerOfTwo );
        if( image.valid() )
        {
            if( modulateAlphaByLuminance )
            {
                modifyImage( image.get(), ModulateAlphaByLuminanceOperator() );
            }
            return image.release();
        }
        else
        {
            return 0;
        }
    }

    static void
    fillSpotLightImage( unsigned char*   ptr,
                        const osg::vec4& centerColour,
                        const osg::vec4& backgroudColour,
                        unsigned int     size,
                        float            power )
    {
        if( size == 1 )
        {
            float     r     = 0.5F;
            osg::vec4 color = centerColour * r + backgroudColour * ( 1.0F - r );
            *ptr++          = ( unsigned char )( ( color[0] ) * 255.0F );
            *ptr++          = ( unsigned char )( ( color[1] ) * 255.0F );
            *ptr++          = ( unsigned char )( ( color[2] ) * 255.0F );
            *ptr++          = ( unsigned char )( ( color[3] ) * 255.0F );
            return;
        }

        float mid = ( float( size ) - 1.0F ) * 0.5F;
        float div = 2.0F / float( size );
        for( unsigned int row = 0; row < size; ++row )
        {
            // unsigned char* ptr = image->data(0,r,0);
            for( unsigned int col = 0; col < size; ++col )
            {
                float dx = ( float( col ) - mid ) * div;
                float dy = ( float( row ) - mid ) * div;
                float r  = powf( 1.0F - sqrtf( dx * dx + dy * dy ), power );
                if( r < 0.0F )
                {
                    r = 0.0F;
                }
                osg::vec4 color = centerColour * r + backgroudColour * ( 1.0F - r );
                *ptr++          = ( unsigned char )( ( color[0] ) * 255.0F );
                *ptr++          = ( unsigned char )( ( color[1] ) * 255.0F );
                *ptr++          = ( unsigned char )( ( color[2] ) * 255.0F );
                *ptr++          = ( unsigned char )( ( color[3] ) * 255.0F );
            }
        }
    }

    osg::Image*
    createSpotLightImage( const osg::vec4& centerColour,
                          const osg::vec4& backgroudColour,
                          unsigned int     size,
                          float            power )
    {

#if 0
    osg::Image* image = new osg::Image;
    unsigned char* ptr = image->data(0,0,0);
    fillSpotLightImage(ptr, centerColour, backgroudColour, size, power);

    return image;
#else
        osg::Image*                image = new osg::Image;
        osg::Image::MipmapDataType mipmapData;
        unsigned int               s         = size;
        unsigned int               totalSize = 0;
        unsigned                   i;
        for( i = 0; s > 0; s >>= 1, ++i )
        {
            if( i > 0 )
            {
                mipmapData.push_back( totalSize );
            }
            totalSize += s * s * 4;
        }

        unsigned char* ptr = new unsigned char[totalSize];
        image->setImage( static_cast<int>( size ),
                         static_cast<int>( size ),
                         static_cast<int>( size ),
                         GL_RGBA,
                         GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         ptr,
                         osg::Image::USE_NEW_DELETE,
                         1 );

        image->setMipmapLevels( mipmapData );

        s = size;
        for( i = 0; s > 0; s >>= 1, ++i )
        {
            fillSpotLightImage( ptr, centerColour, backgroudColour, s, power );
            ptr += s * s * 4;
        }

        return image;
#endif
    }

    struct ModulateAlphaByColorOperator
    {
            ModulateAlphaByColorOperator( const osg::vec4& colour ) :
                _colour( colour )
            {
                _lum = osg::length( _colour );
            }

            osg::vec4 _colour;
            float     _lum;

            inline void
            luminance( float& ) const
            {
            }

            inline void
            alpha( float& ) const
            {
            }

            inline void
            luminance_alpha( float& l,
                             float& a ) const
            {
                a *= l * _lum;
            }

            inline void
            rgb( float&,
                 float&,
                 float& ) const
            {
            }

            inline void
            rgba( float& r,
                  float& g,
                  float& b,
                  float& a ) const
            {
                a = ( r * _colour.r + g * _colour.g + b * _colour.b + a * _colour.a );
            }
    };

    struct ReplaceAlphaWithLuminanceOperator
    {
            ReplaceAlphaWithLuminanceOperator()
            {
            }

            inline void
            luminance( float& ) const
            {
            }

            inline void
            alpha( float& ) const
            {
            }

            inline void
            luminance_alpha( float& l,
                             float& a ) const
            {
                a = l;
            }

            inline void
            rgb( float&,
                 float&,
                 float& ) const
            {
            }

            inline void
            rgba( float& r,
                  float& g,
                  float& b,
                  float& a ) const
            {
                float l = ( r + g + b ) * 0.3333333F;
                a       = l;
            }
    };

    osg::Image*
    colorSpaceConversion( ColorSpaceOperation op,
                          osg::Image*         image,
                          const osg::vec4&    colour )
    {
        GLenum requiredPixelFormat = image->getPixelFormat();
        switch( op )
        {
            case( MODULATE_ALPHA_BY_LUMINANCE ) :
            case( MODULATE_ALPHA_BY_COLOR ) :
            case( REPLACE_ALPHA_WITH_LUMINANCE ) :
                if( image->getPixelFormat() ==
                    GL_RGB ||
                    image->getPixelFormat() == GL_BGR )
                {
                    requiredPixelFormat = GL_RGBA;
                }
                break;
            case( REPLACE_RGB_WITH_LUMINANCE ) :
                if( image->getPixelFormat() ==
                    GL_RGB ||
                    image->getPixelFormat() == GL_BGR )
                {
                    requiredPixelFormat = GL_RED;
                }
                break;
            default :
                break;
        }

        if( requiredPixelFormat != image->getPixelFormat() )
        {
            osg::Image* newImage = new osg::Image;
            newImage->allocateImage( image->s(),
                                     image->t(),
                                     image->r(),
                                     requiredPixelFormat,
                                     image->getDataType() );
            osg::copyImage( image,
                            0,
                            0,
                            0,
                            image->s(),
                            image->t(),
                            image->r(),
                            newImage,
                            0,
                            0,
                            0,
                            false );

            image = newImage;
        }

        switch( op )
        {
            case( MODULATE_ALPHA_BY_LUMINANCE ) :
                {
                    OSG_NOTICE << "doing conversion MODULATE_ALPHA_BY_LUMINANCE"
                               << std::endl;
                    osg::modifyImage( image, ModulateAlphaByLuminanceOperator() );
                    return image;
                }
            case( MODULATE_ALPHA_BY_COLOR ) :
                {
                    OSG_NOTICE << "doing conversion MODULATE_ALPHA_BY_COLOUR"
                               << std::endl;
                    osg::modifyImage( image, ModulateAlphaByColorOperator( colour ) );
                    return image;
                }
            case( REPLACE_ALPHA_WITH_LUMINANCE ) :
                {
                    OSG_NOTICE << "doing conversion REPLACE_ALPHA_WITH_LUMINANCE"
                               << std::endl;
                    osg::modifyImage( image, ReplaceAlphaWithLuminanceOperator() );
                    return image;
                }
            case( REPLACE_RGB_WITH_LUMINANCE ) :
                {
                    OSG_NOTICE << "doing conversion REPLACE_RGB_WITH_LUMINANCE"
                               << std::endl;
                    // no work here required to be done as it'll already be done by
                    // copyImage above.
                    return image;
                }
            default :
                return image;
        }
    }

    OSG_EXPORT osg::Image*
               createImageWithOrientationConversion( const osg::Image* srcImage,
                                                     const osg::ivec3& srcOrigin,
                                                     const osg::ivec3& srcRow,
                                                     const osg::ivec3& srcColumn,
                                                     const osg::ivec3& srcLayer )
    {
        osg::ref_ptr<osg::Image> dstImage = new osg::Image;
        int                      width =
            std::max( std::max( osg::absolute( srcRow.x ), osg::absolute( srcRow.y ) ),
                      osg::absolute( srcRow.z ) );
        int          height = std::max( std::max( osg::absolute( srcColumn.x ),
                                                  osg::absolute( srcColumn.y ) ),
                                        osg::absolute( srcColumn.z ) );
        int          depth  = std::max( std::max( osg::absolute( srcLayer.x ),
                                                  osg::absolute( srcLayer.y ) ),
                                        osg::absolute( srcLayer.z ) );

        osg::ivec3   rowDelta( osg::signOrZero( srcRow.x ),
                               osg::signOrZero( srcRow.y ),
                               osg::signOrZero( srcRow.z ) );
        osg::ivec3   columnDelta( osg::signOrZero( srcColumn.x ),
                                  osg::signOrZero( srcColumn.y ),
                                  osg::signOrZero( srcColumn.z ) );
        osg::ivec3   layerDelta( osg::signOrZero( srcLayer.x ),
                                 osg::signOrZero( srcLayer.y ),
                                 osg::signOrZero( srcLayer.z ) );

        unsigned int pixelSizeInBits    = srcImage->getPixelSizeInBits();
        unsigned int pixelSizeInBytes   = pixelSizeInBits / 8;
        unsigned int pixelSizeRemainder = pixelSizeInBits % 8;
        if( dxtc_tool::isDXTC( srcImage->getPixelFormat() ) )
        {
            unsigned int DXTblockSize = 8;
            if( ( srcImage->getPixelFormat() == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT ) ||
                ( srcImage->getPixelFormat() == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT ) )
            {
                DXTblockSize = 16;
            }
            unsigned int DXTblocksWidht =
                static_cast<unsigned int>( ( srcImage->s() + 3 ) /
                                           4 );    // width in 4x4 blocks
            unsigned int DXTblocksHeight =
                static_cast<unsigned int>( ( srcImage->t() + 3 ) /
                                           4 );    // height in 4x4 blocks
            unsigned int dst_DXTblocksWidht =
                static_cast<unsigned int>( ( width + 3 ) / 4 );    // width in 4x4
                                                                   // blocks
            unsigned int dst_DXTblocksHeight =
                static_cast<unsigned int>( ( height + 3 ) /
                                           4 );    // height in 4x4 blocks

            dstImage->allocateImage( width,
                                     height,
                                     depth,
                                     srcImage->getPixelFormat(),
                                     srcImage->getDataType() );
            // copy across the pixels from the source image to the destination image.
            if( depth != 1 )
            {
                OSG_NOTICE << "Warning: createImageWithOrientationConversion(..) cannot "
                              "handle dxt-compressed images with depth."
                           << std::endl;
                return const_cast<osg::Image*>( srcImage );
            }
            for( int l = 0; l < depth; l += 4 )
            {
                for( int r = 0; r < height; r += 4 )
                {
                    osg::ivec3 cp( srcOrigin.x + columnDelta.x * r + layerDelta.x * l,
                                   srcOrigin.y + columnDelta.y * r + layerDelta.y * l,
                                   srcOrigin.z + columnDelta.z * r + layerDelta.z * l );
                    for( int c = 0; c < width; c += 4 )
                    {
                        unsigned int src_blockIndex =
                            static_cast<unsigned int>( cp.x >> 2 ) +
                            DXTblocksWidht *
                            ( static_cast<unsigned int>( cp.y >> 2 ) +
                              static_cast<unsigned int>( cp.z >> 2 ) *
                              DXTblocksHeight );
                        const unsigned char* src_block =
                            srcImage->data() + src_blockIndex * DXTblockSize;

                        unsigned int dst_blockIndex =
                            static_cast<unsigned int>( c >> 2 ) +
                            dst_DXTblocksWidht *
                            ( static_cast<unsigned int>( r >> 2 ) +
                              static_cast<unsigned int>( l >> 2 ) *
                              dst_DXTblocksHeight );
                        unsigned char* dst_block =
                            dstImage->data() + dst_blockIndex * DXTblockSize;

                        memcpy( ( void* )dst_block, ( void* )src_block, DXTblockSize );
                        osg::ivec3 srcSubOrigin( cp.x & 0X7, cp.y & 0X7, cp.z & 0X7 );
                        dxtc_tool::compressedBlockOrientationConversion(
                            srcImage->getPixelFormat(),
                            src_block,
                            dst_block,
                            srcSubOrigin,
                            rowDelta,
                            columnDelta
                        );

                        cp.x += 4 * rowDelta.x;
                        cp.y += 4 * rowDelta.y;
                        cp.z += 4 * rowDelta.z;
                    }
                }
            }
            return dstImage.release();
        }
        if( pixelSizeRemainder != 0 )
        {
            OSG_NOTICE << "Warning: createImageWithOrientationConversion(..) cannot "
                          "handle non byte aligned pixel formats."
                       << std::endl;
            return const_cast<osg::Image*>( srcImage );
        }

        dstImage->allocateImage( width,
                                 height,
                                 depth,
                                 srcImage->getPixelFormat(),
                                 srcImage->getDataType() );

        // copy across the pixels from the source image to the destination image.
        for( int l = 0; l < depth; l++ )
        {
            for( int r = 0; r < height; r++ )
            {
                osg::ivec3 cp( srcOrigin.x + columnDelta.x * r + layerDelta.x * l,
                               srcOrigin.y + columnDelta.y * r + layerDelta.y * l,
                               srcOrigin.z + columnDelta.z * r + layerDelta.z * l );

                for( int c = 0; c < width; c++ )
                {
                    // OSG_NOTICE<<"source cp = ("<<cp<<")  destination
                    // ("<<c<<","<<r<<","<<l<<")"<<std::endl;
                    const unsigned char* src_pixel =
                        srcImage->data( static_cast<unsigned int>( cp.x ),
                                        static_cast<unsigned int>( cp.y ),
                                        static_cast<unsigned int>( cp.z ) );
                    unsigned char* dst_pixel =
                        dstImage->data( static_cast<unsigned int>( c ),
                                        static_cast<unsigned int>( r ),
                                        static_cast<unsigned int>( l ) );
                    for( unsigned int i = 0; i < pixelSizeInBytes; ++i )
                    {
                        *( dst_pixel++ ) = *( src_pixel++ );
                    }
                    cp.x += rowDelta.x;
                    cp.y += rowDelta.y;
                    cp.z += rowDelta.z;
                }
            }
        }

        return dstImage.release();
    }

}
