/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * dxtc_pixels class.
 * Provides: VFlip.
 */
#pragma once

#include <osg/GL>
#include <osg/maths/vec3.hpp>
#include <osg/textures/Texture.hpp>

#if defined( _MSC_VER )

typedef __int8  dxtc_int8;
typedef __int16 dxtc_int16;
typedef __int32 dxtc_int32;
typedef __int64 dxtc_int64;

    #define HEX_0x000000000000FFFF 0X00'00'00'00'00'00'FF'FF
    #define HEX_0x000000000FFF0000 0X00'00'00'00'0F'FF'00'00
    #define HEX_0x000000FFF0000000 0X00'00'00'FF'F0'00'00'00
    #define HEX_0x000FFF0000000000 0X00'0F'FF'00'00'00'00'00
    #define HEX_0xFFF0000000000000 0XFF'F0'00'00'00'00'00'00

#else

typedef char      dxtc_int8;
typedef short     dxtc_int16;
typedef int       dxtc_int32;
typedef long long dxtc_int64;

    #define HEX_0x000000000000FFFF 0X00'00'00'00'00'00'FF'FFLL
    #define HEX_0x000000000FFF0000 0X00'00'00'00'0F'FF'00'00LL
    #define HEX_0x000000FFF0000000 0X00'00'00'FF'F0'00'00'00LL
    #define HEX_0x000FFF0000000000 0X00'0F'FF'00'00'00'00'00LL
    #define HEX_0xFFF0000000000000 0XFF'F0'00'00'00'00'00'00LL

#endif

namespace dxtc_tool
{

    // C-like function wrappers
    bool
    isDXTC( GLenum pixelFormat );

    bool
    VerticalFlip( size_t Width,
                  size_t Height,
                  GLenum Format,
                  void*  pPixels );

    bool
    isCompressedImageTranslucent( size_t Width,
                                  size_t Height,
                                  GLenum Format,
                                  void*  pPixels );

    // interpolate RGB565 colors with 2/3 part color1 and 1/3 part color2
    unsigned short
    interpolateColors21( unsigned short color1,
                         unsigned short color2 );
    // interpolate RGB565 colors with equal weights
    unsigned short
    interpolateColors11( unsigned short color1,
                         unsigned short color2 );

    bool
    CompressedImageGetColor( unsigned char  color[4],
                             unsigned int   s,
                             unsigned int   t,
                             unsigned int   r,
                             int            width,
                             int            height,
                             int            depth,
                             GLenum         format,
                             unsigned char* imageData );

    void
    compressedBlockOrientationConversion( const GLenum         format,
                                          const unsigned char* src_block,
                                          unsigned char*       dst_block,
                                          const osg::ivec3&    srcOrigin,
                                          const osg::ivec3&    rowDelta,
                                          const osg::ivec3&    columnDelta );

    void
    compressedBlockStripAlhpa( const GLenum         format,
                               const unsigned char* src_block,
                               unsigned char*       dst_block );

    // Class holding reference to DXTC image pixels
    class dxtc_pixels
    {
        public:

            inline dxtc_pixels( size_t Width,
                                size_t Height,
                                GLenum Format,
                                void*  pPixels );

            // Vertically flip the whole picture
            bool
            VFlip() const;

        protected:

            dxtc_pixels&
            operator=( const dxtc_pixels& )
            {
                return *this;
            }

            // Limitation check functions
            inline bool
            DXT1() const;
            inline bool
            DXT3() const;
            inline bool
            DXT5() const;
            inline bool
            OpenGLSize() const;
            inline bool
            SupportedFormat() const;

            // Vertical flipping functions
            void
            VFlip_DXT1() const;
            void
            VFlip_DXT3() const;
            void
            VFlip_DXT5() const;

            // Block vertical flipping functions
            inline void
            BVF_Color_H2(
                void* const pBlock
            ) const;    // V. flip one color block with its virtual height == 2
            inline void
            BVF_Color_H4(
                void* const pBlock
            ) const;    // V. flip one color block with its virtual height == 4
            inline void
            BVF_Color(
                void* const pBlock1,
                void* const pBlock2
            ) const;    // V. flip and swap two color blocks, with their virtual height
                        // == 4
            inline void
            BVF_Alpha_DXT3_H2(
                void* const pBlock
            ) const;    // V. flip one alpha (DXT3) block with its virtual height == 2
            inline void
            BVF_Alpha_DXT3_H4(
                void* const pBlock
            ) const;    // V. flip one alpha (DXT3) block with its virtual height == 4
            inline void
            BVF_Alpha_DXT3(
                void* const pBlock1,
                void* const pBlock2
            ) const;    // V. flip and swap two alpha (DXT3) blocks, with their virtual
                        // height == 4
            inline void
            BVF_Alpha_DXT5_H2(
                void* const pBlock
            ) const;    // V. flip one alpha (DXT5) block with its virtual height == 2
            inline void
            BVF_Alpha_DXT5_H4(
                void* const pBlock
            ) const;    // V. flip one alpha (DXT5) block with its virtual height == 4
            inline void
            BVF_Alpha_DXT5(
                void* const pBlock1,
                void* const pBlock2
            ) const;    // V. flip and swap two alpha (DXT5) blocks, with their virtual
                        // height == 4

