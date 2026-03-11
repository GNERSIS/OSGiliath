/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Deserialization input stream. Reads osg objects from binary
 * or XML format with type resolution via the ObjectWrapper registry.
 */
// Written by Wang Rui, (C) 2010

#pragma once

#include <iostream>
#include <osg/core/Endian.hpp>
#include <osg/geometry/Array.hpp>
#include <osg/geometry/PrimitiveSet.hpp>
#include <osg/maths/box.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/maths/quat.hpp>
#include <osg/maths/sphere.hpp>
#include <osg/maths/vec2.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>
#include <osgDB/registry/Options.hpp>
#include <osgDB/registry/ReaderWriter.hpp>
#include <osgDB/serialization/StreamOperator.hpp>
#include <sstream>

namespace osgDB
{

    class InputException : public osg::Referenced
    {
        public:

            InputException( const std::vector<std::string>& fields,
                            const std::string&              err ) :
                _error( err )
            {
                for( unsigned int i = 0; i < fields.size(); ++i )
                {
                    _field += fields[i];
                    _field += " ";
                }
            }

            const std::string&
            getField() const
            {
                return _field;
            }

            const std::string&
            getError() const
            {
                return _error;
            }

        protected:

            std::string _field;
            std::string _error;
    };

    class OSGDB_EXPORT InputStream
    {
        public:

            typedef std::map<unsigned int, osg::ref_ptr<osg::Array>>  ArrayMap;
            typedef std::map<unsigned int, osg::ref_ptr<osg::Object>> IdentifierMap;

            enum ReadType
            {
                READ_UNKNOWN = 0,
                READ_SCENE,
                READ_IMAGE,
                READ_OBJECT,
            };

            InputStream( const osgDB::Options* options );
            virtual ~InputStream();

            void
            setFileVersion( const std::string& d,
                            int                v )
            {
                _domainVersionMap[d] = v;
            }

            int
            getFileVersion( const std::string& d = std::string() ) const;

            bool
            isBinary() const
            {
                return _in->isBinary();
            }

            const osgDB::Options*
            getOptions() const
            {
                return _options.get();
            }

            // Serialization related functions
            InputStream&
            operator>>( bool& b )
            {
                _in->readBool( b );
                checkStream();
                return *this;
            }

            InputStream&
            operator>>( char& c )
            {
                _in->readChar( c );
                checkStream();
                return *this;
            }

            InputStream&
            operator>>( signed char& c )
            {
                _in->readSChar( c );
                checkStream();
                return *this;
            }

            InputStream&
            operator>>( unsigned char& c )
            {
                _in->readUChar( c );
                checkStream();
                return *this;
            }

            InputStream&
            operator>>( short& s )
            {
                _in->readShort( s );
                checkStream();
                return *this;
            }

            InputStream&
            operator>>( unsigned short& s )
            {
                _in->readUShort( s );
                checkStream();
                return *this;
            }

            InputStream&
            operator>>( int& i )
            {
                _in->readInt( i );
                checkStream();
                return *this;
            }

            InputStream&
            operator>>( unsigned int& i )
            {
                _in->readUInt( i );
                checkStream();
                return *this;
            }

            InputStream&
            operator>>( long& l )
            {
                _in->readLong( l );
                checkStream();
                return *this;
            }

            InputStream&
            operator>>( unsigned long& l )
            {
                _in->readULong( l );
                checkStream();
                return *this;
            }

            InputStream&
            operator>>( float& f )
            {
                _in->readFloat( f );
                checkStream();
                return *this;
            }

            InputStream&
            operator>>( double& d )
            {
                _in->readDouble( d );
                checkStream();
                return *this;
            }

            InputStream&
            operator>>( std::string& s )
            {
                _in->readString( s );
                checkStream();
                return *this;
            }

            InputStream&
            operator>>( std::istream& ( *fn )( std::istream& ))
            {
                _in->readStream( fn );
                checkStream();
                return *this;
            }

