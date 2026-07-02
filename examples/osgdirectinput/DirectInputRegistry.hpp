/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * DirectInputRegistry example application
 */
#ifndef OSGDIRECTINPUT_H
#define OSGDIRECTINPUT_H

#define DIRECTINPUT_VERSION 0X08'00
#include <dinput.h>
#include <iostream>
#include <windows.h>

class JoystickEvent : public osg::Referenced
{
    public:

        JoystickEvent()
        {
        }

        virtual ~JoystickEvent()
        {
        }

        DIJOYSTATE2 _js;
};

class DirectInputRegistry : public osg::Referenced
{
    public:

        static DirectInputRegistry*
        instance()
        {
            static osg::ref_ptr<DirectInputRegistry> s_registry =
                new DirectInputRegistry;
            return s_registry.get();
        }

        LPDIRECTINPUT8&
        getDevice()
        {
            return _inputDevice;
        }

        LPDIRECTINPUTDEVICE8&
        getKeyboard()
        {
            return _keyboard;
        }

        LPDIRECTINPUTDEVICE8&
        getMouse()
        {
            return _mouse;
        }

        LPDIRECTINPUTDEVICE8&
        getJoyStick()
        {
            return _joystick;
        }

        bool
        valid() const
        {
            return _supportDirectInput;
        }

        bool
        initKeyboard( HWND handle );
        bool
        initMouse( HWND handle );
        bool
        initJoystick( HWND handle );

        void
        updateState( osgGA::EventQueue* eventQueue );

    protected:

        DirectInputRegistry();
        virtual ~DirectInputRegistry();

        bool
        initImplementation( HWND                 handle,
                            LPDIRECTINPUTDEVICE8 device,
                            LPCDIDATAFORMAT      format );
        void
        pollDevice( LPDIRECTINPUTDEVICE8 device );
        void
        releaseDevice( LPDIRECTINPUTDEVICE8 device );

        static BOOL CALLBACK
                             EnumJoysticksCallback( const DIDEVICEINSTANCE* didInstance,
                                                    VOID* );

        LPDIRECTINPUT8       _inputDevice;
        LPDIRECTINPUTDEVICE8 _keyboard;
        LPDIRECTINPUTDEVICE8 _mouse;
        LPDIRECTINPUTDEVICE8 _joystick;
        bool                 _supportDirectInput;
};

#endif
