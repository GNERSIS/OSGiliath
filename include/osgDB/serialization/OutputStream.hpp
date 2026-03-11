/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Serialization output stream. Writes osg objects to binary
 * or XML format using ObjectWrapper property mappings.
 */
// Written by Wang Rui, (C) 2010

#pragma once

#include <iostream>
#include <osg/geometry/Array.hpp>
#include <osg/geometry/PrimitiveSet.hpp>
#include <osg/maths/box.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/maths/quat.hpp>
#include <osg/maths/sphere.hpp>
#include <osg/maths/vec2.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>
#include <osg/Version>
#include <osgDB/registry/ReaderWriter.hpp>
#include <osgDB/serialization/StreamOperator.hpp>
#include <sstream>

namespace osgDB
{

    class OutputException : public osg::Referenced
    {
        public:

            OutputException( const std::vector<std::string>& fields,
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

    class OSGDB_EXPORT OutputStream
    {
        public:

            typedef std::map<const osg::Array*, unsigned int>  ArrayMap;
            typedef std::map<const osg::Object*, unsigned int> ObjectMap;

            enum WriteType
            {
                WRITE_UNKNOWN = 0,
                WRITE_SCENE,
                WRITE_IMAGE,
                WRITE_OBJECT,
            };

            enum WriteImageHint
            {
                WRITE_USE_IMAGE_HINT =
                    0, /*!< Use image hint, write inline data or use external */
                WRITE_USE_EXTERNAL,  /*!< Use external file on disk and write only the
                                        filename */
                WRITE_INLINE_DATA,   /*!< Write Image::data() to stream */
                WRITE_INLINE_FILE,   /*!< Write the image file itself to stream */
                WRITE_EXTERNAL_FILE, /*!< Write Image::data() to disk and use it as
                                        external file */
            };

            OutputStream( const osgDB::Options* options );
            virtual ~OutputStream();

            void
            setFileVersion( const std::string& d,
                            int                v );
            int
            getFileVersion( const std::string& d = std::string() ) const;

            bool
            isBinary() const
            {
                return _out->isBinary();
            }

            const std::string&
            getSchemaName() const
            {
                return _schemaName;
            }

            const osgDB::Options*
            getOptions() const
            {
                return _options.get();
            }

            void
            setWriteImageHint( WriteImageHint hint )
            {
                _writeImageHint = hint;
            }

            WriteImageHint
            getWriteImageHint() const
            {
                return _writeImageHint;
            }

            // Serialization related functions
            OutputStream&
            operator<<( bool b )
            {
                _out->writeBool( b );
                return *this;
            }

            OutputStream&
            operator<<( char c )
            {
                _out->writeChar( c );
                return *this;
            }

            OutputStream&
            operator<<( signed char c )
            {
                _out->writeChar( c );
                return *this;
            }

            OutputStream&
            operator<<( unsigned char c )
            {
                _out->writeUChar( c );
                return *this;
            }

            OutputStream&
            operator<<( short s )
            {
                _out->writeShort( s );
                return *this;
            }

            OutputStream&
            operator<<( unsigned short s )
            {
                _out->writeUShort( s );
                return *this;
            }

            OutputStream&
            operator<<( int i )
            {
                _out->writeInt( i );
                return *this;
            }

            OutputStream&
            operator<<( unsigned int i )
            {
                _out->writeUInt( i );
                return *this;
            }

            OutputStream&
            operator<<( long l )
            {
                _out->writeLong( l );
                return *this;
            }

            OutputStream&
            operator<<( unsigned long l )
            {
                _out->writeULong( l );
                return *this;
            }

            OutputStream&
            operator<<( float f )
            {
                _out->writeFloat( f );
                return *this;
            }

            OutputStream&
            operator<<( double d )
            {
                _out->writeDouble( d );
                return *this;
            }

            OutputStream&
            operator<<( long long ll )
            {
                _out->writeInt64( ll );
                return *this;
            }

            OutputStream&
            operator<<( unsigned long long ull )
            {
                _out->writeUInt64( ull );
                return *this;
            }

            OutputStream&
            operator<<( const std::string& s )
            {
                _out->writeString( s );
                return *this;
            }

            OutputStream&
            operator<<( const char* s )
            {
                _out->writeString( s );
                return *this;
            }

            OutputStream&
            operator<<( std::ostream& ( *fn )( std::ostream& ))
            {
                _out->writeStream( fn );
                return *this;
            }

            OutputStream&
            operator<<( std::ios_base& ( *fn )( std::ios_base& ))
            {
                _out->writeBase( fn );
                return *this;
            }

            OutputStream&
            operator<<( const ObjectGLenum& value )
            {
                _out->writeGLenum( value );
                return *this;
            }

            OutputStream&
            operator<<( const ObjectProperty& prop )
            {
                _out->writeProperty( prop );
                return *this;
            }

            OutputStream&
            operator<<( const ObjectMark& mark )
            {
                _out->writeMark( mark );
                return *this;
            }

            OutputStream&
            operator<<( const osg::bvec2& v );
            OutputStream&
            operator<<( const osg::bvec3& v );
            OutputStream&
            operator<<( const osg::bvec4& v );
            OutputStream&
            operator<<( const osg::ubvec2& v );
            OutputStream&
            operator<<( const osg::ubvec3& v );
            OutputStream&
            operator<<( const osg::ubvec4& v );
            OutputStream&
            operator<<( const osg::svec2& v );
            OutputStream&
            operator<<( const osg::svec3& v );
            OutputStream&
            operator<<( const osg::svec4& v );
            OutputStream&
            operator<<( const osg::usvec2& v );
            OutputStream&
            operator<<( const osg::usvec3& v );
            OutputStream&
            operator<<( const osg::usvec4& v );
            OutputStream&
            operator<<( const osg::ivec2& v );
            OutputStream&
            operator<<( const osg::ivec3& v );
            OutputStream&
            operator<<( const osg::ivec4& v );
            OutputStream&
            operator<<( const osg::uivec2& v );
            OutputStream&
            operator<<( const osg::uivec3& v );
            OutputStream&
            operator<<( const osg::uivec4& v );
            OutputStream&
            operator<<( const osg::vec2& v );
            OutputStream&
            operator<<( const osg::vec3& v );
            OutputStream&
            operator<<( const osg::vec4& v );
            OutputStream&
            operator<<( const osg::dvec2& v );
            OutputStream&
            operator<<( const osg::dvec3& v );
            OutputStream&
            operator<<( const osg::dvec4& v );
            OutputStream&
            operator<<( const osg::quat& q );
            OutputStream&
            operator<<( const osg::Plane& p );
            OutputStream&
            operator<<( const osg::mat4& mat );
            OutputStream&
            operator<<( const osg::dmat4& mat );
            OutputStream&
            operator<<( const osg::box& bb );
            OutputStream&
            operator<<( const osg::dbox& bb );
            OutputStream&
            operator<<( const osg::sphere& bb );
            OutputStream&
            operator<<( const osg::dsphere& bb );

            OutputStream&
            operator<<( const osg::Image* img )
            {
                writeImage( img );
                return *this;
            }

            OutputStream&
            operator<<( const osg::Array* a )
            {
                if( _targetFileVersion >= 112 )
                {
                    writeObject( a );
                }
                else
                {
                    writeArray( a );
                }
                return *this;
            }

            OutputStream&
            operator<<( const osg::PrimitiveSet* p )
            {
                if( _targetFileVersion >= 112 )
                {
                    writeObject( p );
                }
                else
                {
                    writePrimitiveSet( p );
                }
                return *this;
            }

            OutputStream&
            operator<<( const osg::Object* obj )
            {
                writeObject( obj );
                return *this;
            }

            OutputStream&
            operator<<( const osg::ref_ptr<osg::Image>& ptr )
            {
                writeImage( ptr.get() );
                return *this;
            }

            OutputStream&
            operator<<( const osg::ref_ptr<osg::Array>& ptr )
            {
                if( _targetFileVersion >= 112 )
                {
                    writeObject( ptr.get() );
                }
                else
                {
                    writeArray( ptr.get() );
                }
                return *this;
            }

            OutputStream&
            operator<<( const osg::ref_ptr<osg::PrimitiveSet>& ptr )
            {
                if( _targetFileVersion >= 112 )
                {
                    writeObject( ptr.get() );
                }
                else
                {
                    writePrimitiveSet( ptr.get() );
                }
                return *this;
            }

            template<typename T>
            OutputStream&
            operator<<( const osg::ref_ptr<T>& ptr )
            {
                writeObject( ptr.get() );
                return *this;
            }

            // Convenient methods for writing
            void
            writeWrappedString( const std::string& str )
            {
                _out->writeWrappedString( str );
            }

            void
            writeCharArray( const char*  s,
                            unsigned int size )
            {
                _out->writeCharArray( s, size );
            }

            // method for converting all data structure sizes to unsigned int to ensure
            // architecture portability.
            template<typename T>
            void
            writeSize( T size )
            {
                *this << static_cast<unsigned int>( size );
            }

            // Global writing functions
            void
            writeArray( const osg::Array* a );
            void
            writePrimitiveSet( const osg::PrimitiveSet* p );
            void
            writeImage( const osg::Image* img );
            void
            writeObject( const osg::Object* obj );
            void
            writeObjectFields( const osg::Object* obj );
            void
            writeObjectFields( const osg::Object* obj,
                               const std::string& compoundName );

            /// set an output iterator, used directly when not using OutputStream with a
            /// traditional file related stream.
            void
            setOutputIterator( OutputIterator* oi )
            {
                _out = oi;
            }

            /// start writing to OutputStream treating it as a traditional file related
            /// stream, handles headers and versioning
            void
            start( OutputIterator* outIterator,
                   WriteType       type );

            void
            compress( std::ostream* ostream );

            // Schema handlers
            void
            writeSchema( std::ostream& fout );

            // Exception handlers
            inline void
            throwException( const std::string& msg );

            const OutputException*
            getException() const
            {
                return _exception.get();
            }

            // Property & mask variables
            ObjectProperty PROPERTY;
            ObjectMark     BEGIN_BRACKET;
            ObjectMark     END_BRACKET;

        protected:

            template<typename T>
            void
            writeArrayImplementation( const T*,
                                      int          write_size,
                                      unsigned int numInRow = 1 );

            unsigned int
            findOrCreateArrayID( const osg::Array* array,
                                 bool&             newID );
            unsigned int
                      findOrCreateObjectID( const osg::Object* obj,
                                            bool&              newID );

            ArrayMap  _arrayMap;
            ObjectMap _objectMap;

            typedef std::map<std::string, int>         VersionMap;
            VersionMap                                 _domainVersionMap;
            WriteImageHint                             _writeImageHint;
            bool                                       _useSchemaData;
            bool                                       _useRobustBinaryFormat;

            typedef std::map<std::string, std::string> SchemaMap;
            SchemaMap                                  _inbuiltSchemaMap;
            std::vector<std::string>                   _fields;
            std::string                                _schemaName;
            std::string                                _compressorName;
            std::stringstream                          _compressSource;
            osg::ref_ptr<OutputIterator>               _out;
            osg::ref_ptr<OutputException>              _exception;
            osg::ref_ptr<const osgDB::Options>         _options;

            int                                        _targetFileVersion;
    };

    void
    OutputStream::throwException( const std::string& msg )
    {
        _exception = new OutputException( _fields, msg );
    }

}