            InputStream&
            operator>>( std::ios_base& ( *fn )( std::ios_base& ))
            {
                _in->readBase( fn );
                checkStream();
                return *this;
            }

            InputStream&
            operator>>( ObjectGLenum& value )
            {
                _in->readGLenum( value );
                checkStream();
                return *this;
            }

            InputStream&
            operator>>( ObjectProperty& prop )
            {
                _in->readProperty( prop );
                checkStream();
                return *this;
            }

            InputStream&
            operator>>( ObjectMark& mark )
            {
                _in->readMark( mark );
                checkStream();
                return *this;
            }

            InputStream&
            operator>>( osg::bvec2& v );
            InputStream&
            operator>>( osg::bvec3& v );
            InputStream&
            operator>>( osg::bvec4& v );
            InputStream&
            operator>>( osg::ubvec2& v );
            InputStream&
            operator>>( osg::ubvec3& v );
            InputStream&
            operator>>( osg::ubvec4& v );
            InputStream&
            operator>>( osg::svec2& v );
            InputStream&
            operator>>( osg::svec3& v );
            InputStream&
            operator>>( osg::svec4& v );
            InputStream&
            operator>>( osg::usvec2& v );
            InputStream&
            operator>>( osg::usvec3& v );
            InputStream&
            operator>>( osg::usvec4& v );
            InputStream&
            operator>>( osg::ivec2& v );
            InputStream&
            operator>>( osg::ivec3& v );
            InputStream&
            operator>>( osg::ivec4& v );
            InputStream&
            operator>>( osg::uivec2& v );
            InputStream&
            operator>>( osg::uivec3& v );
            InputStream&
            operator>>( osg::uivec4& v );
            InputStream&
            operator>>( osg::vec2& v );
            InputStream&
            operator>>( osg::vec3& v );
            InputStream&
            operator>>( osg::vec4& v );
            InputStream&
            operator>>( osg::dvec2& v );
            InputStream&
            operator>>( osg::dvec3& v );
            InputStream&
            operator>>( osg::dvec4& v );
            InputStream&
            operator>>( osg::quat& q );
            InputStream&
            operator>>( osg::Plane& p );
            InputStream&
            operator>>( osg::mat4& mat );
            InputStream&
            operator>>( osg::dmat4& mat );
            InputStream&
            operator>>( osg::box& bb );
            InputStream&
            operator>>( osg::dbox& bb );
            InputStream&
            operator>>( osg::sphere& bs );
            InputStream&
            operator>>( osg::dsphere& bs );

            InputStream&
            operator>>( osg::ref_ptr<osg::Image>& ptr )
            {
                ptr = readImage();
                return *this;
            }

            InputStream&
            operator>>( osg::ref_ptr<osg::Array>& ptr )
            {
                if( _fileVersion >= 112 )
                {
                    ptr = readObjectOfType<osg::Array>();
                }
                else
                {
                    ptr = readArray();
                }
                return *this;
            }

            InputStream&
            operator>>( osg::ref_ptr<osg::PrimitiveSet>& ptr )
            {
                if( _fileVersion >= 112 )
                {
                    ptr = readObjectOfType<osg::PrimitiveSet>();
                }
                else
                {
                    ptr = readPrimitiveSet();
                }
                return *this;
            }

            template<typename T>
            InputStream&
            operator>>( osg::ref_ptr<T>& ptr )
            {
                ptr = readObjectOfType<T>();
                return *this;
            }

            // Convenient methods for reading
            bool
            matchString( const std::string& str )
            {
                return _in->matchString( str );
            }

            void
            advanceToCurrentEndBracket()
            {
                _in->advanceToCurrentEndBracket();
            }

            void
            readWrappedString( std::string& str )
            {
                _in->readWrappedString( str );
                checkStream();
            }

