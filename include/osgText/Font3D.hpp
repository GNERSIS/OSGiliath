/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * 3D font resource providing extruded glyph geometry.
 * Generates front/back/wall meshes for Text3D rendering.
 */
#pragma once

#include <osgText/Font.hpp>

namespace osgText
{

    typedef Font Font3D;

#ifdef OSG_PROVIDE_READFILE
    /** deprecated, use readFontFile() instead.*/
    inline Font*
    readFont3DFile( const std::string&                  filename,
                    const osgDB::ReaderWriter::Options* userOptions = 0 )
    {
        return readFontFile( filename, userOptions );
    }

    /** deprecated, use readFontStream() instead.*/
    inline Font*
    readFont3DStream( std::istream&                       stream,
                      const osgDB::ReaderWriter::Options* userOptions = 0 )
    {
        return readFontStream( stream, userOptions );
    }
#endif

    /** deprecated, use readRefFontFile() instead.*/
    inline osg::ref_ptr<Font>
    readRefFont3DFile( const std::string&                  filename,
                       const osgDB::ReaderWriter::Options* userOptions = 0 )
    {
        return readRefFontFile( filename, userOptions );
    }

    /** deprecated, use readRefFontStream() instead.*/
    inline osg::ref_ptr<Font>
    readRefFont3DStream( std::istream&                       stream,
                         const osgDB::ReaderWriter::Options* userOptions = 0 )
    {
        return readRefFontStream( stream, userOptions );
    }

    /** deprecated, use findFontFile() instead.*/
    inline std::string
    findFont3DFile( const std::string& str )
    {
        return findFontFile( str );
    }

}
