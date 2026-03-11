/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osgPlugins: GetHostByName.
 */
#include "ip/NetworkingUtils.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winsock2.h>    // this must come first to prevent errors with MSVC7

static LONG initCount_          = 0;
static bool winsockInitialized_ = false;

NetworkInitializer::NetworkInitializer()
{
    if( InterlockedIncrement( &initCount_ ) == 1 )
    {
        // there is a race condition here if one thread tries to access
        // the library while another is still initializing it.
        // i can't think of an easy way to fix it so i'm telling you here
        // in case you need to init the library from two threads at once.
        // this is why the header file advises to instantiate one of these
        // in main() so that the initialization happens globally

        // initialize winsock
        WSAData wsaData;
        int     nCode = WSAStartup( MAKEWORD( 1, 1 ), &wsaData );
        if( nCode != 0 )
        {
            // std::cout << "WSAStartup() failed with error code " << nCode << "\n";
        }
        else
        {
            winsockInitialized_ = true;
        }
    }
}

NetworkInitializer::~NetworkInitializer()
{
    if( InterlockedDecrement( &initCount_ ) == 0 )
    {
        if( winsockInitialized_ )
        {
            WSACleanup();
            winsockInitialized_ = false;
        }
    }
}

unsigned long
GetHostByName( const char* name )
{
    NetworkInitializer networkInitializer;

    unsigned long      result = 0;

    struct hostent*    h      = gethostbyname( name );
    if( h )
    {
        struct in_addr a;
        memcpy( &a, h->h_addr_list[0], h->h_length );
        result = ntohl( a.s_addr );
    }

    return result;
}
