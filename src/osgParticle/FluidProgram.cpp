#include <osgParticle/FluidProgram>

osgParticle::FluidProgram::FluidProgram()
{
    setFluidToAir();
}

osgParticle::FluidProgram::FluidProgram( const FluidProgram& copy,
                                         const osg::CopyOp&  copyop ) :
    Inherit( copy,
             copyop ),
    _acceleration( copy._acceleration ),
    _viscosity( copy._viscosity ),
    _density( copy._density ),
    _wind( copy._wind ),
    _viscosityCoefficient( copy._viscosityCoefficient ),
    _densityCoefficient( copy._densityCoefficient )
{
}

void
osgParticle::FluidProgram::execute( double dt )
{
    const float     four_over_three = 4.0F / 3.0F;
    ParticleSystem* ps              = getParticleSystem();
    int             n               = ps->numParticles();
    for( int i = 0; i < n; ++i )
    {
        Particle* particle = ps->getParticle( i );
        if( particle->isAlive() )
        {
            float     radius = particle->getRadius();
            float     Area   = static_cast<float>( osg::PI ) * radius * radius;
            float     Volume = Area * radius * four_over_three;

            // compute force due to gravity + boyancy of displacing the fluid that the
            // particle is emersed in.
            osg::vec3 accel_gravity =
                _acceleration *
                ( ( particle->getMass() - _density * Volume ) * particle->getMassInv() );

            // compute force due to friction
            osg::vec3 relative_wind   = particle->getVelocity() - _wind;
            osg::vec3 wind_force      = -relative_wind *
                                        Area *
                                        ( _viscosityCoefficient +
                                          _densityCoefficient *
                                          osg::length( relative_wind ) );
            osg::vec3 wind_accel      = wind_force * particle->getMassInv();

            double    compenstated_dt = dt;
            if( osg::length2( relative_wind ) < dt * dt * osg::length2( wind_accel ) )
            {
                // OSG_NOTICE<<"** Could be critical: dt="<<dt<<"
                // critical_dt="<<sqrtf(critical_dt2)<<std::endl;
                double critical_dt2 =
                    osg::length2( relative_wind ) / osg::length2( wind_accel );
                compenstated_dt = sqrt( critical_dt2 ) * 0.8;
            }

            particle->addVelocity( accel_gravity *
                                   static_cast<float>( dt ) +
                                   wind_accel *
                                   static_cast<float>( compenstated_dt ) );
        }
    }
}
