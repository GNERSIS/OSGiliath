/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#include <string.h>

#define STB_DXT_IMPLEMENTATION
#include <stb_dxt.h>

#include <osgDB/serialization/TextureCompression.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <functional>
#include <limits>
#include <optional>
#include <osg/core/Notify.hpp>
#include <osg/images/Image.hpp>
#include <osg/nodes/Node.hpp>
#include <osg/state/StateAttribute.hpp>
#include <osg/state/StateSet.hpp>
#include <osg/textures/Texture.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <unordered_map>
#include <vector>

namespace osgDB::serialization
{
    namespace
    {

        constexpr std::size_t kRgbaChannels      = 4U;
        constexpr std::size_t kDxtBlockWidth     = 4U;
        constexpr std::size_t kDxtBlockHeight    = 4U;
        constexpr std::size_t kDxtBlockPixels    = kDxtBlockWidth * kDxtBlockHeight;
        constexpr std::size_t kDxtBlockRgbaBytes = kDxtBlockPixels * kRgbaChannels;
        constexpr std::size_t kDxt1BlockBytes    = 8U;
        constexpr std::size_t kDxt5BlockBytes    = 16U;
        constexpr double      kBytesPerMiB       = 1024.0 * 1024.0;

        bool
        checkedMultiply( std::size_t lhs,
                         std::size_t rhs,
                         std::size_t& result ) noexcept
        {
            if( lhs != 0U && rhs > std::numeric_limits<std::size_t>::max() / lhs )
            {
                return false;
            }
            result = lhs * rhs;
            return true;
        }

        bool
        isSrgbInternalFormat( GLint internalFormat ) noexcept
        {
            switch( internalFormat )
            {
                case GL_SRGB8 :
                case GL_SRGB8_ALPHA8 :
#ifdef GL_SRGB
                case GL_SRGB :
#endif
#ifdef GL_SRGB_ALPHA
                case GL_SRGB_ALPHA :
#endif
                    return true;
                default :
                    return false;
            }
        }

        std::optional<std::size_t>
        bytesPerSourcePixel( GLenum pixelFormat ) noexcept
        {
            switch( pixelFormat )
            {
                case GL_LUMINANCE :
                    return 1U;
                case GL_RG :
                    return 2U;
                case GL_RGB :
                    return 3U;
                case GL_RGBA :
                    return 4U;
                default :
                    return std::nullopt;
            }
        }

        std::size_t
        rgbaOffset( std::size_t x,
                    std::size_t y,
                    std::size_t width ) noexcept
        {
            return ( y * width + x ) * kRgbaChannels;
        }

        std::optional<std::vector<unsigned char>>
        expandToRgba8( const osg::Image& src )
        {
            const std::optional<std::size_t> sourcePixelBytes =
                bytesPerSourcePixel( src.getPixelFormat() );
            if( !sourcePixelBytes.has_value() )
            {
                return std::nullopt;
            }

            const std::size_t width  = static_cast<std::size_t>( src.s() );
            const std::size_t height = static_cast<std::size_t>( src.t() );
            std::size_t       pixelCount = 0U;
            if( !checkedMultiply( width, height, pixelCount ) )
            {
                return std::nullopt;
            }
            std::size_t rgbaByteCount = 0U;
            if( !checkedMultiply( pixelCount, kRgbaChannels, rgbaByteCount ) )
            {
                return std::nullopt;
            }

            std::vector<unsigned char> rgba( rgbaByteCount );
            for( std::size_t y = 0U; y < height; ++y )
            {
                for( std::size_t x = 0U; x < width; ++x )
                {
                    const unsigned char* sourcePixel =
                        src.data( static_cast<unsigned int>( x ),
                                  static_cast<unsigned int>( y ) );
                    unsigned char* targetPixel = rgba.data() + rgbaOffset( x, y, width );

                    switch( src.getPixelFormat() )
                    {
                        case GL_LUMINANCE :
                            targetPixel[0] = sourcePixel[0];
                            targetPixel[1] = sourcePixel[0];
                            targetPixel[2] = sourcePixel[0];
                            targetPixel[3] = 0XFFU;
                            break;
                        case GL_RG :
                            targetPixel[0] = sourcePixel[0];
                            targetPixel[1] = sourcePixel[1];
                            targetPixel[2] = 0U;
                            targetPixel[3] = 0XFFU;
                            break;
                        case GL_RGB :
                            targetPixel[0] = sourcePixel[0];
                            targetPixel[1] = sourcePixel[1];
                            targetPixel[2] = sourcePixel[2];
                            targetPixel[3] = 0XFFU;
                            break;
                        case GL_RGBA :
                            targetPixel[0] = sourcePixel[0];
                            targetPixel[1] = sourcePixel[1];
                            targetPixel[2] = sourcePixel[2];
                            targetPixel[3] = sourcePixel[3];
                            break;
                        default :
                            return std::nullopt;
                    }
                }
            }

            return rgba;
        }

