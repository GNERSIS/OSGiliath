/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract animation channel binding a sampler to a named target.
 * Evaluates interpolated values at a given time.
 */
#include <osgAnimation/core/Channel.hpp>
using namespace osgAnimation;

Channel::Channel()
{
}

Channel::~Channel()
{
}

Channel::Channel( const Channel& channel ) :
    osg::Object( channel ),
    _targetName( channel._targetName ),
    _name( channel._name )
{
}

const std::string&
Channel::getName() const
{
    return _name;
}

void
Channel::setName( const std::string& name )
{
    _name = name;
}

const std::string&
Channel::getTargetName() const
{
    return _targetName;
}

void
Channel::setTargetName( const std::string& name )
{
    _targetName = name;
}
