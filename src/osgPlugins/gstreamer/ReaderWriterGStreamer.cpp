/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * ReaderWriterGStreamer, derived from ReaderWriter.
 * Provides: supportsExtension, supportsExtension, supportsExtension, supportsExtension,
 * supportsExtension, supportsExtension.
 */
#include "GStreamerImageStream.hpp"

#include <osgDB/io/FileNameUtils.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/registry/Registry.hpp>

class ReaderWriterGStreamer : public osgDB::ReaderWriter
{
    public:

        ReaderWriterGStreamer()
        {
            supportsExtension( "avi", "" );
            supportsExtension( "flv", "Flash video" );
            supportsExtension( "mov", "Quicktime" );
            supportsExtension( "ogg", "Theora movie format" );
            supportsExtension( "mpg", "Mpeg movie format" );
            supportsExtension( "mpv", "Mpeg movie format" );
            supportsExtension( "wmv", "Windows Media Video format" );
            supportsExtension( "mkv", "Matroska" );
            supportsExtension( "mjpeg", "Motion JPEG" );
            supportsExtension( "mp4", "MPEG-4" );
            supportsExtension( "m4v", "MPEG-4" );
            supportsExtension( "sav", "Unknown" );
            supportsExtension( "3gp", "3G multi-media format" );
            supportsExtension( "sdp", "Session Description Protocol" );
            supportsExtension( "m2ts", "MPEG-2 Transport Stream" );

            gst_init( NULL, NULL );
        }

        virtual ~ReaderWriterGStreamer()
        {
        }

        virtual const char*
        className() const
        {
            return "ReaderWriterGStreamer";
        }

        virtual ReadResult
        readObject( const std::string&                  file,
                    const osgDB::ReaderWriter::Options* options ) const
        {
            return readImage( file, options );
        }

        virtual ReadResult
        readImage( const std::string&                  filename,
                   const osgDB::ReaderWriter::Options* options ) const
        {
            const std::string ext = osgDB::getLowerCaseFileExtension( filename );

            if( !acceptsExtension( ext ) )
            {
                return ReadResult::FILE_NOT_HANDLED;
            }

            const std::string path = osgDB::containsServerAddress( filename )
                                       ? filename
                                       : osgDB::findDataFile( filename, options );

            if( path.empty() )
            {
                return ReadResult::FILE_NOT_FOUND;
            }

            osg::ref_ptr<osgGStreamer::GStreamerImageStream> imageStream =
                new osgGStreamer::GStreamerImageStream();

            if( !imageStream->open( filename ) )
            {
                return ReadResult::FILE_NOT_HANDLED;
            }

            return imageStream.release();
        }
};

REGISTER_OSGPLUGIN( gstreamer,
                    ReaderWriterGStreamer )
