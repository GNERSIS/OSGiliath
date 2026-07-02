#include <osgParticle/ModularProgram.hpp>

#include <osgParticle/Particle.hpp>
#include <osgParticle/ParticleSystem.hpp>
#include <osgParticle/Program.hpp>

osgParticle::ModularProgram::ModularProgram()
{
}

osgParticle::ModularProgram::ModularProgram( const ModularProgram& copy,
                                             const osg::CopyOp&    copyop ) :
    Inherit( copy,
             copyop )
{
    Operator_vector::const_iterator ci;
    for( ci = copy._operators.begin(); ci != copy._operators.end(); ++ci )
    {
        _operators.push_back( static_cast<Operator*>( copyop( ci->get() ) ) );
    }
}

void
osgParticle::ModularProgram::execute( double dt )
{
    Operator_vector::iterator ci;
    Operator_vector::iterator ci_end = _operators.end();

    ParticleSystem*           ps     = getParticleSystem();
    for( ci = _operators.begin(); ci != ci_end; ++ci )
    {
        ( *ci )->beginOperate( this );
        ( *ci )->operateParticles( ps, dt );
        ( *ci )->endOperate();
    }
}
