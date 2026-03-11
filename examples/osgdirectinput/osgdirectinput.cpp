/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgdirectinput example application
 */
#include "DirectInputRegistry"

#include <iostream>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgViewer/api/Win32/GraphicsWindowWin32>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

class CustomViewer : public osgViewer::Viewer
{
    public:

        CustomViewer() :
            osgViewer::Viewer()
        {
        }

        virtual ~CustomViewer()
        {
        }

        virtual void
        eventTraversal()
        {
            DirectInputRegistry::instance()->updateState( _eventQueue.get() );
            osgViewer::Viewer::eventTraversal();
        }

    protected:

        virtual void
        viewerInit()
        {
            osgViewer::GraphicsWindowWin32* windowWin32 =
                dynamic_cast<osgViewer::GraphicsWindowWin32*>(
                    _camera->getGraphicsContext()
                );
            if( windowWin32 )
            {
                HWND hwnd = windowWin32->getHWND();
                DirectInputRegistry::instance()->initKeyboard( hwnd );
                // DirectInputRegistry::instance()->initMouse( hwnd );
                DirectInputRegistry::instance()->initJoystick( hwnd );
            }
            osgViewer::Viewer::viewerInit();
        }
};

class JoystickHandler : public osgGA::GUIEventHandler
{
    public:

        JoystickHandler()
        {
        }

        bool
        handle( const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter&      aa )
        {
            switch( ea.getEventType() )
            {
                case osgGA::GUIEventAdapter::KEYDOWN :
                    std::cout << "*** Key 0x" << std::hex << ea.getKey() << std::dec
                              << " down ***" << std::endl;
                    break;
                case osgGA::GUIEventAdapter::KEYUP :
                    std::cout << "*** Key 0x" << std::hex << ea.getKey() << std::dec
                              << " up ***" << std::endl;
                    break;
                case osgGA::GUIEventAdapter::USER :
                    {
                        const JoystickEvent* event =
                            dynamic_cast<const JoystickEvent*>( ea.getUserData() );
                        if( !event )
                        {
                            break;
                        }

                        const DIJOYSTATE2& js = event->_js;
                        for( unsigned int i = 0; i < 128; ++i )
                        {
                            if( js.rgbButtons[i] )
                            {
                                std::cout << "*** Joystick Btn" << i << " = "
                                          << ( int )js.rgbButtons[i] << std::endl;
                            }
                        }

                        if( js.lX == 0X00'00 )
                        {
                            std::cout << "*** Joystick X-" << std::endl;
                        }
                        else if( js.lX == 0XFF'FF )
                        {
                            std::cout << "*** Joystick X+" << std::endl;
                        }

                        if( js.lY == 0 )
                        {
                            std::cout << "*** Joystick Y-" << std::endl;
                        }
                        else if( js.lY == 0XFF'FF )
                        {
                            std::cout << "*** Joystick Y+" << std::endl;
                        }
                    }
                    return true;
                default :
                    break;
            }
            return false;
        }
};

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    // Headless capture: early check before viewer construction
    {
        std::string headlessOutput;
        if( arguments.read( "--headless", headlessOutput ) )
        {
            auto node = osgDB::readRefNodeFiles( arguments );
            if( !node )
            {
                node = osgDB::readRefNodeFile( "duck.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    osg::Node* model = osgDB::readNodeFiles( arguments );
    if( !model )
    {
        model = osgDB::readNodeFile( "cow.glb" );
    }
    if( !model )
    {
        std::cout << arguments.getApplicationName() << ": No data loaded" << std::endl;
        return 1;
    }

    CustomViewer viewer;
    viewer.addEventHandler( new JoystickHandler );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.addEventHandler(
        new osgGA::StateSetManipulator( viewer.getCamera()->getOrCreateStateSet() )
    );
    viewer.setSceneData( model );
    viewer.setUpViewInWindow( 1'000, 100, 640, 480 );
    return viewer.run();
}
