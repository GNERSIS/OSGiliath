/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Base class for GUI event handlers. Receives keyboard, mouse,
 * and window events from the viewer's event queue.
 */
#include <osgGA/events/GUIEventHandler.hpp>

#include <osgGA/events/EventVisitor.hpp>

using namespace osgGA;

GUIEventHandler::~GUIEventHandler()
{
}

// adapt EventHandler usage to old style GUIEventHandler usage
bool
GUIEventHandler::handle( osgGA::Event*     event,
                         osg::Object*      object,
                         osg::NodeVisitor* nv )
{
    osgGA::EventVisitor*    ev = nv->asEventVisitor();
    osgGA::GUIEventAdapter* ea = event->asGUIEventAdapter();
    if( ea && ev && ev->getActionAdapter() )
    {
#if 1
        bool handled = handle( *ea, *( ev->getActionAdapter() ), object, nv );
        if( handled )
        {
            ea->setHandled( true );
        }
        return handled;
#else
        return handleWithCheckAgainstIgnoreHandledEventsMask(
            *ea,
            *( ev->getActionAdapter() ),
            object,
            nv
        );
#endif
    }
    return false;
}