            // Block localization functions
            inline void*
                                GetBlock( size_t i,
                                          size_t j,
                                          size_t BlockSize ) const;

            // mighty const and var
            static const size_t BSIZE_DXT1;
            static const size_t BSIZE_DXT3;
            static const size_t BSIZE_DXT5;
            static const size_t BSIZE_ALPHA_DXT3;
            static const size_t BSIZE_ALPHA_DXT5;

            const size_t        m_Width;
            const size_t        m_Height;
            const GLenum        m_Format;
            void* const         m_pPixels;
    };

    //////////////////////////////////////////////////////////////////////
    // C-Like Function Wrappers
    //////////////////////////////////////////////////////////////////////

    inline bool
    isDXTC( GLenum pixelFormat )
    {
        switch( pixelFormat )
        {
            case( GL_COMPRESSED_RGB_S3TC_DXT1_EXT ) :
            case( GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ) :
            case( GL_COMPRESSED_RGBA_S3TC_DXT3_EXT ) :
            case( GL_COMPRESSED_RGBA_S3TC_DXT5_EXT ) :
            case( GL_COMPRESSED_SRGB_S3TC_DXT1_EXT ) :
            case( GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT ) :
            case( GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT ) :
            case( GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT ) :
                return true;
            default :
                return false;
        }
    }

    inline bool
    VerticalFlip( size_t Width,
                  size_t Height,
                  GLenum Format,
                  void*  pPixels )
    {
        return ( dxtc_pixels( Width, Height, Format, pPixels ) ).VFlip();
    }

    //////////////////////////////////////////////////////////////////////
    // dxtc_pixels Inline Functions
    //////////////////////////////////////////////////////////////////////

    inline dxtc_pixels::dxtc_pixels( size_t Width,
                                     size_t Height,
                                     GLenum Format,
                                     void*  pPixels ) :
        m_Width( Width ),
        m_Height( Height ),
        m_Format( Format ),
        m_pPixels( pPixels )
    {
    }

    inline bool
    dxtc_pixels::DXT1() const
    {
        return ( ( m_Format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ) ||
                 ( m_Format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ) ||
                 ( m_Format == GL_COMPRESSED_SRGB_S3TC_DXT1_EXT ) ||
                 ( m_Format == GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT ) );
    }

    inline bool
    dxtc_pixels::DXT3() const
    {
        return ( ( m_Format == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT ) ||
                 ( m_Format == GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT ) );
    }

    inline bool
    dxtc_pixels::DXT5() const
    {
        return ( ( m_Format == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT ) ||
                 ( m_Format == GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT ) );
    }

    inline bool
    dxtc_pixels::SupportedFormat() const
    {
        return ( DXT1() || DXT3() || DXT5() );
    }

    inline void
    dxtc_pixels::BVF_Color_H2( void* const pBlock ) const
    {
        // Swap the two first row of pixels
        dxtc_int8* pP = ( ( dxtc_int8* )pBlock ) + 4;

        std::swap( pP[0], pP[1] );
    }

    inline void
    dxtc_pixels::BVF_Color_H4( void* const pBlock ) const
    {
        // Swap the first row of pixels with the last one, then the 2 middle row of
        // pixels
        dxtc_int8* pP = ( ( dxtc_int8* )pBlock ) + 4;

        std::swap( pP[0], pP[3] );
        std::swap( pP[1], pP[2] );
    }

    inline void
    dxtc_pixels::BVF_Color( void* const pBlock1,
                            void* const pBlock2 ) const
    {
        // Swap the "2 colors" header (32bits each header)
        dxtc_int32* pHdr1 = ( dxtc_int32* )pBlock1;
        dxtc_int32* pHdr2 = ( dxtc_int32* )pBlock2;

        std::swap( *pHdr1, *pHdr2 );

        // Now swap the pixel values
        dxtc_int8* pP1 = ( ( dxtc_int8* )pBlock1 ) + 4;
        dxtc_int8* pP2 = ( ( dxtc_int8* )pBlock2 ) + 4;

        std::swap( pP1[0], pP2[3] );
        std::swap( pP1[1], pP2[2] );
        std::swap( pP1[2], pP2[1] );
        std::swap( pP1[3], pP2[0] );
    }

    inline void
    dxtc_pixels::BVF_Alpha_DXT3_H2( void* const pBlock ) const
    {
        // Swap the two first row of pixels
        dxtc_int16* pP = ( dxtc_int16* )pBlock;

        std::swap( pP[0], pP[1] );
    }

    inline void
    dxtc_pixels::BVF_Alpha_DXT3_H4( void* const pBlock ) const
    {
        // Swap the first row of pixels with the last one, then the 2 middle row of
        // pixels
        dxtc_int16* pP = ( dxtc_int16* )pBlock;

        std::swap( pP[0], pP[3] );
        std::swap( pP[1], pP[2] );
    }

