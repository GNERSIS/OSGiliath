/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * OscPacketListener, derived from PacketListener.
 * Provides: ProcessPacket, p, ProcessBundle, ProcessMessage.
 */
#pragma once

#include "../ip/PacketListener.hpp"
#include "OscReceivedElements.hpp"

namespace osc
{

    class OscPacketListener : public PacketListener
    {
        protected:

            virtual void
            ProcessBundle( const osc::ReceivedBundle& b,
                           const IpEndpointName&      remoteEndpoint )
            {
                // ignore bundle time tag for now

                for( ReceivedBundle::const_iterator i = b.ElementsBegin();
                     i != b.ElementsEnd();
                     ++i )
                {
                    if( i->IsBundle() )
                    {
                        ProcessBundle( ReceivedBundle( *i ), remoteEndpoint );
                    }
                    else
                    {
                        ProcessMessage( ReceivedMessage( *i ), remoteEndpoint );
                    }
                }
            }

            virtual void
            ProcessMessage( const osc::ReceivedMessage& m,
                            const IpEndpointName&       remoteEndpoint ) = 0;

        public:

            virtual void
            ProcessPacket( const char*           data,
                           int                   size,
                           const IpEndpointName& remoteEndpoint )
            {
                osc::ReceivedPacket p( data, size );
                if( p.IsBundle() )
                {
                    ProcessBundle( ReceivedBundle( p ), remoteEndpoint );
                }
                else
                {
                    ProcessMessage( ReceivedMessage( p ), remoteEndpoint );
                }
            }
    };

}    // namespace osc
