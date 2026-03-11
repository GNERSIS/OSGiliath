/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * ReaderWriterDOT, derived from ReaderWriter.
 * Provides: className, acceptsExtension, writeNode, o, writeNode, WriteResult.
 */
#include "SimpleDotVisitor.hpp"

#include <osgDB/io/FileNameUtils.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/registry/ReaderWriter.hpp>

class ReaderWriterDOT : public osgDB::ReaderWriter
{
    public:

        virtual const char*
        className() const
        {
            return "DOT Writer";
        }

        virtual bool
        acceptsExtension( const std::string& extension ) const
        {
            return osgDB::equalCaseInsensitive( extension, "dot" );
        }

        virtual WriteResult
        writeNode( const osg::Node&   node,
                   const std::string& fileName,
                   const Options*     options = NULL ) const
        {
            std::string ext = osgDB::getFileExtension( fileName );
            if( !acceptsExtension( ext ) )
            {
                return WriteResult::FILE_NOT_HANDLED;
            }

            osgDB::ofstream o( fileName.c_str(), std::ios_base::out );
            if( o )
            {
                return writeNode( node, o, options );
            }

            return WriteResult( WriteResult::ERROR_IN_WRITING_FILE );
        }

        virtual WriteResult
        writeNode( const osg::Node& node,
                   std::ostream&    fout,
                   const Options*   options = NULL ) const
        {
            osgDot::SimpleDotVisitor sdv;
            sdv.setOptions( options );
            sdv.run( node, &fout );
            return WriteResult( WriteResult::FILE_SAVED );
        }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN( dot,
                    ReaderWriterDOT )
