#include "GLTFLoader.hpp"

#include <osgDB/io/FileNameUtils.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/registry/Registry.hpp>

class ReaderWriterGLTF : public osgDB::ReaderWriter
{
    public:

        ReaderWriterGLTF()
        {
            supportsExtension( "gltf", "glTF 2.0 ASCII format" );
            supportsExtension( "glb", "glTF 2.0 Binary format" );
        }

        const char*
        className() const override
        {
            return "glTF 2.0 Reader";
        }

        ReadResult
        readNode( const std::string& file,
                  const Options*     options ) const override
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

            // Set up database path for relative resource loading
            osg::ref_ptr<Options> local_opt =
                options ? static_cast<Options*>(
                              options->clone( osg::CopyOp::SHALLOW_COPY )
                          )
                        : new Options;
            local_opt->getDatabasePathList().push_front(
                osgDB::getFilePath( fileName )
            );

            GLTFLoader              loader;
            osg::ref_ptr<osg::Node> node = loader.load( fileName, local_opt.get() );

            if( !node )
            {
                return ReadResult::ERROR_IN_READING_FILE;
            }

            return node.release();
        }
};

REGISTER_OSGPLUGIN( gltf,
                    ReaderWriterGLTF )
