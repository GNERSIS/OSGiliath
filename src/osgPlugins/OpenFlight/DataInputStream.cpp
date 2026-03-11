/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osgPlugins: color, vec, seekg, vforward.
 */
//
// OpenFlight� loader for OpenSceneGraph
//
//  Copyright (C) 2005-2007  Brede Johansen
//

#include "DataInputStream.hpp"

#include <osg/core/Endian.hpp>
#include <osg/core/Notify.hpp>

using namespace flt;

DataInputStream::DataInputStream( std::streambuf* sb ) :
    std::istream( sb )
{
    _byteswap = osg::getCpuByteOrder() == osg::LittleEndian;
}

int8
DataInputStream::readInt8( int8 def )
{
    int8 d;
    read( ( char* )&d, sizeof( int8 ) );

    if( !good() )
    {
        return def;
    }

    return d;
}

uint8
DataInputStream::readUInt8( uint8 def )
{
    uint8 d;
    read( ( char* )&d, sizeof( uint8 ) );

    if( !good() )
    {
        return def;
    }

    return d;
}

int16
DataInputStream::readInt16( int16 def )
{
    int16 d;
    read( ( char* )&d, sizeof( int16 ) );

    if( !good() )
    {
        return def;
    }

    if( _byteswap )
    {
        osg::swapBytes2( ( char* )&d );
    }

    return d;
}

uint16
DataInputStream::readUInt16( uint16 def )
{
    uint16 d;
    read( ( char* )&d, sizeof( uint16 ) );

    if( !good() )
    {
        return def;
    }

    if( _byteswap )
    {
        osg::swapBytes2( ( char* )&d );
    }

    return d;
}

int32
DataInputStream::readInt32( int32 def )
{
    int32 d;
    read( ( char* )&d, sizeof( int32 ) );

    if( !good() )
    {
        return def;
    }

    if( _byteswap )
    {
        osg::swapBytes4( ( char* )&d );
    }

    return d;
}

uint32
DataInputStream::readUInt32( uint32 def )
{
    uint32 d;
    read( ( char* )&d, sizeof( uint32 ) );

    if( !good() )
    {
        return def;
    }

    if( _byteswap )
    {
        osg::swapBytes4( ( char* )&d );
    }

    return d;
}

float32
DataInputStream::readFloat32( float32 def )
{
    float32 d;
    read( ( char* )&d, sizeof( float32 ) );

    if( !good() )
    {
        return def;
    }

    if( _byteswap )
    {
        osg::swapBytes4( ( char* )&d );
    }

    return d;
}

float64
DataInputStream::readFloat64( float64 def )
{
    float64 d;
    read( ( char* )&d, sizeof( float64 ) );

    if( !good() )
    {
        return def;
    }

    if( _byteswap )
    {
        osg::swapBytes8( ( char* )&d );
    }

    return d;
}

void
DataInputStream::readCharArray( char* data,
                                int   size )
{
    read( data, size );
}

std::string
DataInputStream::readString( int size )
{
    char* buf = new char[size + 1];
    read( buf, size );
    buf[size]       = '\0';
    std::string str = buf;
    delete[] buf;
    return str;
}

osg::vec4
DataInputStream::readColor32()
{
    uint8     alpha = readUInt8();
    uint8     blue  = readUInt8();
    uint8     green = readUInt8();
    uint8     red   = readUInt8();

    osg::vec4 color( ( float )red / 255,
                     ( float )green / 255,
                     ( float )blue / 255,
                     ( float )alpha / 255 );

    return color;
}

osg::vec2
DataInputStream::readVec2f()
{
    float32   x = readFloat32();
    float32   y = readFloat32();

    osg::vec2 vec( x, y );

    return vec;
}

osg::vec3
DataInputStream::readVec3f()
{
    float32   x = readFloat32();
    float32   y = readFloat32();
    float32   z = readFloat32();

    osg::vec3 vec( x, y, z );

    return vec;
}

osg::vec4
DataInputStream::readVec4f()
{
    float32   x = readFloat32();
    float32   y = readFloat32();
    float32   z = readFloat32();
    float32   w = readFloat32();

    osg::vec4 vec( x, y, z, w );

    return vec;
}

osg::dvec3
DataInputStream::readVec3d()
{
    float64    x = readFloat64();
    float64    y = readFloat64();
    float64    z = readFloat64();

    osg::dvec3 vec( x, y, z );

    return vec;
}

int16
DataInputStream::peekInt16()
{
    // Get current read position in stream.
    std::istream::pos_type pos   = tellg();

    int16                  value = readInt16();

    // Restore position
    seekg( pos, std::ios_base::beg );

    return value;
}

std::istream&
DataInputStream::forward( std::istream::off_type off )
{
    // return vforward(off);
    return seekg( off, std::ios_base::cur );
}

#if 0
std::istream& DataInputStream::vread(char_type *str, std::streamsize count)
{
    return read(str,count);
}


std::istream& DataInputStream::vforward(std::istream::off_type off)
{
    return seekg(off, std::ios_base::cur);
}
#endif