        std::optional<std::vector<unsigned char>>
        makeNextMipLevel( const std::vector<unsigned char>& rgba,
                          std::size_t                      width,
                          std::size_t                      height )
        {
            const std::size_t nextWidth  = std::max<std::size_t>( 1U, width / 2U );
            const std::size_t nextHeight = std::max<std::size_t>( 1U, height / 2U );

            std::size_t pixelCount = 0U;
            if( !checkedMultiply( nextWidth, nextHeight, pixelCount ) )
            {
                return std::nullopt;
            }
            std::size_t rgbaByteCount = 0U;
            if( !checkedMultiply( pixelCount, kRgbaChannels, rgbaByteCount ) )
            {
                return std::nullopt;
            }

            std::vector<unsigned char> next( rgbaByteCount );
            for( std::size_t y = 0U; y < nextHeight; ++y )
            {
                for( std::size_t x = 0U; x < nextWidth; ++x )
                {
                    const std::size_t sampleX0 = std::min( x * 2U, width - 1U );
                    const std::size_t sampleX1 = std::min( sampleX0 + 1U, width - 1U );
                    const std::size_t sampleY0 = std::min( y * 2U, height - 1U );
                    const std::size_t sampleY1 = std::min( sampleY0 + 1U, height - 1U );
                    const std::array<std::size_t, 4U> sampleOffsets{
                        rgbaOffset( sampleX0, sampleY0, width ),
                        rgbaOffset( sampleX1, sampleY0, width ),
                        rgbaOffset( sampleX0, sampleY1, width ),
                        rgbaOffset( sampleX1, sampleY1, width )
                    };

                    unsigned char* targetPixel =
                        next.data() + rgbaOffset( x, y, nextWidth );
                    for( std::size_t channel = 0U; channel < kRgbaChannels; ++channel )
                    {
                        unsigned int sum = 0U;
                        for( const std::size_t sampleOffset : sampleOffsets )
                        {
                            sum += rgba[sampleOffset + channel];
                        }
                        targetPixel[channel] =
                            static_cast<unsigned char>( ( sum + 2U ) / 4U );
                    }
                }
            }

            return next;
        }

        bool
        appendCompressedLevel( const std::vector<unsigned char>& rgba,
                               std::size_t                      width,
                               std::size_t                      height,
                               bool                             useDxt5,
                               std::vector<unsigned char>&      compressedBytes )
        {
            const std::size_t blocksX =
                ( width + kDxtBlockWidth - 1U ) / kDxtBlockWidth;
            const std::size_t blocksY =
                ( height + kDxtBlockHeight - 1U ) / kDxtBlockHeight;
            const std::size_t blockByteCount =
                useDxt5 ? kDxt5BlockBytes : kDxt1BlockBytes;

            std::size_t blockCount = 0U;
            if( !checkedMultiply( blocksX, blocksY, blockCount ) )
            {
                return false;
            }
            std::size_t levelByteCount = 0U;
            if( !checkedMultiply( blockCount, blockByteCount, levelByteCount ) )
            {
                return false;
            }

            const std::size_t levelOffset = compressedBytes.size();
            if( levelByteCount >
                std::numeric_limits<std::size_t>::max() - compressedBytes.size() )
            {
                return false;
            }
            compressedBytes.resize( compressedBytes.size() + levelByteCount );

            std::size_t blockIndex = 0U;
            for( std::size_t blockY = 0U; blockY < blocksY; ++blockY )
            {
                for( std::size_t blockX = 0U; blockX < blocksX; ++blockX )
                {
                    std::array<unsigned char, kDxtBlockRgbaBytes> block{};
                    for( std::size_t y = 0U; y < kDxtBlockHeight; ++y )
                    {
                        const std::size_t sourceY =
                            std::min( blockY * kDxtBlockHeight + y, height - 1U );
                        for( std::size_t x = 0U; x < kDxtBlockWidth; ++x )
                        {
                            const std::size_t sourceX =
                                std::min( blockX * kDxtBlockWidth + x, width - 1U );
                            const std::size_t sourceOffset =
                                rgbaOffset( sourceX, sourceY, width );
                            const std::size_t targetOffset =
                                ( y * kDxtBlockWidth + x ) * kRgbaChannels;
                            std::copy_n( rgba.data() + sourceOffset,
                                         kRgbaChannels,
                                         block.data() + targetOffset );
                        }
                    }

                    unsigned char* blockDest =
                        compressedBytes.data() + levelOffset +
                        blockIndex * blockByteCount;
                    stb_compress_dxt_block( blockDest,
                                            block.data(),
                                            useDxt5 ? 1 : 0,
                                            STB_DXT_HIGHQUAL );
                    ++blockIndex;
                }
            }

            return true;
        }

