/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract animation target receiving interpolated values.
 * Accumulates weighted contributions from multiple channels.
 */
#include <osgAnimation/core/Target.hpp>

#include <osgAnimation/core/Channel.hpp>

using namespace osgAnimation;

Target::Target() :
    _weight( 0 ),
    _priorityWeight( 0 ),
    _lastPriority( 0 )
{
}
