/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * FreeTypeFont, derived from FontImplementation.
 * Provides: getFileName, supportsMultipleFontResolutions, getGlyph, getGlyph3D,
 * getKerning, hasVertical.
 */
#pragma once

#include <ft2build.h>
#include <osgText/Font.hpp>
#include FT_FREETYPE_H

class FreeTypeFont : public osgText::Font::FontImplementation
{
        // declare the interface to a font.

    public:

        FreeTypeFont( const std::string& filename,
                      FT_Face            face,
                      unsigned int       flags );
        FreeTypeFont( FT_Byte*     buffer,
                      FT_Face      face,
                      unsigned int flags );

        virtual ~FreeTypeFont();

        virtual std::string
        getFileName() const
        {
            return _filename;
        }

        virtual bool
        supportsMultipleFontResolutions() const
        {
            return true;
        }

        virtual osgText::Glyph*
        getGlyph( const osgText::FontResolution& fontRes,
                  unsigned int                   charcode );

        virtual osgText::Glyph3D*
        getGlyph3D( const osgText::FontResolution& fontRes,
                    unsigned int                   charcode );

        virtual osg::vec2
        getKerning( const osgText::FontResolution& fontRes,
                    unsigned int                   leftcharcode,
                    unsigned int                   rightcharcode,
                    osgText::KerningType           _kerningType );

        virtual bool
        hasVertical() const;

        virtual bool
        getVerticalSize( float& ascender,
                         float& descender ) const;

        float
        getCoordScale() const;

    protected:

        void
        init();

        void
        setFontResolution( const osgText::FontResolution& fontSize );

        osgText::FontResolution _currentRes;

        long
        ft_round( long x )
        {
            return ( ( x + 32 ) & -64 );
        }

        long
        ft_floor( long x )
        {
            return ( x & -64 );
        }

        long
        ft_ceiling( long x )
        {
            return ( ( x + 63 ) & -64 );
        }

        std::string  _filename;
        FT_Byte*     _buffer;
        FT_Face      _face;
        unsigned int _flags;
};
