/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Pre-configured radial burst particle effect.
 * Quick explosion visualization with configurable parameters.
 */
#include <osgParticle/ExplosionEffect>

#include <osg/nodes/Geode.hpp>
#include <osgParticle/AccelOperator>
#include <osgParticle/ExplosionEffect>
#include <osgParticle/FluidFrictionOperator>
#include <osgParticle/ModularEmitter>
#include <osgParticle/ModularProgram>
#include <osgParticle/ParticleSystemUpdater>
#include <osgParticle/RadialShooter>
#include <osgParticle/RandomRateCounter>
#include <osgParticle/SectorPlacer>

using namespace osgParticle;

ExplosionEffect::ExplosionEffect( bool automaticSetup ) :
    Inherit( automaticSetup )
{
    setDefaults();

    _position.set( 0.0F, 0.0F, 0.0F );
    _scale           = 1.0F;
    _intensity       = 1.0F;

    _emitterDuration = 1.0;

    if( _automaticSetup )
    {
        buildEffect();
    }
}

ExplosionEffect::ExplosionEffect( const osg::vec3& position,
                                  float            scale,
                                  float            intensity )
{
    setDefaults();

    _position        = position;
    _scale           = scale;
    _intensity       = intensity;

    _emitterDuration = 1.0;

    if( _automaticSetup )
    {
        buildEffect();
    }
}

ExplosionEffect::ExplosionEffect( const ExplosionEffect& copy,
                                  const osg::CopyOp&     copyop ) :
    Inherit( copy,
             copyop )
{
    if( _automaticSetup )
    {
        buildEffect();
    }
}

void
ExplosionEffect::setDefaults()
{
    ParticleEffect::setDefaults();

    _textureFileName = "Images/smoke.rgb";
    _emitterDuration = 1.0;

    // set up unit particle.
    _defaultParticleTemplate.setLifeTime( 0.5 + 0.1 * _scale );
    _defaultParticleTemplate.setSizeRange( osgParticle::rangef( 0.75F, 3.0F ) );
    _defaultParticleTemplate.setAlphaRange( osgParticle::rangef( 0.1F, 1.0F ) );
    _defaultParticleTemplate.setColorRange(
        osgParticle::rangev4( osg::vec4( 1.0F, 0.8F, 0.2F, 1.0F ),
                              osg::vec4( 1.0F, 0.4F, 0.1F, 0.0F ) )
    );
}

void
ExplosionEffect::setUpEmitterAndProgram()
{
    // set up particle system
    if( !_particleSystem )
    {
        _particleSystem = new osgParticle::ParticleSystem;
    }

    if( _particleSystem.valid() )
    {
        _particleSystem->setDefaultAttributes( _textureFileName, false, false );

        osgParticle::Particle& ptemplate = _particleSystem->getDefaultParticleTemplate();

        float                  radius    = 0.4F * _scale;
        float                  density   = 1.2F;    // 1.0kg/m^3

        ptemplate.setLifeTime( _defaultParticleTemplate.getLifeTime() );

        // the following ranges set the envelope of the respective
        // graphical properties in time.
        ptemplate.setSizeRange( osgParticle::rangef(
            radius * _defaultParticleTemplate.getSizeRange().minimum,
            radius * _defaultParticleTemplate.getSizeRange().maximum
        ) );
        ptemplate.setAlphaRange( _defaultParticleTemplate.getAlphaRange() );
        ptemplate.setColorRange( _defaultParticleTemplate.getColorRange() );

        // these are physical properties of the particle
        ptemplate.setRadius( radius );
        ptemplate.setMass( density *
                           radius *
                           radius *
                           radius *
                           static_cast<float>( osg::PI * 4.0 / 3.0 ) );
    }

    // set up emitter
    if( !_emitter )
    {
        _emitter = new osgParticle::ModularEmitter;
        _emitter->setCounter( new osgParticle::RandomRateCounter );
        _emitter->setPlacer( new osgParticle::SectorPlacer );
        _emitter->setShooter( new osgParticle::RadialShooter );
    }

    if( _emitter.valid() )
    {
        _emitter->setParticleSystem( _particleSystem.get() );
        _emitter->setReferenceFrame( _useLocalParticleSystem
                                         ? osgParticle::ParticleProcessor::ABSOLUTE_RF
                                         : osgParticle::ParticleProcessor::RELATIVE_RF );

        _emitter->setStartTime( _startTime );
        _emitter->setLifeTime( _emitterDuration );
        _emitter->setEndless( false );

        osgParticle::RandomRateCounter* counter =
            dynamic_cast<osgParticle::RandomRateCounter*>( _emitter->getCounter() );
        if( counter )
        {
            counter->setRateRange( 50 * _intensity, 100 * _intensity );
        }

        osgParticle::SectorPlacer* placer =
            dynamic_cast<osgParticle::SectorPlacer*>( _emitter->getPlacer() );
        if( placer )
        {
            placer->setCenter( _position );
            placer->setRadiusRange( 0.0F * _scale, 0.25F * _scale );
        }

        osgParticle::RadialShooter* shooter =
            dynamic_cast<osgParticle::RadialShooter*>( _emitter->getShooter() );
        if( shooter )
        {
            shooter->setThetaRange( 0.0F, static_cast<float>( osg::PI_2 ) );
            shooter->setInitialSpeedRange( 1.0F * _scale, 10.0F * _scale );
        }
    }

    // set up the program
    if( !_program )
    {
        _program = new osgParticle::FluidProgram;
    }

    if( _program.valid() )
    {
        _program->setParticleSystem( _particleSystem.get() );
        _program->setWind( _wind );
    }
}
