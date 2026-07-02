#include <osgParticle/Program.hpp>

#include <osg/core/CopyOp.hpp>
#include <osgParticle/ParticleProcessor.hpp>

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
