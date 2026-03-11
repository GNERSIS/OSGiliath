/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Timeline action that blends an animation in over a duration.
 * Ramps the animation weight from 0 to target.
 */
#include <osgAnimation/core/ActionBlendIn.hpp>

using namespace osgAnimation;

ActionBlendIn::ActionBlendIn() :
    _weight( 0 )
{
}

ActionBlendIn::ActionBlendIn( const ActionBlendIn& a,
                              const osg::CopyOp&   c ) :
    Action( a,
            c )
{
    _weight    = a._weight;
    _animation = a._animation;
}

ActionBlendIn::ActionBlendIn( Animation* animation,
                              double     duration,
                              double     weight )
{
    _animation = animation;
    _weight    = weight;
    float d    = static_cast<float>( duration * _fps );
    setNumFrames( static_cast<unsigned int>( floor( d ) ) + 1 );
    setName( "BlendIn" );
}

void
ActionBlendIn::computeWeight( unsigned int frame )
{

    // frame + 1 because the start is 0 and we want to start the blend in at the first
    // frame.
    double ratio = ( ( frame + 1 ) * 1.0 / ( getNumFrames() ) );
    double w     = _weight * ratio;

    OSG_DEBUG << getName() << " BlendIn frame " << frame << " weight " << w << std::endl;
    _animation->setWeight( static_cast<float>( w ) );
}
