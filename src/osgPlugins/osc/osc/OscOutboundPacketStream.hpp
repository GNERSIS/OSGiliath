/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * OutOfBufferMemoryException, derived from Exception.
 * Provides: Exception, BundleNotInProgressException, Exception,
 * MessageInProgressException, Exception, MessageNotInProgressException.
 */
#pragma once

#include "OscException.hpp"
#include "OscTypes.hpp"

namespace osc
{

    class OutOfBufferMemoryException : public Exception
    {
        public:

            OutOfBufferMemoryException( const char* w = "out of buffer memory" ) :
                Exception( w )
            {
            }
    };

    class BundleNotInProgressException : public Exception
    {
        public:

            BundleNotInProgressException(
                const char* w = "call to EndBundle when bundle is not in progress"
            ) :
                Exception( w )
            {
            }
    };

    class MessageInProgressException : public Exception
    {
        public:

            MessageInProgressException(
                const char* w =
                    "opening or closing bundle or message while message is in progress"
            ) :
                Exception( w )
            {
            }
    };

    class MessageNotInProgressException : public Exception
    {
        public:

            MessageNotInProgressException(
                const char* w = "call to EndMessage when message is not in progress"
            ) :
                Exception( w )
            {
            }
    };

    class OutboundPacketStream
    {
        public:

            OutboundPacketStream( char*         buffer,
                                  unsigned long capacity );
            ~OutboundPacketStream();

            void
            Clear();

            unsigned int
            Capacity() const;

            // invariant: size() is valid even while building a message.
            unsigned int
            Size() const;

            const char*
            Data() const;

            // indicates that all messages have been closed with a matching EndMessage
            // and all bundles have been closed with a matching EndBundle
            bool
            IsReady() const;

            bool
            IsMessageInProgress() const;
            bool
            IsBundleInProgress() const;

            OutboundPacketStream&
            operator<<( const BundleInitiator& rhs );
            OutboundPacketStream&
            operator<<( const BundleTerminator& rhs );

            OutboundPacketStream&
            operator<<( const BeginMessage& rhs );
            OutboundPacketStream&
            operator<<( const MessageTerminator& rhs );

            OutboundPacketStream&
            operator<<( bool rhs );
            OutboundPacketStream&
            operator<<( const NilType& rhs );
            OutboundPacketStream&
            operator<<( const InfinitumType& rhs );
            OutboundPacketStream&
            operator<<( int32 rhs );

#if !( defined( __x86_64__ ) || defined( _M_X64 ) )
            OutboundPacketStream&
            operator<<( int rhs )
            {
                *this << ( int32 )rhs;
                return *this;
            }
#endif

            OutboundPacketStream&
            operator<<( float rhs );
            OutboundPacketStream&
            operator<<( char rhs );
            OutboundPacketStream&
            operator<<( const RgbaColor& rhs );
            OutboundPacketStream&
            operator<<( const MidiMessage& rhs );
            OutboundPacketStream&
            operator<<( int64 rhs );
            OutboundPacketStream&
            operator<<( const TimeTag& rhs );
            OutboundPacketStream&
            operator<<( double rhs );
            OutboundPacketStream&
            operator<<( const char* rhs );
            OutboundPacketStream&
            operator<<( const Symbol& rhs );
            OutboundPacketStream&
            operator<<( const Blob& rhs );

        private:

            char*
            BeginElement( char* beginPtr );
            void
            EndElement( char* endPtr );

            bool
            ElementSizeSlotRequired() const;
            void
            CheckForAvailableBundleSpace();
            void
            CheckForAvailableMessageSpace( const char* addressPattern );
            void
                    CheckForAvailableArgumentSpace( long argumentLength );

            char*   data_;
            char*   end_;

            char*   typeTagsCurrent_;    // stored in reverse order
            char*   messageCursor_;
            char*   argumentCurrent_;

            // elementSizePtr_ has two special values: 0 indicates that a bundle
            // isn't open, and elementSizePtr_==data_ indicates that a bundle is
            // open but that it doesn't have a size slot (ie the outermost bundle)
            uint32* elementSizePtr_;

            bool    messageIsInProgress_;
    };

}    // namespace osc
