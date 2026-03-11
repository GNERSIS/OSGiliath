/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract archive interface for bundled file access.
 * Provides file-in-archive reading for packaged datasets.
 */
#pragma once

#include <fstream>
#include <list>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/registry/ReaderWriter.hpp>
#include <osgDB/registry/Registry.hpp>

namespace osgDB
{

    /** Base class for implementing database Archives. See src/osgPlugins/osga for an
     * example of a concrete implementation. */
    class OSGDB_EXPORT Archive : public ReaderWriter
    {
        public:

            Archive();
            virtual ~Archive();

            virtual const char*
            libraryName() const
            {
                return "osgDB";
            }

            virtual const char*
            className() const
            {
                return "Archive";
            }

            virtual bool
            acceptsExtension( const std::string& /*extension*/ ) const
            {
                return true;
            }

            /** close the archive.*/
            virtual void
            close() = 0;

            /** Get the file name which represents the archived file.*/
            virtual std::string
            getArchiveFileName() const = 0;

            /** Get the file name which represents the master file recorded in the
             * Archive.*/
            virtual std::string
            getMasterFileName() const = 0;

            using ReaderWriter::fileExists;

            /** return true if file exists in archive.*/
            virtual bool
            fileExists( const std::string& filename ) const = 0;

            /** return type of file. */
            virtual FileType
            getFileType( const std::string& filename ) const = 0;

            typedef osgDB::DirectoryContents FileNameList;

            /** Get the full list of file names available in the archive.*/
            virtual bool
            getFileNames( FileNameList& fileNames ) const = 0;

            /** return the contents of a directory.
             * returns an empty array on any error.*/
            virtual DirectoryContents
            getDirectoryContents( const std::string& dirName ) const;

            virtual ReadResult
            readObject( const std::string& /*fileName*/,
                        const Options* = NULL ) const = 0;
            virtual ReadResult
            readImage( const std::string& /*fileName*/,
                       const Options* = NULL ) const = 0;
            virtual ReadResult
            readHeightField( const std::string& /*fileName*/,
                             const Options* = NULL ) const = 0;
            virtual ReadResult
            readNode( const std::string& /*fileName*/,
                      const Options* = NULL ) const = 0;
            virtual ReadResult
            readShader( const std::string& /*fileName*/,
                        const Options* = NULL ) const = 0;

            virtual WriteResult
            writeObject( const osg::Object& /*obj*/,
                         const std::string& /*fileName*/,
                         const Options* = NULL ) const = 0;
            virtual WriteResult
            writeImage( const osg::Image& /*image*/,
                        const std::string& /*fileName*/,
                        const Options* = NULL ) const = 0;
            virtual WriteResult
            writeHeightField( const osg::HeightField& /*heightField*/,
                              const std::string& /*fileName*/,
                              const Options* = NULL ) const = 0;
            virtual WriteResult
            writeNode( const osg::Node& /*node*/,
                       const std::string& /*fileName*/,
                       const Options* = NULL ) const = 0;
            virtual WriteResult
            writeShader( const osg::Shader& /*shader*/,
                         const std::string& /*fileName*/,
                         const Options* = NULL ) const = 0;
    };

    /** Open an archive for reading or writing.*/
    OSGDB_EXPORT Archive*
    openArchive( const std::string&          filename,
                 ReaderWriter::ArchiveStatus status,
                 unsigned int                indexBlockSizeHint = 4'096 );

    /** Open an archive for reading or writing.*/
    OSGDB_EXPORT Archive*
    openArchive( const std::string&          filename,
                 ReaderWriter::ArchiveStatus status,
                 unsigned int                indexBlockSizeHint,
                 Options*                    options );

}
