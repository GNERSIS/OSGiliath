/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Base class for GUI event handlers. Receives keyboard, mouse,
 * and window events from the viewer's event queue.
 */
#pragma once

#include <osg/core/ApplicationUsage.hpp>
#include <osg/core/Inherit.hpp>
#include <osg/geometry/Drawable.hpp>
#include <osgGA/events/EventHandler.hpp>
#include <osgGA/events/GUIActionAdapter.hpp>
#include <osgGA/events/GUIEventAdapter.hpp>
#include <vector>

// #define COMPILE_COMPOSITE_EVENTHANDLER

namespace osgGA
{

    /**

    GUIEventHandler provides a basic interface for any class which wants to handle
    a GUI Events.

    The GUIEvent is supplied by a GUIEventAdapter. Feedback resulting from the
    handle method is supplied by a GUIActionAdapter, which allows the GUIEventHandler
    to ask the GUI to take some action in response to an incoming event.

    For example, consider a Trackball Viewer class which takes mouse events and
    manipulates a scene camera in response. The Trackball Viewer is a GUIEventHandler,
    and receives the events via the handle method. If the user 'throws' the model,
    the Trackball Viewer class can detect this via the incoming events, and
    request that the GUI set up a timer callback to continually redraw the view.
    This request is made via the GUIActionAdapter class.

    */

    class OSGGA_EXPORT GUIEventHandler
        : public osg::Inherit<EventHandler, GUIEventHandler>
    {
        public:

            GUIEventHandler()
            {
            }

            GUIEventHandler( const GUIEventHandler& eh,
                             const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY ) :
                Inherit( eh,
                         copyop )
            {
            }

            OSG_REGISTER_TYPE( osgGA,
                               GUIEventHandler )

            /** Handle event. Override the handle(..) method in your event handlers to
             * respond to events. */
            virtual bool
            handle( osgGA::Event*     event,
                    osg::Object*      object,
                    osg::NodeVisitor* nv );

            /** Handle events, return true if handled, false otherwise. */
            virtual bool
            handle( const GUIEventAdapter& ea,
                    GUIActionAdapter&      aa,
                    osg::Object*,
                    osg::NodeVisitor* )
            {
                return handle( ea, aa );
            }

            /** Deprecated, Handle events, return true if handled, false otherwise. */
            virtual bool
            handle( const GUIEventAdapter&,
                    GUIActionAdapter& )
            {
                return false;
            }

        protected:

            virtual ~GUIEventHandler();
    };

}
