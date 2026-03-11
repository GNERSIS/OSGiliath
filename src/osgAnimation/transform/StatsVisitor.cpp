/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Visitor collecting animation statistics (channel counts,
 * active animations, bone counts) from the scene.
 */
#include <osgAnimation/transform/StatsVisitor.hpp>

#include <osgAnimation/core/ActionAnimation.hpp>
#include <osgAnimation/core/ActionBlendIn.hpp>
#include <osgAnimation/core/ActionBlendOut.hpp>
#include <osgAnimation/core/ActionStripAnimation.hpp>
#include <osgAnimation/core/Timeline.hpp>

using namespace osgAnimation;

StatsActionVisitor::StatsActionVisitor()
{
}

void
StatsActionVisitor::reset()
{
    _channels.clear();
}

StatsActionVisitor::StatsActionVisitor( osg::Stats*  stats,
                                        unsigned int frame )
{
    _frame = frame;
    _stats = stats;
}

void
StatsActionVisitor::apply( Timeline& tm )
{
    _stats->setAttribute( _frame, "Timeline", tm.getCurrentTime() );
    tm.traverse( *this );
}

void
StatsActionVisitor::apply( Action& action )
{
    if( isActive( action ) )
    {
        _channels.push_back( action.getName() );
        _stats->setAttribute( _frame, action.getName(), 1 );
    }
}

void
StatsActionVisitor::apply( ActionBlendIn& action )
{
    if( isActive( action ) )
    {
        _channels.push_back( action.getName() );
        _stats->setAttribute( _frame, action.getName(), action.getWeight() );
    }
}

void
StatsActionVisitor::apply( ActionBlendOut& action )
{
    if( isActive( action ) )
    {
        _channels.push_back( action.getName() );
        _stats->setAttribute( _frame, action.getName(), action.getWeight() );
    }
}

void
StatsActionVisitor::apply( ActionAnimation& action )
{
    if( isActive( action ) )
    {
        _channels.push_back( action.getName() );
        _stats->setAttribute( _frame,
                              action.getName(),
                              action.getAnimation()->getWeight() );
    }
}

void
StatsActionVisitor::apply( ActionStripAnimation& action )
{
    if( isActive( action ) )
    {
        _channels.push_back( action.getName() );
        _stats->setAttribute( _frame,
                              action.getName(),
                              action.getAnimation()->getAnimation()->getWeight() );
    }
}
