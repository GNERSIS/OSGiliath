/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Pre-configured radial burst particle effect.
 * Quick explosion visualization with configurable parameters.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osgParticle/FluidProgram.hpp>
#include <osgParticle/ModularEmitter.hpp>
#include <osgParticle/ParticleEffect.hpp>

namespace osgParticle
{

    class OSGPARTICLE_EXPORT ExplosionEffect
        : public osg::Inherit<ParticleEffect, ExplosionEffect>
    {
        public:

            explicit ExplosionEffect( bool automaticSetup = true );

            ExplosionEffect( const osg::vec3& position,
                             float            scale     = 1.0F,
                             float            intensity = 1.0F );

            ExplosionEffect( const ExplosionEffect& copy,
                             const osg::CopyOp&     copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgParticle,
                               ExplosionEffect )

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

            virtual ~ExplosionEffect()
            {
            }

            osg::ref_ptr<ModularEmitter> _emitter;
            osg::ref_ptr<FluidProgram>   _program;
    };

}
