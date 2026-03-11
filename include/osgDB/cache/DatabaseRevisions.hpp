/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Database revision tracking for incremental scene updates.
 * Supports version-aware loading and change detection.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/nodes/Node.hpp>
#include <osgDB/registry/ReaderWriter.hpp>
#include <set>

namespace osgDB
{

    class OSGDB_EXPORT FileList : public osg::Inherit<osg::Object, FileList>
    {
        public:

            FileList();
            FileList( const FileList&    fileList,
                      const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgDB,
                               FileList )

            typedef std::set<std::string> FileNames;

            FileNames&
            getFileNames()
            {
                return _files;
            }

            const FileNames&
            getFileNames() const
            {
                return _files;
            }

            bool
            empty() const
            {
                return _files.empty();
            }

            bool
            containsFile( const std::string& filename ) const
            {
                return _files.count( filename ) != 0;
            }

            void
            addFile( const std::string& filename )
            {
                _files.insert( filename );
            }

            bool
            removeFile( const std::string& filename );

            void
            append( FileList* fileList );

        protected:

            virtual ~FileList();

            FileNames _files;
    };

    class OSGDB_EXPORT DatabaseRevision
        : public osg::Inherit<osg::Object, DatabaseRevision>
    {
        public:

            DatabaseRevision();
            DatabaseRevision( const DatabaseRevision& revision,
                              const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgDB,
                               DatabaseRevision )

            void
            setDatabasePath( const std::string& path )
            {
                _databasePath = path;
            }

            const std::string&
            getDatabasePath() const
            {
                return _databasePath;
            }

            typedef std::set<std::string> FileNames;

            void
            setFilesAdded( FileList* fileList )
            {
                _filesAdded = fileList;
            }

            FileList*
            getFilesAdded()
            {
                return _filesAdded.get();
            }

            const FileList*
            getFilesAdded() const
            {
                return _filesAdded.get();
            }

            void
            setFilesRemoved( FileList* fileList )
            {
                _filesRemoved = fileList;
            }

            FileList*
            getFilesRemoved()
            {
                return _filesRemoved.get();
            }

            const FileList*
            getFilesRemoved() const
            {
                return _filesRemoved.get();
            }

            void
            setFilesModified( FileList* fileList )
            {
                _filesModified = fileList;
            }

            FileList*
            getFilesModified()
            {
                return _filesModified.get();
            }

            const FileList*
            getFilesModified() const
            {
                return _filesModified.get();
            }

            bool
            isFileBlackListed( const std::string& filename ) const;

            bool
            removeFile( const std::string& filename );

        protected:

            virtual ~DatabaseRevision();

            std::string            _databasePath;

            osg::ref_ptr<FileList> _filesAdded;
            osg::ref_ptr<FileList> _filesRemoved;
            osg::ref_ptr<FileList> _filesModified;
    };

    class OSGDB_EXPORT DatabaseRevisions
        : public osg::Inherit<osg::Object, DatabaseRevisions>
    {
        public:

            DatabaseRevisions();
            DatabaseRevisions( const DatabaseRevisions& revisions,
                               const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgDB,
                               DatabaseRevisions )

            typedef std::vector<osg::ref_ptr<DatabaseRevision>> DatabaseRevisionList;

            void
            setDatabasePath( const std::string& path )
            {
                _databasePath = path;
            }

            const std::string&
            getDatabasePath() const
            {
                return _databasePath;
            }

            void
            addRevision( DatabaseRevision* revision );
            void
            removeRevision( DatabaseRevision* revision );

            DatabaseRevision*
            getDatabaseRevision( unsigned int i )
            {
                return i < _revisionList.size() ? _revisionList[i].get() : 0;
            }

            DatabaseRevisionList&
            getDatabaseRevisionList()
            {
                return _revisionList;
            }

            const DatabaseRevisionList&
            getDatabaseRevisionList() const
            {
                return _revisionList;
            }

            bool
            isFileBlackListed( const std::string& filename ) const;

            bool
            removeFile( const std::string& filename );

        protected:

            virtual ~DatabaseRevisions();

            std::string          _databasePath;
            DatabaseRevisionList _revisionList;
    };

}
