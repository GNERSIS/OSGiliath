/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Deserialization input stream. Reads osg objects from binary
 * or XML format with type resolution via the ObjectWrapper registry.
 */
// Written by Wang Rui, (C) 2010

#include <osg/core/Notify.hpp>
#include <osg/maths/Math.hpp>
#include <osg/textures/ImageSequence.hpp>
#include <osgDB/io/ConvertBase64.hpp>
#include <osgDB/io/FileNameUtils.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/io/WriteFile.hpp>
#include <osgDB/io/XmlParser.hpp>
#include <osgDB/serialization/ObjectWrapper.hpp>

using namespace osgDB;

static std::string s_lastSchema;

InputStream::InputStream( const osgDB::Options* options ) :
    _fileVersion( 0 ),
    _useSchemaData( false ),
    _forceReadingImage( false ),
    _dataDecompress( 0 )
{
    BEGIN_BRACKET.set( "{", +INDENT_VALUE );
    END_BRACKET.set( "}", -INDENT_VALUE );

    if( !options )
    {
        return;
    }
    _options = options;

    if( options->getPluginStringData( "ForceReadingImage" ) == "true" )
    {
        _forceReadingImage = true;
    }

    if( !options->getPluginStringData( "CustomDomains" ).empty() )
    {
        StringList domains, keyAndValue;
        split( options->getPluginStringData( "CustomDomains" ), domains, ';' );
        for( unsigned int i = 0; i < domains.size(); ++i )
        {
            split( domains[i], keyAndValue, ':' );
            if( keyAndValue.size() > 1 )
            {
                _domainVersionMap[keyAndValue.front()] =
                    atoi( keyAndValue.back().c_str() );
            }
        }
    }

    std::string schema;
    if( !options->getPluginStringData( "SchemaFile" ).empty() )
    {
        schema = options->getPluginStringData( "SchemaFile" );
        if( s_lastSchema != schema )
        {
            osgDB::ifstream schemaStream( schema.c_str(), std::ios::in );
            if( !schemaStream.fail() )
            {
                readSchema( schemaStream );
            }
            schemaStream.close();
            s_lastSchema = schema;
        }
    }

    if( schema.empty() )
    {
        resetSchema();
        s_lastSchema.clear();
    }

    // assign dummy object to used for reading field properties that will be discarded.
    _dummyReadObject = new osg::DummyObject;
}

InputStream::~InputStream()
{
    if( _dataDecompress )
    {
        delete _dataDecompress;
    }
}

int
InputStream::getFileVersion( const std::string& d ) const
{
    if( d.empty() )
    {
        return _fileVersion;
    }
    VersionMap::const_iterator itr = _domainVersionMap.find( d );
    return itr == _domainVersionMap.end() ? 0 : itr->second;
}

InputStream&
InputStream::operator>>( osg::bvec2& v )
{
    char x, y;
    *this >> x >> y;
    v.set( x, y );
    return *this;
}

InputStream&
InputStream::operator>>( osg::bvec3& v )
{
    char x, y, z;
    *this >> x >> y >> z;
    v.set( x, y, z );
    return *this;
}

InputStream&
InputStream::operator>>( osg::bvec4& v )
{
    char x, y, z, w;
    *this >> x >> y >> z >> w;
    v.set( x, y, z, w );
    return *this;
}

InputStream&
InputStream::operator>>( osg::ubvec2& v )
{
    unsigned char x, y;
    *this >> x >> y;
    v.set( x, y );
    return *this;
}

InputStream&
InputStream::operator>>( osg::ubvec3& v )
{
    unsigned char x, y, z;
    *this >> x >> y >> z;
    v.set( x, y, z );
    return *this;
}

InputStream&
InputStream::operator>>( osg::ubvec4& v )
{
    unsigned char r, g, b, a;
    *this >> r >> g >> b >> a;
    v.set( r, g, b, a );
    return *this;
}

InputStream&
InputStream::operator>>( osg::svec2& v )
{
    *this >> v.x >> v.y;
    return *this;
}

InputStream&
InputStream::operator>>( osg::svec3& v )
{
    *this >> v.x >> v.y >> v.z;
    return *this;
}

InputStream&
InputStream::operator>>( osg::svec4& v )
{
    *this >> v.x >> v.y >> v.z >> v.w;
    return *this;
}

InputStream&
InputStream::operator>>( osg::usvec2& v )
{
    *this >> v.x >> v.y;
    return *this;
}

InputStream&
InputStream::operator>>( osg::usvec3& v )
{
    *this >> v.x >> v.y >> v.z;
    return *this;
}

InputStream&
InputStream::operator>>( osg::usvec4& v )
{
    *this >> v.x >> v.y >> v.z >> v.w;
    return *this;
}

