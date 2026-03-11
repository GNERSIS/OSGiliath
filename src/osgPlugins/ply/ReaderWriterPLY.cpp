/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * ReaderWriterPLY, derived from ReaderWriter.
 * Provides: supportsExtension, className, readObject, readNode, readNode.
 */
#include "vertexData.hpp"

#include <osgDB/io/FileNameUtils.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/registry/Registry.hpp>

using namespace osg;
using namespace osgDB;
using namespace std;

///////////////////////////////////////////////////////////////////////////////
//!
//! \class ReaderWriterPLY
//! \brief This is the Reader for the ply file format
//!
//////////////////////////////////////////////////////////////////////////////
class ReaderWriterPLY : public osgDB::ReaderWriter
{
    public:

        ReaderWriterPLY()
        {
            supportsExtension( "ply", "Stanford Triangle Format" );
        }

        virtual const char*
        className() const
        {
            return "ReaderWriterPLY";
        }

        virtual ReadResult
        readObject( const std::string&                  filename,
                    const osgDB::ReaderWriter::Options* options ) const
        {
            return readNode( filename, options );
        }

        virtual ReadResult
        readNode( const std::string& fileName,
                  const osgDB::ReaderWriter::Options* ) const;

    protected:
};

// register with Registry to instantiate the above reader/writer.
REGISTER_OSGPLUGIN( ply,
                    ReaderWriterPLY )

///////////////////////////////////////////////////////////////////////////////
//!
//! \brief Function which is called when any ply file is requested to load in
//! \osgDB. Load read ply file and if it successes return the osg::Node
//!
///////////////////////////////////////////////////////////////////////////////
osgDB::ReaderWriter::ReadResult
ReaderWriterPLY::readNode( const std::string&                  filename,
                           const osgDB::ReaderWriter::Options* options ) const
{
    // Get the file extension
    std::string ext = osgDB::getFileExtension( filename );

    // If the file extension does not support then return that file is not handled
    if( !acceptsExtension( ext ) )
    {
        return ReadResult::FILE_NOT_HANDLED;
    }

    // Check whether or not file exist or not
    std::string fileName = osgDB::findDataFile( filename, options );
    if( fileName.empty() )
    {
        return ReadResult::FILE_NOT_FOUND;
    }

    // Instance of vertex data which will read the ply file and convert in to osg::Node
    ply::VertexData vertexData;
    osg::Node*      node = vertexData.readPlyFile( fileName.c_str() );

    if( node )
    {
        return node;
    }

    return ReadResult::FILE_NOT_HANDLED;
}
