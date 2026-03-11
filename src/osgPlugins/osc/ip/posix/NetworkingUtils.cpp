/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osgPlugins: GetHostByName.
 */
#include "ip/NetworkingUtils.hpp"

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

NetworkInitializer::NetworkInitializer()
{
}

NetworkInitializer::~NetworkInitializer()
{
}

unsigned long
GetHostByName( const char* name )
{
    unsigned long   result = 0;

    struct hostent* h      = gethostbyname( name );
    if( h )
    {
        struct in_addr a;
        memcpy( &a, h->h_addr_list[0], h->h_length );
        result = ntohl( a.s_addr );
    }

    return result;
}