InputStream&
InputStream::operator>>( osg::ivec2& v )
{
    *this >> v.x >> v.y;
    return *this;
}

InputStream&
InputStream::operator>>( osg::ivec3& v )
{
    *this >> v.x >> v.y >> v.z;
    return *this;
}

InputStream&
InputStream::operator>>( osg::ivec4& v )
{
    *this >> v.x >> v.y >> v.z >> v.w;
    return *this;
}

InputStream&
InputStream::operator>>( osg::uivec2& v )
{
    *this >> v.x >> v.y;
    return *this;
}

InputStream&
InputStream::operator>>( osg::uivec3& v )
{
    *this >> v.x >> v.y >> v.z;
    return *this;
}

InputStream&
InputStream::operator>>( osg::uivec4& v )
{
    *this >> v.x >> v.y >> v.z >> v.w;
    return *this;
}

InputStream&
InputStream::operator>>( osg::vec2& v )
{
    *this >> v.x >> v.y;
    return *this;
}

InputStream&
InputStream::operator>>( osg::vec3& v )
{
    *this >> v.x >> v.y >> v.z;
    return *this;
}

InputStream&
InputStream::operator>>( osg::vec4& v )
{
    *this >> v.x >> v.y >> v.z >> v.w;
    return *this;
}

InputStream&
InputStream::operator>>( osg::dvec2& v )
{
    *this >> v.x >> v.y;
    return *this;
}

InputStream&
InputStream::operator>>( osg::dvec3& v )
{
    *this >> v.x >> v.y >> v.z;
    return *this;
}

InputStream&
InputStream::operator>>( osg::dvec4& v )
{
    *this >> v.x >> v.y >> v.z >> v.w;
    return *this;
}

InputStream&
InputStream::operator>>( osg::quat& q )
{
    double x, y, z, w;
    *this >> x >> y >> z >> w;
    q.set( static_cast<float>( x ),
           static_cast<float>( y ),
           static_cast<float>( z ),
           static_cast<float>( w ) );
    return *this;
}

InputStream&
InputStream::operator>>( osg::Plane& p )
{
    double p0, p1, p2, p3;
    *this >> p0 >> p1 >> p2 >> p3;
    p.set( p0, p1, p2, p3 );
    return *this;
}

#if 0
InputStream& InputStream::operator>>( osg::mat4& mat )
{
   ObjectProperty property("");
   *this >> property  >> BEGIN_BRACKET;

   if (property._name == "mat4")
   {
        // stream has same type as what we want to read so read directly
        for ( int r=0; r<4; ++r )
        {
            *this >> mat(r, 0) >> mat(r, 1) >> mat(r, 2) >> mat(r, 3);
        }
   }
   else if (property._name == "dmat4")
   {
        // stream has different type than what we want to read so read stream into
        // a temporary and then copy across to the final matrix
        double value;
        for ( int r=0; r<4; ++r )
        {
            for ( int c=0; c<4; ++c)
            {
                *this >> value;
                mat(r,c) = static_cast<float>(value);
            }
        }
   }

   *this >> END_BRACKET;
   return *this;
}

InputStream& InputStream::operator>>( osg::dmat4& mat )
{
   ObjectProperty property("");
   *this >> property  >> BEGIN_BRACKET;

   if (property._name == "mat4")
   {
        // stream has different type than what we want to read so read stream into
        // a temporary and then copy across to the final matrix
        float value;
        for ( int r=0; r<4; ++r )
        {
            for ( int c=0; c<4; ++c)
            {
                *this >> value;
                mat(r,c) = static_cast<float>(value);
            }
        }
   }
   else if (property._name == "dmat4")
   {
        // stream has same type as what we want to read so read directly
        for ( int r=0; r<4; ++r )
        {
            *this >> mat(r, 0) >> mat(r, 1) >> mat(r, 2) >> mat(r, 3);
        }
   }

   *this >> END_BRACKET;
   return *this;
}
#else
InputStream&
InputStream::operator>>( osg::mat4& mat )
{
    *this >> BEGIN_BRACKET;

    // stream has different type than what we want to read so read stream into
    // a temporary and then copy across to the final matrix
    double value;
    for( std::size_t r = 0; r < 4; ++r )
    {
        for( std::size_t c = 0; c < 4; ++c )
        {
            *this >> value;
            mat( r, c ) = static_cast<float>( value );
        }
    }

    *this >> END_BRACKET;
    return *this;
}

InputStream&
InputStream::operator>>( osg::dmat4& mat )
{
    *this >> BEGIN_BRACKET;

    for( std::size_t r = 0; r < 4; ++r )
    {
        *this >> mat( r, 0 ) >> mat( r, 1 ) >> mat( r, 2 ) >> mat( r, 3 );
    }

    *this >> END_BRACKET;
    return *this;
}
#endif

