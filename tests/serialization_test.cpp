// tests/serialization_test.cpp
#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <flatbuffers/flexbuffers.h>
#include <fstream>
#include <gtest/gtest.h>
#include <istream>
#include <limits>
#include <osg/geometry/Array.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/PrimitiveSet.hpp>
#include <osg/images/Image.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/maths/transform.hpp>
#include <osg/maths/vec2.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/nodes/Transform.hpp>
#include <osg/state/BlendFunc.hpp>
#include <osg/state/Material.hpp>
#include <osg/state/Program.hpp>
#include <osg/state/Shader.hpp>
#include <osg/state/StateAttribute.hpp>
#include <osg/state/StateSet.hpp>
#include <osg/state/Uniform.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/serialization/BinaryArchive.hpp>
#include <osgDB/serialization/FlexBufferArchive.hpp>
#include <osgDB/serialization/ObjectSerializer.hpp>
#include <osgDB/serialization/SceneCook.hpp>
#include <osgDB/serialization/TextureCompression.hpp>
#include <ostream>
#include <span>
#include <sstream>
#include <string>
#include <vector>

namespace
{

    constexpr std::uint32_t kAnswer              = 42U;
    constexpr std::int64_t  kSmokeValue          = 17;
    constexpr int           kImageWidth          = 2;
    constexpr int           kImageHeight         = 2;
    constexpr int           kImageDepth          = 1;
    constexpr std::size_t   kRgbaPixelSize       = 4U;
    constexpr std::size_t   kCookMagicSize       = 8U;
    constexpr std::size_t   kCookVersionSize     = sizeof( std::uint32_t );
    constexpr std::size_t   kCookPayloadSizeSize = sizeof( std::uint64_t );
    constexpr std::size_t   kCookChecksumOffset =
        kCookMagicSize + kCookVersionSize + kCookPayloadSizeSize;
    constexpr std::uint8_t kChecksumFlipMask   = 0X01U;
    constexpr std::size_t  kTruncatedByteCount = 1U;
    constexpr int          kCompressionWidth   = 8;
    constexpr int          kCompressionHeight  = 8;
    constexpr std::size_t  kDxt5BlockBytes     = 16U;

    enum class ArchiveBackend
    {
        Binary,
        FlexBuffer
    };

    std::string
    backendName( const testing::TestParamInfo<ArchiveBackend>& info )
    {
        return info.param == ArchiveBackend::Binary ? "BinaryArchive"
                                                    : "FlexBufferArchive";
    }

    template<typename WriteFn,
             typename ReadFn>
    void
    roundTripWithBackend( ArchiveBackend backend,
                          WriteFn        writeFn,
                          ReadFn         readFn )
    {
        if( backend == ArchiveBackend::Binary )
        {
            std::stringstream buffer;
            {
                osgDB::serialization::BinaryArchive out(
                    static_cast<std::ostream&>( buffer )
                );
                writeFn( out );
            }
            {
                osgDB::serialization::BinaryArchive in(
                    static_cast<std::istream&>( buffer )
                );
                readFn( in );
            }
            return;
        }

        std::vector<std::uint8_t> buffer;
        {
            osgDB::serialization::FlexBufferArchive out( buffer );
            writeFn( out );
            out.finish();
        }
        {
            osgDB::serialization::FlexBufferArchive in(
                std::span<const std::uint8_t>( buffer.data(), buffer.size() )
            );
            readFn( in );
        }
    }

    class SerializationArchiveTest : public testing::TestWithParam<ArchiveBackend>
    {
    };

    std::vector<std::byte>
    objectBytes( const void* data,
                 std::size_t size )
    {
        std::vector<std::byte> bytes( size );
        if( size > 0U )
        {
            std::memcpy( bytes.data(), data, size );
        }
        return bytes;
    }

    std::vector<std::byte>
    arrayBytes( const osg::Array& array )
    {
        return objectBytes( array.getDataPointer(), array.getTotalDataSize() );
    }

    std::vector<std::byte>
    primitiveBytes( const osg::PrimitiveSet& primitive )
    {
        return objectBytes( primitive.getDataPointer(), primitive.getTotalDataSize() );
    }

    std::vector<std::byte>
    imageBytes( const osg::Image& image )
    {
        return objectBytes( image.data(), image.getTotalSizeInBytesIncludingMipmaps() );
    }

    std::size_t
    dxtMipChainByteCount( int         width,
                          int         height,
                          std::size_t blockBytes )
    {
        std::size_t total = 0U;
        while( true )
        {
            const std::size_t blocksX =
                ( static_cast<std::size_t>( width ) + 3U ) / 4U;
            const std::size_t blocksY =
                ( static_cast<std::size_t>( height ) + 3U ) / 4U;
            total += blocksX * blocksY * blockBytes;

            if( width == 1 && height == 1 )
            {
                break;
            }
            width  = std::max( 1, width / 2 );
            height = std::max( 1, height / 2 );
        }
        return total;
    }

    osg::Image::MipmapDataType
    dxtMipOffsets( int         width,
                   int         height,
                   std::size_t blockBytes )
    {
        osg::Image::MipmapDataType offsets;
        std::size_t                offset     = 0U;
        bool                       firstLevel = true;
        while( true )
        {
            if( !firstLevel )
            {
                offsets.push_back( static_cast<unsigned int>( offset ) );
            }
            firstLevel = false;

            const std::size_t blocksX =
                ( static_cast<std::size_t>( width ) + 3U ) / 4U;
            const std::size_t blocksY =
                ( static_cast<std::size_t>( height ) + 3U ) / 4U;
            offset += blocksX * blocksY * blockBytes;

            if( width == 1 && height == 1 )
            {
                break;
            }
            width  = std::max( 1, width / 2 );
            height = std::max( 1, height / 2 );
        }
        return offsets;
    }

    osg::ref_ptr<osg::Vec3Array>
    makeVec3Array()
    {
        osg::ref_ptr<osg::Vec3Array> array = new osg::Vec3Array;
        array->push_back( osg::vec3( 1.0F, 2.0F, 3.0F ) );
        array->push_back( osg::vec3( 4.0F, 5.0F, 6.0F ) );
        array->push_back( osg::vec3( 7.0F, 8.0F, 9.0F ) );
        array->setBinding( osg::Array::BIND_PER_VERTEX );
        return array;
    }

