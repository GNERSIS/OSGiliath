/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * PacketListener class.
 * Provides: ProcessPacket.
 */
#pragma once

class IpEndpointName;

class PacketListener
{
    public:

        virtual ~PacketListener()
        {
        }

        virtual void
        ProcessPacket( const char*           data,
                       int                   size,
                       const IpEndpointName& remoteEndpoint ) = 0;
};
