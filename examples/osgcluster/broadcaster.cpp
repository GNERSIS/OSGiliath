/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * broadcaster example application
 */
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <sys/types.h>

#if !defined( _WIN32 ) || defined( __CYGWIN__ )
    #include <arpa/inet.h>
    #include <net/if.h>
    #include <netdb.h>
    #include <netinet/in.h>
    #include <sys/ioctl.h>
    #include <sys/socket.h>
    #include <sys/time.h>
    #include <sys/uio.h>
#endif

#include <string.h>

#if defined( __linux )
    #include <linux/sockios.h>
    #include <unistd.h>
#elif defined( __FreeBSD__ ) || defined( __FreeBSD_kernel__ )
    #include <sys/sockio.h>
    #include <unistd.h>
#elif defined( __sgi )
    #include <net/soioctl.h>
    #include <unistd.h>
#elif defined( __CYGWIN__ )
    #include <unistd.h>
#elif defined( __GNU__ )
    #include <unistd.h>
#elif defined( __sun )
    #include <sys/sockio.h>
    #include <unistd.h>
#elif defined( __APPLE__ )
    #include <sys/sockio.h>
    #include <unistd.h>
#elif defined( _WIN32 )
    #include <stdio.h>
    #include <winsock.h>
#elif defined( __hpux )
    #include <unistd.h>
#else
    #error Teach me how to build on this system
#endif

#include "broadcaster.hpp"

#define _VERBOSE 1

Broadcaster::Broadcaster( void )
{
#if defined( __linux ) || defined( __CYGWIN__ )
    _ifr_name = "eth0";
#elif defined( __sun )
    _ifr_name = "hme0";
#elif !defined( _WIN32 )
    _ifr_name = "ef0";
#endif
    _port        = 0;
    _initialized = false;
    _buffer      = 0L;
    _address     = 0;
}

Broadcaster::~Broadcaster( void )
{
#if defined( _WIN32 ) && !defined( __CYGWIN__ )
    closesocket( _so );
#else
    close( _so );
#endif
}

bool
Broadcaster::init( void )
{
#if defined( _WIN32 ) && !defined( __CYGWIN__ )
    WORD    version = MAKEWORD( 1, 1 );
    WSADATA wsaData;
    // First, we start up Winsock
    WSAStartup( version, &wsaData );
#endif

    if( _port == 0 )
    {
        fprintf( stderr, "Broadcaster::init() - port not defined\n" );
        return false;
    }

    if( ( _so = socket( AF_INET, SOCK_DGRAM, 0 ) ) < 0 )
    {
        perror( "Socket" );
        return false;
    }
#if defined( _WIN32 ) && !defined( __CYGWIN__ )
    const BOOL on = TRUE;
#else
    int on = 1;
#endif

#if defined( _WIN32 ) && !defined( __CYGWIN__ )
    setsockopt( _so, SOL_SOCKET, SO_REUSEADDR, ( const char* )&on, sizeof( int ) );
#else
    setsockopt( _so, SOL_SOCKET, SO_REUSEADDR, &on, sizeof( on ) );
#endif

    saddr.sin_family = AF_INET;
    saddr.sin_port   = htons( _port );
    if( _address == 0 )
    {
#if defined( _WIN32 ) && !defined( __CYGWIN__ )
        setsockopt( _so, SOL_SOCKET, SO_BROADCAST, ( const char* )&on, sizeof( int ) );
#else
        setsockopt( _so, SOL_SOCKET, SO_BROADCAST, &on, sizeof( on ) );
#endif

#if !defined( _WIN32 ) || defined( __CYGWIN__ )
        struct ifreq ifr;
        strcpy( ifr.ifr_name, _ifr_name.c_str() );
#endif
#if defined( _WIN32 )    // get the server address
        saddr.sin_addr.s_addr = htonl( INADDR_BROADCAST );
    }
#else
        if( ( ioctl( _so, SIOCGIFBRDADDR, &ifr ) ) < 0 )
        {
            printf( " ifr.ifr_name = %s\n", ifr.ifr_name );
            perror( "Broadcaster::init() Cannot get Broadcast Address" );
            return false;
        }
        saddr.sin_addr.s_addr =
            ( ( ( sockaddr_in* )&ifr.ifr_broadaddr )->sin_addr.s_addr );
    }
    else
    {
        saddr.sin_addr.s_addr = _address;
    }
#endif
#define _VERBOSE 1
#ifdef _VERBOSE
    unsigned char* ptr = ( unsigned char* )&saddr.sin_addr.s_addr;
    printf( "Broadcast address : %u.%u.%u.%u\n", ptr[0], ptr[1], ptr[2], ptr[3] );
#endif

    _initialized = true;
    return _initialized;
}

void
Broadcaster::setIFRName( const std::string& name )
{
    _ifr_name = name;
}

void
Broadcaster::setHost( const char* hostname )
{
    struct hostent* h;
    if( ( h = gethostbyname( hostname ) ) == 0L )
    {
        fprintf( stderr,
                 "Broadcaster::setHost() - Cannot resolve an address for \"%s\".\n",
                 hostname );
        _address = 0;
    }
    else
    {
        _address = *( ( unsigned long* )h->h_addr );
    }
}

void
Broadcaster::setPort( const short port )
{
    _port = port;
}

void
Broadcaster::setBuffer( void*              buffer,
                        const unsigned int size )
{
    _buffer      = buffer;
    _buffer_size = size;
}

void
Broadcaster::sync( void )
{
    if( !_initialized )
    {
        init();
    }

    if( _buffer == 0L )
    {
        fprintf( stderr, "Broadcaster::sync() - No buffer\n" );
        return;
    }

#if defined( _WIN32 ) && !defined( __CYGWIN__ )
    unsigned int size = sizeof( SOCKADDR_IN );
    sendto( _so,
            ( const char* )_buffer,
            _buffer_size,
            0,
            ( struct sockaddr* )&saddr,
            size );
    int err = WSAGetLastError();
    if( err != 0 )
    {
        fprintf( stderr, "Broadcaster::sync() - error %d\n", err );
    }
#else
    unsigned int size = sizeof( struct sockaddr_in );
    sendto( _so,
            ( const void* )_buffer,
            _buffer_size,
            0,
            ( struct sockaddr* )&saddr,
            size );
    std::cout << "sento(" << _buffer << ", " << _buffer_size << std::endl;
#endif
}