InputStream&
InputStream::operator>>( osg::box& bb )
{
    float p0, p1, p2, p3, p4, p5;
    *this >> p0 >> p1 >> p2 >> p3 >> p4 >> p5;
    bb = osg::box( p0, p1, p2, p3, p4, p5 );
    return *this;
}

InputStream&
InputStream::operator>>( osg::dbox& bb )
{
    double p0, p1, p2, p3, p4, p5;
    *this >> p0 >> p1 >> p2 >> p3 >> p4 >> p5;
    bb = osg::dbox( p0, p1, p2, p3, p4, p5 );
    return *this;
}

InputStream&
InputStream::operator>>( osg::sphere& bs )
{
    float p0, p1, p2, p3;
    *this >> p0 >> p1 >> p2 >> p3;
    bs.set( osg::vec3( p0, p1, p2 ), p3 );
    return *this;
}

InputStream&
InputStream::operator>>( osg::dsphere& bs )
{
    double p0, p1, p2, p3;
    *this >> p0 >> p1 >> p2 >> p3;
    bs.set( osg::dvec3( p0, p1, p2 ), p3 );
    return *this;
}

osg::ref_ptr<osg::Array>
InputStream::readArray()
{
    osg::ref_ptr<osg::Array> array = NULL;

    unsigned int             id    = 0;
    *this >> PROPERTY( "ArrayID" ) >> id;

    ArrayMap::iterator itr = _arrayMap.find( id );
    if( itr != _arrayMap.end() )
    {
        return itr->second.get();
    }

    DEF_MAPPEE( ArrayType, type );
    *this >> type;
    switch( type.get() )
    {
        case ID_BYTE_ARRAY :
            {
                osg::ByteArray* ba = new osg::ByteArray;
                readArrayImplementation( ba, 1, CHAR_SIZE );
                array = ba;
            }
            break;
        case ID_UBYTE_ARRAY :
            {
                osg::UByteArray* uba = new osg::UByteArray;
                readArrayImplementation( uba, 1, CHAR_SIZE );
                array = uba;
            }
            break;
        case ID_SHORT_ARRAY :
            {
                osg::ShortArray* sa = new osg::ShortArray;
                readArrayImplementation( sa, 1, SHORT_SIZE );
                array = sa;
            }
            break;
        case ID_USHORT_ARRAY :
            {
                osg::UShortArray* usa = new osg::UShortArray;
                readArrayImplementation( usa, 1, SHORT_SIZE );
                array = usa;
            }
            break;
        case ID_INT_ARRAY :
            {
                osg::IntArray* ia = new osg::IntArray;
                readArrayImplementation( ia, 1, INT_SIZE );
                array = ia;
            }
            break;
        case ID_UINT_ARRAY :
            {
                osg::UIntArray* uia = new osg::UIntArray;
                readArrayImplementation( uia, 1, INT_SIZE );
                array = uia;
            }
            break;
        case ID_FLOAT_ARRAY :
            {
                osg::FloatArray* fa = new osg::FloatArray;
                readArrayImplementation( fa, 1, FLOAT_SIZE );
                array = fa;
            }
            break;
        case ID_DOUBLE_ARRAY :
            {
                osg::DoubleArray* da = new osg::DoubleArray;
                readArrayImplementation( da, 1, DOUBLE_SIZE );
                array = da;
            }
            break;
        case ID_VEC2B_ARRAY :
            {
                osg::Vec2bArray* va = new osg::Vec2bArray;
                readArrayImplementation( va, 2, CHAR_SIZE );
                array = va;
            }
            break;
        case ID_VEC3B_ARRAY :
            {
                osg::Vec3bArray* va = new osg::Vec3bArray;
                readArrayImplementation( va, 3, CHAR_SIZE );
                array = va;
            }
            break;
        case ID_VEC4B_ARRAY :
            {
                osg::Vec4bArray* va = new osg::Vec4bArray;
                readArrayImplementation( va, 4, CHAR_SIZE );
                array = va;
            }
            break;
        case ID_VEC2UB_ARRAY :
            {
                osg::Vec2ubArray* va = new osg::Vec2ubArray;
                readArrayImplementation( va, 2, CHAR_SIZE );
                array = va;
            }
            break;
        case ID_VEC3UB_ARRAY :
            {
                osg::Vec3ubArray* va = new osg::Vec3ubArray;
                readArrayImplementation( va, 3, CHAR_SIZE );
                array = va;
            }
            break;
        case ID_VEC4UB_ARRAY :
            {
                osg::Vec4ubArray* va = new osg::Vec4ubArray;
                readArrayImplementation( va, 4, CHAR_SIZE );
                array = va;
            }
            break;
        case ID_VEC2S_ARRAY :
            {
                osg::Vec2sArray* va = new osg::Vec2sArray;
                readArrayImplementation( va, 2, SHORT_SIZE );
                array = va;
            }
            break;
        case ID_VEC3S_ARRAY :
            {
                osg::Vec3sArray* va = new osg::Vec3sArray;
                readArrayImplementation( va, 3, SHORT_SIZE );
                array = va;
            }
            break;
        case ID_VEC4S_ARRAY :
            {
                osg::Vec4sArray* va = new osg::Vec4sArray;
                readArrayImplementation( va, 4, SHORT_SIZE );
                array = va;
            }
            break;
        case ID_VEC2US_ARRAY :
            {
                osg::Vec2usArray* va = new osg::Vec2usArray;
                readArrayImplementation( va, 2, SHORT_SIZE );
                array = va;
            }
            break;
        case ID_VEC3US_ARRAY :
            {
                osg::Vec3usArray* va = new osg::Vec3usArray;
                readArrayImplementation( va, 3, SHORT_SIZE );
                array = va;
            }
            break;
        case ID_VEC4US_ARRAY :
            {
                osg::Vec4usArray* va = new osg::Vec4usArray;
                readArrayImplementation( va, 4, SHORT_SIZE );
                array = va;
            }
            break;
        case ID_VEC2_ARRAY :
            {
                osg::Vec2Array* va = new osg::Vec2Array;
                readArrayImplementation( va, 2, FLOAT_SIZE );
                array = va;
            }
            break;
        case ID_VEC3_ARRAY :
            {
                osg::Vec3Array* va = new osg::Vec3Array;
                readArrayImplementation( va, 3, FLOAT_SIZE );
                array = va;
            }
            break;
        case ID_VEC4_ARRAY :
            {
                osg::Vec4Array* va = new osg::Vec4Array;
                readArrayImplementation( va, 4, FLOAT_SIZE );
                array = va;
            }
            break;
        case ID_VEC2D_ARRAY :
            {
                osg::Vec2dArray* va = new osg::Vec2dArray;
                readArrayImplementation( va, 2, DOUBLE_SIZE );
                array = va;
            }
            break;
        case ID_VEC3D_ARRAY :
            {
                osg::Vec3dArray* va = new osg::Vec3dArray;
                readArrayImplementation( va, 3, DOUBLE_SIZE );
                array = va;
            }
            break;
        case ID_VEC4D_ARRAY :
            {
                osg::Vec4dArray* va = new osg::Vec4dArray;
                readArrayImplementation( va, 4, DOUBLE_SIZE );
                array = va;
            }
            break;

        case ID_VEC2I_ARRAY :
            {
                osg::Vec2iArray* va = new osg::Vec2iArray;
                readArrayImplementation( va, 2, INT_SIZE );
                array = va;
            }
            break;
        case ID_VEC3I_ARRAY :
            {
                osg::Vec3iArray* va = new osg::Vec3iArray;
                readArrayImplementation( va, 3, INT_SIZE );
                array = va;
            }
            break;
        case ID_VEC4I_ARRAY :
            {
                osg::Vec4iArray* va = new osg::Vec4iArray;
                readArrayImplementation( va, 4, INT_SIZE );
                array = va;
            }
            break;

        case ID_VEC2UI_ARRAY :
            {
                osg::Vec2uiArray* va = new osg::Vec2uiArray;
                readArrayImplementation( va, 2, INT_SIZE );
                array = va;
            }
            break;
        case ID_VEC3UI_ARRAY :
            {
                osg::Vec3uiArray* va = new osg::Vec3uiArray;
                readArrayImplementation( va, 3, INT_SIZE );
                array = va;
            }
            break;
        case ID_VEC4UI_ARRAY :
            {
                osg::Vec4uiArray* va = new osg::Vec4uiArray;
                readArrayImplementation( va, 4, INT_SIZE );
                array = va;
            }
            break;

        default :
            throwException( "InputStream::readArray(): Unsupported array type." );
    }

    if( getException() )
    {
        return NULL;
    }
    _arrayMap[id] = array;

    return array;
}