    osg::ref_ptr<osg::Image>
    makeImage()
    {
        osg::ref_ptr<osg::Image> image = new osg::Image;
        image->allocateImage( kImageWidth,
                              kImageHeight,
                              kImageDepth,
                              GL_RGBA,
                              GL_UNSIGNED_BYTE,
                              1 );
        const std::array<unsigned char, kImageWidth * kImageHeight * kRgbaPixelSize>
            pixels{
                0X10U,
                0X20U,
                0X30U,
                0XFFU,
                0X40U,
                0X50U,
                0X60U,
                0XFFU,
                0X70U,
                0X80U,
                0X90U,
                0XFFU,
                0XA0U,
                0XB0U,
                0XC0U,
                0XFFU
        };
        std::memcpy( image->data(), pixels.data(), pixels.size() );
        return image;
    }

    osg::ref_ptr<osg::Image>
    makeCompressionImage()
    {
        osg::ref_ptr<osg::Image> image = new osg::Image;
        image->allocateImage( kCompressionWidth,
                              kCompressionHeight,
                              1,
                              GL_RGBA,
                              GL_UNSIGNED_BYTE,
                              1 );
        image->setInternalTextureFormat( GL_SRGB8_ALPHA8 );
        for( int y = 0; y < kCompressionHeight; ++y )
        {
            for( int x = 0; x < kCompressionWidth; ++x )
            {
                unsigned char* pixel =
                    image->data( static_cast<unsigned int>( x ),
                                 static_cast<unsigned int>( y ) );
                pixel[0] = static_cast<unsigned char>( ( x * 29 + y * 11 ) & 0XFF );
                pixel[1] = static_cast<unsigned char>( ( x * 7 + y * 37 ) & 0XFF );
                pixel[2] = static_cast<unsigned char>( ( x * 17 + y * 19 ) & 0XFF );
                pixel[3] = static_cast<unsigned char>( 0X40 + ( ( x + y ) & 0X3F ) );
            }
        }
        return image;
    }

    osg::ref_ptr<osg::Geometry>
    makeTexturedGeometry( osg::Texture2D* texture )
    {
        osg::ref_ptr<osg::Geometry>  geometry = osg::Geometry::create();

        osg::ref_ptr<osg::Vec3Array> vertices = makeVec3Array();
        osg::ref_ptr<osg::Vec3Array> normals  = new osg::Vec3Array;
        normals->push_back( osg::vec3( 0.0F, 0.0F, 1.0F ) );
        normals->push_back( osg::vec3( 0.0F, 0.0F, 1.0F ) );
        normals->push_back( osg::vec3( 0.0F, 0.0F, 1.0F ) );
        normals->setBinding( osg::Array::BIND_PER_VERTEX );

        osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;
        texcoords->push_back( osg::vec2( 0.0F, 0.0F ) );
        texcoords->push_back( osg::vec2( 1.0F, 0.0F ) );
        texcoords->push_back( osg::vec2( 0.0F, 1.0F ) );
        texcoords->setBinding( osg::Array::BIND_PER_VERTEX );

        geometry->setVertexArray( vertices.get() );
        geometry->setNormalArray( normals.get(), osg::Array::BIND_PER_VERTEX );
        geometry->setTexCoordArray( 0U, texcoords.get(), osg::Array::BIND_PER_VERTEX );

        osg::ref_ptr<osg::DrawElementsUInt> indices =
            new osg::DrawElementsUInt( GL_TRIANGLES );
        indices->push_back( 0U );
        indices->push_back( 1U );
        indices->push_back( 2U );
        geometry->addPrimitiveSet( indices.get() );

        osg::ref_ptr<osg::StateSet> stateSet = new osg::StateSet;
        stateSet->setTextureAttributeAndModes( 0U, texture, osg::StateAttribute::ON );
        geometry->setStateSet( stateSet.get() );
        return geometry;
    }

    std::filesystem::path
    makeSceneCookTempPath( const std::string& suffix )
    {
        const auto uniqueId =
            std::chrono::steady_clock::now().time_since_epoch().count();
        return std::filesystem::temp_directory_path() /
               ( std::string( "osgiliath_scene_cook_" ) +
                 std::to_string( uniqueId ) +
                 suffix );
    }

    std::vector<std::uint8_t>
    readFileBytes( const std::filesystem::path& path )
    {
        const std::uintmax_t fileSize = std::filesystem::file_size( path );
        EXPECT_LE(
            fileSize,
            static_cast<std::uintmax_t>( std::numeric_limits<std::size_t>::max() )
        );
        std::vector<std::uint8_t> bytes( static_cast<std::size_t>( fileSize ) );
        std::ifstream             input( path, std::ios::binary );
        EXPECT_TRUE( input );
        if( !bytes.empty() )
        {
            input.read( reinterpret_cast<char*>( bytes.data() ),
                        static_cast<std::streamsize>( bytes.size() ) );
        }
        EXPECT_TRUE( input );
        return bytes;
    }

    osg::ref_ptr<osg::Object>
    roundTripObject( ArchiveBackend            backend,
                     osg::ref_ptr<osg::Object> object )
    {
        osg::ref_ptr<osg::Object> restored;
        roundTripWithBackend(
            backend,
            [&]( osgDB::serialization::Archive& out )
            {
                osg::ref_ptr<osg::Object> obj = object;
                osgDB::serialization::serialize( out, obj );
            },
            [&]( osgDB::serialization::Archive& in )
            {
                osgDB::serialization::serialize( in, restored );
            }
        );
        return restored;
    }

    void
    expectVec3Near( const osg::vec3& actual,
                    const osg::vec3& expected )
    {
        EXPECT_FLOAT_EQ( actual.x, expected.x );
        EXPECT_FLOAT_EQ( actual.y, expected.y );
        EXPECT_FLOAT_EQ( actual.z, expected.z );
    }

    void
    expectVec4Near( const osg::vec4& actual,
                    const osg::vec4& expected )
    {
        EXPECT_FLOAT_EQ( actual.x, expected.x );
        EXPECT_FLOAT_EQ( actual.y, expected.y );
        EXPECT_FLOAT_EQ( actual.z, expected.z );
        EXPECT_FLOAT_EQ( actual.w, expected.w );
    }

