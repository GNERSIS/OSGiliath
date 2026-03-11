/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * JoystickDevice, derived from Device.
 * Provides: checkEvents, addMouseButtonMapping, getMouseButtonMapping, addKeyMapping,
 * getKeyMapping.
 */
#pragma once

#include <map>
#include <osgGA/events/Device.hpp>
#include <SDL.h>
#include <SDL_events.h>
#include <SDL_joystick.h>
#include <vector>

class JoystickDevice : public osgGA::Device
{
    public:

        JoystickDevice();

        typedef std::vector<int>   ValueList;
        typedef std::map<int, int> ButtonMap;

        virtual bool
        checkEvents();

        void
        addMouseButtonMapping( int joystickButton,
                               int mouseButton )
        {
            _mouseButtonMap[joystickButton] = mouseButton;
        }

        int
        getMouseButtonMapping( int joystickButton )
        {
            ButtonMap::const_iterator itr = _mouseButtonMap.find( joystickButton );
            if( itr != _mouseButtonMap.end() )
            {
                return itr->second;
            }
            else
            {
                return -1;
            }
        }

        void
        addKeyMapping( int joystickButton,
                       int key )
        {
            _keyMap[joystickButton] = key;
        }

        int
        getKeyMapping( int joystickButton )
        {
            ButtonMap::const_iterator itr = _keyMap.find( joystickButton );
            if( itr != _keyMap.end() )
            {
                return itr->second;
            }
            else
            {
                return -1;
            }
        }

    protected:

        virtual ~JoystickDevice();

        void
                      capture( ValueList& axisValues,
                               ValueList& buttonValues ) const;

        SDL_Joystick* _joystick;
        int           _numAxes;
        int           _numBalls;
        int           _numHats;
        int           _numButtons;
        bool          _verbose;

        ValueList     _axisValues;
        ValueList     _buttonValues;
        ButtonMap     _mouseButtonMap;
        ButtonMap     _keyMap;
};
