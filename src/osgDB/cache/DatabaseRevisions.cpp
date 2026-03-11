/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Database revision tracking for incremental scene updates.
 * Supports version-aware loading and change detection.
 */
#include <osgDB/cache/DatabaseRevisions.hpp>

#include <osgDB/io/FileNameUtils.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/io/WriteFile.hpp>

using namespace osgDB;

////////////////////////////////////////////////////////////////////////////////////////////
//
// FilelList
//
FileList::FileList()
{
}

FileList::FileList( const FileList&    fileList,
                    const osg::CopyOp& copyop ) :
    Inherit( fileList,
             copyop ),
    _files( fileList._files )
{
}

FileList::~FileList()
{
}

bool
FileList::removeFile( const std::string& filename )
{
    FileNames::iterator itr = _files.find( filename );
    if( itr == _files.end() )
    {
        return false;
    }

    _files.erase( itr );
    return true;
}

void
FileList::append( FileList* fileList )
{
    for( FileNames::iterator itr = fileList->_files.begin();
         itr != fileList->_files.end();
         ++itr )
    {
        _files.insert( *itr );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// DatabaseRevision
//
DatabaseRevision::DatabaseRevision()
{
}

DatabaseRevision::DatabaseRevision( const DatabaseRevision& revision,
                                    const osg::CopyOp&      copyop ) :
    Inherit( revision,
             copyop ),
    _databasePath( revision._databasePath ),
    _filesAdded( revision._filesAdded ),
    _filesRemoved( revision._filesRemoved ),
    _filesModified( revision._filesModified )
{
}

DatabaseRevision::~DatabaseRevision()
{
}

bool
DatabaseRevision::isFileBlackListed( const std::string& filename ) const
{
    OSG_INFO << "DatabaseRevision(" << getName() << ")::isFileBlackListed(" << filename
             << ")" << std::endl;

    if( _databasePath.length() >= filename.length() )
    {
        return false;
    }
    if( filename.compare( 0, _databasePath.length(), _databasePath ) != 0 )
    {
        return false;
    }

    std::string localPath( filename,
                           _databasePath.empty() ? 0 : _databasePath.length() + 1,
                           std::string::npos );

    return ( _filesRemoved.valid() && _filesRemoved->containsFile( localPath ) ) ||
           ( _filesModified.valid() && _filesModified->containsFile( localPath ) );
}

bool
DatabaseRevision::removeFile( const std::string& filename )
{
    bool removed = false;
    if( _filesAdded.valid() )
    {
        removed = _filesAdded->removeFile( filename ) | removed;
    }
    if( _filesRemoved.valid() )
    {
        removed = _filesRemoved->removeFile( filename ) | removed;
    }
    if( _filesModified.valid() )
    {
        removed = _filesModified->removeFile( filename ) | removed;
    }
    return removed;
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// DatabaseRevisions
//
DatabaseRevisions::DatabaseRevisions()
{
}

DatabaseRevisions::DatabaseRevisions( const DatabaseRevisions& revisions,
                                      const osg::CopyOp&       copyop ) :
    Inherit( revisions,
             copyop ),
    _databasePath( revisions._databasePath ),
    _revisionList( revisions._revisionList )
{
}

DatabaseRevisions::~DatabaseRevisions()
{
}

void
DatabaseRevisions::addRevision( DatabaseRevision* revision )
{
    if( !revision )
    {
        return;
    }

    for( DatabaseRevisionList::iterator itr = _revisionList.begin();
         itr != _revisionList.end();
         ++itr )
    {
        if( *itr == revision )
        {
            return;
        }
        if( ( *itr )->getName() == revision->getName() )
        {
            ( *itr ) = revision;
            return;
        }
    }

    _revisionList.push_back( revision );
}

void
DatabaseRevisions::removeRevision( DatabaseRevision* revision )
{
    for( DatabaseRevisionList::iterator itr = _revisionList.begin();
         itr != _revisionList.end();
         ++itr )
    {
        if( *itr == revision )
        {
            _revisionList.erase( itr );
            return;
        }
    }
}

bool
DatabaseRevisions::isFileBlackListed( const std::string& filename ) const
{
    for( DatabaseRevisionList::const_iterator itr = _revisionList.begin();
         itr != _revisionList.end();
         ++itr )
    {
        if( ( *itr )->isFileBlackListed( filename ) )
        {
            OSG_INFO << "File is black listed " << filename << std::endl;
            return true;
        }
    }
    return false;
}

bool
DatabaseRevisions::removeFile( const std::string& filename )
{
    OSG_INFO << "Remove file " << filename << std::endl;

    bool removed = false;
    for( DatabaseRevisionList::iterator itr = _revisionList.begin();
         itr != _revisionList.end();
         ++itr )
    {
        removed = ( *itr )->removeFile( filename ) | removed;
    }
    return removed;
}
