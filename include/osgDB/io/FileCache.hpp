/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Disk cache for downloaded remote files. Stores and retrieves
 * files by URL hash for offline access.
 */
#pragma once

#include <osg/nodes/Node.hpp>
#include <osgDB/cache/DatabaseRevisions.hpp>
#include <osgDB/registry/ReaderWriter.hpp>
#include <set>

namespace osgDB
{

    class OSGDB_EXPORT FileCache : public osg::Referenced
    {
        public:

            FileCache( const std::string& path );

            const std::string&
            getFileCachePath() const
            {
                return _fileCachePath;
            }

            virtual bool
            isFileAppropriateForFileCache( const std::string& originalFileName ) const;

            virtual std::string
            createCacheFileName( const std::string& originalFileName ) const;

            virtual bool
            existsInCache( const std::string& originalFileName ) const;

            virtual ReaderWriter::ReadResult
            readImage( const std::string&    originalFileName,
                       const osgDB::Options* options ) const;
            virtual ReaderWriter::WriteResult
            writeImage( const osg::Image&     image,
                        const std::string&    originalFileName,
                        const osgDB::Options* options ) const;

            virtual ReaderWriter::ReadResult
            readObject( const std::string&    originalFileName,
                        const osgDB::Options* options ) const;
            virtual ReaderWriter::WriteResult
            writeObject( const osg::Object&    object,
                         const std::string&    originalFileName,
                         const osgDB::Options* options ) const;

            virtual ReaderWriter::ReadResult
            readHeightField( const std::string&    originalFileName,
                             const osgDB::Options* options ) const;
            virtual ReaderWriter::WriteResult
            writeHeightField( const osg::HeightField& hf,
                              const std::string&      originalFileName,
                              const osgDB::Options*   options ) const;

            virtual ReaderWriter::ReadResult
            readNode( const std::string&    originalFileName,
                      const osgDB::Options* options,
                      bool                  buildKdTreeIfRequired = true ) const;
            virtual ReaderWriter::WriteResult
            writeNode( const osg::Node&      node,
                       const std::string&    originalFileName,
                       const osgDB::Options* options ) const;

            virtual ReaderWriter::ReadResult
            readShader( const std::string&    originalFileName,
                        const osgDB::Options* options ) const;
            virtual ReaderWriter::WriteResult
            writeShader( const osg::Shader&    shader,
                         const std::string&    originalFileName,
                         const osgDB::Options* options ) const;

            bool
            loadDatabaseRevisionsForFile( const std::string& originanlFileName );

            typedef std::list<osg::ref_ptr<DatabaseRevisions>> DatabaseRevisionsList;

            DatabaseRevisionsList&
            getDatabaseRevisionsList()
            {
                return _databaseRevisionsList;
            }

            bool
            isCachedFileBlackListed( const std::string& originalFileName ) const;

        protected:

            virtual ~FileCache();

            std::string           _fileCachePath;

            DatabaseRevisionsList _databaseRevisionsList;

            FileList*
            readFileList( const std::string& originalFileName ) const;
            bool
            removeFileFromBlackListed( const std::string& originalFileName ) const;
    };

}