osg::ref_ptr<osg::PrimitiveSet>
InputStream::readPrimitiveSet()
{
    osg::ref_ptr<osg::PrimitiveSet> primitive = NULL;

    DEF_MAPPEE( PrimitiveType, type );
    DEF_MAPPEE( PrimitiveType, mode );
    unsigned int numInstances = 0U;
    *this >> type >> mode;
    if( _fileVersion > 96 )
    {
        *this >> numInstances;
    }

    switch( type.get() )
    {
        case ID_DRAWARRAYS :
            {
                int first = 0, count = 0;
                *this >> first >> count;
                osg::DrawArrays* da =
                    new osg::DrawArrays( static_cast<GLenum>( mode.get() ),
                                         first,
                                         count );
                primitive = da;
                primitive->setNumInstances( static_cast<int>( numInstances ) );
            }
            break;
        case ID_DRAWARRAY_LENGTH :
            {
                int          first = 0, value = 0;
                unsigned int size = 0;
                *this >> first >> size >> BEGIN_BRACKET;
                osg::DrawArrayLengths* dl =
                    new osg::DrawArrayLengths( static_cast<GLenum>( mode.get() ),
                                               first );
                for( unsigned int i = 0; i < size; ++i )
                {
                    *this >> value;
                    dl->push_back( value );
                }
                *this >> END_BRACKET;
                primitive = dl;
                primitive->setNumInstances( static_cast<int>( numInstances ) );
            }
            break;
        case ID_DRAWELEMENTS_UBYTE :
            {
                osg::DrawElementsUByte* de =
                    new osg::DrawElementsUByte( static_cast<GLenum>( mode.get() ) );
                unsigned int  size  = 0;
                unsigned char value = 0;
                *this >> size >> BEGIN_BRACKET;
                for( unsigned int i = 0; i < size; ++i )
                {
                    *this >> value;
                    de->push_back( value );
                }
                *this >> END_BRACKET;
                primitive = de;
                primitive->setNumInstances( static_cast<int>( numInstances ) );
            }
            break;
        case ID_DRAWELEMENTS_USHORT :
            {
                osg::DrawElementsUShort* de =
                    new osg::DrawElementsUShort( static_cast<GLenum>( mode.get() ) );
                unsigned int   size  = 0;
                unsigned short value = 0;
                *this >> size >> BEGIN_BRACKET;
                for( unsigned int i = 0; i < size; ++i )
                {
                    *this >> value;
                    de->push_back( value );
                }
                *this >> END_BRACKET;
                primitive = de;
                primitive->setNumInstances( static_cast<int>( numInstances ) );
            }
            break;
        case ID_DRAWELEMENTS_UINT :
            {
                osg::DrawElementsUInt* de =
                    new osg::DrawElementsUInt( static_cast<GLenum>( mode.get() ) );
                unsigned int size = 0, value = 0;
                *this >> size >> BEGIN_BRACKET;
                for( unsigned int i = 0; i < size; ++i )
                {
                    *this >> value;
                    de->push_back( value );
                }
                *this >> END_BRACKET;
                primitive = de;
                primitive->setNumInstances( static_cast<int>( numInstances ) );
            }
            break;
        default :
            throwException( "InputStream::readPrimitiveSet(): Unsupported array type." );
    }

    if( getException() )
    {
        return NULL;
    }
    return primitive;
}

