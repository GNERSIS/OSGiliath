/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * State inheritance tree built during culling. Groups render leaves
 * that share the same accumulated StateSet chain.
 */
#include <osgUtil/culling/StateGraph.hpp>

#include <osg/core/Notify.hpp>

using namespace osg;
using namespace osgUtil;

void
StateGraph::reset()
{
    _parent   = NULL;
    _stateset = NULL;

    _depth    = 0;

    _children.clear();
    _leaves.clear();
}

/** recursively clean the StateGraph of all its drawables, lights and depths.
 * Leaves children intact, and ready to be populated again.*/
void
StateGraph::clean()
{

    // clean local drawables etc.
    _leaves.clear();

    // call clean on all children.
    for( ChildList::iterator itr = _children.begin(); itr != _children.end(); ++itr )
    {
        itr->second->clean();
    }
}

/** recursively prune the StateGraph of empty children.*/
void
StateGraph::prune()
{
    // call prune on all children.
    ChildList::iterator citr = _children.begin();
    while( citr != _children.end() )
    {
        citr->second->prune();

        if( citr->second->empty() )
        {
            ChildList::iterator ditr = citr++;
            _children.erase( ditr );
        }
        else
        {
            ++citr;
        }
    }
}