    void
    expectMat4Near( const osg::mat4& actual,
                    const osg::mat4& expected )
    {
        for( std::size_t col = 0U; col < 4U; ++col )
        {
            for( std::size_t row = 0U; row < 4U; ++row )
            {
                EXPECT_FLOAT_EQ( actual.value[col][row], expected.value[col][row] );
            }
        }
    }

    osg::ref_ptr<osg::Material>
    makeMaterial()
    {
        osg::ref_ptr<osg::Material> material = osg::Material::create();
        material->setAmbient( osg::Material::FRONT,
                              osg::vec4( 0.10F, 0.20F, 0.30F, 1.00F ) );
        material->setAmbient( osg::Material::BACK,
                              osg::vec4( 0.40F, 0.50F, 0.60F, 1.00F ) );
        material->setDiffuse( osg::Material::FRONT_AND_BACK,
                              osg::vec4( 0.70F, 0.60F, 0.50F, 1.00F ) );
        material->setSpecular( osg::Material::FRONT,
                               osg::vec4( 0.90F, 0.80F, 0.70F, 1.00F ) );
        material->setSpecular( osg::Material::BACK,
                               osg::vec4( 0.30F, 0.20F, 0.10F, 1.00F ) );
        material->setEmission( osg::Material::FRONT_AND_BACK,
                               osg::vec4( 0.01F, 0.02F, 0.03F, 1.00F ) );
        material->setShininess( osg::Material::FRONT, 17.0F );
        material->setShininess( osg::Material::BACK, 23.0F );
        material->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE );
        return material;
    }

    osg::ref_ptr<osg::Program>
    makeProgram()
    {
        osg::ref_ptr<osg::Program> program = osg::Program::create();
        program->addShader(
            osg::Shader::create( osg::Shader::VERTEX,
                                 "void main() { gl_Position = vec4(1.0); }\n" )
        );
        program->addShader(
            osg::Shader::create( osg::Shader::FRAGMENT,
                                 "void main() { gl_FragColor = vec4(0.5); }\n" )
        );
        program->setParameter( GL_GEOMETRY_VERTICES_OUT_EXT, 3 );
        program->setParameter( GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES );
        program->setParameter( GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP );
        program->addBindAttribLocation( "a_position", 0U );
        program->addBindFragDataLocation( "fragColor", 0U );
        program->addBindUniformBlock( "FrameBlock", 2U );
        program->addTransformFeedBackVarying( "tf_position" );
        program->setTransformFeedBackMode( GL_INTERLEAVED_ATTRIBS );
        return program;
    }

    void
    expectProgramRoundTripped( const osg::Program& restored,
                               const osg::Program& original )
    {
        ASSERT_EQ( restored.getNumShaders(), original.getNumShaders() );
        for( unsigned int i = 0U; i < original.getNumShaders(); ++i )
        {
            ASSERT_NE( restored.getShader( i ), nullptr );
            ASSERT_NE( original.getShader( i ), nullptr );
            EXPECT_EQ( restored.getShader( i )->getType(),
                       original.getShader( i )->getType() );
            EXPECT_EQ( restored.getShader( i )->getShaderSource(),
                       original.getShader( i )->getShaderSource() );
        }

        EXPECT_EQ( restored.getParameter( GL_GEOMETRY_VERTICES_OUT_EXT ),
                   original.getParameter( GL_GEOMETRY_VERTICES_OUT_EXT ) );
        EXPECT_EQ( restored.getParameter( GL_GEOMETRY_INPUT_TYPE_EXT ),
                   original.getParameter( GL_GEOMETRY_INPUT_TYPE_EXT ) );
        EXPECT_EQ( restored.getParameter( GL_GEOMETRY_OUTPUT_TYPE_EXT ),
                   original.getParameter( GL_GEOMETRY_OUTPUT_TYPE_EXT ) );
        EXPECT_EQ( restored.getAttribBindingList(), original.getAttribBindingList() );
        EXPECT_EQ( restored.getFragDataBindingList(),
                   original.getFragDataBindingList() );
        EXPECT_EQ( restored.getUniformBlockBindingList(),
                   original.getUniformBlockBindingList() );
        ASSERT_EQ( restored.getNumTransformFeedBackVaryings(),
                   original.getNumTransformFeedBackVaryings() );
        EXPECT_EQ( restored.getTransformFeedBackVarying( 0U ),
                   original.getTransformFeedBackVarying( 0U ) );
        EXPECT_EQ( restored.getTransformFeedBackMode(),
                   original.getTransformFeedBackMode() );
    }

    template<typename T>
    void
    expectUniformElement( const osg::Uniform& uniform,
                          unsigned int        index,
                          const T&            expected )
    {
        T actual{};
        ASSERT_TRUE( uniform.getElement( index, actual ) );
        EXPECT_EQ( actual, expected );
    }

    void
    expectUniformFloatElement( const osg::Uniform& uniform,
                               unsigned int        index,
                               float               expected )
    {
        float actual = 0.0F;
        ASSERT_TRUE( uniform.getElement( index, actual ) );
        EXPECT_FLOAT_EQ( actual, expected );
    }

    void
    expectUniformVec3Element( const osg::Uniform& uniform,
                              unsigned int        index,
                              const osg::vec3&    expected )
    {
        osg::vec3 actual;
        ASSERT_TRUE( uniform.getElement( index, actual ) );
        expectVec3Near( actual, expected );
    }

    void
    expectUniformMat4Element( const osg::Uniform& uniform,
                              unsigned int        index,
                              const osg::mat4&    expected )
    {
        osg::mat4 actual;
        ASSERT_TRUE( uniform.getElement( index, actual ) );
        expectMat4Near( actual, expected );
    }

    void
    writeFileBytes( const std::filesystem::path&  path,
                    std::span<const std::uint8_t> bytes )
    {
        std::ofstream output( path, std::ios::binary | std::ios::out | std::ios::trunc );
        ASSERT_TRUE( output );
        if( !bytes.empty() )
        {
            output.write( reinterpret_cast<const char*>( bytes.data() ),
                          static_cast<std::streamsize>( bytes.size() ) );
        }
        ASSERT_TRUE( output );
    }

    void
    expectSharedTextureAndImageIdentity( osg::Object* restored )
    {
        ASSERT_NE( restored, nullptr );
        ASSERT_NE( restored->asNode(), nullptr );
        osg::Group* restoredRoot = restored->asNode()->asGroup();
        ASSERT_NE( restoredRoot, nullptr );
        ASSERT_EQ( restoredRoot->getNumChildren(), 2U );

        osg::Geometry* firstGeometry  = restoredRoot->getChild( 0U )->asGeometry();
        osg::Geometry* secondGeometry = restoredRoot->getChild( 1U )->asGeometry();
        ASSERT_NE( firstGeometry, nullptr );
        ASSERT_NE( secondGeometry, nullptr );
        ASSERT_NE( firstGeometry->getStateSet(), nullptr );
        ASSERT_NE( secondGeometry->getStateSet(), nullptr );

        const osg::StateAttribute* firstAttribute =
            firstGeometry->getStateSet()->getTextureAttribute(
                0U,
                osg::StateAttribute::Type::TEXTURE
            );
        const osg::StateAttribute* secondAttribute =
            secondGeometry->getStateSet()->getTextureAttribute(
                0U,
                osg::StateAttribute::Type::TEXTURE
            );
        ASSERT_NE( firstAttribute, nullptr );
        ASSERT_NE( secondAttribute, nullptr );
        ASSERT_NE( firstAttribute->asTexture(), nullptr );
        ASSERT_NE( secondAttribute->asTexture(), nullptr );

        const osg::Texture2D* firstTexture =
            firstAttribute->asTexture()->getImage( 0U ) != nullptr
                ? static_cast<const osg::Texture2D*>( firstAttribute->asTexture() )
                : nullptr;
        const osg::Texture2D* secondTexture =
            secondAttribute->asTexture()->getImage( 0U ) != nullptr
                ? static_cast<const osg::Texture2D*>( secondAttribute->asTexture() )
                : nullptr;
        ASSERT_NE( firstTexture, nullptr );
        ASSERT_NE( secondTexture, nullptr );
        EXPECT_EQ( firstTexture, secondTexture );
        ASSERT_NE( firstTexture->getImage(), nullptr );
        ASSERT_NE( secondTexture->getImage(), nullptr );
        EXPECT_EQ( firstTexture->getImage(), secondTexture->getImage() );
    }

}

