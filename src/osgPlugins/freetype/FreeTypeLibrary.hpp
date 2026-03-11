/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * FreeTypeLibrary, derived from Referenced.
 * Provides: instance, getMutex, getFont, getFont, removeFontImplmentation.
 */
#pragma once

#include "FreeTypeFont.hpp"

#include <ft2build.h>
#include <istream>
#include <osgText/Font3D>
#include <osgText/Font>
#include <set>

class FreeTypeLibrary : public osg::Referenced
{
    public:

        /** protected destrcutor to prevent inappropriate deletion.*/
        virtual ~FreeTypeLibrary();

        /** get the singleton instance.*/
        static FreeTypeLibrary*
        instance();

        std::mutex&
        getMutex()
        {
            return _mutex;
        }

        osgText::Font*
        getFont( const std::string& fontfile,
                 unsigned int       index = 0,
                 unsigned int       flags = 0 );
        osgText::Font*
        getFont( std::istream& fontstream,
                 unsigned int  index = 0,
                 unsigned int  flags = 0 );

        void
        removeFontImplmentation( FreeTypeFont* fontImpl )
        {
            _fontImplementationSet.erase( fontImpl );
        }

    protected:

        /** common method to load a FT_Face from a file*/
        bool
        getFace( const std::string& fontfile,
                 unsigned int       index,
                 FT_Face&           face );
        /** common method to load a FT_Face from a stream */
        FT_Byte*
        getFace( std::istream& fontstream,
                 unsigned int  index,
                 FT_Face&      face );

        /** Verify the correct character mapping for MS windows */
        void
        verifyCharacterMap( FT_Face face );

        /** protected constructor to ensure the only way to create the
         * library is via the singleton instance method.*/
        FreeTypeLibrary();

        typedef std::set<FreeTypeFont*> FontImplementationSet;

        mutable std::mutex              _mutex;
        FT_Library                      _ftlibrary;
        FontImplementationSet           _fontImplementationSet;
};
