/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * ReaderWriterV8, derived from ReaderWriter.
 * Provides: supportsExtension, supportsExtension, className, readObject, readScript,
 * readObject.
 */
#include "V8ScriptEngine.hpp"

#include <osgDB/io/FileNameUtils.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/registry/Registry.hpp>

class ReaderWriterV8 : public osgDB::ReaderWriter
{
    public:

        ReaderWriterV8()
        {
            supportsExtension( "v8", "JavaScript" );
            supportsExtension( "js", "JavaScript" );
        }

        virtual const char*
        className() const
        {
            return "V8 JavaScript ScriptEngine plugin";
        }

        virtual ReadResult
        readObject( std::istream&                       fin,
                    const osgDB::ReaderWriter::Options* options = NULL ) const
        {
            return readScript( fin, options );
        }

        virtual ReadResult
        readObject( const std::string&                  file,
                    const osgDB::ReaderWriter::Options* options = NULL ) const
        {
            if( file == "ScriptEngine.V8" )
            {
                return new v8::V8ScriptEngine();
            }
            if( file == "ScriptEngine.js" )
            {
                return new v8::V8ScriptEngine();
            }

            return readScript( file );
        }

        virtual ReadResult
        readScript( std::istream&                       fin,
                    const osgDB::ReaderWriter::Options* options = NULL ) const
        {
            osg::ref_ptr<osg::Script> script = new osg::Script;
            script->setLanguage( "js" );

            std::string str;
            while( fin )
            {
                int c = fin.get();
                if( c >= 0 && c <= 255 )
                {
                    str.push_back( c );
                }
            }
            script->setScript( str );

            return script.release();
        }

        virtual ReadResult
        readScript( const std::string&                  file,
                    const osgDB::ReaderWriter::Options* options = NULL ) const
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

            osgDB::ifstream istream( fileName.c_str(), std::ios::in );
            if( !istream )
            {
                return ReadResult::FILE_NOT_HANDLED;
            }

            return readScript( istream, options );
        }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN( v8,
                    ReaderWriterV8 )
