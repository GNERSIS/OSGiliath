/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * ReaderWriterTXF, derived from ReaderWriter.
 * Provides: supportsExtension, className, readObject, readObject, REGISTER_OSGPLUGIN.
 */
#include "TXFFont.hpp"

#include <osg/core/Notify.hpp>
#include <osgDB/io/FileNameUtils.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/io/fstream.hpp>
#include <osgDB/registry/Registry.hpp>

class ReaderWriterTXF : public osgDB::ReaderWriter
{
    public:

        ReaderWriterTXF()
        {
            supportsExtension( "txf", "TXF Font format" );
        }

        virtual const char*
        className() const
        {
            return "TXF Font Reader/Writer";
        }

        virtual ReadResult
        readObject( const std::string&                  file,
                    const osgDB::ReaderWriter::Options* options ) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension( file );
            if( !acceptsExtension( ext ) )
            {
                return ReadResult::FILE_NOT_HANDLED;
            }

            std::string fileName = osgDB::findDataFile( file, options );
            if( fileName.empty() )
            {
                return ReadResult::FILE_NOT_FOUND;
            }

            osgDB::ifstream stream;
            stream.open( fileName.c_str(), std::ios::in | std::ios::binary );
            if( !stream.is_open() )
            {
                return ReadResult::FILE_NOT_FOUND;
            }

            TXFFont*                    impl = new TXFFont( fileName );
            osg::ref_ptr<osgText::Font> font = new osgText::Font( impl );
            if( !impl->loadFont( stream ) )
            {
                return ReadResult::FILE_NOT_HANDLED;
            }
            return font.release();
        }

        virtual ReadResult
        readObject( std::istream& stream,
                    const osgDB::ReaderWriter::Options* ) const
        {
            TXFFont*                    impl = new TXFFont( "streamed font" );
            osg::ref_ptr<osgText::Font> font = new osgText::Font( impl );
            if( !impl->loadFont( stream ) )
            {
                return ReadResult::FILE_NOT_HANDLED;
            }
            return font.release();
        }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN( txf,
                    ReaderWriterTXF )
