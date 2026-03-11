/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Plugin callback interfaces: FindFileCallback, ReadFileCallback,
 * WriteFileCallback for customizing I/O behavior.
 */
#pragma once

#include <deque>
#include <iosfwd>
#include <list>
#include <osgDB/cache/AuthenticationMap.hpp>
#include <osgDB/io/FileCache.hpp>
#include <osgDB/registry/ReaderWriter.hpp>

namespace osgDB
{

    /** list of directories to search through which searching for files. */
    typedef std::deque<std::string> FilePathList;

    enum CaseSensitivity
    {
        CASE_SENSITIVE,
        CASE_INSENSITIVE,
    };

    // forward declare
    class Options;

    class OSGDB_EXPORT FindFileCallback : public virtual osg::Referenced
    {
        public:

            virtual std::string
            findDataFile( const std::string& filename,
                          const Options*     options,
                          CaseSensitivity    caseSensitivity );

            virtual std::string
            findLibraryFile( const std::string& filename,
                             const Options*     options,
                             CaseSensitivity    caseSensitivity );

        protected:

            virtual ~FindFileCallback()
            {
            }
    };

    class OSGDB_EXPORT ReadFileCallback : public virtual osg::Referenced
    {
        public:

            virtual ReaderWriter::ReadResult
            openArchive( const std::string&          filename,
                         ReaderWriter::ArchiveStatus status,
                         unsigned int                indexBlockSizeHint,
                         const Options*              useObjectCache );

            virtual ReaderWriter::ReadResult
            readObject( const std::string& filename,
                        const Options*     options );

            virtual ReaderWriter::ReadResult
            readImage( const std::string& filename,
                       const Options*     options );

            virtual ReaderWriter::ReadResult
            readHeightField( const std::string& filename,
                             const Options*     options );

            virtual ReaderWriter::ReadResult
            readNode( const std::string& filename,
                      const Options*     options );

            virtual ReaderWriter::ReadResult
            readShader( const std::string& filename,
                        const Options*     options );

            virtual ReaderWriter::ReadResult
            readScript( const std::string& filename,
                        const Options*     options );

        protected:

            virtual ~ReadFileCallback()
            {
            }
    };

    class OSGDB_EXPORT WriteFileCallback : public virtual osg::Referenced
    {
        public:

            virtual ReaderWriter::WriteResult
            writeObject( const osg::Object& obj,
                         const std::string& fileName,
                         const Options*     options );

            virtual ReaderWriter::WriteResult
            writeImage( const osg::Image&  obj,
                        const std::string& fileName,
                        const Options*     options );

            virtual ReaderWriter::WriteResult
            writeHeightField( const osg::HeightField& obj,
                              const std::string&      fileName,
                              const Options*          options );

            virtual ReaderWriter::WriteResult
            writeNode( const osg::Node&   obj,
                       const std::string& fileName,
                       const Options*     options );

            virtual ReaderWriter::WriteResult
            writeShader( const osg::Shader& obj,
                         const std::string& fileName,
                         const Options*     options );

            virtual ReaderWriter::WriteResult
            writeScript( const osg::Script& obj,
                         const std::string& fileName,
                         const Options*     options );

        protected:

            virtual ~WriteFileCallback()
            {
            }
    };

    class OSGDB_EXPORT FileLocationCallback : public virtual osg::Referenced
    {
        public:

            enum Location
            {
                LOCAL_FILE,
                REMOTE_FILE,
            };

            virtual Location
            fileLocation( const std::string& filename,
                          const Options*     options ) = 0;

            virtual bool
            useFileCache() const = 0;

        protected:

            virtual ~FileLocationCallback()
            {
            }
    };

}
