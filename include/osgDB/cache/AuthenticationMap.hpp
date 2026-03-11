/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Maps hostnames to username/password credentials for
 * authenticated HTTP downloads of remote data.
 */
#pragma once

#include <map>
#include <osg/core/ref_ptr.hpp>
#include <osg/core/Referenced.hpp>
#include <osgDB/Export>
#include <string>

namespace osgDB
{

    class Archive;

    class AuthenticationDetails : public osg::Referenced
    {
        public:

            /** Http authentication techniques, see libcurl docs for details on names and
             * associated functionality.*/
            enum HttpAuthentication
            {
                BASIC        = 1 << 0,
                DIGEST       = 1 << 1,
                NEGOTIATE    = 1 << 2,
                GSSNegotiate = NEGOTIATE,
                NTLM         = 1 << 3,
                DIGEST_IE    = 1 << 4,
                NTLM_WB      = 1 << 5,
                ONLY         = 1 << 31,
                ANY          = ~( DIGEST_IE ),
                ANYSAFE      = ~( BASIC | DIGEST_IE ),
            };

            AuthenticationDetails( const std::string& u,
                                   const std::string& p,
                                   HttpAuthentication auth = BASIC ) :
                username( u ),
                password( p ),
                httpAuthentication( auth )
            {
            }

            std::string        username;
            std::string        password;
            HttpAuthentication httpAuthentication;

        protected:

            virtual ~AuthenticationDetails()
            {
            }
    };

    class OSGDB_EXPORT AuthenticationMap : public osg::Referenced
    {
        public:

            AuthenticationMap()
            {
            }

            virtual void
            addAuthenticationDetails( const std::string&     path,
                                      AuthenticationDetails* details );

            virtual const AuthenticationDetails*
            getAuthenticationDetails( const std::string& path ) const;

        protected:

            virtual ~AuthenticationMap()
            {
            }

            typedef std::map<std::string, osg::ref_ptr<AuthenticationDetails>>
                                     AuthenticationDetailsMap;
            AuthenticationDetailsMap _authenticationMap;
    };

}
