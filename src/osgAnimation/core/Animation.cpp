/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Named animation clip with a collection of channels.
 * Manages playback weight, mode (ONCE, LOOP), and duration.
 */
#include <osgAnimation/core/Animation.hpp>

using namespace osgAnimation;

Animation::Animation( const osgAnimation::Animation& anim,
                      const osg::CopyOp&             copyop ) :
    Inherit( anim,
             copyop ),
    _duration( anim._duration ),
    _originalDuration( anim._originalDuration ),
    _weight( anim._weight ),
    _startTime( anim._startTime ),
    _playmode( anim._playmode )
{
    const ChannelList& cl = anim.getChannels();
    for( ChannelList::const_iterator it = cl.begin(); it != cl.end(); ++it )
    {
        addChannel( it->get()->clone() );
    }
}

void
Animation::addChannel( Channel* pChannel )
{
    _channels.push_back( pChannel );
    if( _duration == _originalDuration )
    {
        computeDuration();
    }
    else
    {
        _originalDuration = computeDurationFromChannels();
    }
}

void
Animation::removeChannel( Channel* pChannel )
{
    ChannelList::iterator it = _channels.begin();
    while( it != _channels.end() && it->get() != pChannel )
    {
        ++it;
    }

    if( it != _channels.end() )
    {
        _channels.erase( it );
    }
    computeDuration();
}

double
Animation::computeDurationFromChannels() const
{
    if( _channels.empty() )
    {
        return 0;
    }

    double                      tmin = 1E5;
    double                      tmax = -1E5;
    ChannelList::const_iterator chan;
    for( chan = _channels.begin(); chan != _channels.end(); chan++ )
    {
        auto min = ( *chan )->getStartTime();
        if( min < tmin )
        {
            tmin = min;
        }
        auto max = ( *chan )->getEndTime();
        if( max > tmax )
        {
            tmax = max;
        }
    }
    return tmax - tmin;
}

void
Animation::computeDuration()
{
    _duration         = computeDurationFromChannels();
    _originalDuration = _duration;
}

osgAnimation::ChannelList&
Animation::getChannels()
{
    return _channels;
}

const osgAnimation::ChannelList&
Animation::getChannels() const
{
    return _channels;
}

void
Animation::setDuration( double duration )
{
    _originalDuration = computeDurationFromChannels();
    _duration         = duration;
}

double
Animation::getDuration() const
{
    return _duration;
}

float
Animation::getWeight() const
{
    return _weight;
}

void
Animation::setWeight( float weight )
{
    _weight = weight;
}

bool
Animation::update( double time,
                   int    priority )
{
    if( _duration == 0.0 )    // if not initialized then do it
    {
        computeDuration();
    }

    double ratio = _originalDuration / _duration;

    double t     = ( time - _startTime ) * ratio;
    switch( _playmode )
    {
        case ONCE :
            if( t > _originalDuration )
            {
                for( ChannelList::const_iterator chan = _channels.begin();
                     chan != _channels.end();
                     ++chan )
                {
                    ( *chan )->update( _originalDuration, _weight, priority );
                }

                return false;
            }
            break;
        case STAY :
            if( t > _originalDuration )
            {
                t = _originalDuration;
            }
            break;
        case LOOP :
            if( _originalDuration == 0.0 )
            {
                t = _startTime;
            }
            else if( t > _originalDuration )
            {
                t = fmod( t, _originalDuration );
            }
            // std::cout << "t " << t << " duration " << _duration << std::endl;
            break;
        case PPONG :
            if( _originalDuration == 0.0 )
            {
                t = _startTime;
            }
            else
            {
                int tt = ( int )( t / _originalDuration );
                t      = fmod( t, _originalDuration );
                if( tt % 2 )
                {
                    t = _originalDuration - t;
                }
            }
            break;
    }

    ChannelList::const_iterator chan;
    for( chan = _channels.begin(); chan != _channels.end(); ++chan )
    {
        ( *chan )->update( t, _weight, priority );
    }
    return true;
}

void
Animation::resetTargets()
{
    ChannelList::const_iterator chan;
    for( chan = _channels.begin(); chan != _channels.end(); ++chan )
    {
        ( *chan )->reset();
    }
}
