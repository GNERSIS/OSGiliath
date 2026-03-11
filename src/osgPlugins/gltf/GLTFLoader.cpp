#include "GLTFLoader.hpp"

#include <array>
#include <fstream>
#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <sstream>

using json = nlohmann::json;

namespace
{

    // Base64 decoding table
    static const std::array<int,
                            256>
    makeBase64Table()
    {
        std::array<int, 256> t;
        t.fill( -1 );
        for( int i = 0; i < 26; ++i )
        {
            t['A' + i] = i;
            t['a' + i] = i + 26;
        }
        for( int i = 0; i < 10; ++i )
        {
            t['0' + i] = i + 52;
        }
        t['+'] = 62;
        t['/'] = 63;
        t['='] = 0;    // padding
        return t;
    }

    std::vector<uint8_t>
    decodeBase64( const std::string& input )
    {
        static const auto    table = makeBase64Table();
        std::vector<uint8_t> out;
        out.reserve( input.size() * 3 / 4 );

        int val = 0, bits = -8;
        for( unsigned char c : input )
        {
            if( table[c] == -1 )
            {
                continue;    // skip whitespace/invalid
            }
            if( c == '=' )
            {
                break;
            }
            val   = ( val << 6 ) | table[c];
            bits += 6;
            if( bits >= 0 )
            {
                out.push_back( static_cast<uint8_t>( ( val >> bits ) & 0XFF ) );
                bits -= 8;
            }
        }
        return out;
    }

}    // anonymous namespace

osg::ref_ptr<osg::Node>
GLTFLoader::load( const std::string&                  filename,
                  const osgDB::ReaderWriter::Options* options )
{
    _options        = options;
    _baseDir        = filename.substr( 0, filename.find_last_of( "/\\" ) + 1 );

    std::string ext = filename.substr( filename.find_last_of( '.' ) + 1 );
    bool        ok  = false;
    if( ext == "glb" )
    {
        ok = parseGLB( filename );
    }
    else
    {
        ok = parseJSON( filename );
    }

    if( !ok )
    {
        return nullptr;
    }

    loadBuffers();
    loadImages();
    loadTextures();
    loadMaterials();
    auto scene = buildScene();
    loadAnimations();
    return scene;
}

