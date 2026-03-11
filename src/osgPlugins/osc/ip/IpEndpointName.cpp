/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * IpEndpointName — osgPlugins library implementation.
 */
#include "IpEndpointName.hpp"
#include "NetworkingUtils.hpp"

#include <cstdio>

unsigned long
IpEndpointName::GetHostByName( const char* s )
{
    return ::GetHostByName( s );
}

void
IpEndpointName::AddressAsString( char* s ) const
{
    if( address == ANY_ADDRESS )
    {
        sprintf( s, "<any>" );
    }
    else
    {
        sprintf( s,
                 "%d.%d.%d.%d",
                 ( int )( ( address >> 24 ) & 0XFF ),
                 ( int )( ( address >> 16 ) & 0XFF ),
                 ( int )( ( address >> 8 ) & 0XFF ),
                 ( int )( address & 0XFF ) );
    }
}

void
IpEndpointName::AddressAndPortAsString( char* s ) const
{
    if( port == ANY_PORT )
    {
        if( address == ANY_ADDRESS )
        {
            sprintf( s, "<any>:<any>" );
        }
        else
        {
            sprintf( s,
                     "%d.%d.%d.%d:<any>",
                     ( int )( ( address >> 24 ) & 0XFF ),
                     ( int )( ( address >> 16 ) & 0XFF ),
                     ( int )( ( address >> 8 ) & 0XFF ),
                     ( int )( address & 0XFF ) );
        }
    }
    else
    {
        if( address == ANY_ADDRESS )
        {
            sprintf( s, "<any>:%d", port );
        }
        else
        {
            sprintf( s,
                     "%d.%d.%d.%d:%d",
                     ( int )( ( address >> 24 ) & 0XFF ),
                     ( int )( ( address >> 16 ) & 0XFF ),
                     ( int )( ( address >> 8 ) & 0XFF ),
                     ( int )( address & 0XFF ),
                     ( int )port );
        }
    }
}
