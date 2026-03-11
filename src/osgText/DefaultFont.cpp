/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osgText: fontRes, Glyph.
 */
#include "DefaultFont.hpp"

#include <osg/core/Notify.hpp>
#include <stdlib.h>

using namespace osgText;

DefaultFont::DefaultFont()
{
    _fontSize      = FontResolution( 8, 12 );

    _minFilterHint = osg::Texture::LINEAR_MIPMAP_LINEAR;
    _magFilterHint = osg::Texture::LINEAR;

    constructGlyphs();
}

DefaultFont::~DefaultFont()
{
}

osgText::Glyph*
DefaultFont::getGlyph( const FontResolution& fontRes,
                       unsigned int          charcode )
{
    if( _sizeGlyphMap.empty() )
    {
        return 0;
    }

    FontSizeGlyphMap::iterator itr = _sizeGlyphMap.find( fontRes );
    if( itr == _sizeGlyphMap.end() )
    {
        // no font found of correct size, will need to find the nearest.
        itr              = _sizeGlyphMap.begin();
        int mindeviation = abs( ( int )fontRes.first - ( int )itr->first.first ) +
                           abs( ( int )fontRes.second - ( int )itr->first.second );
        FontSizeGlyphMap::iterator sitr = itr;
        ++sitr;
        for( ; sitr != _sizeGlyphMap.end(); ++sitr )
        {
            int deviation = abs( ( int )fontRes.first - ( int )sitr->first.first ) +
                            abs( ( int )fontRes.second - ( int )sitr->first.second );
            if( deviation < mindeviation )
            {
                mindeviation = deviation;
                itr          = sitr;
            }
        }
    }

    // new find the glyph for the required charcode.
    GlyphMap&          glyphmap = itr->second;
    GlyphMap::iterator gitr     = glyphmap.find( charcode );

    if( gitr != glyphmap.end() )
    {
        return gitr->second.get();
    }
    else
    {
        return 0;
    }
}

osg::vec2
DefaultFont::getKerning( const osgText::FontResolution&,
                         unsigned int,
                         unsigned int,
                         KerningType )
{
    // no kerning on default font.
    return osg::vec2( 0.0F, 0.0F );
}

bool
DefaultFont::hasVertical() const
{
    return true;
}