bool
GLTFLoader::parseJSON( const std::string& filename )
{
    std::ifstream file( filename );
    if( !file.is_open() )
    {
        OSG_WARN << "GLTFLoader: cannot open " << filename << std::endl;
        return false;
    }
    try
    {
        _json = json::parse( file );
    }
    catch( const json::parse_error& e )
    {
        OSG_WARN << "GLTFLoader: JSON parse error: " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool
GLTFLoader::parseGLB( const std::string& filename )
{
    std::ifstream file( filename, std::ios::binary );
    if( !file.is_open() )
    {
        OSG_WARN << "GLTFLoader: cannot open " << filename << std::endl;
        return false;
    }

    // Read GLB header
    uint32_t magic, version, length;
    file.read( reinterpret_cast<char*>( &magic ), 4 );
    file.read( reinterpret_cast<char*>( &version ), 4 );
    file.read( reinterpret_cast<char*>( &length ), 4 );

    if( magic != 0X46'54'6C'67 || version != 2 )
    {
        OSG_WARN << "GLTFLoader: invalid GLB header" << std::endl;
        return false;
    }

    // Read JSON chunk
    uint32_t chunkLength, chunkType;
    file.read( reinterpret_cast<char*>( &chunkLength ), 4 );
    file.read( reinterpret_cast<char*>( &chunkType ), 4 );

    if( chunkType != 0X4E'4F'53'4A )
    {    // "JSON"
        OSG_WARN << "GLTFLoader: expected JSON chunk" << std::endl;
        return false;
    }

    std::vector<char> jsonData( chunkLength );
    file.read( jsonData.data(), chunkLength );

    try
    {
        _json = json::parse( jsonData.begin(), jsonData.end() );
    }
    catch( const json::parse_error& e )
    {
        OSG_WARN << "GLTFLoader: GLB JSON parse error: " << e.what() << std::endl;
        return false;
    }

    // Read BIN chunk if present
    if( file.tellg() < static_cast<std::streampos>( length ) )
    {
        file.read( reinterpret_cast<char*>( &chunkLength ), 4 );
        file.read( reinterpret_cast<char*>( &chunkType ), 4 );
        if( chunkType == 0X00'4E'49'42 )
        {    // "BIN\0"
            std::vector<uint8_t> binData( chunkLength );
            file.read( reinterpret_cast<char*>( binData.data() ), chunkLength );
            _buffers.push_back( std::move( binData ) );
        }
    }

    return true;
}

void
GLTFLoader::loadBuffers()
{
    if( !_json.contains( "buffers" ) )
    {
        return;
    }

    size_t startIdx = _buffers.size();    // GLB may have already loaded buffer 0
    for( size_t i = startIdx; i < _json["buffers"].size(); ++i )
    {
        const auto& buf = _json["buffers"][i];
        if( buf.contains( "uri" ) )
        {
            std::string uri = buf["uri"].get<std::string>();
            // Check for data URI
            if( uri.rfind( "data:", 0 ) == 0 )
            {
                // data:[<mediatype>][;base64],<data>
                size_t commaPos = uri.find( ',' );
                if( commaPos != std::string::npos )
                {
                    std::string header  = uri.substr( 0, commaPos );
                    std::string encoded = uri.substr( commaPos + 1 );
                    if( header.find( "base64" ) != std::string::npos )
                    {
                        std::vector<uint8_t> decoded = decodeBase64( encoded );
                        OSG_INFO << "GLTFLoader: decoded base64 buffer, "
                                 << decoded.size() << " bytes" << std::endl;
                        _buffers.push_back( std::move( decoded ) );
                    }
                    else
                    {
                        OSG_WARN << "GLTFLoader: non-base64 data URIs not supported"
                                 << std::endl;
                        _buffers.push_back( {} );
                    }
                }
                else
                {
                    OSG_WARN << "GLTFLoader: malformed data URI" << std::endl;
                    _buffers.push_back( {} );
                }
            }
            else
            {
                // External file
                std::string   path = _baseDir + uri;
                std::ifstream f( path, std::ios::binary );
                if( f.is_open() )
                {
                    size_t               byteLength = buf["byteLength"].get<size_t>();
                    std::vector<uint8_t> data( byteLength );
                    f.read( reinterpret_cast<char*>( data.data() ), byteLength );
                    _buffers.push_back( std::move( data ) );
                }
                else
                {
                    OSG_WARN << "GLTFLoader: cannot open buffer " << path << std::endl;
                    _buffers.push_back( {} );
                }
            }
        }
        else if( i >= startIdx )
        {
            // No URI and not already loaded (GLB buffer 0)
            _buffers.push_back( {} );
        }
    }
}

void
GLTFLoader::loadImages()
{
    if( !_json.contains( "images" ) )
    {
        return;
    }

    for( const auto& img : _json["images"] )
    {
        osg::ref_ptr<osg::Image> osgImage;
        if( img.contains( "uri" ) )
        {
            std::string uri = img["uri"].get<std::string>();
            if( uri.rfind( "data:", 0 ) != 0 )
            {
                // External image file
                std::string path = _baseDir + uri;
                osgImage         = osgDB::readImageFile( path, _options );
            }
        }
        else if( img.contains( "bufferView" ) )
        {
            // Image embedded in buffer
            int         bvIdx  = img["bufferView"].get<int>();
            const auto& bv     = _json["bufferViews"][bvIdx];
            int         bufIdx = bv["buffer"].get<int>();
            size_t      offset = bv.value( "byteOffset", 0 );
            size_t      length = bv["byteLength"].get<size_t>();

            if( bufIdx < static_cast<int>( _buffers.size() ) )
            {
                const uint8_t* data     = _buffers[bufIdx].data() + offset;
                std::string    mimeType = img.value( "mimeType", "image/png" );
                std::string    ext      = ( mimeType == "image/jpeg" ) ? "jpg" : "png";

                // Load via osgDB ReaderWriter from memory stream
                osg::ref_ptr<osgDB::ReaderWriter> rw =
                    osgDB::Registry::instance()->getReaderWriterForExtension( ext );
                if( rw )
                {
                    std::string dataStr( reinterpret_cast<const char*>( data ), length );
                    std::istringstream iss( dataStr );
                    auto               result = rw->readImage( iss, _options );
                    if( result.validImage() )
                    {
                        osgImage = result.getImage();
                    }
                }
            }
        }
        _images.push_back( osgImage );
    }
}

void
GLTFLoader::loadTextures()
{
    if( !_json.contains( "textures" ) )
    {
        return;
    }

    for( const auto& tex : _json["textures"] )
    {
        osg::ref_ptr<osg::Texture2D> osgTex = new osg::Texture2D;

        if( tex.contains( "source" ) )
        {
            int imgIdx = tex["source"].get<int>();
            if( imgIdx < static_cast<int>( _images.size() ) && _images[imgIdx].valid() )
            {
                osgTex->setImage( _images[imgIdx] );
            }
        }

        // Apply sampler settings
        if( tex.contains( "sampler" ) && _json.contains( "samplers" ) )
        {
            int         sampIdx = tex["sampler"].get<int>();
            const auto& samp    = _json["samplers"][sampIdx];

            if( samp.contains( "minFilter" ) )
            {
                osgTex->setFilter( osg::Texture2D::MIN_FILTER,
                                   static_cast<osg::Texture2D::FilterMode>(
                                       samp["minFilter"].get<int>()
                                   ) );
            }
            else
            {
                osgTex->setFilter( osg::Texture2D::MIN_FILTER,
                                   osg::Texture2D::LINEAR_MIPMAP_LINEAR );
            }
            if( samp.contains( "magFilter" ) )
            {
                osgTex->setFilter( osg::Texture2D::MAG_FILTER,
                                   static_cast<osg::Texture2D::FilterMode>(
                                       samp["magFilter"].get<int>()
                                   ) );
            }
            else
            {
                osgTex->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
            }

            auto wrapMode = []( int val ) -> osg::Texture::WrapMode
            {
                switch( val )
                {
                    case 33'071 :
                        return osg::Texture::CLAMP_TO_EDGE;
                    case 33'648 :
                        return osg::Texture::MIRROR;
                    default :
                        return osg::Texture::REPEAT;
                }
            };
            if( samp.contains( "wrapS" ) )
            {
                osgTex->setWrap( osg::Texture2D::WRAP_S,
                                 wrapMode( samp["wrapS"].get<int>() ) );
            }
            if( samp.contains( "wrapT" ) )
            {
                osgTex->setWrap( osg::Texture2D::WRAP_T,
                                 wrapMode( samp["wrapT"].get<int>() ) );
            }
        }
        else
        {
            // Default sampler: repeat wrap, linear filtering
            osgTex->setFilter( osg::Texture2D::MIN_FILTER,
                               osg::Texture2D::LINEAR_MIPMAP_LINEAR );
            osgTex->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
            osgTex->setWrap( osg::Texture2D::WRAP_S, osg::Texture::REPEAT );
            osgTex->setWrap( osg::Texture2D::WRAP_T, osg::Texture::REPEAT );
        }

        _textures.push_back( osgTex );
    }
}

void
GLTFLoader::loadMaterials()
{
    if( !_json.contains( "materials" ) )
    {
        return;
    }

    for( const auto& mat : _json["materials"] )
    {
        osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;

        if( mat.contains( "pbrMetallicRoughness" ) )
        {
            const auto&                 pbr = mat["pbrMetallicRoughness"];

            // Base color factor -> Material diffuse
            osg::ref_ptr<osg::Material> osgMat = new osg::Material;
            if( pbr.contains( "baseColorFactor" ) )
            {
                auto      c = pbr["baseColorFactor"];
                osg::vec4 color( c[0].get<float>(),
                                 c[1].get<float>(),
                                 c[2].get<float>(),
                                 c[3].get<float>() );
                osgMat->setDiffuse( osg::Material::FRONT_AND_BACK, color );
                osgMat->setAmbient( osg::Material::FRONT_AND_BACK, color * 0.2F );
            }
            ss->setAttributeAndModes( osgMat );

            // Base color texture
            if( pbr.contains( "baseColorTexture" ) )
            {
                int texIdx = pbr["baseColorTexture"]["index"].get<int>();
                if( texIdx <
                    static_cast<int>( _textures.size() ) &&
                    _textures[texIdx].valid() )
                {
                    ss->setTextureAttributeAndModes( 0, _textures[texIdx] );
                }
            }
        }

        // Double-sided -> disable backface culling
        if( mat.value( "doubleSided", false ) )
        {
            ss->setMode( GL_CULL_FACE, osg::StateAttribute::OFF );
        }

        // Alpha blending
        std::string alphaMode = mat.value( "alphaMode", "OPAQUE" );
        if( alphaMode == "BLEND" )
        {
            ss->setAttributeAndModes( new osg::BlendFunc( GL_SRC_ALPHA,
                                                          GL_ONE_MINUS_SRC_ALPHA ) );
            ss->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
        }

        _materials.push_back( ss );
    }
}

const uint8_t*
GLTFLoader::getAccessorData( int     accessorIdx,
                             size_t& count,
                             int&    componentType,
                             int&    type,
                             size_t& stride ) const
{
    const auto& acc = _json["accessors"][accessorIdx];
    count           = acc["count"].get<size_t>();
    componentType   = acc["componentType"].get<int>();

    // Type string -> component count
    std::string typeStr = acc["type"].get<std::string>();
    if( typeStr == "SCALAR" )
    {
        type = 1;
    }
    else if( typeStr == "VEC2" )
    {
        type = 2;
    }
    else if( typeStr == "VEC3" )
    {
        type = 3;
    }
    else if( typeStr == "VEC4" )
    {
        type = 4;
    }
    else
    {
        type = 1;
    }

    if( !acc.contains( "bufferView" ) )
    {
        return nullptr;
    }

    int         bvIdx     = acc["bufferView"].get<int>();
    const auto& bv        = _json["bufferViews"][bvIdx];
    int         bufIdx    = bv["buffer"].get<int>();
    size_t      bvOffset  = bv.value( "byteOffset", 0 );
    size_t      accOffset = acc.value( "byteOffset", 0 );

    // Compute stride
    size_t      componentSize = 4;    // default float
    switch( componentType )
    {
        case 5'120 :
        case 5'121 :
            componentSize = 1;
            break;
        case 5'122 :
        case 5'123 :
            componentSize = 2;
            break;
        case 5'125 :
        case 5'126 :
            componentSize = 4;
            break;
    }
    size_t defaultStride = componentSize * type;
    stride =
        bv.contains( "byteStride" ) ? bv["byteStride"].get<size_t>() : defaultStride;

    if( bufIdx >= static_cast<int>( _buffers.size() ) || _buffers[bufIdx].empty() )
    {
        return nullptr;
    }

    return _buffers[bufIdx].data() + bvOffset + accOffset;
}

osg::ref_ptr<osg::Geometry>
GLTFLoader::buildPrimitive( const nlohmann::json& primitive ) const
{
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setUseVertexBufferObjects( true );

    if( !primitive.contains( "attributes" ) )
    {
        return nullptr;
    }
    const auto& attrs = primitive["attributes"];

    // POSITION (required)
    if( attrs.contains( "POSITION" ) )
    {
        size_t         count;
        int            compType, type;
        size_t         stride;
        const uint8_t* data = getAccessorData( attrs["POSITION"].get<int>(),
                                               count,
                                               compType,
                                               type,
                                               stride );
        if( data && type == 3 && compType == 5'126 )
        {
            osg::ref_ptr<osg::Vec3Array> arr = new osg::Vec3Array( count );
            for( size_t i = 0; i < count; ++i )
            {
                ( *arr )[i] = *reinterpret_cast<const osg::vec3*>( data + i * stride );
            }
            geom->setVertexArray( arr.get() );
        }
    }

    // NORMAL
    if( attrs.contains( "NORMAL" ) )
    {
        size_t         count;
        int            compType, type;
        size_t         stride;
        const uint8_t* data =
            getAccessorData( attrs["NORMAL"].get<int>(), count, compType, type, stride );
        if( data && type == 3 && compType == 5'126 )
        {
            osg::ref_ptr<osg::Vec3Array> arr = new osg::Vec3Array( count );
            for( size_t i = 0; i < count; ++i )
            {
                ( *arr )[i] = *reinterpret_cast<const osg::vec3*>( data + i * stride );
            }
            geom->setNormalArray( arr.get(), osg::Array::BIND_PER_VERTEX );
        }
    }

    // TEXCOORD_0
    // glTF uses top-left origin (V=0 at top), OpenGL uses bottom-left (V=0 at bottom).
    // Flip V coordinate: v = 1.0 - v
    if( attrs.contains( "TEXCOORD_0" ) )
    {
        size_t         count;
        int            compType, type;
        size_t         stride;
        const uint8_t* data = getAccessorData( attrs["TEXCOORD_0"].get<int>(),
                                               count,
                                               compType,
                                               type,
                                               stride );
        if( data && type == 2 && compType == 5'126 )
        {
            osg::ref_ptr<osg::Vec2Array> arr = new osg::Vec2Array( count );
            for( size_t i = 0; i < count; ++i )
            {
                const osg::vec2& uv =
                    *reinterpret_cast<const osg::vec2*>( data + i * stride );
                ( *arr )[i] = osg::vec2( uv.x, 1.0F - uv.y );
            }
            geom->setTexCoordArray( 0, arr.get() );
        }
    }

    // COLOR_0
    if( attrs.contains( "COLOR_0" ) )
    {
        size_t         count;
        int            compType, type;
        size_t         stride;
        const uint8_t* data = getAccessorData( attrs["COLOR_0"].get<int>(),
                                               count,
                                               compType,
                                               type,
                                               stride );
        if( data && compType == 5'126 )
        {
            if( type == 4 )
            {
                osg::ref_ptr<osg::Vec4Array> arr = new osg::Vec4Array( count );
                for( size_t i = 0; i < count; ++i )
                {
                    ( *arr )[i] =
                        *reinterpret_cast<const osg::vec4*>( data + i * stride );
                }
                geom->setColorArray( arr.get(), osg::Array::BIND_PER_VERTEX );
            }
            else if( type == 3 )
            {
                osg::ref_ptr<osg::Vec4Array> arr = new osg::Vec4Array( count );
                for( size_t i = 0; i < count; ++i )
                {
                    const osg::vec3& v =
                        *reinterpret_cast<const osg::vec3*>( data + i * stride );
                    ( *arr )[i] = osg::vec4( v, 1.0F );
                }
                geom->setColorArray( arr.get(), osg::Array::BIND_PER_VERTEX );
            }
        }
    }

    // No COLOR_0: leave color array unset.
    // Geometry::drawImplementation sets osg_ColorMaterial=0 when no color array,
    // which makes the shader use osg_FrontMaterial.diffuse from the Material attribute.
    // The Material is set from baseColorFactor in loadMaterials().

    // Indices
    if( primitive.contains( "indices" ) )
    {
        size_t         count;
        int            compType, type;
        size_t         stride;
        const uint8_t* data = getAccessorData( primitive["indices"].get<int>(),
                                               count,
                                               compType,
                                               type,
                                               stride );
        if( data )
        {
            int    mode   = primitive.value( "mode", 4 );    // default GL_TRIANGLES
            GLenum glMode = GL_TRIANGLES;
            switch( mode )
            {
                case 0 :
                    glMode = GL_POINTS;
                    break;
                case 1 :
                    glMode = GL_LINES;
                    break;
                case 2 :
                    glMode = GL_LINE_LOOP;
                    break;
                case 3 :
                    glMode = GL_LINE_STRIP;
                    break;
                case 4 :
                    glMode = GL_TRIANGLES;
                    break;
                case 5 :
                    glMode = GL_TRIANGLE_STRIP;
                    break;
                case 6 :
                    glMode = GL_TRIANGLE_FAN;
                    break;
            }

            if( compType == 5'123 )
            {    // UNSIGNED_SHORT
                osg::ref_ptr<osg::DrawElementsUShort> de =
                    new osg::DrawElementsUShort( glMode, count );
                const uint16_t* idx = reinterpret_cast<const uint16_t*>( data );
                for( size_t i = 0; i < count; ++i )
                {
                    ( *de )[i] = idx[i];
                }
                geom->addPrimitiveSet( de.get() );
            }
            else if( compType == 5'125 )
            {    // UNSIGNED_INT
                osg::ref_ptr<osg::DrawElementsUInt> de =
                    new osg::DrawElementsUInt( glMode, count );
                const uint32_t* idx = reinterpret_cast<const uint32_t*>( data );
                for( size_t i = 0; i < count; ++i )
                {
                    ( *de )[i] = idx[i];
                }
                geom->addPrimitiveSet( de.get() );
            }
            else if( compType == 5'121 )
            {    // UNSIGNED_BYTE
                osg::ref_ptr<osg::DrawElementsUByte> de =
                    new osg::DrawElementsUByte( glMode, count );
                for( size_t i = 0; i < count; ++i )
                {
                    ( *de )[i] = data[i];
                }
                geom->addPrimitiveSet( de.get() );
            }
        }
    }
    else
    {
        // Non-indexed: use DrawArrays
        osg::Vec3Array* verts = dynamic_cast<osg::Vec3Array*>( geom->getVertexArray() );
        if( verts )
        {
            int    mode   = primitive.value( "mode", 4 );
            GLenum glMode = GL_TRIANGLES;
            switch( mode )
            {
                case 0 :
                    glMode = GL_POINTS;
                    break;
                case 1 :
                    glMode = GL_LINES;
                    break;
                case 4 :
                    glMode = GL_TRIANGLES;
                    break;
                case 5 :
                    glMode = GL_TRIANGLE_STRIP;
                    break;
            }
            geom->addPrimitiveSet( new osg::DrawArrays( glMode, 0, verts->size() ) );
        }
    }

    // Apply material
    if( primitive.contains( "material" ) )
    {
        int matIdx = primitive["material"].get<int>();
        if( matIdx <
            static_cast<int>( _materials.size() ) &&
            _materials[matIdx].valid() )
        {
            geom->setStateSet( _materials[matIdx] );
        }
    }

    return geom;
}

osg::ref_ptr<osg::Node>
GLTFLoader::buildNode( int nodeIdx )
{
    const auto&              node = _json["nodes"][nodeIdx];

    osg::ref_ptr<osg::Group> group;

    // Check for transform
    bool                     hasTransform = node.contains( "matrix" ) ||
                                            node.contains( "translation" ) ||
                                            node.contains( "rotation" ) ||
                                            node.contains( "scale" );

    if( hasTransform )
    {
        osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
        osg::dmat4                         mat;

        if( node.contains( "matrix" ) )
        {
            auto m = node["matrix"];
            // glTF uses column-major
            mat.set( m[0].get<double>(),
                     m[1].get<double>(),
                     m[2].get<double>(),
                     m[3].get<double>(),
                     m[4].get<double>(),
                     m[5].get<double>(),
                     m[6].get<double>(),
                     m[7].get<double>(),
                     m[8].get<double>(),
                     m[9].get<double>(),
                     m[10].get<double>(),
                     m[11].get<double>(),
                     m[12].get<double>(),
                     m[13].get<double>(),
                     m[14].get<double>(),
                     m[15].get<double>() );
        }
        else
        {
            // TRS
            osg::dvec3 T( 0, 0, 0 );
            osg::quat  R( 0, 0, 0, 1 );
            osg::dvec3 S( 1, 1, 1 );

            if( node.contains( "translation" ) )
            {
                auto t = node["translation"];
                T.set( t[0].get<double>(), t[1].get<double>(), t[2].get<double>() );
            }
            if( node.contains( "rotation" ) )
            {
                auto r = node["rotation"];
                // glTF: x, y, z, w
                R.set( r[0].get<double>(),
                       r[1].get<double>(),
                       r[2].get<double>(),
                       r[3].get<double>() );
            }
            if( node.contains( "scale" ) )
            {
                auto s = node["scale"];
                S.set( s[0].get<double>(), s[1].get<double>(), s[2].get<double>() );
            }

            // glTF spec: M = T * R * S (scale first, then rotate, then translate)
            mat = osg::translate( T ) * osg::rotate( R ) * osg::scale( S );
        }

        mt->setMatrix( mat );
        _nodeTransformMap[nodeIdx] = mt;
        group                      = mt;
    }
    else
    {
        group = new osg::Group;
    }

    // Set name
    if( node.contains( "name" ) )
    {
        group->setName( node["name"].get<std::string>() );
    }

    // Add mesh geometry
    if( node.contains( "mesh" ) )
    {
        int         meshIdx = node["mesh"].get<int>();
        const auto& mesh    = _json["meshes"][meshIdx];
        for( const auto& prim : mesh["primitives"] )
        {
            osg::ref_ptr<osg::Geometry> geom = buildPrimitive( prim );
            if( geom )
            {
                group->addChild( geom );
            }
        }
    }

    // Recurse into children
    if( node.contains( "children" ) )
    {
        for( int childIdx : node["children"] )
        {
            osg::ref_ptr<osg::Node> child = buildNode( childIdx );
            if( child )
            {
                group->addChild( child );
            }
        }
    }

    return group;
}

osg::ref_ptr<osg::Node>
GLTFLoader::buildScene()
{
    if( !_json.contains( "scenes" ) || _json["scenes"].empty() )
    {
        return nullptr;
    }

    // Use default scene or first scene
    int                      sceneIdx = _json.value( "scene", 0 );
    const auto&              scene    = _json["scenes"][sceneIdx];

    osg::ref_ptr<osg::Group> root     = new osg::Group;
    if( scene.contains( "name" ) )
    {
        root->setName( scene["name"].get<std::string>() );
    }

    if( scene.contains( "nodes" ) )
    {
        for( int nodeIdx : scene["nodes"] )
        {
            osg::ref_ptr<osg::Node> child = buildNode( nodeIdx );
            if( child )
            {
                root->addChild( child );
            }
        }
    }

    // If only one child, return it directly
    if( root->getNumChildren() == 1 )
    {
        return root->getChild( 0 );
    }

    return root;
}

void
GLTFLoader::loadAnimations()
{
    if( !_json.contains( "animations" ) || _json["animations"].empty() )
    {
        return;
    }

    // Only load the first animation
    const auto& anim = _json["animations"][0];
    if( !anim.contains( "channels" ) || !anim.contains( "samplers" ) )
    {
        return;
    }

    const auto& channels = anim["channels"];
    const auto& samplers = anim["samplers"];

    // Map from node index to AnimationPath (to merge multiple channels targeting the
    // same node)
    std::unordered_map<int, osg::ref_ptr<osg::AnimationPath>> nodeAnimPaths;

    for( const auto& channel : channels )
    {
        if( !channel.contains( "target" ) || !channel.contains( "sampler" ) )
        {
            continue;
        }

        const auto& target = channel["target"];
        if( !target.contains( "node" ) || !target.contains( "path" ) )
        {
            continue;
        }

        int         nodeIdx    = target["node"].get<int>();
        std::string path       = target["path"].get<std::string>();
        int         samplerIdx = channel["sampler"].get<int>();

        // Only handle translation, rotation, scale
        if( path != "translation" && path != "rotation" && path != "scale" )
        {
            continue;
        }

        // Check that we have a MatrixTransform for this node
        auto it = _nodeTransformMap.find( nodeIdx );
        if( it == _nodeTransformMap.end() )
        {
            OSG_WARN << "GLTFLoader: animation targets node " << nodeIdx
                     << " which has no MatrixTransform" << std::endl;
            continue;
        }

        const auto& sampler       = samplers[samplerIdx];
        std::string interpolation = sampler.value( "interpolation", "LINEAR" );

        // Skip CUBICSPLINE — only handle LINEAR and STEP
        if( interpolation == "CUBICSPLINE" )
        {
            OSG_INFO << "GLTFLoader: skipping CUBICSPLINE animation channel"
                     << std::endl;
            continue;
        }

        int            inputAccessor  = sampler["input"].get<int>();
        int            outputAccessor = sampler["output"].get<int>();

        // Read input (time keyframes)
        size_t         inputCount;
        int            inputCompType, inputType;
        size_t         inputStride;
        const uint8_t* inputData = getAccessorData( inputAccessor,
                                                    inputCount,
                                                    inputCompType,
                                                    inputType,
                                                    inputStride );
        if( !inputData || inputCompType != 5'126 || inputType != 1 )
        {
            OSG_WARN << "GLTFLoader: invalid animation input accessor" << std::endl;
            continue;
        }

        // Read output (values)
        size_t         outputCount;
        int            outputCompType, outputType;
        size_t         outputStride;
        const uint8_t* outputData = getAccessorData( outputAccessor,
                                                     outputCount,
                                                     outputCompType,
                                                     outputType,
                                                     outputStride );
        if( !outputData || outputCompType != 5'126 )
        {
            OSG_WARN << "GLTFLoader: invalid animation output accessor" << std::endl;
            continue;
        }

        // Validate output type matches path
        if( ( path == "translation" || path == "scale" ) && outputType != 3 )
        {
            continue;
        }
        if( path == "rotation" && outputType != 4 )
        {
            continue;
        }

        // Get or create AnimationPath for this node
        osg::ref_ptr<osg::AnimationPath>& animPath = nodeAnimPaths[nodeIdx];
        if( !animPath )
        {
            animPath = new osg::AnimationPath;
            animPath->setLoopMode( osg::AnimationPath::LOOP );
        }

        auto& tcpMap = animPath->getTimeControlPointMap();

        for( size_t i = 0; i < inputCount; ++i )
        {
            double time = static_cast<double>(
                *reinterpret_cast<const float*>( inputData + i * inputStride )
            );
            const float* vals =
                reinterpret_cast<const float*>( outputData + i * outputStride );

            // Get existing control point at this time, or create a new one
            auto                             cpIt = tcpMap.find( time );
            osg::AnimationPath::ControlPoint cp;
            if( cpIt != tcpMap.end() )
            {
                cp = cpIt->second;
            }

            if( path == "translation" )
            {
                cp.setPosition( osg::dvec3( vals[0], vals[1], vals[2] ) );
            }
            else if( path == "rotation" )
            {
                // glTF quaternion: (x, y, z, w) — same as osg::quat constructor order
                cp.setRotation( osg::quat( vals[0], vals[1], vals[2], vals[3] ) );
            }
            else if( path == "scale" )
            {
                cp.setScale( osg::dvec3( vals[0], vals[1], vals[2] ) );
            }

            animPath->insert( time, cp );
        }
    }

    // Attach AnimationPathCallbacks to the corresponding MatrixTransforms
    for( auto& [nodeIdx, animPath] : nodeAnimPaths )
    {
        auto it = _nodeTransformMap.find( nodeIdx );
        if( it == _nodeTransformMap.end() )
        {
            continue;
        }

        osg::MatrixTransform*                    mt = it->second.get();
        osg::ref_ptr<osg::AnimationPathCallback> apc =
            new osg::AnimationPathCallback( animPath.get() );
        mt->setUpdateCallback( apc.get() );

        OSG_INFO << "GLTFLoader: attached animation to node " << nodeIdx;
        if( !mt->getName().empty() )
        {
            OSG_INFO << " (\"" << mt->getName() << "\")";
        }
        OSG_INFO << " with " << animPath->getTimeControlPointMap().size() << " keyframes"
                 << std::endl;
    }
}
