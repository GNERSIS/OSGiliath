/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * ZeroConfRegisterDevice, derived from Device.
 * Provides: setCapabilities, advertise, checkEvents, sendEvent, advertise,
 * ZeroConfDiscoverDevice.
 */
#include "AutoDiscovery.hpp"

#include <osg/core/UserDataContainer.hpp>
#include <osg/core/ValueObject.hpp>
#include <osgDB/io/FileNameUtils.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/registry/Registry.hpp>
#include <osgGA/events/Device.hpp>

class ZeroConfRegisterDevice : public osgGA::Device
{
    public:

        ZeroConfRegisterDevice() :
            osgGA::Device(),
            _autoDiscovery( new AutoDiscovery() )
        {
            setCapabilities( RECEIVE_EVENTS );
        }

        void
        advertise( const std::string& type,
                   unsigned int       port )
        {
            OSG_NOTICE << "ZeroConfDevice :: advertise: " << type << ":" << port
                       << std::endl;
            _autoDiscovery->registerService( type, port );
        }

        virtual bool
        checkEvents()
        {
            _autoDiscovery->update();
            return !( getEventQueue()->empty() );
        }

        virtual void
        sendEvent( const osgGA::Event& event )
        {
            if( event.getName() == "/zeroconf/advertise" )
            {
                std::string  type;
                unsigned int port = 0;
                event.getUserValue( "type", type );
                event.getUserValue( "port", port );
                if( type.empty() || ( port == 0 ) )
                {
                    OSG_WARN << "ZeroConfRegisterDevice :: could not advertise service, "
                                "missing type/port "
                             << std::endl;
                }
                else
                {
                    advertise( type, port );
                }
            }
        }

    private:

        osg::ref_ptr<AutoDiscovery> _autoDiscovery;
};

class ZeroConfDiscoverDevice : public osgGA::Device
{
    public:

        ZeroConfDiscoverDevice( const std::string& type );

        virtual bool
        checkEvents()
        {
            _autoDiscovery->update();
            return !( getEventQueue()->empty() );
        }

    private:

        osg::ref_ptr<AutoDiscovery> _autoDiscovery;
};

class MyDiscoveredServicesCallback : public DiscoveredServicesCallback
{
    public:

        MyDiscoveredServicesCallback( ZeroConfDiscoverDevice* device,
                                      const std::string&      type ) :
            DiscoveredServicesCallback(),
            _device( device ),
            _type( type )
        {
        }

        virtual bool
        ignoreIP6Addresses()
        {
            return true;
        }

        virtual void
        serviceAdded( const std::string& host,
                      unsigned int       port )
        {
            osg::ref_ptr<osgGA::Event> event = new osgGA::Event();

            OSG_NOTICE << "ZeroConfDevice :: serviceAdded: " << host << ":" << port
                       << " event " << event << std::endl;

            event->setName( "/zeroconf/service-added" );
            event->setUserValue( "host", host );
            event->setUserValue( "port", port );
            event->setUserValue( "type", _type );
            event->setTime( _device->getEventQueue()->getTime() );
            _device->getEventQueue()->addEvent( event );
        }

        virtual void
        serviceRemoved( const std::string& host,
                        unsigned int       port )
        {
            osg::ref_ptr<osgGA::Event> event = new osgGA::Event();

            OSG_NOTICE << "ZeroConfDevice :: serviceRemoved: " << host << ":" << port
                       << " event " << event << std::endl;

            event->setName( "/zeroconf/service-removed" );
            event->setUserValue( "host", host );
            event->setUserValue( "port", port );
            event->setUserValue( "type", _type );
            event->setTime( _device->getEventQueue()->getTime() );
            _device->getEventQueue()->addEvent( event );
        }

    private:

        osg::observer_ptr<ZeroConfDiscoverDevice> _device;
        std::string                               _type;
};

ZeroConfDiscoverDevice::ZeroConfDiscoverDevice( const std::string& type ) :
    osgGA::Device(),
    _autoDiscovery( new AutoDiscovery() )
{
    setCapabilities( RECEIVE_EVENTS );
    _autoDiscovery->discoverServices( type,
                                      new MyDiscoveredServicesCallback( this, type ) );
}

class ReaderWriterZeroConf : public osgDB::ReaderWriter
{
    public:

        ReaderWriterZeroConf()
        {
            supportsExtension(
                "zeroconf",
                "zeroconf plugin to advertise ip-services and discover them"
            );
        }

        virtual const char*
        className() const
        {
            return "ZeroConf Virtual Device Integration plugin";
        }

        virtual ReadResult
        readObject( const std::string&                  file,
                    const osgDB::ReaderWriter::Options* options = NULL ) const
        {
            if( osgDB::getFileExtension( file ) == "zeroconf" )
            {
                std::string file_name = osgDB::getNameLessExtension( file );

                if( osgDB::getFileExtension( file_name ) == "discover" )
                {
                    std::string type = osgDB::getNameLessExtension( file_name );
                    return new ZeroConfDiscoverDevice( type );
                }
                else if( osgDB::getFileExtension( file_name ) == "advertise" )
                {
                    file_name        = osgDB::getNameLessExtension( file_name );

                    std::string type = file_name.substr( 0, file_name.find( ':' ) );
                    std::string port = file_name.substr( file_name.find( ':' ) + 1 );

                    ZeroConfRegisterDevice* device = new ZeroConfRegisterDevice();
                    device->advertise( type, atoi( port.c_str() ) );

                    return device;
                }
            }

            return ReadResult::FILE_NOT_HANDLED;
        }

    private:
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN( zeroconf,
                    ReaderWriterZeroConf )
