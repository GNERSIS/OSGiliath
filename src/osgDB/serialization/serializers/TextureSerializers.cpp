/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <osg/images/Image.hpp>
#include <osg/maths/vec4.hpp>
#include <osg/state/StateAttribute.hpp>
#include <osg/state/StateSet.hpp>
#include <osg/state/Uniform.hpp>
#include <osg/textures/Texture.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/serialization/ObjectSerializer.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace osgDB::serialization
{
    namespace
    {

        std::vector<std::byte>
        copyBytes( const void* source,
                   std::size_t size )
        {
            std::vector<std::byte> bytes( size );
            if( size > 0U )
            {
                if( source == nullptr )
                {
                    throw std::runtime_error( "Cannot serialize null data pointer" );
                }
                std::memcpy( bytes.data(), source, size );
            }
            return bytes;
        }

        void
        serializeObjectField( Archive&                   ar,
                              std::string_view           name,
                              osg::ref_ptr<osg::Object>& object )
        {
            ar.beginObject( name );
            serialize( ar, object );
            ar.endObject();
        }

        void
        serializeDVec4( Archive&         ar,
                        std::string_view name,
                        osg::dvec4&      value )
        {
            constexpr std::uint32_t kElementCount = 4U;
            std::uint32_t           count         = kElementCount;
            ar.beginArray( name, count );
            if( count != kElementCount )
            {
                throw std::runtime_error( "dvec4 archive size mismatch" );
            }
            for( std::size_t i = 0U; i < kElementCount; ++i )
            {
                ar.value( "Element", value[i] );
            }
            ar.endArray();
        }

        void
        serializeIVec4( Archive&         ar,
                        std::string_view name,
                        osg::ivec4&      value )
        {
            constexpr std::uint32_t kElementCount = 4U;
            std::uint32_t           count         = kElementCount;
            ar.beginArray( name, count );
            if( count != kElementCount )
            {
                throw std::runtime_error( "ivec4 archive size mismatch" );
            }
            for( std::size_t i = 0U; i < kElementCount; ++i )
            {
                ar.value( "Element", value[i] );
            }
            ar.endArray();
        }

        void
        serializeTextureWrap( Archive&                    ar,
                              osg::Texture&               texture,
                              osg::Texture::WrapParameter parameter,
                              std::string_view            name )
        {
            std::uint32_t mode =
                ar.writing() ? static_cast<std::uint32_t>( texture.getWrap( parameter ) )
                             : 0U;
            ar.value( name, mode );
            if( ar.reading() )
            {
                texture.setWrap( parameter,
                                 static_cast<osg::Texture::WrapMode>( mode ) );
            }
        }

        void
        serializeTextureFilter( Archive&                      ar,
                                osg::Texture&                 texture,
                                osg::Texture::FilterParameter parameter,
                                std::string_view              name )
        {
            std::uint32_t mode =
                ar.writing()
                    ? static_cast<std::uint32_t>( texture.getFilter( parameter ) )
                    : 0U;
            ar.value( name, mode );
            if( ar.reading() )
            {
                texture.setFilter( parameter,
                                   static_cast<osg::Texture::FilterMode>( mode ) );
            }
        }

        template<typename SetMode>
        void
        serializeModeList( Archive&                       ar,
                           std::string_view               name,
                           const osg::StateSet::ModeList& modes,
                           SetMode                        setMode )
        {
            std::uint32_t count =
                ar.writing() ? static_cast<std::uint32_t>( modes.size() ) : 0U;
            ar.beginArray( name, count );
            auto it = modes.begin();
            for( std::uint32_t i = 0U; i < count; ++i )
            {
                ar.beginObject( "Mode" );
                std::uint32_t mode =
                    ar.writing() ? static_cast<std::uint32_t>( it->first ) : 0U;
                std::uint32_t value =
                    ar.writing() ? static_cast<std::uint32_t>( it->second ) : 0U;
                ar.value( "Mode", mode );
                ar.value( "Value", value );
                if( ar.reading() )
                {
                    setMode( static_cast<osg::StateAttribute::GLMode>( mode ),
                             static_cast<osg::StateAttribute::GLModeValue>( value ) );
                }
                else
                {
                    ++it;
                }
                ar.endObject();
            }
            ar.endArray();
        }

        template<typename SetAttribute>
        void
        serializeAttributeList( Archive&                            ar,
                                std::string_view                    name,
                                const osg::StateSet::AttributeList& attributes,
                                SetAttribute                        setAttribute )
        {
            std::uint32_t count =
                ar.writing() ? static_cast<std::uint32_t>( attributes.size() ) : 0U;
            ar.beginArray( name, count );
            auto it = attributes.begin();
            for( std::uint32_t i = 0U; i < count; ++i )
            {
                ar.beginObject( "Attribute" );
                osg::ref_ptr<osg::Object> attribute =
                    ar.writing() ? osg::ref_ptr<osg::Object>( it->second.first.get() )
                                 : osg::ref_ptr<osg::Object>();
                std::uint32_t value =
                    ar.writing() ? static_cast<std::uint32_t>( it->second.second ) : 0U;
                serializeObjectField( ar, "Object", attribute );
                ar.value( "Value", value );
                if( ar.reading() )
                {
                    if( attribute.valid() && attribute->asStateAttribute() == nullptr )
                    {
                        throw std::runtime_error(
                            "StateSet attribute is not osg::StateAttribute"
                        );
                    }
                    if( attribute.valid() )
                    {
                        setAttribute(
                            attribute->asStateAttribute(),
                            static_cast<osg::StateAttribute::OverrideValue>( value )
                        );
                    }
                }
                else
                {
                    ++it;
                }
                ar.endObject();
            }
            ar.endArray();
        }

        void
        serializeTextureModeList( Archive&       ar,
                                  osg::StateSet& stateSet )
        {
            const osg::StateSet::TextureModeList& lists = stateSet.getTextureModeList();
            std::uint32_t                         count =
                ar.writing() ? static_cast<std::uint32_t>( lists.size() ) : 0U;
            ar.beginArray( "TextureModeList", count );
            for( std::uint32_t unit = 0U; unit < count; ++unit )
            {
                ar.beginObject( "TextureModeUnit" );
                std::uint32_t storedUnit = ar.writing() ? unit : 0U;
                ar.value( "Unit", storedUnit );
                const osg::StateSet::ModeList& modes =
                    ar.writing() ? lists[unit] : stateSet.getModeList();
                serializeModeList( ar,
                                   "Modes",
                                   modes,
                                   [&]( osg::StateAttribute::GLMode      mode,
                                        osg::StateAttribute::GLModeValue value )
                                   {
                                       stateSet.setTextureMode( storedUnit,
                                                                mode,
                                                                value );
                                   } );
                ar.endObject();
            }
            ar.endArray();
        }

        void
        serializeTextureAttributeList( Archive&       ar,
                                       osg::StateSet& stateSet )
        {
            const osg::StateSet::TextureAttributeList& lists =
                stateSet.getTextureAttributeList();
            std::uint32_t count =
                ar.writing() ? static_cast<std::uint32_t>( lists.size() ) : 0U;
            ar.beginArray( "TextureAttributeList", count );
            for( std::uint32_t unit = 0U; unit < count; ++unit )
            {
                ar.beginObject( "TextureAttributeUnit" );
                std::uint32_t storedUnit = ar.writing() ? unit : 0U;
                ar.value( "Unit", storedUnit );
                const osg::StateSet::AttributeList& attributes =
                    ar.writing() ? lists[unit] : stateSet.getAttributeList();
                serializeAttributeList( ar,
                                        "Attributes",
                                        attributes,
                                        [&]( osg::StateAttribute* attribute,
                                             osg::StateAttribute::OverrideValue value )
                                        {
                                            stateSet.setTextureAttribute( storedUnit,
                                                                          attribute,
                                                                          value );
                                        } );
                ar.endObject();
            }
            ar.endArray();
        }

        void
        serializeUniformList( Archive&       ar,
                              osg::StateSet& stateSet )
        {
            const osg::StateSet::UniformList& uniforms = stateSet.getUniformList();
            std::uint32_t                     count =
                ar.writing() ? static_cast<std::uint32_t>( uniforms.size() ) : 0U;
            ar.beginArray( "UniformList", count );
            auto it = uniforms.begin();
            for( std::uint32_t i = 0U; i < count; ++i )
            {
                ar.beginObject( "Uniform" );
                osg::ref_ptr<osg::Object> uniform =
                    ar.writing() ? osg::ref_ptr<osg::Object>( it->second.first.get() )
                                 : osg::ref_ptr<osg::Object>();
                std::uint32_t value =
                    ar.writing() ? static_cast<std::uint32_t>( it->second.second ) : 0U;
                serializeObjectField( ar, "Object", uniform );
                ar.value( "Value", value );
                if( ar.reading() )
                {
                    if( uniform.valid() && uniform->asUniformBase() == nullptr )
                    {
                        throw std::runtime_error(
                            "StateSet uniform is not osg::UniformBase"
                        );
                    }
                    if( uniform.valid() )
                    {
                        stateSet.addUniform(
                            uniform->asUniformBase(),
                            static_cast<osg::StateAttribute::OverrideValue>( value )
                        );
                    }
                }
                else
                {
                    ++it;
                }
                ar.endObject();
            }
            ar.endArray();
        }

        void
        serializeDefineList( Archive&       ar,
                             osg::StateSet& stateSet )
        {
            const osg::StateSet::DefineList& defines = stateSet.getDefineList();
            std::uint32_t                    count =
                ar.writing() ? static_cast<std::uint32_t>( defines.size() ) : 0U;
            ar.beginArray( "DefineList", count );
            auto it = defines.begin();
            for( std::uint32_t i = 0U; i < count; ++i )
            {
                ar.beginObject( "Define" );
                std::string   name  = ar.writing() ? it->first : std::string();
                std::string   value = ar.writing() ? it->second.first : std::string();
                std::uint32_t overrideValue =
                    ar.writing() ? static_cast<std::uint32_t>( it->second.second ) : 0U;
                ar.value( "Name", name );
                ar.value( "Value", value );
                ar.value( "Override", overrideValue );
                if( ar.reading() )
                {
                    stateSet.setDefine(
                        name,
                        value,
                        static_cast<osg::StateAttribute::OverrideValue>( overrideValue )
                    );
                }
                else
                {
                    ++it;
                }
                ar.endObject();
            }
            ar.endArray();
        }

    }

    void
    serialize( Archive&    ar,
               osg::Image& image )
    {
        std::string  fileName = ar.writing() ? image.getFileName() : std::string();
        std::int32_t width  = ar.writing() ? static_cast<std::int32_t>( image.s() ) : 0;
        std::int32_t height = ar.writing() ? static_cast<std::int32_t>( image.t() ) : 0;
        std::int32_t depth  = ar.writing() ? static_cast<std::int32_t>( image.r() ) : 0;
        std::int32_t internalFormat =
            ar.writing() ? static_cast<std::int32_t>( image.getInternalTextureFormat() )
                         : 0;
        std::uint32_t pixelFormat =
            ar.writing() ? static_cast<std::uint32_t>( image.getPixelFormat() ) : 0U;
        std::uint32_t dataType =
            ar.writing() ? static_cast<std::uint32_t>( image.getDataType() ) : 0U;
        std::int32_t rowLength =
            ar.writing() ? static_cast<std::int32_t>( image.getRowLength() ) : 0;
        std::uint32_t packing =
            ar.writing() ? static_cast<std::uint32_t>( image.getPacking() ) : 0U;
        std::int32_t origin =
            ar.writing() ? static_cast<std::int32_t>( image.getOrigin() ) : 0;
        std::int32_t writeHint =
            ar.writing() ? static_cast<std::int32_t>( image.getWriteHint() ) : 0;
        float pixelAspectRatio = ar.writing() ? image.getPixelAspectRatio() : 1.0F;
        std::vector<std::byte> bytes =
            ar.writing()
                ? copyBytes( image.data(), image.getTotalSizeInBytesIncludingMipmaps() )
                : std::vector<std::byte>();
        const osg::Image::MipmapDataType& writeMipmaps = image.getMipmapLevels();
        std::uint32_t                     mipmapCount =
            ar.writing() ? static_cast<std::uint32_t>( writeMipmaps.size() ) : 0U;

        ar.value( "FileName", fileName );
        ar.value( "Width", width );
        ar.value( "Height", height );
        ar.value( "Depth", depth );
        ar.value( "InternalTextureFormat", internalFormat );
        ar.value( "PixelFormat", pixelFormat );
        ar.value( "DataType", dataType );
        ar.value( "RowLength", rowLength );
        ar.value( "Packing", packing );
        ar.value( "Origin", origin );
        ar.value( "WriteHint", writeHint );
        ar.value( "PixelAspectRatio", pixelAspectRatio );
        ar.blob( "Pixels", bytes );
        osg::Image::MipmapDataType mipmaps;
        ar.beginArray( "Mipmaps", mipmapCount );
        if( ar.reading() )
        {
            mipmaps.reserve( mipmapCount );
        }
        for( std::uint32_t i = 0U; i < mipmapCount; ++i )
        {
            std::uint32_t offset = ar.writing() ? writeMipmaps[i] : 0U;
            ar.value( "Offset", offset );
            if( ar.reading() )
            {
                mipmaps.push_back( offset );
            }
        }
        ar.endArray();

        if( ar.reading() )
        {
            std::unique_ptr<unsigned char[]> ownedBytes;
            unsigned char*                   data = nullptr;
            if( !bytes.empty() )
            {
                ownedBytes = std::make_unique<unsigned char[]>( bytes.size() );
                std::memcpy( ownedBytes.get(), bytes.data(), bytes.size() );
                data = ownedBytes.get();
            }
            image.setFileName( fileName );
            image.setWriteHint( static_cast<osg::Image::WriteHint>( writeHint ) );
            image.setOrigin( static_cast<osg::Image::Origin>( origin ) );
            image.setPixelAspectRatio( pixelAspectRatio );
            image.setImage( static_cast<int>( width ),
                            static_cast<int>( height ),
                            static_cast<int>( depth ),
                            static_cast<GLint>( internalFormat ),
                            static_cast<GLenum>( pixelFormat ),
                            static_cast<GLenum>( dataType ),
                            data,
                            osg::Image::USE_NEW_DELETE,
                            static_cast<int>( packing ),
                            static_cast<int>( rowLength ) );
            static_cast<void>( ownedBytes.release() );
            image.setMipmapLevels( mipmaps );
        }
    }

    void
    serialize( Archive&      ar,
               osg::Texture& texture )
    {
        serializeTextureWrap( ar, texture, osg::Texture::WRAP_S, "WrapS" );
        serializeTextureWrap( ar, texture, osg::Texture::WRAP_T, "WrapT" );
        serializeTextureWrap( ar, texture, osg::Texture::WRAP_R, "WrapR" );
        serializeTextureFilter( ar, texture, osg::Texture::MIN_FILTER, "MinFilter" );
        serializeTextureFilter( ar, texture, osg::Texture::MAG_FILTER, "MagFilter" );

        float maxAnisotropy = ar.writing() ? texture.getMaxAnisotropy() : 1.0F;
        bool  useHardwareMipMapGeneration =
            ar.writing() ? texture.getUseHardwareMipMapGeneration() : true;
        bool unrefImageDataAfterApply =
            ar.writing() ? texture.getUnRefImageDataAfterApply() : false;
        bool clientStorageHint = ar.writing() ? texture.getClientStorageHint() : false;
        bool resizeNonPowerOfTwoHint =
            ar.writing() ? texture.getResizeNonPowerOfTwoHint() : true;
        osg::dvec4 borderColor = ar.writing() ? texture.getBorderColor() : osg::dvec4();
        std::int32_t borderWidth =
            ar.writing() ? static_cast<std::int32_t>( texture.getBorderWidth() ) : 0;
        std::int32_t internalFormatMode =
            ar.writing() ? static_cast<std::int32_t>( texture.getInternalFormatMode() )
                         : 0;
        std::int32_t internalFormat =
            ar.writing() &&
                    texture.getInternalFormatMode() ==
                    osg::Texture::USE_USER_DEFINED_FORMAT
                ? static_cast<std::int32_t>( texture.getInternalFormat() )
                : 0;
        std::uint32_t sourceFormat =
            ar.writing() ? static_cast<std::uint32_t>( texture.getSourceFormat() ) : 0U;
        std::uint32_t sourceType =
            ar.writing() ? static_cast<std::uint32_t>( texture.getSourceType() ) : 0U;
        bool shadowComparison = ar.writing() ? texture.getShadowComparison() : false;
        std::uint32_t shadowCompareFunc =
            ar.writing() ? static_cast<std::uint32_t>( texture.getShadowCompareFunc() )
                         : 0U;
        std::uint32_t shadowTextureMode =
            ar.writing() ? static_cast<std::uint32_t>( texture.getShadowTextureMode() )
                         : 0U;
        float      shadowAmbient = ar.writing() ? texture.getShadowAmbient() : 0.0F;
        osg::ivec4 swizzle       = ar.writing() ? texture.getSwizzle() : osg::ivec4();
        float      minLod        = ar.writing() ? texture.getMinLOD() : 0.0F;
        float      maxLod        = ar.writing() ? texture.getMaxLOD() : -1.0F;
        float      lodBias       = ar.writing() ? texture.getLODBias() : 0.0F;

        ar.value( "MaxAnisotropy", maxAnisotropy );
        ar.value( "UseHardwareMipMapGeneration", useHardwareMipMapGeneration );
        ar.value( "UnRefImageDataAfterApply", unrefImageDataAfterApply );
        ar.value( "ClientStorageHint", clientStorageHint );
        ar.value( "ResizeNonPowerOfTwoHint", resizeNonPowerOfTwoHint );
        serializeDVec4( ar, "BorderColor", borderColor );
        ar.value( "BorderWidth", borderWidth );
        ar.value( "InternalFormatMode", internalFormatMode );
        ar.value( "InternalFormat", internalFormat );
        ar.value( "SourceFormat", sourceFormat );
        ar.value( "SourceType", sourceType );
        ar.value( "ShadowComparison", shadowComparison );
        ar.value( "ShadowCompareFunc", shadowCompareFunc );
        ar.value( "ShadowTextureMode", shadowTextureMode );
        ar.value( "ShadowAmbient", shadowAmbient );
        serializeIVec4( ar, "Swizzle", swizzle );
        ar.value( "MinLOD", minLod );
        ar.value( "MaxLOD", maxLod );
        ar.value( "LODBias", lodBias );

        if( ar.reading() )
        {
            texture.setMaxAnisotropy( maxAnisotropy );
            texture.setUseHardwareMipMapGeneration( useHardwareMipMapGeneration );
            texture.setUnRefImageDataAfterApply( unrefImageDataAfterApply );
            texture.setClientStorageHint( clientStorageHint );
            texture.setResizeNonPowerOfTwoHint( resizeNonPowerOfTwoHint );
            texture.setBorderColor( borderColor );
            texture.setBorderWidth( static_cast<GLint>( borderWidth ) );
            texture.setInternalFormatMode(
                static_cast<osg::Texture::InternalFormatMode>( internalFormatMode )
            );
            if( static_cast<osg::Texture::InternalFormatMode>( internalFormatMode ) ==
                osg::Texture::USE_USER_DEFINED_FORMAT )
            {
                texture.setInternalFormat( static_cast<GLint>( internalFormat ) );
            }
            texture.setSourceFormat( static_cast<GLenum>( sourceFormat ) );
            texture.setSourceType( static_cast<GLenum>( sourceType ) );
            texture.setShadowComparison( shadowComparison );
            texture.setShadowCompareFunc(
                static_cast<osg::Texture::ShadowCompareFunc>( shadowCompareFunc )
            );
            texture.setShadowTextureMode(
                static_cast<osg::Texture::ShadowTextureMode>( shadowTextureMode )
            );
            texture.setShadowAmbient( shadowAmbient );
            texture.setSwizzle( swizzle );
            texture.setMinLOD( minLod );
            texture.setMaxLOD( maxLod );
            texture.setLODBias( lodBias );
        }
    }

    void
    serialize( Archive&        ar,
               osg::Texture2D& texture )
    {
        serialize( ar, static_cast<osg::Texture&>( texture ) );

        osg::ref_ptr<osg::Object> image =
            ar.writing() ? osg::ref_ptr<osg::Object>( texture.getImage() )
                         : osg::ref_ptr<osg::Object>();
        std::int32_t width =
            ar.writing() ? static_cast<std::int32_t>( texture.getTextureWidth() ) : 0;
        std::int32_t height =
            ar.writing() ? static_cast<std::int32_t>( texture.getTextureHeight() ) : 0;

        serializeObjectField( ar, "Image", image );
        ar.value( "TextureWidth", width );
        ar.value( "TextureHeight", height );

        if( ar.reading() )
        {
            if( image.valid() && image->asImage() == nullptr )
            {
                throw std::runtime_error(
                    "Texture2D image field did not contain osg::Image"
                );
            }
            texture.setImage( image.valid() ? image->asImage() : nullptr );
            texture.setTextureWidth( static_cast<int>( width ) );
            texture.setTextureHeight( static_cast<int>( height ) );
        }
    }

    void
    serialize( Archive&       ar,
               osg::StateSet& stateSet )
    {
        serializeModeList( ar,
                           "ModeList",
                           stateSet.getModeList(),
                           [&]( osg::StateAttribute::GLMode      mode,
                                osg::StateAttribute::GLModeValue value )
                           {
                               stateSet.setMode( mode, value );
                           } );
        serializeAttributeList( ar,
                                "AttributeList",
                                stateSet.getAttributeList(),
                                [&]( osg::StateAttribute*               attribute,
                                     osg::StateAttribute::OverrideValue value )
                                {
                                    stateSet.setAttribute( attribute, value );
                                } );
        serializeTextureModeList( ar, stateSet );
        serializeTextureAttributeList( ar, stateSet );
        serializeUniformList( ar, stateSet );
        serializeDefineList( ar, stateSet );

        std::int32_t renderingHint =
            ar.writing() ? static_cast<std::int32_t>( stateSet.getRenderingHint() ) : 0;
        std::int32_t renderBinMode =
            ar.writing() ? static_cast<std::int32_t>( stateSet.getRenderBinMode() ) : 0;
        std::int32_t binNumber =
            ar.writing() ? static_cast<std::int32_t>( stateSet.getBinNumber() ) : 0;
        std::string binName = ar.writing() ? stateSet.getBinName() : std::string();
        bool        nestRenderBins = ar.writing() ? stateSet.getNestRenderBins() : true;

        ar.value( "RenderingHint", renderingHint );
        ar.value( "RenderBinMode", renderBinMode );
        ar.value( "BinNumber", binNumber );
        ar.value( "BinName", binName );
        ar.value( "NestRenderBins", nestRenderBins );

        if( ar.reading() )
        {
            stateSet.setRenderingHint( static_cast<int>( renderingHint ) );
            stateSet.setRenderBinMode(
                static_cast<osg::StateSet::RenderBinMode>( renderBinMode )
            );
            stateSet.setBinNumber( static_cast<int>( binNumber ) );
            stateSet.setBinName( binName );
            stateSet.setNestRenderBins( nestRenderBins );
        }
    }

}

OSG_REGISTER_SERIALIZER_FACTORY( osg::Image,
                                 new osg::Image );
OSG_REGISTER_SERIALIZER( osg,
                         Texture2D );
OSG_REGISTER_SERIALIZER_FACTORY( osg::StateSet,
                                 new osg::StateSet );
