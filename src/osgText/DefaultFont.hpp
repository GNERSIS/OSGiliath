/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * DefaultFont, derived from Font.
 * Provides: className, getFileName, supportsMultipleFontResolutions, getGlyph,
 * getGlyph3D, getKerning.
 */
#pragma once

#include <map>
#include <osg/core/ref_ptr.hpp>
#include <osgText/Font.hpp>

namespace osgText
{

    class DefaultFont : public Font
    {
        public:

            DefaultFont();

            virtual const char*
            className() const
            {
                return "DefaultFont";
            }

            virtual std::string
            getFileName() const
            {
                return "";
            }

            virtual bool
            supportsMultipleFontResolutions() const
            {
                return false;
            }

            virtual osgText::Glyph*
            getGlyph( const FontResolution& fontRes,
                      unsigned int          charcode );

            virtual osgText::Glyph3D*
            getGlyph3D( const osgText::FontResolution& /*fontRes*/,
                        unsigned int /*charcode*/ )
            {
                return 0;
            }

            virtual osg::vec2
            getKerning( const osgText::FontResolution& fontRes,
                        unsigned int                   leftcharcode,
                        unsigned int                   rightcharcode,
                        KerningType                    kerningType );

            virtual bool
            hasVertical() const;

            virtual float
            getScale() const
            {
                return 1.0;
            }

        protected:

            virtual ~DefaultFont();

            void
            constructGlyphs();
    };

}
