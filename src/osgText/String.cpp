#include <osgText/String.hpp>

#include <iterator>
#include <osg/core/Notify.hpp>
#include <osg/maths/Math.hpp>
#include <osgDB/io/ConvertUTF.hpp>

using namespace osgText;

////////////////////////////////////////////////////////////////////////
//
// helper class to make it safer to query std::string's for encoding.
//
struct look_ahead_iterator
{
        look_ahead_iterator( const std::string& string ) :
            _string( string ),
            _index( 0 ),
            _nullCharacter( 0 )
        {
        }

        look_ahead_iterator( const look_ahead_iterator& rhs ) = default;

        bool
        valid() const
        {
            return _index < _string.length();
        }

        look_ahead_iterator&
        operator++()
        {
            if( _index < _string.length() )
            {
                ++_index;
            }
            return *this;
        }

        look_ahead_iterator
        operator++( int )
        {
            look_ahead_iterator tmp( *this );
            if( _index < _string.length() )
            {
                ++_index;
            }
            return tmp;
        }

        look_ahead_iterator&
        operator+=( int offset )
        {
            if( _index < _string.length() )
            {
                _index = std::min( static_cast<unsigned int>(
                                       _index + static_cast<unsigned int>( offset )
                                   ),
                                   static_cast<unsigned int>( _string.length() ) );
            }
            return *this;
        }

        unsigned char
        operator*() const
        {
            if( _index < _string.length() )
            {
                return static_cast<unsigned char>( _string[_index] );
            }
            else
            {
                return _nullCharacter;
            }
        }

        unsigned char
        operator[]( unsigned int offset ) const
        {

            if( _index + offset < _string.length() )
            {
                return static_cast<unsigned char>( _string[_index + offset] );
            }
            else
            {
                return _nullCharacter;
            }
        }

        const std::string& _string;
        unsigned int       _index;
        unsigned char      _nullCharacter;

    protected:

        look_ahead_iterator&
        operator=( const look_ahead_iterator& )
        {
            return *this;
        }
};

String::Encoding
findEncoding( look_ahead_iterator& charString,
              String::Encoding     overrideEncoding )
{
    switch( charString[0] )
    {
        case 0XEF :    // 8-bit encoding
            {
                // 8-bit signature = EF BB BF
                if( ( charString[1] == 0XBB ) && ( charString[2] == 0XBF ) )
                {
                    charString += 3;
                    return String::ENCODING_UTF8;
                }
                break;
            }
        case 0XFE :    // big-endian 16-bit
            {
                // 16-bit signature = FE FF
                if( charString[1] == 0XFF )
                {
                    charString += 2;
                    return String::ENCODING_UTF16_BE;
                }
                break;
            }
        case 0XFF :    // little-endian
            {
                // 16-bit signature = FF FE
                // 32-bit signature = FF FE 00 00
                if( charString[1] == 0XFE )
                {
                    // NOTE: There is an a potential problem as a 16-bit empty string
                    // is identical to a 32-bit start signature
                    if( ( charString[2] == 0 ) &&
                        ( charString[3] == 0 ) &&
                        ( overrideEncoding != String::ENCODING_UTF16 ) )    // 32-bit
                    {
                        charString += 4;
                        return String::ENCODING_UTF32_LE;
                    }
                    else    // 16-bit
                    {
                        charString += 2;
                        return String::ENCODING_UTF16_LE;
                    }
                }
                break;
            }
        case 0X00 :    // 32-bit big-endian
            {
                // 32-bit signature = 00 00 FE FF
                if( ( charString[1] == 0X00 ) &&
                    ( charString[2] == 0XFE ) &&
                    ( charString[3] == 0XFF ) )
                {
                    charString += 4;
                    return String::ENCODING_UTF32_BE;
                }
                break;
            }
    }
    return String::ENCODING_ASCII;
}

