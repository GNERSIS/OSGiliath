/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract event handler base. Provides handle() method receiving
 * Event and NodeVisitor for scene-graph-attached event processing.
 */
#include <osgGA/events/EventVisitor.hpp>
#include <osgGA/events/GUIEventHandler.hpp>

using namespace osgGA;

void
EventHandler::operator()( osg::Node*        node,
                          osg::NodeVisitor* nv )
{
    osgGA::EventVisitor* ev = nv->asEventVisitor();
    if( ev && ev->getActionAdapter() && !ev->getEvents().empty() )
    {
        for( osgGA::EventQueue::Events::iterator itr = ev->getEvents().begin();
             itr != ev->getEvents().end();
             ++itr )
        {
            handle( itr->get(), node, nv );
        }
    }
    if( node->getNumChildrenRequiringEventTraversal() > 0 || _nestedCallback.valid() )
    {
        traverse( node, nv );
    }
}

void
EventHandler::event( osg::NodeVisitor* nv,
                     osg::Drawable*    drawable )
{
    osgGA::EventVisitor* ev = nv->asEventVisitor();
    if( ev && ev->getActionAdapter() && !ev->getEvents().empty() )
    {
        for( osgGA::EventQueue::Events::iterator itr = ev->getEvents().begin();
             itr != ev->getEvents().end();
             ++itr )
        {
            handle( itr->get(), drawable, nv );
        }
    }
}

bool
EventHandler::handle( osgGA::Event* event,
                      osg::Object* /*object*/,
                      osg::NodeVisitor* /*nv*/ )
{
    OSG_NOTICE << "Handle event " << event << std::endl;
    return false;
}
