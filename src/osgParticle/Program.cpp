#include <osgParticle/Program>

#include <osg/core/CopyOp.hpp>
#include <osgParticle/ParticleProcessor>

osgParticle::Program::Program() :
    ParticleProcessor()
{
}

osgParticle::Program::Program( const Program&     copy,
                               const osg::CopyOp& copyop ) :
    ParticleProcessor( copy,
                       copyop )
{
}
