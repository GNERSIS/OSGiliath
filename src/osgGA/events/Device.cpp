/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract input/output device interface. Provides event source
 * and event sink for custom hardware integration.
 */
#include <osgGA/events/Device.hpp>

using namespace osgGA;

Device::Device() :
    _capabilities( UNKNOWN )
{
    setEventQueue( new EventQueue );
}

Device::Device( const Device&      es,
                const osg::CopyOp& copyop ) :
    Inherit( es,
             copyop ),
    _capabilities( es._capabilities )
{
    setEventQueue( new EventQueue );
}

void
Device::sendEvent( const Event& /*event*/ )
{
    OSG_WARN << "Device::sendEvent not implemented!" << std::endl;
}

void
Device::sendEvents( const EventQueue::Events& events )
{
    for( EventQueue::Events::const_iterator i = events.begin(); i != events.end(); i++ )
    {
        sendEvent( **i );
    }
}

Device::~Device()
{
}
