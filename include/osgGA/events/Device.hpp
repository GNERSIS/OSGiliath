/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract input/output device interface. Provides event source
 * and event sink for custom hardware integration.
 */
#pragma once

#include <osgGA/events/EventQueue.hpp>

namespace osgGA
{

    /**
     * Device base class from abstracting away from devices/windows that can generate
     * events.
     */
    class OSGGA_EXPORT Device : public osg::Inherit<osg::Object, Device>
    {
        public:

            enum Capabilities
            {
                UNKNOWN        = 0,
                RECEIVE_EVENTS = 1,
                SEND_EVENTS    = 2,
            };

            Device();
            Device( const Device&      es,
                    const osg::CopyOp& copyop );

            OSG_REGISTER_TYPE( osgGA,
                               Device )

            int
            getCapabilities() const
            {
                return _capabilities;
            }

            virtual bool
            checkEvents()
            {
                return _eventQueue.valid() ? !( getEventQueue()->empty() ) : false;
            }

            virtual void
            sendEvent( const Event& ea );
            virtual void
            sendEvents( const EventQueue::Events& events );

            void
            setEventQueue( osgGA::EventQueue* eventQueue )
            {
                _eventQueue = eventQueue;
            }

            osgGA::EventQueue*
            getEventQueue()
            {
                return _eventQueue.get();
            }

            const osgGA::EventQueue*
            getEventQueue() const
            {
                return _eventQueue.get();
            }

        protected:

            void
            setCapabilities( int capabilities )
            {
                _capabilities = capabilities;
            }

            virtual ~Device();

            /** Prevent unwanted copy operator.*/
            Device&
            operator=( const Device& )
            {
                return *this;
            }

            osg::ref_ptr<osgGA::EventQueue> _eventQueue;

        private:

            int _capabilities;
    };

}
