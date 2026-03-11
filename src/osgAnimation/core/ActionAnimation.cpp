/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Timeline action that plays an Animation clip.
 * Wraps Animation with timeline start/duration scheduling.
 */
#include <osgAnimation/core/ActionAnimation.hpp>

using namespace osgAnimation;

ActionAnimation::ActionAnimation()
{
}

ActionAnimation::ActionAnimation( const ActionAnimation& a,
                                  const osg::CopyOp&     c ) :
    Action( a,
            c )
{
    _animation = a._animation;
}

ActionAnimation::ActionAnimation( Animation* animation ) :
    _animation( animation )
{
    Action::setDuration( animation->getDuration() );
    setName( animation->getName() );
}

void
ActionAnimation::updateAnimation( unsigned int frame,
                                  int          priority )
{
    _animation->update( frame * 1.0 / _fps, priority );
}
