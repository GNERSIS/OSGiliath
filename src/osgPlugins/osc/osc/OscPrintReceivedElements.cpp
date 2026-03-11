/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osgPlugins: b, m.
 */
#include "OscPrintReceivedElements.hpp"

#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>

namespace osc
{

    std::ostream&
    operator<<( std::ostream&                  os,
                const ReceivedMessageArgument& arg )
    {
        switch( arg.TypeTag() )
        {
            case TRUE_TYPE_TAG :
                os << "bool:true";
                break;

            case FALSE_TYPE_TAG :
                os << "bool:false";
                break;

            case NIL_TYPE_TAG :
                os << "(Nil)";
                break;

            case INFINITUM_TYPE_TAG :
                os << "(Infinitum)";
                break;

            case INT32_TYPE_TAG :
                os << "int32:" << arg.AsInt32Unchecked();
                break;

            case FLOAT_TYPE_TAG :
                os << "float32:" << arg.AsFloatUnchecked();
                break;

            case CHAR_TYPE_TAG :
                {
                    char s[2] = { 0 };
                    s[0]      = arg.AsCharUnchecked();
                    os << "char:'" << s << "'";
                }
                break;

            case RGBA_COLOR_TYPE_TAG :
                {
                    uint32 color = arg.AsRgbaColorUnchecked();

                    os << "RGBA:0x" << std::hex << std::setfill( '0' ) << std::setw( 2 )
                       << ( int )( ( color >> 24 ) & 0XFF ) << std::setw( 2 )
                       << ( int )( ( color >> 16 ) & 0XFF ) << std::setw( 2 )
                       << ( int )( ( color >> 8 ) & 0XFF ) << std::setw( 2 )
                       << ( int )( color & 0XFF ) << std::setfill( ' ' );
                    os.unsetf( std::ios::basefield );
                }
                break;

            case MIDI_MESSAGE_TYPE_TAG :
                {
                    uint32 m = arg.AsMidiMessageUnchecked();
                    os << "midi (port, status, data1, data2):<<" << std::hex
                       << std::setfill( '0' ) << "0x" << std::setw( 2 )
                       << ( int )( ( m >> 24 ) & 0XFF ) << " 0x" << std::setw( 2 )
                       << ( int )( ( m >> 16 ) & 0XFF ) << " 0x" << std::setw( 2 )
                       << ( int )( ( m >> 8 ) & 0XFF ) << " 0x" << std::setw( 2 )
                       << ( int )( m & 0XFF ) << std::setfill( ' ' ) << ">>";
                    os.unsetf( std::ios::basefield );
                }
                break;

            case INT64_TYPE_TAG :
                os << "int64:" << arg.AsInt64Unchecked();
                break;

            case TIME_TAG_TYPE_TAG :
                {
                    os << "OSC-timetag:" << arg.AsTimeTagUnchecked();

                    std::time_t t = ( unsigned long )( arg.AsTimeTagUnchecked() >> 32 );

                    // strip trailing newline from string returned by ctime
                    const char* timeString = std::ctime( &t );
                    size_t      len        = strlen( timeString );
                    char*       s          = new char[len + 1];
                    strcpy( s, timeString );
                    if( len )
                    {
                        s[len - 1] = '\0';
                    }

                    os << " " << s;

                    delete[] s;
                }
                break;

            case DOUBLE_TYPE_TAG :
                os << "double:" << arg.AsDoubleUnchecked();
                break;

            case STRING_TYPE_TAG :
                os << "OSC-string:`" << arg.AsStringUnchecked() << "'";
                break;

            case SYMBOL_TYPE_TAG :
                os << "OSC-string (symbol):`" << arg.AsSymbolUnchecked() << "'";
                break;

            case BLOB_TYPE_TAG :
                {
                    unsigned long size;
                    const void*   data;
                    arg.AsBlobUnchecked( data, size );
                    os << "OSC-blob:<<" << std::hex << std::setfill( '0' );
                    unsigned char* p = ( unsigned char* )data;
                    for( unsigned long i = 0; i < size; ++i )
                    {
                        os << "0x" << std::setw( 2 ) << int( p[i] );
                        if( i != size - 1 )
                        {
                            os << ' ';
                        }
                    }
                    os.unsetf( std::ios::basefield );
                    os << ">>" << std::setfill( ' ' );
                }
                break;

            default :
                os << "unknown";
        }

        return os;
    }

    std::ostream&
    operator<<( std::ostream&          os,
                const ReceivedMessage& m )
    {
        os << "[";
        if( m.AddressPatternIsUInt32() )
        {
            os << m.AddressPatternAsUInt32();
        }
        else
        {
            os << m.AddressPattern();
        }

        bool first = true;
        for( ReceivedMessage::const_iterator i = m.ArgumentsBegin();
             i != m.ArgumentsEnd();
             ++i )
        {
            if( first )
            {
                os << " ";
                first = false;
            }
            else
            {
                os << ", ";
            }

            os << *i;
        }

        os << "]";

        return os;
    }

    std::ostream&
    operator<<( std::ostream&         os,
                const ReceivedBundle& rb )
    {
        static int indent = 0;

        for( int j = 0; j < indent; ++j )
        {
            os << "  ";
        }
        os << "{ ( ";
        if( rb.TimeTag() == 1 )
        {
            os << "immediate";
        }
        else
        {
            os << rb.TimeTag();
        }
        os << " )\n";

        ++indent;

        for( ReceivedBundle::const_iterator i = rb.ElementsBegin();
             i != rb.ElementsEnd();
             ++i )
        {
            if( i->IsBundle() )
            {
                ReceivedBundle b( *i );
                os << b << "\n";
            }
            else
            {
                ReceivedMessage m( *i );
                for( int j = 0; j < indent; ++j )
                {
                    os << "  ";
                }
                os << m << "\n";
            }
        }

        --indent;

        for( int j = 0; j < indent; ++j )
        {
            os << "  ";
        }
        os << "}";

        return os;
    }

    std::ostream&
    operator<<( std::ostream&         os,
                const ReceivedPacket& p )
    {
        if( p.IsBundle() )
        {
            ReceivedBundle b( p );
            os << b << "\n";
        }
        else
        {
            ReceivedMessage m( p );
            os << m << "\n";
        }

        return os;
    }

}    // namespace osc
