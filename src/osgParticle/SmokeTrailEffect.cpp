/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Pre-configured smoke trail particle effect.
 * Generates a lingering trail behind a moving emitter.
 */
#include <osgParticle/SmokeTrailEffect.hpp>

#include <osg/nodes/Geode.hpp>
#include <osgParticle/ConnectedParticleSystem.hpp>
#include <osgParticle/ConstantRateCounter.hpp>
#include <osgParticle/ParticleSystemUpdater.hpp>
#include <osgParticle/RadialShooter.hpp>
#include <osgParticle/SectorPlacer.hpp>

using namespace osgParticle;

SmokeTrailEffect::SmokeTrailEffect( bool automaticSetup ) :
    Inherit( automaticSetup )
{
    setDefaults();

    _position.set( 0.0F, 0.0F, 0.0F );
    _scale           = 1.0F;
    _intensity       = 1.0F;

    _emitterDuration = 65.0;
    _defaultParticleTemplate.setLifeTime( 5.0 * _scale );

    if( _automaticSetup )
    {
        buildEffect();
    }
}

SmokeTrailEffect::SmokeTrailEffect( const osg::vec3& position,
                                    float            scale,
                                    float            intensity )
{
    setDefaults();

    _position        = position;
    _scale           = scale;
    _intensity       = intensity;

    _emitterDuration = 65.0;
    _defaultParticleTemplate.setLifeTime( 5.0 * _scale );

    if( _automaticSetup )
    {
        buildEffect();
    }
}

SmokeTrailEffect::SmokeTrailEffect( const SmokeTrailEffect& copy,
                                    const osg::CopyOp&      copyop ) :
    Inherit( copy,
             copyop )
{
    if( _automaticSetup )
    {
        buildEffect();
    }
}

void
SmokeTrailEffect::setDefaults()
{
    ParticleEffect::setDefaults();

    _textureFileName = "Images/continous_smoke.rgb";
    _emitterDuration = 65.0;

    // set up unit particle.
    _defaultParticleTemplate.setLifeTime( 5.0 * _scale );
    _defaultParticleTemplate.setSizeRange( osgParticle::rangef( 0.75F, 2.0F ) );
    _defaultParticleTemplate.setAlphaRange( osgParticle::rangef( 0.7F, 1.0F ) );
    _defaultParticleTemplate.setColorRange(
        osgParticle::rangev4( osg::vec4( 1, 1.0F, 1.0F, 1.0F ),
                              osg::vec4( 1, 1.0F, 1.F, 0.0F ) )
    );
}

void
SmokeTrailEffect::setUpEmitterAndProgram()
{
    // set up particle system
    if( !_particleSystem )
    {
        _particleSystem = new osgParticle::ConnectedParticleSystem;
    }

    if( _particleSystem.valid() )
    {
        _particleSystem->setDefaultAttributes( _textureFileName, false, false );

        osgParticle::Particle& ptemplate = _particleSystem->getDefaultParticleTemplate();

        float                  radius    = 0.5F * _scale;
        float                  density   = 1.0F;    // 1.0kg/m^3

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
        _emitter->setCounter( new osgParticle::ConstantRateCounter );
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

        osgParticle::ConstantRateCounter* counter =
            dynamic_cast<osgParticle::ConstantRateCounter*>( _emitter->getCounter() );
        if( counter )
        {
            counter->setMinimumNumberOfParticlesToCreate( 1 );
            counter->setNumberOfParticlesPerSecondToCreate( 0.0 );
        }

        osgParticle::SectorPlacer* placer =
            dynamic_cast<osgParticle::SectorPlacer*>( _emitter->getPlacer() );
        if( placer )
        {
            placer->setCenter( _position );
            placer->setRadiusRange( 0.0F * _scale, 0.0F * _scale );
        }

        osgParticle::RadialShooter* shooter =
            dynamic_cast<osgParticle::RadialShooter*>( _emitter->getShooter() );
        if( shooter )
        {
            shooter->setThetaRange( 0.0F, 0.0F );
            shooter->setInitialSpeedRange( 0.0F * _scale, 0.0F * _scale );
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
