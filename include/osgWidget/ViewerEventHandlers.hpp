/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Event handlers bridging osgViewer events to osgWidget.
 * Routes mouse/keyboard events to the WindowManager.
 */
// Code by: Jeremy Moles (cubicool) 2007-2008

#pragma once

#include <osgGA/events/GUIEventAdapter.hpp>
#include <osgGA/events/GUIEventHandler.hpp>
#include <osgWidget/WindowManager.hpp>

// NOTE! These are all just examples of some default event handlers--they are not
// required. You are more than welcome to provide your own even handlers that
// communicate with a WindowManager using it's public API.

namespace osgWidget
{

    // This handles the pressing/moving of mouse buttons, etc.
    class OSGWIDGET_EXPORT MouseHandler : public osgGA::GUIEventHandler
    {
        public:

            MouseHandler( WindowManager* );

            virtual bool
                         handle( const osgGA::GUIEventAdapter&,
                                 osgGA::GUIActionAdapter&,
                                 osg::Object*,
                                 osg::NodeVisitor* );

            typedef bool ( MouseHandler::*MouseAction )( float,
                                                         float,
                                                         int );
            typedef bool ( WindowManager::*MouseEvent )( float,
                                                         float );

        protected:

            osg::observer_ptr<WindowManager> _wm;

            bool
            _handleMousePush( float,
                              float,
                              int );
            bool
            _handleMouseRelease( float,
                                 float,
                                 int );
            bool
            _handleMouseDoubleClick( float,
                                     float,
                                     int );
            bool
            _handleMouseDrag( float,
                              float,
                              int );
            bool
            _handleMouseMove( float,
                              float,
                              int );
            bool
                        _handleMouseScroll( float,
                                            float,
                                            int );

            MouseAction _isMouseEvent( osgGA::GUIEventAdapter::EventType ) const;
            bool
            _doMouseEvent( float,
                           float,
                           MouseEvent );
    };

    // This handles the forwarding of keypress events.
    class OSGWIDGET_EXPORT KeyboardHandler : public osgGA::GUIEventHandler
    {
        public:

            KeyboardHandler( WindowManager* );

            virtual bool
            handle( const osgGA::GUIEventAdapter&,
                    osgGA::GUIActionAdapter&,
                    osg::Object*,
                    osg::NodeVisitor* );

        protected:

            osg::observer_ptr<WindowManager> _wm;
    };

    // This class offers a default kind of handling for resizing.
    class OSGWIDGET_EXPORT ResizeHandler : public osgGA::GUIEventHandler
    {
        public:

            ResizeHandler( WindowManager*,
                           osg::Camera* = 0 );

            virtual bool
            handle( const osgGA::GUIEventAdapter&,
                    osgGA::GUIActionAdapter&,
                    osg::Object*,
                    osg::NodeVisitor* );

        protected:

            osg::observer_ptr<WindowManager> _wm;
            osg::observer_ptr<osg::Camera>   _camera;
    };

    // This class provides a hotkey that lets you toggle back and forth between
    // a camera and setting the CameraManipulator's home point.
    class OSGWIDGET_EXPORT CameraSwitchHandler : public osgGA::GUIEventHandler
    {
        public:

            CameraSwitchHandler( WindowManager*,
                                 osg::Camera* );

            virtual bool
            handle( const osgGA::GUIEventAdapter&,
                    osgGA::GUIActionAdapter&,
                    osg::Object*,
                    osg::NodeVisitor* );

        protected:

            osg::observer_ptr<WindowManager> _wm;
            osg::observer_ptr<osg::Camera>   _camera;
            osg::ref_ptr<osg::Node>          _oldNode;
    };

}