TEST( FlexBuffers,
      BuilderSmokeRoundTripsInt )
{
    flexbuffers::Builder builder;
    const std::size_t    mapStart = builder.StartMap();
    builder.Int( "value", kSmokeValue );
    builder.EndMap( mapStart );
    builder.Finish();

    flexbuffers::Reference root = flexbuffers::GetRoot( builder.GetBuffer() );
    EXPECT_EQ( root.AsMap()["value"].AsInt64(), kSmokeValue );
}

TEST_P( SerializationArchiveTest,
        RoundTripsUint32 )
{
    std::uint32_t restored = 0U;
    roundTripWithBackend(
        GetParam(),
        []( osgDB::serialization::Archive& out )
        {
            out.beginObject( "root" );
            std::uint32_t written = kAnswer;
            out.value( "answer", written );
            out.endObject();
        },
        [&]( osgDB::serialization::Archive& in )
        {
            in.beginObject( "root" );
            in.value( "answer", restored );
            in.endObject();
        }
    );
    EXPECT_EQ( restored, kAnswer );
}

TEST_P( SerializationArchiveTest,
        RoundTripsStringAndBlob )
{
    const std::string            text{ "sponza" };
    const std::vector<std::byte> payload{
        std::byte{ 0X01 },
        std::byte{ 0X02 },
        std::byte{ 0X03 }
    };
    std::string            restoredText;
    std::vector<std::byte> restoredBlob;
    roundTripWithBackend(
        GetParam(),
        [&]( osgDB::serialization::Archive& out )
        {
            out.beginObject( "root" );
            std::string            t = text;
            std::vector<std::byte> b = payload;
            out.value( "name", t );
            out.blob( "data", b );
            out.endObject();
        },
        [&]( osgDB::serialization::Archive& in )
        {
            in.beginObject( "root" );
            in.value( "name", restoredText );
            in.blob( "data", restoredBlob );
            in.endObject();
        }
    );
    EXPECT_EQ( restoredText, text );
    EXPECT_EQ( restoredBlob, payload );
}

TEST_P( SerializationArchiveTest,
        CreatesRegisteredTypeByName )
{
    static_cast<void>( GetParam() );
    osg::ref_ptr<osg::Object> obj =
        osgDB::serialization::SerializerRegistry::instance().createByName(
            "osg::Group"
        );
    ASSERT_TRUE( obj.valid() );
    EXPECT_NE( obj->asNode(), nullptr );
}

TEST_P( SerializationArchiveTest,
        GroupWithMatrixTransformChildren )
{
    osg::ref_ptr<osg::Group>           root = osg::Group::create();
    osg::ref_ptr<osg::MatrixTransform> mt   = osg::MatrixTransform::create();
    mt->setMatrix( osg::translate( 1.0, 2.0, 3.0 ) );
    root->addChild( mt );
    root->addChild( osg::Group::create() );

    osg::ref_ptr<osg::Object> restored;
    roundTripWithBackend(
        GetParam(),
        [&]( osgDB::serialization::Archive& out )
        {
            osg::ref_ptr<osg::Object> obj = root;
            osgDB::serialization::serialize( out, obj );
        },
        [&]( osgDB::serialization::Archive& in )
        {
            osgDB::serialization::serialize( in, restored );
        }
    );
    ASSERT_TRUE( restored.valid() );
    ASSERT_NE( restored->asNode(), nullptr );
    osg::Group* restoredGroup = restored->asNode()->asGroup();
    ASSERT_NE( restoredGroup, nullptr );
    ASSERT_EQ( restoredGroup->getNumChildren(), 2U );
    osg::Node* firstChild = restoredGroup->getChild( 0U );
    ASSERT_NE( firstChild->asTransform(), nullptr );
    osg::MatrixTransform* restoredMt = firstChild->asTransform()->asMatrixTransform();
    ASSERT_NE( restoredMt, nullptr );
    EXPECT_EQ( restoredMt->getMatrix(), osg::translate( 1.0, 2.0, 3.0 ) );
}

