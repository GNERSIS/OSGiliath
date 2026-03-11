/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Visitor that distributes events to handlers during the event
 * traversal pass of the viewer frame loop.
 */
#include <osgGA/events/EventVisitor.hpp>

#include <algorithm>

using namespace osg;
using namespace osgGA;

EventVisitor::EventVisitor() :
    DualModeVisitor( EVENT_VISITOR,
                     TRAVERSE_ACTIVE_CHILDREN ),
    _actionAdapter( 0 ),
    _handled( false )
{
}

EventVisitor::~EventVisitor()
{
}

void
EventVisitor::addEvent( Event* event )
{
    _events.push_back( event );
}

void
EventVisitor::removeEvent( Event* event )
{
    EventQueue::Events::iterator itr =
        std::find( _events.begin(), _events.end(), event );
    if( itr != _events.end() )
    {
        _events.erase( itr );
    }
}

void
EventVisitor::reset()
{
    _events.clear();
    _handled = false;
}