void
DefaultFont::constructGlyphs()
{
    static GLubyte rasters[][12] = {
        // ascii symbols 32-127, small font
        {0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00},
        {0X00, 0X00, 0X08, 0X00, 0X08, 0X08, 0X08, 0X08, 0X08, 0X08, 0X08, 0X00},
        {0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X14, 0X14, 0X14, 0X00},
        {0X00, 0X00, 0X28, 0X28, 0X7E, 0X14, 0X14, 0X14, 0X3F, 0X0A, 0X0A, 0X00},
        {0X00, 0X00, 0X08, 0X1C, 0X22, 0X02, 0X1C, 0X20, 0X22, 0X1C, 0X08, 0X00},
        {0X00, 0X00, 0X02, 0X45, 0X22, 0X10, 0X08, 0X04, 0X22, 0X51, 0X20, 0X00},
        {0X00, 0X00, 0X3B, 0X44, 0X4A, 0X49, 0X30, 0X10, 0X20, 0X20, 0X18, 0X00},
        {0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X08, 0X08, 0X08, 0X00},
        {0X04, 0X08, 0X08, 0X10, 0X10, 0X10, 0X10, 0X10, 0X08, 0X08, 0X04, 0X00},
        {0X10, 0X08, 0X08, 0X04, 0X04, 0X04, 0X04, 0X04, 0X08, 0X08, 0X10, 0X00},
        {0X00, 0X00, 0X00, 0X00, 0X36, 0X1C, 0X7F, 0X1C, 0X36, 0X00, 0X00, 0X00},
        {0X00, 0X00, 0X08, 0X08, 0X08, 0X7F, 0X08, 0X08, 0X08, 0X00, 0X00, 0X00},
        {0X00, 0X10, 0X08, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00},
        {0X00, 0X00, 0X00, 0X00, 0X00, 0X7F, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00},
        {0X00, 0X00, 0X08, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00},
        {0X00, 0X00, 0X00, 0X40, 0X20, 0X10, 0X08, 0X04, 0X02, 0X01, 0X00, 0X00},
        {0X00, 0X00, 0X1C, 0X22, 0X22, 0X22, 0X22, 0X22, 0X22, 0X22, 0X1C, 0X00},
        {0X00, 0X00, 0X3E, 0X08, 0X08, 0X08, 0X08, 0X08, 0X08, 0X38, 0X08, 0X00},
        {0X00, 0X00, 0X3E, 0X20, 0X10, 0X08, 0X04, 0X02, 0X02, 0X22, 0X1C, 0X00},
        {0X00, 0X00, 0X1C, 0X22, 0X02, 0X02, 0X0C, 0X02, 0X02, 0X22, 0X1C, 0X00},
        {0X00, 0X00, 0X0E, 0X04, 0X3E, 0X24, 0X14, 0X14, 0X0C, 0X0C, 0X04, 0X00},
        {0X00, 0X00, 0X1C, 0X22, 0X02, 0X02, 0X3C, 0X20, 0X20, 0X20, 0X3E, 0X00},
        {0X00, 0X00, 0X1C, 0X22, 0X22, 0X22, 0X3C, 0X20, 0X20, 0X10, 0X0C, 0X00},
        {0X00, 0X00, 0X10, 0X10, 0X08, 0X08, 0X04, 0X04, 0X02, 0X22, 0X3E, 0X00},
        {0X00, 0X00, 0X1C, 0X22, 0X22, 0X22, 0X1C, 0X22, 0X22, 0X22, 0X1C, 0X00},
        {0X00, 0X00, 0X18, 0X04, 0X02, 0X02, 0X1E, 0X22, 0X22, 0X22, 0X1C, 0X00},
        {0X00, 0X00, 0X08, 0X00, 0X00, 0X00, 0X00, 0X08, 0X00, 0X00, 0X00, 0X00},
        {0X00, 0X10, 0X08, 0X00, 0X00, 0X00, 0X00, 0X08, 0X00, 0X00, 0X00, 0X00},
        {0X00, 0X00, 0X04, 0X08, 0X10, 0X20, 0X10, 0X08, 0X04, 0X00, 0X00, 0X00},
        {0X00, 0X00, 0X00, 0X00, 0X00, 0X3E, 0X00, 0X3E, 0X00, 0X00, 0X00, 0X00},
        {0X00, 0X00, 0X10, 0X08, 0X04, 0X02, 0X04, 0X08, 0X10, 0X00, 0X00, 0X00},
        {0X00, 0X00, 0X08, 0X00, 0X08, 0X08, 0X04, 0X02, 0X02, 0X22, 0X1C, 0X00},
        {0X00, 0X00, 0X1C, 0X20, 0X4E, 0X55, 0X55, 0X55, 0X4D, 0X21, 0X1E, 0X00},
        {0X00, 0X00, 0X77, 0X22, 0X3E, 0X22, 0X14, 0X14, 0X08, 0X08, 0X18, 0X00},
        {0X00, 0X00, 0X7E, 0X21, 0X21, 0X21, 0X3E, 0X21, 0X21, 0X21, 0X7E, 0X00},
        {0X00, 0X00, 0X1E, 0X21, 0X40, 0X40, 0X40, 0X40, 0X40, 0X21, 0X1E, 0X00},
        {0X00, 0X00, 0X7C, 0X22, 0X21, 0X21, 0X21, 0X21, 0X21, 0X22, 0X7C, 0X00},
        {0X00, 0X00, 0X7F, 0X21, 0X20, 0X24, 0X3C, 0X24, 0X20, 0X21, 0X7F, 0X00},
        {0X00, 0X00, 0X78, 0X20, 0X20, 0X24, 0X3C, 0X24, 0X20, 0X21, 0X7F, 0X00},
        {0X00, 0X00, 0X1E, 0X21, 0X41, 0X47, 0X40, 0X40, 0X40, 0X21, 0X1E, 0X00},
        {0X00, 0X00, 0X77, 0X22, 0X22, 0X22, 0X3E, 0X22, 0X22, 0X22, 0X77, 0X00},
        {0X00, 0X00, 0X3E, 0X08, 0X08, 0X08, 0X08, 0X08, 0X08, 0X08, 0X3E, 0X00},
        {0X00, 0X00, 0X38, 0X44, 0X44, 0X04, 0X04, 0X04, 0X04, 0X04, 0X1E, 0X00},
        {0X00, 0X00, 0X73, 0X22, 0X24, 0X38, 0X28, 0X24, 0X24, 0X22, 0X73, 0X00},
        {0X00, 0X00, 0X7F, 0X11, 0X10, 0X10, 0X10, 0X10, 0X10, 0X10, 0X7C, 0X00},
        {0X00, 0X00, 0X77, 0X22, 0X22, 0X2A, 0X2A, 0X36, 0X36, 0X22, 0X63, 0X00},
        {0X00, 0X00, 0X72, 0X22, 0X26, 0X26, 0X2A, 0X32, 0X32, 0X22, 0X67, 0X00},
        {0X00, 0X00, 0X1C, 0X22, 0X41, 0X41, 0X41, 0X41, 0X41, 0X22, 0X1C, 0X00},
        {0X00, 0X00, 0X78, 0X20, 0X20, 0X20, 0X3E, 0X21, 0X21, 0X21, 0X7E, 0X00},
        {0X00, 0X1B, 0X1C, 0X22, 0X41, 0X41, 0X41, 0X41, 0X41, 0X22, 0X1C, 0X00},
        {0X00, 0X00, 0X73, 0X22, 0X24, 0X24, 0X3E, 0X21, 0X21, 0X21, 0X7E, 0X00},
        {0X00, 0X00, 0X3E, 0X41, 0X01, 0X01, 0X3E, 0X40, 0X40, 0X41, 0X3E, 0X00},
        {0X00, 0X00, 0X1C, 0X08, 0X08, 0X08, 0X08, 0X08, 0X08, 0X49, 0X7F, 0X00},
        {0X00, 0X00, 0X1C, 0X22, 0X22, 0X22, 0X22, 0X22, 0X22, 0X22, 0X77, 0X00},
        {0X00, 0X00, 0X08, 0X08, 0X14, 0X14, 0X14, 0X22, 0X22, 0X22, 0X77, 0X00},
        {0X00, 0X00, 0X14, 0X14, 0X2A, 0X2A, 0X2A, 0X22, 0X22, 0X22, 0X77, 0X00},
        {0X00, 0X00, 0X77, 0X22, 0X14, 0X14, 0X08, 0X14, 0X14, 0X22, 0X77, 0X00},
        {0X00, 0X00, 0X1C, 0X08, 0X08, 0X08, 0X14, 0X14, 0X22, 0X22, 0X77, 0X00},
        {0X00, 0X00, 0X7F, 0X21, 0X10, 0X10, 0X08, 0X04, 0X04, 0X42, 0X7F, 0X00},
        {0X1C, 0X10, 0X10, 0X10, 0X10, 0X10, 0X10, 0X10, 0X10, 0X10, 0X1C, 0X00},
        {0X00, 0X00, 0X00, 0X01, 0X02, 0X04, 0X08, 0X10, 0X20, 0X40, 0X00, 0X00},
        {0X1C, 0X04, 0X04, 0X04, 0X04, 0X04, 0X04, 0X04, 0X04, 0X04, 0X1C, 0X00},
        {0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X22, 0X14, 0X08},
        {0XFF, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00},
        {0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X08, 0X10, 0X00},
        {0X00, 0X00, 0X3D, 0X42, 0X42, 0X3E, 0X02, 0X3C, 0X00, 0X00, 0X00, 0X00},
        {0X00, 0X00, 0X7E, 0X21, 0X21, 0X21, 0X21, 0X3E, 0X20, 0X20, 0X60, 0X00},
        {0X00, 0X00, 0X3E, 0X41, 0X40, 0X40, 0X41, 0X3E, 0X00, 0X00, 0X00, 0X00},
        {0X00, 0X00, 0X3F, 0X42, 0X42, 0X42, 0X42, 0X3E, 0X02, 0X02, 0X06, 0X00},
        {0X00, 0X00, 0X3E, 0X41, 0X40, 0X7F, 0X41, 0X3E, 0X00, 0X00, 0X00, 0X00},
        {0X00, 0X00, 0X3C, 0X10, 0X10, 0X10, 0X10, 0X3C, 0X10, 0X10, 0X0C, 0X00},
        {0X3C, 0X02, 0X02, 0X3E, 0X42, 0X42, 0X42, 0X3F, 0X00, 0X00, 0X00, 0X00},
        {0X00, 0X00, 0X77, 0X22, 0X22, 0X22, 0X32, 0X2C, 0X20, 0X20, 0X60, 0X00},
        {0X00, 0X00, 0X3E, 0X08, 0X08, 0X08, 0X08, 0X38, 0X00, 0X00, 0X08, 0X00},
        {0X38, 0X04, 0X04, 0X04, 0X04, 0X04, 0X04, 0X3C, 0X00, 0X00, 0X04, 0X00},
        {0X00, 0X00, 0X63, 0X24, 0X38, 0X28, 0X24, 0X26, 0X20, 0X20, 0X60, 0X00},
        {0X00, 0X00, 0X3E, 0X08, 0X08, 0X08, 0X08, 0X08, 0X08, 0X08, 0X18, 0X00},
        {0X00, 0X00, 0X6B, 0X2A, 0X2A, 0X2A, 0X2A, 0X74, 0X00, 0X00, 0X00, 0X00},
        {0X00, 0X00, 0X77, 0X22, 0X22, 0X22, 0X32, 0X6C, 0X00, 0X00, 0X00, 0X00},
        {0X00, 0X00, 0X3E, 0X41, 0X41, 0X41, 0X41, 0X3E, 0X00, 0X00, 0X00, 0X00},
        {0X70, 0X20, 0X3E, 0X21, 0X21, 0X21, 0X21, 0X7E, 0X00, 0X00, 0X00, 0X00},
        {0X07, 0X02, 0X3E, 0X42, 0X42, 0X42, 0X42, 0X3F, 0X00, 0X00, 0X00, 0X00},
        {0X00, 0X00, 0X7C, 0X10, 0X10, 0X10, 0X19, 0X76, 0X00, 0X00, 0X00, 0X00},
        {0X00, 0X00, 0X3E, 0X41, 0X06, 0X38, 0X41, 0X3E, 0X00, 0X00, 0X00, 0X00},
        {0X00, 0X00, 0X0C, 0X12, 0X10, 0X10, 0X10, 0X3C, 0X10, 0X10, 0X00, 0X00},
        {0X00, 0X00, 0X1B, 0X26, 0X22, 0X22, 0X22, 0X66, 0X00, 0X00, 0X00, 0X00},
        {0X00, 0X00, 0X08, 0X14, 0X14, 0X22, 0X22, 0X77, 0X00, 0X00, 0X00, 0X00},
        {0X00, 0X00, 0X14, 0X14, 0X2A, 0X2A, 0X22, 0X77, 0X00, 0X00, 0X00, 0X00},
        {0X00, 0X00, 0X77, 0X22, 0X1C, 0X1C, 0X22, 0X77, 0X00, 0X00, 0X00, 0X00},
        {0X30, 0X08, 0X08, 0X14, 0X14, 0X22, 0X22, 0X77, 0X00, 0X00, 0X00, 0X00},
        {0X00, 0X00, 0X7E, 0X22, 0X10, 0X08, 0X44, 0X7E, 0X00, 0X00, 0X00, 0X00},
        {0X06, 0X08, 0X08, 0X08, 0X08, 0X30, 0X08, 0X08, 0X08, 0X08, 0X06, 0X00},
        {0X08, 0X08, 0X08, 0X08, 0X08, 0X08, 0X08, 0X08, 0X08, 0X08, 0X08, 0X08},
        {0X30, 0X08, 0X08, 0X08, 0X08, 0X06, 0X08, 0X08, 0X08, 0X08, 0X30, 0X00},
        {0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X46, 0X49, 0X31, 0X00, 0X00},
        {0X00, 0X1C, 0X1C, 0X1C, 0X1C, 0X1C, 0X1C, 0X1C, 0X1C, 0X1C, 0X00, 0X00}
    };

    unsigned int   sourceWidth  = 8;
    unsigned int   sourceHeight = 12;

    FontResolution fontRes( sourceWidth, sourceHeight );

    // populate the glyph mp
    for( unsigned int i = 32; i < 127; i++ )
    {
        osg::ref_ptr<Glyph> glyph    = new Glyph( this, i );

        unsigned int        dataSize = sourceWidth * sourceHeight;
        unsigned char*      data     = new unsigned char[dataSize];

        // clear the image to zeros.
        for( unsigned char* p = data; p < data + dataSize; )
        {
            *p++ = 0;
        }

        glyph->setImage( static_cast<int>( sourceWidth ),
                         static_cast<int>( sourceHeight ),
                         1,
                         GL_ALPHA,
                         GL_ALPHA,
                         GL_UNSIGNED_BYTE,
                         data,
                         osg::Image::USE_NEW_DELETE,
                         1 );

        // now populate data array by converting bitmap into a luminance_alpha map.
        unsigned char* ptr       = rasters[i - 32];
        unsigned char  value_on  = 255;
        unsigned char  value_off = 0;

        for( unsigned int row = 0; row < sourceHeight; ++row, ++ptr )
        {
            ( *data++ ) = ( ( *ptr ) & 128 ) ? value_on : value_off;
            ( *data++ ) = ( ( *ptr ) & 64 ) ? value_on : value_off;
            ( *data++ ) = ( ( *ptr ) & 32 ) ? value_on : value_off;
            ( *data++ ) = ( ( *ptr ) & 16 ) ? value_on : value_off;
            ( *data++ ) = ( ( *ptr ) & 8 ) ? value_on : value_off;
            ( *data++ ) = ( ( *ptr ) & 4 ) ? value_on : value_off;
            ( *data++ ) = ( ( *ptr ) & 2 ) ? value_on : value_off;
            ( *data++ ) = ( ( *ptr ) & 1 ) ? value_on : value_off;
        }

        float coord_scale = 1.0F / float( sourceHeight );

        glyph->setWidth( static_cast<float>( sourceWidth ) * coord_scale );
        glyph->setHeight( static_cast<float>( sourceHeight ) * coord_scale );

        glyph->setHorizontalBearing( osg::vec2( 0.0F,
                                                -2 * coord_scale ) );    // bottom left.
        glyph->setHorizontalAdvance( static_cast<float>( sourceWidth ) * coord_scale );
        glyph->setVerticalBearing( osg::vec2( 0.5F, 1.0F ) );            // top middle.
        glyph->setVerticalAdvance( static_cast<float>( sourceHeight ) * coord_scale );

        glyph->setFontResolution( fontRes );

        addGlyph( fontRes, i, glyph.get() );
    }
}