osg::ref_ptr<osg::Image>
InputStream::readImage( bool readFromExternal )
{
    std::string className = "osg::Image";
    if( _fileVersion >
        94 )    // ClassName property is only supported in 3.1.4 and higher
    {
        *this >> PROPERTY( "ClassName" ) >> className;
    }

    unsigned int id = 0;
    *this >> PROPERTY( "UniqueID" ) >> id;
    if( getException() )
    {
        return NULL;
    }

    IdentifierMap::iterator itr = _identifierMap.find( id );
    if( itr != _identifierMap.end() )
    {
        return static_cast<osg::Image*>( itr->second.get() );
    }

    std::string name;
    int         writeHint, decision = IMAGE_EXTERNAL;
    *this >> PROPERTY( "FileName" );
    readWrappedString( name );
    *this >> PROPERTY( "WriteHint" ) >> writeHint >> decision;
    if( getException() )
    {
        return NULL;
    }

    osg::ref_ptr<osg::Image> image = NULL;
    switch( decision )
    {
        case IMAGE_INLINE_DATA :
            if( isBinary() )
            {
                // _origin, _s & _t & _r, _internalTextureFormat
                int origin, s, t, r, internalFormat;
                *this >> origin >> s >> t >> r >> internalFormat;

                // _pixelFormat, _dataType, _packing, _allocationMode
                int pixelFormat, dataType, packing, mode;
                *this >> pixelFormat >> dataType >> packing >> mode;

                // _data
                unsigned int size = 0;
                *this >> size;
                if( size )
                {
                    char* data = new char[size];
                    if( !data )
                    {
                        throwException( "InputStream::readImage() Out of memory." );
                    }

                    if( getException() )
                    {
                        delete[] data;
                        return NULL;
                    }

                    readCharArray( data, size );
                    image = new osg::Image;
                    image->setOrigin( ( osg::Image::Origin )origin );
                    image->setImage( s,
                                     t,
                                     r,
                                     internalFormat,
                                     static_cast<GLenum>( pixelFormat ),
                                     static_cast<GLenum>( dataType ),
                                     ( unsigned char* )data,
                                     osg::Image::USE_NEW_DELETE,
                                     packing );
                }

                // _mipmapData
                unsigned int               levelSize = readSize();
                osg::Image::MipmapDataType levels( levelSize );
                for( unsigned int i = 0; i < levelSize; ++i )
                {
                    *this >> levels[i];
                }
                if( image && levelSize > 0 )
                {
                    image->setMipmapLevels( levels );
                }
                readFromExternal = false;
            }
            else
            {    // ASCII
                // _origin, _s & _t & _r, _internalTextureFormat
                int origin, s, t, r, internalFormat;
                *this >> PROPERTY( "Origin" ) >> origin;
                *this >> PROPERTY( "Size" ) >> s >> t >> r;
                *this >> PROPERTY( "InternalTextureFormat" ) >> internalFormat;

                // _pixelFormat, _dataType, _packing, _allocationMode
                int pixelFormat, dataType, packing, mode;
                *this >> PROPERTY( "PixelFormat" ) >> pixelFormat;
                *this >> PROPERTY( "DataType" ) >> dataType;
                *this >> PROPERTY( "Packing" ) >> packing;
                *this >> PROPERTY( "AllocationMode" ) >> mode;

                *this >> PROPERTY( "Data" );
                unsigned int levelSize = readSize() - 1;
                *this >> BEGIN_BRACKET;

                // _data
                std::vector<std::string> encodedData;
                encodedData.resize( levelSize + 1 );
                readWrappedString( encodedData.at( 0 ) );

                // Read all mipmap levels and to also add them to char* data
                // _mipmapData
                osg::Image::MipmapDataType levels( levelSize );
                for( unsigned int i = 1; i <= levelSize; ++i )
                {
                    //*this >> levels[i];
                    readWrappedString( encodedData.at( i ) );
                }

                Base64decoder d;
                char*         data = d.decode( encodedData, levels );
                // remove last item as we do not need the actual size
                // of the image including all mipmaps
                levels.pop_back();

                *this >> END_BRACKET;

                if( !data )
                {
                    throwException( "InputStream::readImage() Decoding of stream "
                                    "failed. Out of memory." );
                }
                if( getException() )
                {
                    return NULL;
                }

                image = new osg::Image;
                image->setOrigin( ( osg::Image::Origin )origin );
                image->setImage( s,
                                 t,
                                 r,
                                 internalFormat,
                                 static_cast<GLenum>( pixelFormat ),
                                 static_cast<GLenum>( dataType ),
                                 ( unsigned char* )data,
                                 ( osg::Image::AllocationMode )mode,
                                 packing );

                // Level positions (size of mipmap data)
                // from actual size of mipmap data read before
                if( image && levelSize > 0 )
                {
                    image->setMipmapLevels( levels );
                }

                readFromExternal = false;
            }
            break;
        case IMAGE_INLINE_FILE :
            if( isBinary() )
            {
                unsigned int size = readSize();
                if( size > 0 )
                {
                    char* data = new char[size];
                    if( !data )
                    {
                        throwException( "InputStream::readImage(): Out of memory." );
                        if( getException() )
                        {
                            return NULL;
                        }
                    }
                    readCharArray( data, size );

                    std::string          ext = osgDB::getFileExtension( name );
                    osgDB::ReaderWriter* reader =
                        osgDB::Registry::instance()->getReaderWriterForExtension( ext );
                    if( reader )
                    {
                        std::stringstream inputStream;
                        inputStream.write( data, size );

                        osgDB::ReaderWriter::ReadResult rr =
                            reader->readImage( inputStream );
                        if( rr.validImage() )
                        {
                            image = rr.takeImage();
                        }
                        else
                        {
                            OSG_WARN
                                << "InputStream::readImage(): " << rr.statusMessage()
                                << std::endl;
                        }
                    }
                    else
                    {
                        OSG_WARN
                            << "InputStream::readImage(): Unable to find a plugin for "
                            << ext << std::endl;
                    }
                    delete[] data;
                }
                readFromExternal = false;
            }
            break;
        case IMAGE_EXTERNAL :
        case IMAGE_WRITE_OUT :
            break;
        default :
            break;
    }

    bool loadedFromCache = false;
    if( readFromExternal && !name.empty() )
    {
        ReaderWriter::ReadResult rr =
            Registry::instance()->readImage( name, getOptions() );
        if( rr.validImage() )
        {
            image           = rr.takeImage();
            loadedFromCache = rr.loadedFromCache();
        }
        else
        {
            if( !rr.success() )
            {
                OSG_WARN << "InputStream::readImage(): " << rr.statusMessage()
                         << ", filename: " << name << std::endl;
            }
        }

        if( !image && _forceReadingImage )
        {
            image = new osg::Image;
        }
    }

    if( loadedFromCache )
    {
        // we don't want to overwrite the properties of the image in the cache as this
        // could cause threading problems if the object is currently being used so we
        // read the properties from the file into a dummy object and discard the changes.
        osg::ref_ptr<osg::Object> temp_obj =
            readObjectFields( "osg::Object", id, _dummyReadObject.get() );
        _identifierMap[id] = image;
    }
    else
    {
        image = readObjectFieldsOfType<osg::Image>(
            "osg::Object",
            id,
            image.get()
        );    // leaves _identifierMap[id] pointing at DummyObject if image invalid
        if( image.valid() )
        {
            image->setFileName( name );
            image->setWriteHint( ( osg::Image::WriteHint )writeHint );
        }
        _identifierMap[id] =
            image;    // valid or invalid, don't leave this pointing at an
                      // osg::Dummyobject as it's used with a static_cast when recycled
    }
    return image;
}