        osg::ref_ptr<osg::Image>
        compressImageDXTWithInternalFormat( const osg::Image& src,
                                            GLint             sourceInternalFormat )
        {
            if( src.isCompressed() ||
                src.getDataType() != GL_UNSIGNED_BYTE ||
                src.r() != 1 ||
                src.s() <= 0 ||
                src.t() <= 0 ||
                src.data() == nullptr )
            {
                return nullptr;
            }

            if( !bytesPerSourcePixel( src.getPixelFormat() ).has_value() )
            {
                return nullptr;
            }

            std::optional<std::vector<unsigned char>> currentLevel = expandToRgba8( src );
            if( !currentLevel.has_value() )
            {
                return nullptr;
            }

            const bool hasAlpha = src.getPixelFormat() == GL_RGBA;
            const bool useSrgb  = isSrgbInternalFormat( sourceInternalFormat );
            const bool useDxt5  = hasAlpha || useSrgb;
            const GLint compressedFormat =
                useSrgb ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT
                        : useDxt5 ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
                                  : GL_COMPRESSED_RGB_S3TC_DXT1_EXT;

            std::vector<unsigned char> compressedBytes;
            osg::Image::MipmapDataType mipmapOffsets;
            std::size_t                width      = static_cast<std::size_t>( src.s() );
            std::size_t                height     = static_cast<std::size_t>( src.t() );
            bool                       firstLevel = true;
            while( true )
            {
                if( !firstLevel )
                {
                    if( compressedBytes.size() >
                        std::numeric_limits<unsigned int>::max() )
                    {
                        return nullptr;
                    }
                    mipmapOffsets.push_back(
                        static_cast<unsigned int>( compressedBytes.size() )
                    );
                }
                firstLevel = false;

                if( !appendCompressedLevel( *currentLevel,
                                            width,
                                            height,
                                            useDxt5,
                                            compressedBytes ) )
                {
                    return nullptr;
                }

                if( width == 1U && height == 1U )
                {
                    break;
                }

                std::optional<std::vector<unsigned char>> nextLevel =
                    makeNextMipLevel( *currentLevel, width, height );
                if( !nextLevel.has_value() )
                {
                    return nullptr;
                }
                currentLevel = std::move( nextLevel );
                width        = std::max<std::size_t>( 1U, width / 2U );
                height       = std::max<std::size_t>( 1U, height / 2U );
            }

            if( compressedBytes.empty() )
            {
                return nullptr;
            }

            unsigned char* data = new unsigned char[compressedBytes.size()];
            std::memcpy( data, compressedBytes.data(), compressedBytes.size() );

            osg::ref_ptr<osg::Image> compressed = new osg::Image;
            compressed->setFileName( src.getFileName() );
            compressed->setWriteHint( src.getWriteHint() );
            compressed->setOrigin( src.getOrigin() );
            compressed->setPixelAspectRatio( src.getPixelAspectRatio() );
            compressed->setImage( src.s(),
                                  src.t(),
                                  1,
                                  compressedFormat,
                                  static_cast<GLenum>( compressedFormat ),
                                  GL_UNSIGNED_BYTE,
                                  data,
                                  osg::Image::USE_NEW_DELETE,
                                  1,
                                  0 );
            compressed->setMipmapLevels( mipmapOffsets );
            return compressed;
        }

        struct CompressionCacheKey
        {
            const osg::Image* image;
            GLint             sourceInternalFormat;

            bool
            operator==( const CompressionCacheKey& rhs ) const noexcept
            {
                return image == rhs.image &&
                       sourceInternalFormat == rhs.sourceInternalFormat;
            }
        };

