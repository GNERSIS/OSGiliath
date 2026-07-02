/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Pre-configured debris particle effect.
 * Ballistic particles for explosion debris visualization.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osgParticle/FluidProgram.hpp>
#include <osgParticle/ModularEmitter.hpp>
#include <osgParticle/ParticleEffect.hpp>

namespace osgParticle
{

    class OSGPARTICLE_EXPORT ExplosionDebrisEffect
        : public osg::Inherit<ParticleEffect, ExplosionDebrisEffect>
    {
        public:

            explicit ExplosionDebrisEffect( bool automaticSetup = true );

            ExplosionDebrisEffect( const osg::vec3& position,
                                   float            scale     = 1.0F,
                                   float            intensity = 1.0F );

            ExplosionDebrisEffect( const ExplosionDebrisEffect& copy,
                                   const osg::CopyOp&           copyop =
                                       osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgParticle,
                               ExplosionDebrisEffect )

            virtual void
            setDefaults();

            virtual void
            setUpEmitterAndProgram();

            virtual Emitter*
            getEmitter()
            {
                return _emitter.get();
            }

            virtual const Emitter*
            getEmitter() const
            {
                return _emitter.get();
            }

            virtual Program*
            getProgram()
            {
                return _program.get();
            }

            virtual const Program*
            getProgram() const
            {
                return _program.get();
            }

        protected:

            virtual ~ExplosionDebrisEffect()
            {
            }

            osg::ref_ptr<ModularEmitter> _emitter;
            osg::ref_ptr<FluidProgram>   _program;
    };

}