TEST_P( SerializationArchiveTest,
        Vec3ArrayRoundTrips )
{
    osg::ref_ptr<osg::Vec3Array> array = makeVec3Array();
    osg::ref_ptr<osg::Object>    restored;

    roundTripWithBackend(
        GetParam(),
        [&]( osgDB::serialization::Archive& out )
        {
            osg::ref_ptr<osg::Object> obj = array;
            osgDB::serialization::serialize( out, obj );
        },
        [&]( osgDB::serialization::Archive& in )
        {
            osgDB::serialization::serialize( in, restored );
        }
    );

    ASSERT_TRUE( restored.valid() );
    ASSERT_STREQ( restored->className(), "Vec3Array" );
    osg::Array* restoredArray = static_cast<osg::Array*>( restored.get() );
    EXPECT_EQ( restoredArray->getType(), osg::Array::Vec3ArrayType );
    EXPECT_EQ( restoredArray->getNumElements(), array->getNumElements() );
    EXPECT_EQ( restoredArray->getBinding(), osg::Array::BIND_PER_VERTEX );
    EXPECT_EQ( arrayBytes( *restoredArray ), arrayBytes( *array ) );
}

TEST_P( SerializationArchiveTest,
        GeometryWithArraysAndDrawElementsRoundTrips )
{
    osg::ref_ptr<osg::Texture2D> texture  = osg::Texture2D::create();
    osg::ref_ptr<osg::Geometry>  geometry = makeTexturedGeometry( texture.get() );
    osg::ref_ptr<osg::Object>    restored;

    roundTripWithBackend(
        GetParam(),
        [&]( osgDB::serialization::Archive& out )
        {
            osg::ref_ptr<osg::Object> obj = geometry;
            osgDB::serialization::serialize( out, obj );
        },
        [&]( osgDB::serialization::Archive& in )
        {
            osgDB::serialization::serialize( in, restored );
        }
    );

    ASSERT_TRUE( restored.valid() );
    ASSERT_NE( restored->asNode(), nullptr );
    osg::Geometry* restoredGeometry = restored->asNode()->asGeometry();
    ASSERT_NE( restoredGeometry, nullptr );

    ASSERT_NE( restoredGeometry->getVertexArray(), nullptr );
    ASSERT_NE( restoredGeometry->getNormalArray(), nullptr );
    ASSERT_NE( restoredGeometry->getTexCoordArray( 0U ), nullptr );
    EXPECT_EQ( arrayBytes( *restoredGeometry->getVertexArray() ),
               arrayBytes( *geometry->getVertexArray() ) );
    EXPECT_EQ( arrayBytes( *restoredGeometry->getNormalArray() ),
               arrayBytes( *geometry->getNormalArray() ) );
    EXPECT_EQ( arrayBytes( *restoredGeometry->getTexCoordArray( 0U ) ),
               arrayBytes( *geometry->getTexCoordArray( 0U ) ) );
    EXPECT_EQ( restoredGeometry->getNormalArray()->getBinding(),
               osg::Array::BIND_PER_VERTEX );
    EXPECT_EQ( restoredGeometry->getTexCoordArray( 0U )->getBinding(),
               osg::Array::BIND_PER_VERTEX );

    ASSERT_EQ( restoredGeometry->getNumPrimitiveSets(), 1U );
    const osg::PrimitiveSet* restoredPrimitive = restoredGeometry->getPrimitiveSet( 0U );
    ASSERT_NE( restoredPrimitive, nullptr );
    EXPECT_EQ( restoredPrimitive->getType(),
               osg::PrimitiveSet::DrawElementsUIntPrimitiveType );
    EXPECT_EQ( restoredPrimitive->getMode(), GL_TRIANGLES );
    EXPECT_EQ( restoredPrimitive->getNumIndices(), 3U );
    EXPECT_EQ( primitiveBytes( *restoredPrimitive ),
               primitiveBytes( *geometry->getPrimitiveSet( 0U ) ) );
}

TEST_P( SerializationArchiveTest,
        ImageRoundTripsByteIdentical )
{
    osg::ref_ptr<osg::Image>  image = makeImage();
    osg::ref_ptr<osg::Object> restored;

    roundTripWithBackend(
        GetParam(),
        [&]( osgDB::serialization::Archive& out )
        {
            osg::ref_ptr<osg::Object> obj = image;
            osgDB::serialization::serialize( out, obj );
        },
        [&]( osgDB::serialization::Archive& in )
        {
            osgDB::serialization::serialize( in, restored );
        }
    );

    ASSERT_TRUE( restored.valid() );
    osg::Image* restoredImage = restored->asImage();
    ASSERT_NE( restoredImage, nullptr );
    EXPECT_EQ( restoredImage->s(), image->s() );
    EXPECT_EQ( restoredImage->t(), image->t() );
    EXPECT_EQ( restoredImage->r(), image->r() );
    EXPECT_EQ( restoredImage->getPixelFormat(), image->getPixelFormat() );
    EXPECT_EQ( restoredImage->getDataType(), image->getDataType() );
    EXPECT_EQ( imageBytes( *restoredImage ), imageBytes( *image ) );
}

TEST( TextureCompression,
      CompressesRgbaImageToSrgbDxt5MipChain )
{
    osg::ref_ptr<osg::Image> image      = makeCompressionImage();
    osg::ref_ptr<osg::Image> compressed =
        osgDB::serialization::compressImageDXT( *image );

    ASSERT_TRUE( compressed.valid() );
    EXPECT_TRUE( compressed->isCompressed() );
    EXPECT_EQ( compressed->s(), kCompressionWidth );
    EXPECT_EQ( compressed->t(), kCompressionHeight );
    EXPECT_EQ( compressed->r(), 1 );
    EXPECT_EQ( compressed->getInternalTextureFormat(),
               GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT );
    EXPECT_EQ( compressed->getPixelFormat(),
               GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT );
    EXPECT_EQ( compressed->getDataType(), GL_UNSIGNED_BYTE );

    const std::size_t expectedBytes =
        dxtMipChainByteCount( kCompressionWidth, kCompressionHeight, kDxt5BlockBytes );
    EXPECT_EQ( static_cast<std::size_t>(
                   compressed->getTotalSizeInBytesIncludingMipmaps()
               ),
               expectedBytes );
    EXPECT_EQ( compressed->getMipmapLevels(),
               dxtMipOffsets( kCompressionWidth, kCompressionHeight, kDxt5BlockBytes ) );
}

