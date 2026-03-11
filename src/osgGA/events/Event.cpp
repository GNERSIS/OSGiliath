/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Base event class carrying a timestamp. Foundation for
 * GUIEventAdapter and custom application events.
 */
#include <osgGA/events/Event.hpp>

using namespace osgGA;

Event::Event() :
    _handled( false ),
    _time( 0.0 )
{
}

Event::Event( const Event&       rhs,
              const osg::CopyOp& copyop ) :
    Inherit( rhs,
             copyop ),
    _handled( rhs._handled ),
    _time( rhs._time )
{
}
