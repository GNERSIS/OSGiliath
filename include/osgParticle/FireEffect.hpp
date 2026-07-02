/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Pre-configured fire particle effect.
 * Rising, color-shifting particles for flame visualization.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osgParticle/FluidProgram.hpp>
#include <osgParticle/ModularEmitter.hpp>
#include <osgParticle/ParticleEffect.hpp>

namespace osgParticle
{

    class OSGPARTICLE_EXPORT FireEffect : public osg::Inherit<ParticleEffect, FireEffect>
    {
        public:

            explicit FireEffect( bool automaticSetup = true );

            FireEffect( const osg::vec3& position,
                        float            scale     = 1.0F,
                        float            intensity = 1.0F );

            FireEffect( const FireEffect&  copy,
                        const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgParticle,
                               FireEffect )

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

            virtual ~FireEffect()
            {
            }

            osg::ref_ptr<ModularEmitter> _emitter;
            osg::ref_ptr<FluidProgram>   _program;
    };

}