TEST_P( SerializationArchiveTest,
        SharedTextureAndImageIdentityRoundTrips )
{
    osg::ref_ptr<osg::Image>     image   = makeImage();
    osg::ref_ptr<osg::Texture2D> texture = osg::Texture2D::create();
    texture->setImage( image.get() );
    texture->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
    texture->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );

    osg::ref_ptr<osg::Group> root = osg::Group::create();
    root->addChild( makeTexturedGeometry( texture.get() ).get() );
    root->addChild( makeTexturedGeometry( texture.get() ).get() );

    osg::ref_ptr<osg::Object> restored;
    roundTripWithBackend(
        GetParam(),
        [&]( osgDB::serialization::Archive& out )
        {
            osg::ref_ptr<osg::Object> obj = root;
            osgDB::serialization::serialize( out, obj );
        },
        [&]( osgDB::serialization::Archive& in )
        {
            osgDB::serialization::serialize( in, restored );
        }
    );

    ASSERT_TRUE( restored.valid() );
    expectSharedTextureAndImageIdentity( restored.get() );
}

TEST_P( SerializationArchiveTest,
        MaterialRoundTripsState )
{
    osg::ref_ptr<osg::Material> material = makeMaterial();

    osg::ref_ptr<osg::Object>   restoredObject =
        roundTripObject( GetParam(), osg::ref_ptr<osg::Object>( material.get() ) );

    ASSERT_TRUE( restoredObject.valid() );
    ASSERT_NE( restoredObject->asStateAttribute(), nullptr );
    osg::Material* restored =
        dynamic_cast<osg::Material*>( restoredObject->asStateAttribute() );
    ASSERT_NE( restored, nullptr );
    EXPECT_EQ( restored->getColorMode(), material->getColorMode() );
    expectVec4Near( restored->getAmbient( osg::Material::FRONT ),
                    material->getAmbient( osg::Material::FRONT ) );
    expectVec4Near( restored->getAmbient( osg::Material::BACK ),
                    material->getAmbient( osg::Material::BACK ) );
    expectVec4Near( restored->getDiffuse( osg::Material::FRONT ),
                    material->getDiffuse( osg::Material::FRONT ) );
    expectVec4Near( restored->getDiffuse( osg::Material::BACK ),
                    material->getDiffuse( osg::Material::BACK ) );
    expectVec4Near( restored->getSpecular( osg::Material::FRONT ),
                    material->getSpecular( osg::Material::FRONT ) );
    expectVec4Near( restored->getSpecular( osg::Material::BACK ),
                    material->getSpecular( osg::Material::BACK ) );
    EXPECT_FLOAT_EQ( restored->getShininess( osg::Material::FRONT ),
                     material->getShininess( osg::Material::FRONT ) );
    EXPECT_FLOAT_EQ( restored->getShininess( osg::Material::BACK ),
                     material->getShininess( osg::Material::BACK ) );
}

TEST_P( SerializationArchiveTest,
        BlendFuncRoundTripsFactors )
{
    osg::ref_ptr<osg::BlendFunc> blendFunc = osg::BlendFunc::create();
    blendFunc->setFunction( GL_SRC_ALPHA,
                            GL_ONE_MINUS_SRC_ALPHA,
                            GL_ONE,
                            GL_ONE_MINUS_SRC_COLOR );

    osg::ref_ptr<osg::Object> restoredObject =
        roundTripObject( GetParam(), osg::ref_ptr<osg::Object>( blendFunc.get() ) );

    ASSERT_TRUE( restoredObject.valid() );
    ASSERT_NE( restoredObject->asStateAttribute(), nullptr );
    osg::BlendFunc* restored =
        dynamic_cast<osg::BlendFunc*>( restoredObject->asStateAttribute() );
    ASSERT_NE( restored, nullptr );
    EXPECT_EQ( restored->getSourceRGB(), blendFunc->getSourceRGB() );
    EXPECT_EQ( restored->getDestinationRGB(), blendFunc->getDestinationRGB() );
    EXPECT_EQ( restored->getSourceAlpha(), blendFunc->getSourceAlpha() );
    EXPECT_EQ( restored->getDestinationAlpha(), blendFunc->getDestinationAlpha() );
}

TEST_P( SerializationArchiveTest,
        ProgramWithShadersRoundTrips )
{
    osg::ref_ptr<osg::Program> program = makeProgram();

    osg::ref_ptr<osg::Object>  restoredObject =
        roundTripObject( GetParam(), osg::ref_ptr<osg::Object>( program.get() ) );

    ASSERT_TRUE( restoredObject.valid() );
    ASSERT_NE( restoredObject->asStateAttribute(), nullptr );
    osg::Program* restored =
        dynamic_cast<osg::Program*>( restoredObject->asStateAttribute() );
    ASSERT_NE( restored, nullptr );
    expectProgramRoundTripped( *restored, *program );
}

