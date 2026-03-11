/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Simple XML DOM parser for .osgx and configuration files.
 * Builds an in-memory tree of XmlNode elements.
 */
#pragma once

#include <osgDB/registry/Registry.hpp>

namespace osgDB
{

    // forward declare
    class XmlNode;

    /** read an Xml file, find the file in Options DataFilePathList.*/
    extern OSGDB_EXPORT XmlNode*
    readXmlFile( const std::string& filename,
                 const Options*     options );

    /** read an Xml file, find the file in osgDB::Registry's eaderWriter::Options
     * DataFilePathList.*/
    inline XmlNode*
    readXmlFile( const std::string& filename )
    {
        return readXmlFile( filename, osgDB::Registry::instance()->getOptions() );
    }

    /** read an Xml from from an istream.*/
    extern OSGDB_EXPORT XmlNode*
                        readXmlStream( std::istream& fin );

    extern OSGDB_EXPORT std::string
                        trimEnclosingSpaces( const std::string& str );

    /** XmlNode class for very basic reading and writing of xml files.*/
    class OSGDB_EXPORT XmlNode : public osg::Referenced
    {
        public:

            XmlNode();

            enum NodeType
            {
                UNASSIGNED,
                ATOM,
                NODE,
                GROUP,
                ROOT,
                COMMENT,
                INFORMATION,
            };

            typedef std::map<std::string, std::string> Properties;
            typedef std::vector<osg::ref_ptr<XmlNode>> Children;

            NodeType                                   type;
            std::string                                name;
            std::string                                contents;
            Properties                                 properties;
            Children                                   children;

            std::string
            getTrimmedContents() const
            {
                return trimEnclosingSpaces( contents );
            }

        public:

            class OSGDB_EXPORT ControlMap
            {
                public:

                    ControlMap();

                    typedef std::map<std::string, int> ControlToCharacterMap;
                    typedef std::map<int, std::string> CharacterToControlMap;

                    void
                    addControlToCharacter( const std::string& control,
                                           int                c );

                    ControlToCharacterMap _controlToCharacterMap;
                    CharacterToControlMap _characterToControlMap;

                private:

                    void
                    setUpControlMappings();
            };

            class OSGDB_EXPORT Input : public ControlMap
            {
                public:

                    Input();
                    Input( const Input& );

                    ~Input();

                    typedef std::string::size_type size_type;

                    void
                    open( const std::string& filename );
                    void
                    attach( std::istream& istream );

                    void
                    readAllDataIntoBuffer();

                    operator bool() const
                    {
                        return _currentPos < _buffer.size();
                    }

                    size_type
                    currentPosition() const
                    {
                        return _currentPos;
                    }

                    int
                    get()
                    {
                        if( _currentPos < _buffer.size() )
                        {
                            return static_cast<unsigned char>( _buffer[_currentPos++] );
                        }
                        else
                        {
                            return -1;
                        }
                    }

                    int
                    operator[]( size_type i ) const
                    {
                        if( ( _currentPos + i ) < _buffer.size() )
                        {
                            return static_cast<unsigned char>(
                                _buffer[_currentPos + i]
                            );
                        }
                        else
                        {
                            return -1;
                        }
                    }

                    void
                    operator++()
                    {
                        if( _currentPos < _buffer.size() )
                        {
                            ++_currentPos;
                        }
                    }

                    void
                    operator+=( size_type n )
                    {
                        if( ( _currentPos + n ) < _buffer.size() )
                        {
                            _currentPos += n;
                        }
                        else
                        {
                            _currentPos = _buffer.size();
                        }
                    }

                    void
                    skipWhiteSpace();

                    std::string
                    substr( size_type pos,
                            size_type n = std::string::npos )
                    {
                        return ( _currentPos < _buffer.size() )
                                 ? _buffer.substr( _currentPos + pos, n )
                                 : std::string();
                    }

                    size_type
                    find( const std::string& str )
                    {
                        if( _currentPos < _buffer.size() )
                        {
                            size_type pos = _buffer.find( str, _currentPos );
                            if( pos == std::string::npos )
                            {
                                return std::string::npos;
                            }
                            else
                            {
                                return pos - _currentPos;
                            }
                        }
                        else
                        {
                            return std::string::npos;
                        }
                    }

                    bool
                    match( const std::string& str )
                    {
                        return ( _currentPos < _buffer.size() )
                                 ? _buffer.compare( _currentPos, str.size(), str ) == 0
                                 : false;
                    }

                    enum Encoding
                    {
                        ENCODING_ASCII,
                        ENCODING_UTF8,
                    };

                    void
                    setEncoding( Encoding encoding )
                    {
                        _encoding = encoding;
                    }

                    Encoding
                    getEncoding() const
                    {
                        return _encoding;
                    }

                    inline void
                    copyCharacterToString( std::string& str )
                    {
                        if( _currentPos >= _buffer.size() )
                        {
                            return;
                        }
                        switch( _encoding )
                        {
                            case( ENCODING_UTF8 ) :
                                {
                                    int char0 = static_cast<unsigned char>(
                                        _buffer[_currentPos]
                                    );
                                    ++_currentPos;
                                    str.push_back( static_cast<char>( char0 ) );

                                    if( char0 < 0X80 || _currentPos >= _buffer.size() )
                                    {
                                        break;    // 1-byte character
                                    }

                                    str.push_back( _buffer[_currentPos] );
                                    ++_currentPos;
                                    if( char0 < 0XE0 || _currentPos < _buffer.size() )
                                    {
                                        break;    // 2-byte character
                                    }

                                    str.push_back( _buffer[_currentPos] );
                                    ++_currentPos;
                                    if( char0 < 0XF0 || _currentPos >= _buffer.size() )
                                    {
                                        break;    // 3-byte character
                                    }

                                    str.push_back( _buffer[_currentPos] );
                                    ++_currentPos;
                                    if( char0 < 0XF8 || _currentPos >= _buffer.size() )
                                    {
                                        break;    // 4-byte character
                                    }

                                    if( _currentPos >= _buffer.size() )
                                    {
                                        break;
                                    }
                                    str.push_back( _buffer[_currentPos] );
                                    ++_currentPos;    // 5-byte character?

                                    break;
                                }
                            case( ENCODING_ASCII ) :
                            default :
                                str.push_back( _buffer[_currentPos] );
                                ++_currentPos;
                                return;
                        }
                    }

                private:

                    size_type     _currentPos;

                    std::ifstream _fin;
                    std::string   _buffer;
                    Encoding      _encoding;
            };

            bool
            read( Input& input );
            bool
            write( std::ostream&      fout,
                   const std::string& indent = "" ) const;

            bool
            write( const ControlMap&  controlMap,
                   std::ostream&      fout,
                   const std::string& indent = "" ) const;
            bool
            writeString( const ControlMap&  controlMap,
                         std::ostream&      fout,
                         const std::string& str ) const;

        protected:

            bool
            writeChildren( const ControlMap&  controlMap,
                           std::ostream&      fout,
                           const std::string& indent ) const;
            bool
            writeProperties( const ControlMap& controlMap,
                             std::ostream&     fout ) const;

            bool
            readAndReplaceControl( std::string& in_contents,
                                   Input&       input ) const;
    };

}
