/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Timeline action that blends an animation out over a duration.
 * Ramps the animation weight from current to 0.
 */
#include <osgAnimation/core/ActionBlendOut.hpp>

using namespace osgAnimation;

ActionBlendOut::ActionBlendOut() :
    _weight( 0 )
{
}

ActionBlendOut::ActionBlendOut( const ActionBlendOut& a,
                                const osg::CopyOp&    c ) :
    Action( a,
            c )
{
    _weight    = a._weight;
    _animation = a._animation;
}

ActionBlendOut::ActionBlendOut( Animation* animation,
                                double     duration )
{
    _animation = animation;
    float d    = static_cast<float>( duration * _fps );
    setNumFrames( static_cast<unsigned int>( floor( d ) + 1 ) );
    _weight = 1.0;
    setName( "BlendOut" );
}

void
ActionBlendOut::computeWeight( unsigned int frame )
{
    double ratio = ( ( frame + 1 ) * 1.0 / ( getNumFrames() ) );
    double w     = _weight * ( 1.0 - ratio );
    OSG_DEBUG << getName() << " BlendOut frame " << frame << " weight " << w
              << std::endl;
    _animation->setWeight( static_cast<float>( w ) );
}