TEST_P( SerializationArchiveTest,
        UniformRoundTripsCommonTypes )
{
    {
        osg::ref_ptr<osg::Uniform> uniform =
            osg::Uniform::create( osg::Uniform::FLOAT, "u_float" );
        ASSERT_TRUE( uniform->set( 1.25F ) );
        osg::ref_ptr<osg::Object> restoredObject =
            roundTripObject( GetParam(), osg::ref_ptr<osg::Object>( uniform.get() ) );
        ASSERT_TRUE( restoredObject.valid() );
        osg::Uniform* restored = restoredObject->asUniformBase()->asUniform();
        ASSERT_NE( restored, nullptr );
        EXPECT_EQ( restored->getName(), uniform->getName() );
        EXPECT_EQ( restored->getType(), uniform->getType() );
        expectUniformFloatElement( *restored, 0U, 1.25F );
    }

    {
        const osg::vec3            expected( 2.0F, 4.0F, 8.0F );
        osg::ref_ptr<osg::Uniform> uniform =
            osg::Uniform::create( osg::Uniform::FLOAT_VEC3, "u_vec3" );
        ASSERT_TRUE( uniform->set( expected ) );
        osg::ref_ptr<osg::Object> restoredObject =
            roundTripObject( GetParam(), osg::ref_ptr<osg::Object>( uniform.get() ) );
        ASSERT_TRUE( restoredObject.valid() );
        osg::Uniform* restored = restoredObject->asUniformBase()->asUniform();
        ASSERT_NE( restored, nullptr );
        EXPECT_EQ( restored->getName(), uniform->getName() );
        EXPECT_EQ( restored->getType(), uniform->getType() );
        expectUniformVec3Element( *restored, 0U, expected );
    }

    {
        osg::ref_ptr<osg::Uniform> uniform =
            osg::Uniform::create( osg::Uniform::INT, "u_int" );
        ASSERT_TRUE( uniform->set( 7 ) );
        osg::ref_ptr<osg::Object> restoredObject =
            roundTripObject( GetParam(), osg::ref_ptr<osg::Object>( uniform.get() ) );
        ASSERT_TRUE( restoredObject.valid() );
        osg::Uniform* restored = restoredObject->asUniformBase()->asUniform();
        ASSERT_NE( restored, nullptr );
        EXPECT_EQ( restored->getName(), uniform->getName() );
        EXPECT_EQ( restored->getType(), uniform->getType() );
        expectUniformElement( *restored, 0U, 7 );
    }

    {
        osg::ref_ptr<osg::Uniform> uniform =
            osg::Uniform::create( osg::Uniform::SAMPLER_2D, "u_texture" );
        ASSERT_TRUE( uniform->set( 3 ) );
        osg::ref_ptr<osg::Object> restoredObject =
            roundTripObject( GetParam(), osg::ref_ptr<osg::Object>( uniform.get() ) );
        ASSERT_TRUE( restoredObject.valid() );
        osg::Uniform* restored = restoredObject->asUniformBase()->asUniform();
        ASSERT_NE( restored, nullptr );
        EXPECT_EQ( restored->getName(), uniform->getName() );
        EXPECT_EQ( restored->getType(), uniform->getType() );
        expectUniformElement( *restored, 0U, 3 );
    }

    {
        const osg::mat4            expected( 1.0F,
                                             2.0F,
                                             3.0F,
                                             4.0F,
                                             5.0F,
                                             6.0F,
                                             7.0F,
                                             8.0F,
                                             9.0F,
                                             10.0F,
                                             11.0F,
                                             12.0F,
                                             13.0F,
                                             14.0F,
                                             15.0F,
                                             16.0F );
        osg::ref_ptr<osg::Uniform> uniform =
            osg::Uniform::create( osg::Uniform::FLOAT_MAT4, "u_matrix" );
        ASSERT_TRUE( uniform->set( expected ) );
        osg::ref_ptr<osg::Object> restoredObject =
            roundTripObject( GetParam(), osg::ref_ptr<osg::Object>( uniform.get() ) );
        ASSERT_TRUE( restoredObject.valid() );
        osg::Uniform* restored = restoredObject->asUniformBase()->asUniform();
        ASSERT_NE( restored, nullptr );
        EXPECT_EQ( restored->getName(), uniform->getName() );
        EXPECT_EQ( restored->getType(), uniform->getType() );
        expectUniformMat4Element( *restored, 0U, expected );
    }
}

TEST_P( SerializationArchiveTest,
        StateSetRoundTripsStateAttributesUniformsAndProgram )
{
    osg::ref_ptr<osg::Material>  material  = makeMaterial();
    osg::ref_ptr<osg::BlendFunc> blendFunc = osg::BlendFunc::create();
    blendFunc->setFunction( GL_SRC_ALPHA,
                            GL_ONE_MINUS_SRC_ALPHA,
                            GL_ONE,
                            GL_ONE_MINUS_SRC_ALPHA );
    osg::ref_ptr<osg::Uniform> uniform =
        osg::Uniform::create( osg::Uniform::FLOAT_VEC3, "u_state_vec3" );
    ASSERT_TRUE( uniform->set( osg::vec3( 1.0F, 3.0F, 5.0F ) ) );
    osg::ref_ptr<osg::Program>  program  = makeProgram();

    osg::ref_ptr<osg::StateSet> stateSet = new osg::StateSet;
    stateSet->setAttribute( material.get(), osg::StateAttribute::ON );
    stateSet->setAttributeAndModes( blendFunc.get(), osg::StateAttribute::ON );
    stateSet->setAttribute( program.get(), osg::StateAttribute::ON );
    stateSet->addUniform( uniform.get(), osg::StateAttribute::ON );

    osg::ref_ptr<osg::Object> restoredObject =
        roundTripObject( GetParam(), osg::ref_ptr<osg::Object>( stateSet.get() ) );

    ASSERT_TRUE( restoredObject.valid() );
    osg::StateSet* restored = restoredObject->asStateSet();
    ASSERT_NE( restored, nullptr );

    const osg::StateAttribute* restoredMaterialAttribute =
        restored->getAttribute( osg::StateAttribute::Type::MATERIAL );
    ASSERT_NE( restoredMaterialAttribute, nullptr );
    const osg::Material* restoredMaterial =
        dynamic_cast<const osg::Material*>( restoredMaterialAttribute );
    ASSERT_NE( restoredMaterial, nullptr );
    expectVec4Near( restoredMaterial->getAmbient( osg::Material::FRONT ),
                    material->getAmbient( osg::Material::FRONT ) );
    EXPECT_EQ( restoredMaterial->getColorMode(), material->getColorMode() );

    const osg::StateAttribute* restoredBlendFuncAttribute =
        restored->getAttribute( osg::StateAttribute::Type::BLENDFUNC );
    ASSERT_NE( restoredBlendFuncAttribute, nullptr );
    const osg::BlendFunc* restoredBlendFunc =
        dynamic_cast<const osg::BlendFunc*>( restoredBlendFuncAttribute );
    ASSERT_NE( restoredBlendFunc, nullptr );
    EXPECT_EQ( restoredBlendFunc->getSourceRGB(), blendFunc->getSourceRGB() );
    EXPECT_EQ( restoredBlendFunc->getDestinationAlpha(),
               blendFunc->getDestinationAlpha() );

    const osg::Uniform* restoredUniform = restored->getUniform( "u_state_vec3" );
    ASSERT_NE( restoredUniform, nullptr );
    expectUniformVec3Element( *restoredUniform, 0U, osg::vec3( 1.0F, 3.0F, 5.0F ) );

    const osg::StateAttribute* restoredProgramAttribute =
        restored->getAttribute( osg::StateAttribute::Type::PROGRAM );
    ASSERT_NE( restoredProgramAttribute, nullptr );
    const osg::Program* restoredProgram =
        dynamic_cast<const osg::Program*>( restoredProgramAttribute );
    ASSERT_NE( restoredProgram, nullptr );
    expectProgramRoundTripped( *restoredProgram, *program );
}