osg::ref_ptr<osg::Object>
InputStream::readObject( osg::Object* existingObj )
{
    std::string  className;
    unsigned int id = 0;
    *this >> className;

    if( className == "NULL" )
    {
        return 0;
    }

    *this >> BEGIN_BRACKET >> PROPERTY( "UniqueID" ) >> id;
    if( getException() )
    {
        return 0;
    }

    IdentifierMap::iterator itr = _identifierMap.find( id );
    if( itr != _identifierMap.end() )
    {
        advanceToCurrentEndBracket();
        return itr->second;
    }

    osg::ref_ptr<osg::Object> obj = readObjectFields( className, id, existingObj );

    advanceToCurrentEndBracket();

    return obj;
}

osg::ref_ptr<osg::Object>
InputStream::readObjectFields( const std::string& className,
                               unsigned int       id,
                               osg::Object*       existingObj )
{
    ObjectWrapper* wrapper =
        Registry::instance()->getObjectWrapperManager()->findWrapper( className );
    if( !wrapper )
    {
        OSG_WARN << "InputStream::readObject(): Unsupported wrapper class " << className
                 << std::endl;
        return NULL;
    }
    int                       inputVersion = getFileVersion( wrapper->getDomain() );

    osg::ref_ptr<osg::Object> obj =
        existingObj ? existingObj : wrapper->createInstance();
    _identifierMap[id] = obj;
    if( obj.valid() )
    {
        const ObjectWrapper::RevisionAssociateList& associates =
            wrapper->getAssociates();
        for( ObjectWrapper::RevisionAssociateList::const_iterator itr =
                 associates.begin();
             itr != associates.end();
             ++itr )
        {
            if( itr->_firstVersion <= inputVersion && inputVersion <= itr->_lastVersion )
            {
                ObjectWrapper* assocWrapper =
                    Registry::instance()->getObjectWrapperManager()->findWrapper(
                        itr->_name
                    );
                if( !assocWrapper )
                {
                    OSG_WARN
                        << "InputStream::readObject(): Unsupported associated class "
                        << itr->_name << std::endl;
                    continue;
                }
                _fields.push_back( assocWrapper->getName() );
                assocWrapper->read( *this, *obj );
                if( getException() )
                {
                    return NULL;
                }

                _fields.pop_back();
            }
            else
            {
                /* OSG_INFO << "InputStream::readObject():"<<className<<" Ignoring
                   associated class due to version mismatch"
                          << itr->_name<<"["<<itr->_firstVersion <<","<<itr->_lastVersion
                   <<"]for version "<<inputVersion<< std::endl;*/
            }
        }
    }
    return obj;
}