    inline void
    dxtc_pixels::BVF_Alpha_DXT3( void* const pBlock1,
                                 void* const pBlock2 ) const
    {
        // Swap all the pixel values
        dxtc_int16* pP1 = ( dxtc_int16* )pBlock1;
        dxtc_int16* pP2 = ( dxtc_int16* )pBlock2;

        std::swap( pP1[0], pP2[3] );
        std::swap( pP1[1], pP2[2] );
        std::swap( pP1[2], pP2[1] );
        std::swap( pP1[3], pP2[0] );
    }

    inline void
    dxtc_pixels::BVF_Alpha_DXT5_H2( void* const pBlock ) const
    {
        // Swap the two first row of pixels (kinda tricky with DXT5 unaligned encoding)
        dxtc_int32* pP = ( dxtc_int32* )( ( ( dxtc_int8* )pBlock ) + 2 );

        dxtc_int32  TmpDWord =
            static_cast<dxtc_int32>( static_cast<unsigned int>( pP[0] ) &
                                     0XFF'00'00'00U );
        TmpDWord |= static_cast<dxtc_int32>(
            ( static_cast<unsigned int>( pP[0] ) & 0X00'00'0F'FFU ) << 12
        );
        TmpDWord |= static_cast<dxtc_int32>(
            ( static_cast<unsigned int>( pP[0] ) & 0X00'FF'F0'00U ) >> 12
        );
        pP[0] = TmpDWord;
    }

    inline void
    dxtc_pixels::BVF_Alpha_DXT5_H4( void* const pBlock ) const
    {
        // Swap the first row of pixels with the last one, then the 2 middle row of
        // pixels (tricky again)
        dxtc_int64* pB       = ( dxtc_int64* )pBlock;

        auto        uB0      = static_cast<unsigned long long>( pB[0] );
        dxtc_int64  TmpQWord = static_cast<dxtc_int64>( uB0 & HEX_0x000000000000FFFF );
        TmpQWord |= static_cast<dxtc_int64>( ( uB0 & HEX_0x000000000FFF0000 ) << 36 );
        TmpQWord |= static_cast<dxtc_int64>( ( uB0 & HEX_0x000000FFF0000000 ) << 12 );
        TmpQWord |= static_cast<dxtc_int64>( ( uB0 & HEX_0x000FFF0000000000 ) >> 12 );
        TmpQWord |= static_cast<dxtc_int64>( ( uB0 & HEX_0xFFF0000000000000 ) >> 36 );
        pB[0]     = TmpQWord;
    }

    inline void
    dxtc_pixels::BVF_Alpha_DXT5( void* const pBlock1,
                                 void* const pBlock2 ) const
    {
        // Swap all the pixel values (same trick for DXT5)
        dxtc_int64* pB1       = ( dxtc_int64* )pBlock1;
        dxtc_int64* pB2       = ( dxtc_int64* )pBlock2;

        auto        uB1       = static_cast<unsigned long long>( pB1[0] );
        dxtc_int64  TmpQWord1 = static_cast<dxtc_int64>( uB1 & HEX_0x000000000000FFFF );
        TmpQWord1 |= static_cast<dxtc_int64>( ( uB1 & HEX_0x000000000FFF0000 ) << 36 );
        TmpQWord1 |= static_cast<dxtc_int64>( ( uB1 & HEX_0x000000FFF0000000 ) << 12 );
        TmpQWord1 |= static_cast<dxtc_int64>( ( uB1 & HEX_0x000FFF0000000000 ) >> 12 );
        TmpQWord1 |= static_cast<dxtc_int64>( ( uB1 & HEX_0xFFF0000000000000 ) >> 36 );

        auto       uB2       = static_cast<unsigned long long>( pB2[0] );
        dxtc_int64 TmpQWord2 = static_cast<dxtc_int64>( uB2 & HEX_0x000000000000FFFF );
        TmpQWord2 |= static_cast<dxtc_int64>( ( uB2 & HEX_0x000000000FFF0000 ) << 36 );
        TmpQWord2 |= static_cast<dxtc_int64>( ( uB2 & HEX_0x000000FFF0000000 ) << 12 );
        TmpQWord2 |= static_cast<dxtc_int64>( ( uB2 & HEX_0x000FFF0000000000 ) >> 12 );
        TmpQWord2 |= static_cast<dxtc_int64>( ( uB2 & HEX_0xFFF0000000000000 ) >> 36 );

        pB1[0]     = TmpQWord2;
        pB2[0]     = TmpQWord1;
    }

    inline void*
    dxtc_pixels::GetBlock( size_t i,
                           size_t j,
                           size_t BlockSize ) const
    {
        const dxtc_int8* pPixels = ( const dxtc_int8* )m_pPixels;

        return ( void* )( pPixels +
                          i *
                          ( ( m_Width + 3 ) / 4 ) *
                          BlockSize +
                          j *
                          BlockSize );
    }

}    // namespace dxtc_tool