TEST( SceneCook,
      RoundTripsSyntheticSceneAndRejectsInvalidFiles )
{
    osg::ref_ptr<osg::Image>     image   = makeImage();
    osg::ref_ptr<osg::Texture2D> texture = osg::Texture2D::create();
    texture->setImage( image.get() );
    texture->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
    texture->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );

    osg::ref_ptr<osg::Group> root = osg::Group::create();
    root->addChild( makeTexturedGeometry( texture.get() ).get() );
    root->addChild( makeTexturedGeometry( texture.get() ).get() );

    const std::filesystem::path cookPath = makeSceneCookTempPath( ".scenecook" );
    const std::filesystem::path corruptPath =
        makeSceneCookTempPath( ".corrupt.scenecook" );
    const std::filesystem::path truncatedPath =
        makeSceneCookTempPath( ".truncated.scenecook" );

    ASSERT_TRUE( osgDB::serialization::writeSceneCook( *root, cookPath.string() ) );

    osg::ref_ptr<osg::Object> restored =
        osgDB::serialization::readSceneCook( cookPath.string() );
    ASSERT_TRUE( restored.valid() );
    expectSharedTextureAndImageIdentity( restored.get() );

    std::vector<std::uint8_t> bytes = readFileBytes( cookPath );
    ASSERT_GT( bytes.size(), kCookChecksumOffset );
    bytes[kCookChecksumOffset] ^= kChecksumFlipMask;
    writeFileBytes( corruptPath, bytes );
    EXPECT_FALSE( osgDB::serialization::readSceneCook( corruptPath.string() ) );

    ASSERT_GT( bytes.size(), kTruncatedByteCount );
    writeFileBytes(
        truncatedPath,
        std::span<const std::uint8_t>( bytes.data(), bytes.size() - kTruncatedByteCount )
    );
    EXPECT_FALSE( osgDB::serialization::readSceneCook( truncatedPath.string() ) );

    std::error_code cleanupError;
    std::filesystem::remove( cookPath, cleanupError );
    cleanupError.clear();
    std::filesystem::remove( corruptPath, cleanupError );
    cleanupError.clear();
    std::filesystem::remove( truncatedPath, cleanupError );
}

TEST( SceneCook,
      RoundTripsCompressedTextureBytesAndFormat )
{
    osg::ref_ptr<osg::Image> compressed =
        osgDB::serialization::compressImageDXT( *makeCompressionImage() );
    ASSERT_TRUE( compressed.valid() );

    osg::ref_ptr<osg::Texture2D> texture = osg::Texture2D::create();
    texture->setImage( compressed.get() );
    texture->setInternalFormat( compressed->getInternalTextureFormat() );

    osg::ref_ptr<osg::Group> root = osg::Group::create();
    root->addChild( makeTexturedGeometry( texture.get() ).get() );

    const std::filesystem::path cookPath =
        makeSceneCookTempPath( ".compressed.scenecook" );
    ASSERT_TRUE( osgDB::serialization::writeSceneCook( *root, cookPath.string() ) );

    osg::ref_ptr<osg::Object> restored =
        osgDB::serialization::readSceneCook( cookPath.string() );
    ASSERT_TRUE( restored.valid() );
    ASSERT_NE( restored->asNode(), nullptr );
    osg::Group* restoredRoot = restored->asNode()->asGroup();
    ASSERT_NE( restoredRoot, nullptr );
    ASSERT_EQ( restoredRoot->getNumChildren(), 1U );

    osg::Geometry* restoredGeometry = restoredRoot->getChild( 0U )->asGeometry();
    ASSERT_NE( restoredGeometry, nullptr );
    ASSERT_NE( restoredGeometry->getStateSet(), nullptr );
    const osg::StateAttribute* restoredAttribute =
        restoredGeometry->getStateSet()->getTextureAttribute(
            0U,
            osg::StateAttribute::Type::TEXTURE
        );
    ASSERT_NE( restoredAttribute, nullptr );
    const osg::Texture2D* restoredTexture =
        dynamic_cast<const osg::Texture2D*>( restoredAttribute->asTexture() );
    ASSERT_NE( restoredTexture, nullptr );
    const osg::Image* restoredImage = restoredTexture->getImage();
    ASSERT_NE( restoredImage, nullptr );

    EXPECT_TRUE( restoredImage->isCompressed() );
    EXPECT_EQ( restoredImage->getInternalTextureFormat(),
               compressed->getInternalTextureFormat() );
    EXPECT_EQ( restoredImage->getPixelFormat(), compressed->getPixelFormat() );
    EXPECT_EQ( restoredImage->getDataType(), compressed->getDataType() );
    EXPECT_EQ( restoredImage->getMipmapLevels(), compressed->getMipmapLevels() );
    EXPECT_EQ( imageBytes( *restoredImage ), imageBytes( *compressed ) );

    std::error_code cleanupError;
    std::filesystem::remove( cookPath, cleanupError );
}

INSTANTIATE_TEST_SUITE_P( Backends,
                          SerializationArchiveTest,
                          testing::Values( ArchiveBackend::Binary,
                                           ArchiveBackend::FlexBuffer ),
                          backendName );