void
InputStream::readSchema( std::istream& fin )
{
    // Read from external ascii stream
    std::string line;
    while( std::getline( fin, line ) )
    {
        if( line[0] == '#' )
        {
            continue;    // Comment
        }

        StringList keyAndValue;
        split( line, keyAndValue, '=' );
        if( keyAndValue.size() < 2 )
        {
            continue;
        }

        setWrapperSchema( osgDB::trimEnclosingSpaces( keyAndValue[0] ),
                          osgDB::trimEnclosingSpaces( keyAndValue[1] ) );
    }
}

InputStream::ReadType
InputStream::start( InputIterator* inIterator )
{
    _fields.clear();
    _fields.push_back( "Start" );

    ReadType type = READ_UNKNOWN;
    _in           = inIterator;
    if( !_in )
    {
        throwException( "InputStream: Null stream specified." );
    }
    if( getException() )
    {
        return type;
    }

    _in->setInputStream( this );

    // Check OSG header information
    unsigned int version = 0;
    if( isBinary() )
    {
        unsigned int typeValue;
        *this >> typeValue >> version;
        type = static_cast<ReadType>( typeValue );

        unsigned int attributes;
        *this >> attributes;
        if( attributes & 0X4 )
        {
            inIterator->setSupportBinaryBrackets( true );
        }
        if( attributes & 0X2 )
        {
            _useSchemaData = true;
        }

        // Record custom domains
        if( attributes & 0X1 )
        {
            unsigned int numDomains;
            *this >> numDomains;
            for( unsigned int i = 0; i < numDomains; ++i )
            {
                std::string domainName;
                *this >> domainName;
                int domainVersion;
                *this >> domainVersion;
                _domainVersionMap[domainName] = domainVersion;
            }
        }
    }
    if( !isBinary() )
    {
        std::string typeString;
        *this >> typeString;
        if( typeString == "Scene" )
        {
            type = READ_SCENE;
        }
        else if( typeString == "Image" )
        {
            type = READ_IMAGE;
        }
        else if( typeString == "Object" )
        {
            type = READ_OBJECT;
        }

        std::string osgName, osgVersion;
        *this >> PROPERTY( "#Version" ) >> version;
        *this >> PROPERTY( "#Generator" ) >> osgName >> osgVersion;

        while( matchString( "#CustomDomain" ) )
        {
            std::string domainName;
            *this >> domainName;
            int domainVersion;
            *this >> domainVersion;
            _domainVersionMap[domainName] = domainVersion;
        }
    }

    // Record file version for back-compatibility checking of wrappers
    _fileVersion = static_cast<int>( version );
    _fields.pop_back();
    return type;
}

