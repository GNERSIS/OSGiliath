/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Pre-configured debris particle effect.
 * Ballistic particles for explosion debris visualization.
 */
#include <osgParticle/ExplosionDebrisEffect.hpp>

#include <osg/nodes/Geode.hpp>
#include <osgParticle/AccelOperator.hpp>
#include <osgParticle/FluidFrictionOperator.hpp>
#include <osgParticle/ModularEmitter.hpp>
#include <osgParticle/ModularProgram.hpp>
#include <osgParticle/ParticleSystemUpdater.hpp>
#include <osgParticle/RadialShooter.hpp>
#include <osgParticle/RandomRateCounter.hpp>
#include <osgParticle/SectorPlacer.hpp>

using namespace osgParticle;

ExplosionDebrisEffect::ExplosionDebrisEffect( bool automaticSetup ) :
    Inherit( automaticSetup )
{
    setDefaults();

    _position.set( 0.0F, 0.0F, 0.0F );
    _scale           = 1.0F;
    _intensity       = 1.0F;

    _emitterDuration = 0.1;
    _defaultParticleTemplate.setLifeTime( 1.0 + 0.6 * _scale );

    if( _automaticSetup )
    {
        buildEffect();
    }
}

ExplosionDebrisEffect::ExplosionDebrisEffect( const osg::vec3& position,
                                              float            scale,
                                              float            intensity )
{
    setDefaults();

    _position        = position;
    _scale           = scale;
    _intensity       = intensity;

    _emitterDuration = 0.1;
    _defaultParticleTemplate.setLifeTime( 1.0 + 0.6 * _scale );

    if( _automaticSetup )
    {
        buildEffect();
    }
}

ExplosionDebrisEffect::ExplosionDebrisEffect( const ExplosionDebrisEffect& copy,
                                              const osg::CopyOp&           copyop ) :
    Inherit( copy,
             copyop )
{
    if( _automaticSetup )
    {
        buildEffect();
    }
}

void
ExplosionDebrisEffect::setDefaults()
{
    ParticleEffect::setDefaults();

    _textureFileName = "Images/particle.rgb";
    _emitterDuration = 0.1;

    // set up unit particle.
    _defaultParticleTemplate.setLifeTime( 1.0 + 0.6 * _scale );
    _defaultParticleTemplate.setSizeRange( osgParticle::rangef( 0.75F, 3.0F ) );
    _defaultParticleTemplate.setAlphaRange( osgParticle::rangef( 0.0F, 1.0F ) );
    _defaultParticleTemplate.setColorRange(
        osgParticle::rangev4( osg::vec4( 0.5F, 0.5F, 0.0F, 1.0F ),
                              osg::vec4( 0.2F, 0.2F, 0.2F, 0.5F ) )
    );
}

void
ExplosionDebrisEffect::setUpEmitterAndProgram()
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

        float                  radius    = 0.05F * _scale;
        float                  density   = 1000.0F;    // 1000.0kg/m^3

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
            counter->setRateRange( 2'000 * _intensity, 2'000 * _intensity );
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
            shooter->setInitialSpeedRange( 1.0F * _scale, 5.0F * _scale );
        }
    }

    // set up program.
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
