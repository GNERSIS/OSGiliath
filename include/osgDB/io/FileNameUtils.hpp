/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Filename string manipulation: extension extraction, path splitting,
 * name conversion, and URI scheme detection.
 */
#pragma once

#include <osgDB/Export.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace osgDB
{

    /** Gets the parent path from full name (Ex: /a/b/c.Ext => /a/b). */
    extern OSGDB_EXPORT std::string
                        getFilePath( std::string_view filename );
    /** Gets the extension without dot (Ex: /a/b/c.Ext => Ext). */
    extern OSGDB_EXPORT std::string
                        getFileExtension( std::string_view filename );
    /** Gets the extension including dot (Ex: /a/b/c.Ext => .Ext). */
    extern OSGDB_EXPORT std::string
                        getFileExtensionIncludingDot( std::string_view filename );
    /** Gets the lowercase extension without dot (Ex: /a/b/c.Ext => ext). */
    extern OSGDB_EXPORT std::string
                        getLowerCaseFileExtension( std::string_view filename );
    /** Gets file name with extension (Ex: /a/b/c.Ext => c.Ext). */
    extern OSGDB_EXPORT std::string
                        getSimpleFileName( std::string_view fileName );
    /** Gets file path without last extension (Ex: /a/b/c.Ext => /a/b/c ; file.ext1.ext2
     * => file.ext1). */
    extern OSGDB_EXPORT std::string
                        getNameLessExtension( std::string_view fileName );
    /** Gets file path without \b all extensions (Ex: /a/b/c.Ext => /a/b/c ;
     * file.ext1.ext2 => file). */
    extern OSGDB_EXPORT std::string
                        getNameLessAllExtensions( std::string_view fileName );
    /** Gets file name without last extension (Ex: /a/b/c.Ext => c ; file.ext1.ext2 =>
     * file.ext1). */
    extern OSGDB_EXPORT std::string
                        getStrippedName( std::string_view fileName );
    /** If 'to' is in a subdirectory of 'from' then this function returns the subpath,
     * otherwise it just returns the file name. The function does \b not automagically
     * resolve paths as the system does, so be careful to give canonical paths. However,
     * the function interprets slashes ('/') and backslashes ('\') as they were equal.
     */
    extern OSGDB_EXPORT std::string
                        getPathRelative( const std::string& from,
                                         const std::string& to );
    /** Gets root part of a path ("/" or "C:"), or an empty string if none found. */
    extern OSGDB_EXPORT std::string
                        getPathRoot( std::string_view path );
    /** Tests if path is absolute, as !getPathRoot(path).empty(). */
    extern OSGDB_EXPORT bool
                        isAbsolutePath( std::string_view path );

    /** Converts forward slashes (/) to back slashes (\). */
    extern OSGDB_EXPORT std::string
                        convertFileNameToWindowsStyle( std::string_view fileName );
    /** Converts back slashes (\) to forward slashes (/). */
    extern OSGDB_EXPORT std::string
                        convertFileNameToUnixStyle( std::string_view fileName );
    extern OSGDB_EXPORT std::string
                        convertToLowerCase( std::string_view str );

    const char          UNIX_PATH_SEPARATOR    = '/';
    const char          WINDOWS_PATH_SEPARATOR = '\\';

    /** Get the path separator for the current platform. */
    extern OSGDB_EXPORT char
    getNativePathSeparator();
    /** Check if the path contains only the current platform's path separators. */
    extern OSGDB_EXPORT bool
                        isFileNameNativeStyle( std::string_view fileName );
    /** Convert the path to contain only the current platform's path separators. */
    extern OSGDB_EXPORT std::string
                        convertFileNameToNativeStyle( std::string_view fileName );

    extern OSGDB_EXPORT bool
    equalCaseInsensitive( std::string_view lhs,
                          std::string_view rhs );

    extern OSGDB_EXPORT bool
                        containsServerAddress( std::string_view filename );
    extern OSGDB_EXPORT std::string
                        getServerProtocol( std::string_view filename );
    extern OSGDB_EXPORT std::string
                        getServerAddress( std::string_view filename );
    extern OSGDB_EXPORT std::string
                        getServerFileName( std::string_view filename );

    /** Concatenates two paths */
    extern OSGDB_EXPORT std::string
                        concatPaths( const std::string& left,
                                     const std::string& right );

    /** Removes .. and . dirs in a path */
    extern OSGDB_EXPORT std::string
                        getRealPath( const std::string& path );

    /** Splits a path into elements between separators (including Windows' root, if any).
     */
    extern OSGDB_EXPORT void
    getPathElements( const std::string&        path,
                     std::vector<std::string>& out_elements );

    /** Functor for helping sort filename in alphabetical and numerical order when using
     * in conjunction with std::sort.*/
    struct FileNameComparator
    {
            inline bool
            operator()( const std::string& lhs,
                        const std::string& rhs ) const
            {
                std::string::size_type size_lhs = lhs.size();
                std::string::size_type size_rhs = rhs.size();
                std::string::size_type pos_lhs  = 0;
                std::string::size_type pos_rhs  = 0;
                while( pos_lhs < size_lhs && pos_rhs < size_rhs )
                {
                    char c_lhs       = lhs[pos_rhs];
                    char c_rhs       = rhs[pos_rhs];
                    bool numeric_lhs = lhs[pos_lhs] >= '0' && lhs[pos_lhs] <= '9';
                    bool numeric_rhs = rhs[pos_rhs] >= '0' && rhs[pos_rhs] <= '9';
                    if( numeric_lhs && numeric_rhs )
                    {
                        std::string::size_type start_lhs = pos_lhs;
                        ++pos_lhs;
                        while( pos_lhs <
                               size_lhs &&
                               ( lhs[pos_lhs] >= '0' && lhs[pos_lhs] <= '9' ) )
                        {
                            ++pos_lhs;
                        }

                        std::string::size_type start_rhs = pos_rhs;
                        ++pos_rhs;
                        while( pos_rhs <
                               size_rhs &&
                               ( rhs[pos_rhs] >= '0' && rhs[pos_rhs] <= '9' ) )
                        {
                            ++pos_rhs;
                        }

                        if( pos_lhs < pos_rhs )
                        {
                            return true;
                        }
                        else if( pos_rhs < pos_lhs )
                        {
                            return false;
                        }

                        while( start_lhs < pos_lhs && start_rhs < pos_rhs )
                        {
                            if( lhs[start_lhs] < rhs[start_rhs] )
                            {
                                return true;
                            }
                            if( lhs[start_lhs] > rhs[start_rhs] )
                            {
                                return false;
                            }
                            ++start_lhs;
                            ++start_rhs;
                        }
                    }
                    else
                    {
                        if( c_lhs < c_rhs )
                        {
                            return true;
                        }
                        else if( c_rhs < c_lhs )
                        {
                            return false;
                        }

                        ++pos_lhs;
                        ++pos_rhs;
                    }
                }

                return pos_lhs < pos_rhs;
            }
    };

    extern OSGDB_EXPORT void
    stringcopy( char*       dest,
                const char* src,
                size_t      length );

#define stringcopyfixedsize( DEST, SRC ) stringcopy( DEST, SRC, sizeof( DEST ) );

}
