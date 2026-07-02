/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * TXFFont, derived from FontImplementation.
 * Provides: getFileName, supportsMultipleFontResolutions, getGlyph, getGlyph3D,
 * hasVertical, getKerning.
 */
#pragma once

#include <iosfwd>
#include <map>
#include <osgText/Font.hpp>
#include <string>

class TXFFont : public osgText::Font::FontImplementation
{
    public:

        TXFFont( const std::string& filename );

        virtual ~TXFFont();

        virtual std::string
        getFileName() const;

        virtual bool
        supportsMultipleFontResolutions() const
        {
            return false;
        }

        virtual osgText::Glyph*
        getGlyph( const osgText::FontResolution& fontRes,
                  unsigned int                   charcode );

        virtual osgText::Glyph3D*
        getGlyph3D( const osgText::FontResolution&,
                    unsigned int )
        {
            return 0;
        }

        virtual bool
        hasVertical() const;

        virtual osg::vec2
        getKerning( const osgText::FontResolution& fontRes,
                    unsigned int                   leftcharcode,
                    unsigned int                   rightcharcode,
                    osgText::KerningType           kerningType );

        bool
        loadFont( std::istream& stream );

    protected:

        typedef std::map<unsigned int, osg::ref_ptr<osgText::Glyph>> GlyphMap;

        std::string                                                  _filename;
        GlyphMap                                                     _chars;
};
