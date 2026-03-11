/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Blink timing sequence for light points. Defines on/off
 * pulse patterns for flashing navigation lights.
 */
#include <osgSim/BlinkSequence>

#include <stdlib.h>

using namespace osgSim;

BlinkSequence::BlinkSequence() :
    _pulsePeriod( 0.0 ),
    _phaseShift( 0.0 ),
    _pulseData(),
    _sequenceGroup( 0 )
{
}

BlinkSequence::BlinkSequence( const BlinkSequence& bs,
                              const osg::CopyOp&   copyop ) :
    Inherit( bs,
             copyop ),
    _pulsePeriod( bs._pulsePeriod ),
    _phaseShift( bs._phaseShift ),
    _pulseData( bs._pulseData ),
    _sequenceGroup( bs._sequenceGroup )
{
}

SequenceGroup::SequenceGroup()
{
    // set a random base time between 0 and 1000.0
    _baseTime = ( ( double )rand() / ( double )RAND_MAX ) * 1000.0;
}

SequenceGroup::SequenceGroup( const SequenceGroup& sg,
                              const osg::CopyOp&   copyop ) :
    Inherit( sg,
             copyop ),
    _baseTime( sg._baseTime )
{
}

SequenceGroup::SequenceGroup( double baseTime ) :
    _baseTime( baseTime )
{
}
