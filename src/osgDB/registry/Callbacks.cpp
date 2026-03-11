/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Plugin callback interfaces: FindFileCallback, ReadFileCallback,
 * WriteFileCallback for customizing I/O behavior.
 */
#include <osgDB/registry/Callbacks.hpp>

#include <osgDB/registry/Registry.hpp>

using namespace osgDB;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FindFileCallback default implementation
//
std::string
FindFileCallback::findDataFile( const std::string& filename,
                                const Options*     options,
                                CaseSensitivity    caseSensitivity )
{
    return osgDB::Registry::instance()->findDataFileImplementation( filename,
                                                                    options,
                                                                    caseSensitivity );
}

std::string
FindFileCallback::findLibraryFile( const std::string& filename,
                                   const Options*     options,
                                   CaseSensitivity    caseSensitivity )
{
    return osgDB::Registry::instance()->findLibraryFileImplementation( filename,
                                                                       options,
                                                                       caseSensitivity );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ReadFileCallback default implementation
//
ReaderWriter::ReadResult
ReadFileCallback::openArchive( const std::string&          filename,
                               ReaderWriter::ArchiveStatus status,
                               unsigned int                indexBlockSizeHint,
                               const Options*              useObjectCache )
{
    return osgDB::Registry::instance()->openArchiveImplementation( filename,
                                                                   status,
                                                                   indexBlockSizeHint,
                                                                   useObjectCache );
}

ReaderWriter::ReadResult
ReadFileCallback::readObject( const std::string& filename,
                              const Options*     options )
{
    return osgDB::Registry::instance()->readObjectImplementation( filename, options );
}

ReaderWriter::ReadResult
ReadFileCallback::readImage( const std::string& filename,
                             const Options*     options )
{
    return osgDB::Registry::instance()->readImageImplementation( filename, options );
}

ReaderWriter::ReadResult
ReadFileCallback::readHeightField( const std::string& filename,
                                   const Options*     options )
{
    return osgDB::Registry::instance()->readHeightFieldImplementation( filename,
                                                                       options );
}

ReaderWriter::ReadResult
ReadFileCallback::readNode( const std::string& filename,
                            const Options*     options )
{
    return osgDB::Registry::instance()->readNodeImplementation( filename, options );
}

ReaderWriter::ReadResult
ReadFileCallback::readShader( const std::string& filename,
                              const Options*     options )
{
    return osgDB::Registry::instance()->readShaderImplementation( filename, options );
}

ReaderWriter::ReadResult
ReadFileCallback::readScript( const std::string& filename,
                              const Options*     options )
{
    return osgDB::Registry::instance()->readScriptImplementation( filename, options );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WriteFileCallback default implementation
//
ReaderWriter::WriteResult
WriteFileCallback::writeObject( const osg::Object& obj,
                                const std::string& fileName,
                                const Options*     options )
{
    return osgDB::Registry::instance()->writeObjectImplementation( obj,
                                                                   fileName,
                                                                   options );
}

ReaderWriter::WriteResult
WriteFileCallback::writeImage( const osg::Image&  obj,
                               const std::string& fileName,
                               const Options*     options )
{
    return osgDB::Registry::instance()->writeImageImplementation( obj,
                                                                  fileName,
                                                                  options );
}

ReaderWriter::WriteResult
WriteFileCallback::writeHeightField( const osg::HeightField& obj,
                                     const std::string&      fileName,
                                     const Options*          options )
{
    return osgDB::Registry::instance()->writeHeightFieldImplementation( obj,
                                                                        fileName,
                                                                        options );
}

ReaderWriter::WriteResult
WriteFileCallback::writeNode( const osg::Node&   obj,
                              const std::string& fileName,
                              const Options*     options )
{
    return osgDB::Registry::instance()->writeNodeImplementation( obj,
                                                                 fileName,
                                                                 options );
}

ReaderWriter::WriteResult
WriteFileCallback::writeShader( const osg::Shader& obj,
                                const std::string& fileName,
                                const Options*     options )
{
    return osgDB::Registry::instance()->writeShaderImplementation( obj,
                                                                   fileName,
                                                                   options );
}

ReaderWriter::WriteResult
WriteFileCallback::writeScript( const osg::Script& obj,
                                const std::string& fileName,
                                const Options*     options )
{
    return osgDB::Registry::instance()->writeScriptImplementation( obj,
                                                                   fileName,
                                                                   options );
}