            void
            readCharArray( char*        s,
                           unsigned int size )
            {
                _in->readCharArray( s, size );
            }

            void
            readComponentArray( char*        s,
                                unsigned int numElements,
                                unsigned int numComponentsPerElements,
                                unsigned int componentSizeInBytes )
            {
                _in->readComponentArray( s,
                                         numElements,
                                         numComponentsPerElements,
                                         componentSizeInBytes );
            }

            // readSize() use unsigned int for all sizes.
            unsigned int
            readSize()
            {
                unsigned int size;
                *this >> size;
                return size;
            }

            // Global reading functions
            osg::ref_ptr<osg::Array>
            readArray();
            osg::ref_ptr<osg::PrimitiveSet>
            readPrimitiveSet();
            osg::ref_ptr<osg::Image>
            readImage( bool readFromExternal = true );

            template<typename T>
            osg::ref_ptr<T>
            readObjectOfType()
            {
                osg::ref_ptr<osg::Object> obj = readObject();
                T*                        ptr = dynamic_cast<T*>( obj.get() );
                if( ptr )
                {
                    return ptr;
                }
                else
                {
                    return 0;
                }
            }

            osg::ref_ptr<osg::Object>
            readObject( osg::Object* existingObj = 0 );

            osg::ref_ptr<osg::Object>
            readObjectFields( const std::string& className,
                              unsigned int       id,
                              osg::Object*       existingObj = 0 );

            template<typename T>
            osg::ref_ptr<T>
            readObjectFieldsOfType( const std::string& className,
                                    unsigned int       id,
                                    osg::Object*       existingObj = 0 )
            {
                osg::ref_ptr<osg::Object> obj =
                    readObjectFields( className, id, existingObj );
                T* ptr = dynamic_cast<T*>( obj.get() );
                if( ptr )
                {
                    return ptr;
                }
                else
                {
                    return 0;
                }
            }

            /// set an input iterator, used directly when not using InputStream with a
            /// traditional file related stream.
            void
            setInputIterator( InputIterator* ii )
            {
                _in = ii;
            }

            /// start reading from InputStream treating it as a traditional file related
            /// stream, handles headers and versioning
            ReadType
            start( InputIterator* );

            void
            decompress();

            // Schema handlers
            void
            readSchema( std::istream& fin );
            void
            resetSchema();

            // Exception handlers
            inline void
            throwException( const std::string& msg );

            const InputException*
            getException() const
            {
                return _exception.get();
            }

            // Property & mask variables
            ObjectProperty PROPERTY;
            ObjectMark     BEGIN_BRACKET;
            ObjectMark     END_BRACKET;

        protected:

            inline void
            checkStream();
            void
            setWrapperSchema( const std::string& name,
                              const std::string& properties );

            template<typename T>
            void
                          readArrayImplementation( T*           a,
                                                   unsigned int numComponentsPerElements,
                                                   unsigned int componentSizeInBytes );

            ArrayMap      _arrayMap;
            IdentifierMap _identifierMap;

            typedef std::map<std::string, int> VersionMap;
            VersionMap                         _domainVersionMap;
            int                                _fileVersion;
            bool                               _useSchemaData;
            bool                               _forceReadingImage;
            std::vector<std::string>           _fields;
            osg::ref_ptr<InputIterator>        _in;
            osg::ref_ptr<InputException>       _exception;
            osg::ref_ptr<const osgDB::Options> _options;

            // object to used to read field properties that will be discarded.
            osg::ref_ptr<osg::Object>          _dummyReadObject;

            // store here to avoid a new and a leak in InputStream::decompress
            std::stringstream*                 _dataDecompress;
    };

    void
    InputStream::throwException( const std::string& msg )
    {
        _exception = new InputException( _fields, msg );
    }

    void
    InputStream::checkStream()
    {
        _in->checkStream();
        if( _in->isFailed() )
        {
            throwException( "InputStream: Failed to read from stream." );
        }
    }

}