        struct CompressionCacheKeyHash
        {
            std::size_t
            operator()( const CompressionCacheKey& key ) const noexcept
            {
                const std::size_t imageHash =
                    std::hash<const osg::Image*>{}( key.image );
                const std::size_t formatHash =
                    std::hash<int>{}( static_cast<int>( key.sourceInternalFormat ) );
                return imageHash ^ ( formatHash + 0X9E37'79B9U + ( imageHash << 6U ) +
                                     ( imageHash >> 2U ) );
            }
        };

        class TextureCompressionVisitor : public osg::NodeVisitor
        {
            public:
                TextureCompressionVisitor() :
                    osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
                {
                }

                void
                apply( osg::Node& node ) override
                {
                    compressStateSet( node.getStateSet() );
                    traverse( node );
                }

                std::size_t
                texturesCompressed() const noexcept
                {
                    return _texturesCompressed;
                }

                std::size_t
                bytesBefore() const noexcept
                {
                    return _bytesBefore;
                }

                std::size_t
                bytesAfter() const noexcept
                {
                    return _bytesAfter;
                }

            private:
                void
                compressStateSet( osg::StateSet* stateSet )
                {
                    if( stateSet == nullptr )
                    {
                        return;
                    }

                    osg::StateSet::TextureAttributeList& textureAttributes =
                        stateSet->getTextureAttributeList();
                    for( osg::StateSet::AttributeList& unitAttributes :
                         textureAttributes )
                    {
                        for( osg::StateSet::AttributeList::value_type& attributeEntry :
                             unitAttributes )
                        {
                            osg::StateAttribute* attribute =
                                attributeEntry.second.first.get();
                            osg::Texture* texture =
                                attribute != nullptr ? attribute->asTexture() : nullptr;
                            osg::Texture2D* texture2D =
                                dynamic_cast<osg::Texture2D*>( texture );
                            if( texture2D == nullptr )
                            {
                                continue;
                            }

                            osg::Image* image = texture2D->getImage();
                            if( image == nullptr || image->isCompressed() )
                            {
                                continue;
                            }

                            const GLint sourceInternalFormat =
                                sourceInternalFormatForTexture( *texture2D, *image );
                            const CompressionCacheKey cacheKey{ image,
                                                                sourceInternalFormat };
                            osg::ref_ptr<osg::Image> compressed;
                            const auto cacheIt = _compressedImages.find( cacheKey );
                            if( cacheIt != _compressedImages.end() )
                            {
                                compressed = cacheIt->second;
                            }
                            else
                            {
                                compressed = compressImageDXTWithInternalFormat(
                                    *image,
                                    sourceInternalFormat
                                );
                                if( compressed.valid() )
                                {
                                    _compressedImages.emplace( cacheKey, compressed );
                                }
                            }

                            if( !compressed.valid() )
                            {
                                continue;
                            }

                            _bytesBefore += image->getTotalSizeInBytesIncludingMipmaps();
                            _bytesAfter +=
                                compressed->getTotalSizeInBytesIncludingMipmaps();
                            ++_texturesCompressed;
                            texture2D->setImage( compressed.get() );
                            texture2D->setInternalFormat(
                                compressed->getInternalTextureFormat()
                            );
                        }
                    }
                }

                static GLint
                sourceInternalFormatForTexture( const osg::Texture2D& texture,
                                                const osg::Image&     image )
                {
                    const GLint textureInternalFormat = texture.getInternalFormat();
                    if( isSrgbInternalFormat( textureInternalFormat ) )
                    {
                        return textureInternalFormat;
                    }
                    return image.getInternalTextureFormat();
                }

                std::unordered_map<CompressionCacheKey,
                                   osg::ref_ptr<osg::Image>,
                                   CompressionCacheKeyHash>
                            _compressedImages;
                std::size_t _texturesCompressed = 0U;
                std::size_t _bytesBefore        = 0U;
                std::size_t _bytesAfter         = 0U;
        };

    }

    osg::ref_ptr<osg::Image>
    compressImageDXT( const osg::Image& src )
    {
        return compressImageDXTWithInternalFormat( src, src.getInternalTextureFormat() );
    }

    void
    compressSceneTextures( osg::Node& root )
    {
        TextureCompressionVisitor visitor;
        root.accept( visitor );
        OSG_NOTICE << "compressed scene textures: " << visitor.texturesCompressed()
                   << ", "
                   << static_cast<double>( visitor.bytesBefore() ) / kBytesPerMiB
                   << " MB -> "
                   << static_cast<double>( visitor.bytesAfter() ) / kBytesPerMiB
                   << " MB" << std::endl;
    }

}