unsigned int
getNextCharacter( look_ahead_iterator& charString,
                  String::Encoding     encoding )
{
    // For more info on unicode encodings see:
    // http://www-106.ibm.com/developerworks/unicode/library/u-encode.html
    switch( encoding )
    {
        case String::ENCODING_ASCII :
            {
                return *charString++;
            }
        case String::ENCODING_UTF8 :
            {
                int char0 = *charString++;
                if( char0 < 0X80 )    // 1-byte character
                {
                    return static_cast<unsigned int>( char0 );
                }
                int char1 = *charString++;
                if( char0 < 0XE0 )    // 2-byte character
                {
                    return static_cast<unsigned int>( ( ( char0 & 0X1F ) << 6 ) |
                                                      ( char1 & 0X3F ) );
                }
                int char2 = *charString++;
                if( char0 < 0XF0 )    // 3-byte character
                {
                    return static_cast<unsigned int>( ( ( char0 & 0XF ) << 12 ) |
                                                      ( ( char1 & 0X3F ) << 6 ) |
                                                      ( char2 & 0X3F ) );
                }
                int char3 = *charString++;
                if( char0 < 0XF8 )    // 4-byte character
                {
                    return static_cast<unsigned int>( ( ( char0 & 0X7 ) << 18 ) |
                                                      ( ( char1 & 0X3F ) << 12 ) |
                                                      ( ( char2 & 0X3F ) << 6 ) |
                                                      ( char3 & 0X3F ) );
                }
                break;
            }
        case String::ENCODING_UTF16_BE :
            {
                int char0 = *charString++;
                int char1 = *charString++;
                if( ( char0 <= 0XD7 ) || ( char0 >= 0XE0 ) )    // simple character
                {
                    return static_cast<unsigned int>( ( char0 << 8 ) | char1 );
                }
                else if( ( char0 >= 0XD8 ) &&
                         ( char0 <= 0XDB ) )    // using planes (this should get called
                                                // very rarely)
                {
                    int char2         = *charString++;
                    int char3         = *charString++;
                    int highSurrogate = ( char0 << 8 ) | char1;
                    int lowSurrogate  = ( char2 << 8 ) | char3;
                    if( ( char2 >= 0XDC ) &&
                        ( char2 <=
                          0XDF ) )    // only for the valid range of low surrogate
                    {
                        // This covers the range of all 17 unicode planes
                        return static_cast<unsigned int>( ( ( highSurrogate - 0XD8'00 ) *
                                                            0X4'00 ) +
                                                          ( lowSurrogate - 0XD8'00 ) +
                                                          0X1'00'00 );
                    }
                }
                break;
            }
        case String::ENCODING_UTF16_LE :
            {
                int char1 = *charString++;
                int char0 = *charString++;
                if( ( char0 <= 0XD7 ) || ( char0 >= 0XE0 ) )    // simple character
                {
                    return static_cast<unsigned int>( ( char0 << 8 ) | char1 );
                }
                else if( ( char0 >= 0XD8 ) &&
                         ( char0 <= 0XDB ) )    // using planes (this should get called
                                                // very rarely)
                {
                    int char3         = *charString++;
                    int char2         = *charString++;
                    int highSurrogate = ( char0 << 8 ) | char1;
                    int lowSurrogate  = ( char2 << 8 ) | char3;
                    if( ( char2 >= 0XDC ) &&
                        ( char2 <=
                          0XDF ) )    // only for the valid range of low surrogate
                    {
                        // This covers the range of all 17 unicode planes
                        return static_cast<unsigned int>( ( ( highSurrogate - 0XD8'00 ) *
                                                            0X4'00 ) +
                                                          ( lowSurrogate - 0XD8'00 ) +
                                                          0X1'00'00 );
                    }
                }
                break;
            }
        case String::ENCODING_UTF32_BE :
            {
                unsigned int character =
                    ( ( static_cast<unsigned int>( charString[0] ) << 24 ) |
                      ( static_cast<unsigned int>( charString[1] ) << 16 ) |
                      ( static_cast<unsigned int>( charString[2] ) << 8 ) |
                      charString[3] );
                charString += 4;
                if( character < 0X11'00'00U )
                {
                    // Character is constrained to the range set by the unicode standard
                    return character;
                }
                break;
            }
        case String::ENCODING_UTF32_LE :
            {
                unsigned int character =
                    ( ( static_cast<unsigned int>( charString[3] ) << 24 ) |
                      ( static_cast<unsigned int>( charString[2] ) << 16 ) |
                      ( static_cast<unsigned int>( charString[1] ) << 8 ) |
                      charString[0] );
                charString += 4;
                if( character < 0X11'00'00U )
                {
                    // Character is constrained to the range set by the unicode standard
                    return character;
                }
                break;
            }
        default :
            {
                // Should not reach this point unless the encoding is unhandled
                // ENCODING_UTF16, ENCODING_UTF32 and ENCODING_SIGNATURE should never
                // enter this method
                OSG_FATAL << "Error: Invalid string encoding" << std::endl;
                break;
            }
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////
//
// String implementation.
//

String::String( const String& str ) :
    vector_type( str )
{
}

String&
String::operator=( const String& str )
{
    if( &str == this )
    {
        return *this;
    }

    clear();
    std::copy( str.begin(), str.end(), std::back_inserter( *this ) );

    return *this;
}

void
String::set( const std::string& text )
{
    clear();
    for( std::string::const_iterator it = text.begin(); it != text.end(); ++it )
    {
        unsigned int charcode = ( unsigned char )*it;
        push_back( charcode );
    }
}

void
String::set( const wchar_t* text )
{
    clear();
    while( *text )
    {
        push_back( static_cast<unsigned int>( *text++ ) );
    }
}

void
String::set( const std::string& text,
             Encoding           encoding )
{
    if( encoding == ENCODING_CURRENT_CODE_PAGE )
    {
        set( osgDB::convertStringFromCurrentCodePageToUTF8( text ), ENCODING_UTF8 );
        return;
    }

    clear();

    look_ahead_iterator itr( text );

    if( ( encoding == ENCODING_SIGNATURE ) ||
        ( encoding == ENCODING_UTF16 ) ||
        ( encoding == ENCODING_UTF32 ) )
    {
        encoding = findEncoding( itr, encoding );
    }

    while( itr.valid() )
    {
        unsigned int c = getNextCharacter( itr, encoding );
        if( c )
        {
            push_back( c );
        }
    }
}

std::string
String::createUTF8EncodedString() const
{
    std::string utf8string;
    for( const_iterator itr = begin(); itr != end(); ++itr )
    {
        unsigned int currentChar = *itr;
        if( currentChar < 0X80 )
        {
            utf8string += ( char )currentChar;
        }
        else if( currentChar < 0X8'00 )
        {
            utf8string += ( char )( 0XC0 | ( currentChar >> 6 ) );
            utf8string += ( char )( 0X80 | ( currentChar & 0X3F ) );
        }
        else if( currentChar < 0X1'00'00 )
        {
            utf8string += ( char )( 0XE0 | ( currentChar >> 12 ) );
            utf8string += ( char )( 0X80 | ( ( currentChar >> 6 ) & 0X3F ) );
            utf8string += ( char )( 0X80 | ( currentChar & 0X3F ) );
        }
        else
        {
            utf8string += ( char )( 0XF0 | ( currentChar >> 18 ) );
            utf8string += ( char )( 0X80 | ( ( currentChar >> 12 ) & 0X3F ) );
            utf8string += ( char )( 0X80 | ( ( currentChar >> 6 ) & 0X3F ) );
            utf8string += ( char )( 0X80 | ( currentChar & 0X3F ) );
        }
    }
    return utf8string;
}
