/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * ReaderWriterPython, derived from ReaderWriter.
 * Provides: supportsExtension, className, readObject, readScript, readObject,
 * readScript.
 */
#include "PythonScriptEngine.hpp"

#include <osgDB/io/FileNameUtils.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/registry/Registry.hpp>

class ReaderWriterPython : public osgDB::ReaderWriter
{
    public:

        ReaderWriterPython()
        {
            supportsExtension( "python", "python script" );
        }

        virtual const char*
        className() const
        {
            return "Python ScriptEngine plugin";
        }

        virtual ReadResult
        readObject( std::istream&                       fin,
                    const osgDB::ReaderWriter::Options* options = NULL ) const
        {
            return readScript( fin );
        }

        virtual ReadResult
        readObject( const std::string&                  file,
                    const osgDB::ReaderWriter::Options* options = NULL ) const
        {
            if( file == "ScriptEngine.python" )
            {
                return new python::PythonScriptEngine();
            }

            return readScript( file, options );
        }

        virtual ReadResult
        readScript( std::istream&                       fin,
                    const osgDB::ReaderWriter::Options* options = NULL ) const
        {
            osg::ref_ptr<osg::Script> script = new osg::Script;
            script->setLanguage( "python" );

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
            if( file == "ScriptEngine.python" )
            {
                return new python::PythonScriptEngine();
            }
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
REGISTER_OSGPLUGIN( python,
                    ReaderWriterPython )
