/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Maps hostnames to username/password credentials for
 * authenticated HTTP downloads of remote data.
 */
#include <osgDB/cache/AuthenticationMap.hpp>

#include <osgDB/io/FileNameUtils.hpp>

using namespace osgDB;

void
AuthenticationMap::addAuthenticationDetails( const std::string&     path,
                                             AuthenticationDetails* details )
{
    _authenticationMap[path] = details;
}

const AuthenticationDetails*
AuthenticationMap::getAuthenticationDetails( const std::string& path ) const
{
    // see if the full filename has its own authentication details
    AuthenticationDetailsMap::const_iterator itr = _authenticationMap.find( path );
    if( itr != _authenticationMap.end() )
    {
        return itr->second.get();
    }

    // now look to see if the paths to the file have their own authentication details
    std::string basePath = osgDB::getFilePath( path );
    while( !basePath.empty() )
    {
        itr = _authenticationMap.find( basePath );
        if( itr != _authenticationMap.end() )
        {
            return itr->second.get();
        }

        basePath = osgDB::getFilePath( basePath );
    }
    return 0;
}
