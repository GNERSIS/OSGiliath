/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osgPlugins: swap, readByte, readShort, readInt, _filename.
 */
#include "TXFFont.hpp"

#include <algorithm>
#include <iostream>
#include <osg/core/Notify.hpp>

#define FNT_BYTE_FORMAT   0
#define FNT_BITMAP_FORMAT 1

struct GlyphData
{
        unsigned short ch;
        unsigned char  width;
        unsigned char  height;
        signed char    x_off;
        signed char    y_off;
        signed char    advance;
        short          x;
        short          y;
};

static inline void
swap( unsigned short& x,
      bool            isSwapped )
{
    if( !isSwapped )
    {
        return;
    }
    x = ( ( x >> 8 ) & 0X00'FF ) | ( ( x << 8 ) & 0XFF'00 );
}

static inline void
swap( unsigned& x,
      bool      isSwapped )
{
    if( !isSwapped )
    {
        return;
    }
    x = ( ( x >> 24 ) & 0X00'00'00'FF ) |
        ( ( x >> 8 ) & 0X00'00'FF'00 ) |
        ( ( x << 8 ) & 0X00'FF'00'00 ) |
        ( ( x << 24 ) & 0XFF'00'00'00 );
}

static inline unsigned char
readByte( std::istream& stream )
{
    unsigned char x;
    stream.read( reinterpret_cast<std::istream::char_type*>( &x ), 1 );
    return x;
}

static inline unsigned short
readShort( std::istream& stream,
           bool          isSwapped )
{
    unsigned short x;
    stream.read( reinterpret_cast<std::istream::char_type*>( &x ), 2 );
    swap( x, isSwapped );
    return x;
}

static inline unsigned
readInt( std::istream& stream,
         bool          isSwapped )
{
    unsigned x;
    stream.read( reinterpret_cast<std::istream::char_type*>( &x ), 4 );
    swap( x, isSwapped );
    return x;
}

TXFFont::TXFFont( const std::string& filename ) :
    _filename( filename )
{
}

TXFFont::~TXFFont()
{
}

std::string
TXFFont::getFileName() const
{
    return _filename;
}

osgText::Glyph*
TXFFont::getGlyph( const osgText::FontResolution&,
                   unsigned int charcode )
{
    GlyphMap::iterator i = _chars.find( charcode );
    if( i != _chars.end() )
    {
        return i->second.get();
    }

    // ok, not available, we have an additional chance with some translations
    // That is to make the loader compatible with an other prominent one
    if( charcode >= 'A' && charcode <= 'Z' )
    {
        i = _chars.find( charcode - 'A' + 'a' );
        if( i != _chars.end() )
        {
            _chars[charcode] = i->second;
            return i->second.get();
        }
    }
    else if( charcode >= 'a' && charcode <= 'z' )
    {
        i = _chars.find( charcode - 'a' + 'A' );
        if( i != _chars.end() )
        {
            _chars[charcode] = i->second;
            return i->second.get();
        }
    }

    return 0;
}

bool
TXFFont::hasVertical() const
{
    return true;
}

osg::vec2
TXFFont::getKerning( const osgText::FontResolution&,
                     unsigned int,
                     unsigned int,
                     osgText::KerningType )
{
    return osg::vec2( 0, 0 );
}

bool
TXFFont::loadFont( std::istream& stream )
{
    unsigned char magic[4];
    stream.read( reinterpret_cast<std::istream::char_type*>( &magic ), 4 );

    if( magic[0] != 0XFF || magic[1] != 't' || magic[2] != 'x' || magic[3] != 'f' )
    {
        OSG_FATAL << "osgdb_txf: input file \"" << _filename
                  << "\" is not a texture font file!" << std::endl;
        return false;
    }

    // read endianness hint
    bool     isSwapped = 0X12'34'56'78U != readInt( stream, false );

    unsigned format    = readInt( stream, isSwapped );
    unsigned texwidth  = readInt( stream, isSwapped );
    unsigned texheight = readInt( stream, isSwapped );
    unsigned maxheight = readInt( stream, isSwapped );
    readInt( stream, isSwapped );
    unsigned                num_glyphs        = readInt( stream, isSwapped );

    unsigned                computedmaxheight = 0;

    unsigned                w                 = texwidth;
    unsigned                h                 = texheight;

    osgText::FontResolution fontResolution( maxheight, maxheight );

    std::vector<GlyphData>  glyphs;
    for( unsigned i = 0; i < num_glyphs; ++i )
    {
        GlyphData glyphData;
        glyphData.ch      = readShort( stream, isSwapped );
        glyphData.width   = readByte( stream );
        glyphData.height  = readByte( stream );
        glyphData.x_off   = readByte( stream );
        glyphData.y_off   = readByte( stream );
        glyphData.advance = readByte( stream );
        readByte( stream );
        glyphData.x       = readShort( stream, isSwapped );
        glyphData.y       = readShort( stream, isSwapped );

        computedmaxheight = std::max( computedmaxheight, ( unsigned )glyphData.height );

        glyphs.push_back( glyphData );
    }

    unsigned                 ntexels = w * h;
    osg::ref_ptr<osg::Image> image   = new osg::Image;
    image->allocateImage( w, h, 1, GL_RED, GL_UNSIGNED_BYTE );

    if( format == FNT_BYTE_FORMAT )
    {
        stream.read( reinterpret_cast<std::istream::char_type*>( image->data() ),
                     ntexels );
        if( !stream )
        {
            OSG_FATAL << "osgdb_txf: unexpected end of file in txf file \"" << _filename
                      << "\"!" << std::endl;
            return false;
        }
    }
    else if( format == FNT_BITMAP_FORMAT )
    {
        unsigned       stride    = ( w + 7 ) >> 3;
        unsigned char* texbitmap = new unsigned char[stride * h];
        stream.read( reinterpret_cast<std::istream::char_type*>( texbitmap ),
                     stride * h );
        if( !stream )
        {
            delete[] texbitmap;
            OSG_FATAL << "osgdb_txf: unexpected end of file in txf file \"" << _filename
                      << "\"!" << std::endl;
            return false;
        }

        for( unsigned i = 0; i < h; i++ )
        {
            for( unsigned j = 0; j < w; j++ )
            {
                if( texbitmap[i * stride + ( j >> 3 )] & ( 1 << ( j & 7 ) ) )
                {
                    *image->data( j, i ) = 255;
                }
                else
                {
                    *image->data( j, i ) = 0;
                }
            }
        }

        delete[] texbitmap;
    }
    else
    {
        OSG_FATAL << "osgdb_txf: unexpected txf file!" << std::endl;
        return false;
    }

    float coord_scale =
        ( computedmaxheight > 0 ) ? ( 1.0F / float( computedmaxheight ) ) : 1.0F;

    {
        // insert a trivial blank character
        osgText::Glyph* glyph  = new osgText::Glyph( _facade, ' ' );

        unsigned        width  = 1;
        unsigned        height = 1;

        glyph->allocateImage( width, height, 1, GL_RED, GL_UNSIGNED_BYTE );
        glyph->setInternalTextureFormat( GL_RED );

        for( unsigned k = 0; k < width; ++k )
        {
            for( unsigned l = 0; l < height; ++l )
            {
                *glyph->data( k, l ) = 0;
            }
        }

        glyph->setWidth( 0.0F );
        glyph->setHeight( 0.0F );
        glyph->setHorizontalAdvance( 0.5F );
        glyph->setHorizontalBearing( osg::vec2( 0.0F, 0.0F ) );
        glyph->setVerticalAdvance( 1.0F );
        glyph->setVerticalBearing( osg::vec2( -0.25F, 0.0F ) );
        _chars[' '] = glyph;
        addGlyph( fontResolution, ' ', glyph );
    }

    for( unsigned i = 0; i < glyphs.size(); ++i )
    {
        // have a special one for that
        if( glyphs[i].ch == ' ' )
        {
            continue;
        }

        // add the characters ...
        osgText::Glyph* glyph  = new osgText::Glyph( _facade, glyphs[i].ch );

        unsigned        width  = glyphs[i].width;
        unsigned        height = glyphs[i].height;

        glyph->allocateImage( width, height, 1, GL_RED, GL_UNSIGNED_BYTE );
        glyph->setInternalTextureFormat( GL_RED );

        for( unsigned k = 0; k < width; ++k )
        {
            for( unsigned l = 0; l < height; ++l )
            {
                *glyph->data( k, l ) = *image->data( glyphs[i].x + k, glyphs[i].y + l );
            }
        }

        glyph->setWidth( float( width ) * coord_scale );
        glyph->setHeight( float( height ) * coord_scale );

        // OSG_NOTICE<<"char="<<char(glyphs[i].ch)<<",
        // glyphs[i].x_off="<<int(glyphs[i].x_off)<<",
        // glyphs[i].y_off="<<int(glyphs[i].y_off)<<",
        // glyphs[i].advance="<<int(glyphs[i].advance)<<",
        // glyphs[i].width="<<int(glyphs[i].width)<<",
        // glyphs[i].height="<<int(glyphs[i].height)<<std::endl;

        glyph->setHorizontalAdvance( float( glyphs[i].advance ) * coord_scale );
        glyph->setHorizontalBearing( osg::vec2( float( glyphs[i].x_off ) * coord_scale,
                                                float( glyphs[i].y_off ) *
                                                    coord_scale ) );

        glyph->setVerticalAdvance( 1.0 );
        glyph->setVerticalBearing( osg::vec2(
            float( glyphs[i].x_off - 0.5F * float( glyphs[i].advance ) ) * coord_scale,
            float( glyphs[i].y_off ) * coord_scale
        ) );
        _chars[glyphs[i].ch] = glyph;

        addGlyph( fontResolution, glyphs[i].ch, glyph );
    }

    return true;
}