void
InputStream::decompress()
{
    if( !isBinary() )
    {
        return;
    }
    _fields.clear();

    std::string compressorName;
    *this >> compressorName;
    if( compressorName != "0" )
    {
        std::string data;
        _fields.push_back( "Decompression" );

        BaseCompressor* compressor =
            Registry::instance()->getObjectWrapperManager()->findCompressor(
                compressorName
            );
        if( !compressor )
        {
            throwException(
                "InputStream: Failed to decompress stream, No such compressor."
            );
            return;
        }

        if( !compressor->decompress( *( _in->getStream() ), data ) )
        {
            throwException( "InputStream: Failed to decompress stream." );
        }
        if( getException() )
        {
            return;
        }

        _dataDecompress = new std::stringstream( data );
        _in->setStream( _dataDecompress );
        _fields.pop_back();
    }

    if( _useSchemaData )
    {
        _fields.push_back( "SchemaData" );
        std::string schemaSource;
        *this >> schemaSource;
        std::istringstream iss( schemaSource );
        readSchema( iss );
        _fields.pop_back();
    }
}

// PROTECTED METHODS

void
InputStream::setWrapperSchema( const std::string& name,
                               const std::string& properties )
{
    ObjectWrapper* wrapper =
        Registry::instance()->getObjectWrapperManager()->findWrapper( name );
    if( !wrapper )
    {
        OSG_WARN << "InputStream::setSchema(): Unsupported wrapper class " << name
                 << std::endl;
        return;
    }

    StringList              schema, methods, keyAndValue;
    ObjectWrapper::TypeList types;
    split( properties, schema );
    for( StringList::iterator itr = schema.begin(); itr != schema.end(); ++itr )
    {
        split( *itr, keyAndValue, ':' );
        if( keyAndValue.size() > 1 )
        {
            methods.push_back( keyAndValue.front() );
            types.push_back(
                static_cast<BaseSerializer::Type>( atoi( keyAndValue.back().c_str() ) )
            );
        }
        else
        {
            methods.push_back( *itr );
            types.push_back( BaseSerializer::RW_UNDEFINED );
        }
        keyAndValue.clear();
    }
    wrapper->readSchema( methods, types );
}

void
InputStream::resetSchema()
{
    const ObjectWrapperManager::WrapperMap& wrappers =
        Registry::instance()->getObjectWrapperManager()->getWrapperMap();
    for( ObjectWrapperManager::WrapperMap::const_iterator itr = wrappers.begin();
         itr != wrappers.end();
         ++itr )
    {
        ObjectWrapper* wrapper = itr->second.get();
        wrapper->resetSchema();
    }
}

template<typename T>
void
InputStream::readArrayImplementation( T*           a,
                                      unsigned int numComponentsPerElements,
                                      unsigned int componentSizeInBytes )
{
    int size = 0;
    *this >> size >> BEGIN_BRACKET;
    if( size )
    {
        a->resize( static_cast<std::size_t>( size ) );
        if( isBinary() )
        {
            readComponentArray( ( char* )&( ( *a )[0] ),
                                static_cast<unsigned int>( size ),
                                numComponentsPerElements,
                                componentSizeInBytes );
            checkStream();
        }
        else
        {
            for( int i = 0; i < size; ++i )
            {
                *this >> ( *a )[static_cast<std::size_t>( i )];
            }
        }
    }
    *this >> END_BRACKET;
}
