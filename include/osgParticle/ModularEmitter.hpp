/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Emitter composed of pluggable Placer, Shooter, and Counter
 * modules for flexible particle emission configuration.
 */
// osgParticle - Copyright (C) 2002 Marco Jez

#pragma once

#include <osg/core/CopyOp.hpp>
#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>
#include <osg/core/ref_ptr.hpp>
#include <osg/nodes/Node.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osgParticle/Emitter.hpp>
#include <osgParticle/Export.hpp>
#include <osgParticle/Particle.hpp>
#include <osgParticle/ParticleSystem.hpp>
#include <osgParticle/Placer.hpp>
#include <osgParticle/PointPlacer.hpp>
#include <osgParticle/RadialShooter.hpp>
#include <osgParticle/RandomRateCounter.hpp>
#include <osgParticle/Shooter.hpp>

namespace osgParticle
{

    /**    An emitter class that holds three objects to control the creation of
       particles. These objects are a <I>counter</I>, a <I>placer</I> and a
       <I>shooter</I>. The counter controls the number of particles to be emitted at each
       frame; the placer must initialize the particle's position vector, while the
       shooter initializes its velocity vector. You can use the predefined
       counter/placer/shooter classes, or you can create your own.
    */
    class OSGPARTICLE_EXPORT ModularEmitter
        : public osg::Inherit<Emitter, ModularEmitter>
    {
        public:

            ModularEmitter();
            ModularEmitter( const ModularEmitter& copy,
                            const osg::CopyOp&    copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgParticle,
                               ModularEmitter )

            /// Get the counter object.
            inline Counter*
            getCounter();

            /// Get the const Counter object.
            inline const Counter*
            getCounter() const;

            /// Set the Counter object.
            inline void
            setCounter( Counter* c );

            /// Get the ratio between number of particle to create in compensation for
            /// movement of the emitter
            inline float
            getNumParticlesToCreateMovementCompensationRatio() const;

            /// Set the ratio between number of particle to create in compenstation for
            /// movement of the emitter
            inline void
            setNumParticlesToCreateMovementCompensationRatio( float r );

            /// Get the Placer object.
            inline Placer*
            getPlacer();

            /// Get the const Placer object.
            inline const Placer*
            getPlacer() const;

            /// Set the Placer object.
            inline void
            setPlacer( Placer* p );

            /// Get the Shooter object.
            inline Shooter*
            getShooter();

            /// Get the const Shooter object.
            inline const Shooter*
            getShooter() const;

            /// Set the Shooter object.
            inline void
            setShooter( Shooter* s );

        protected:

            virtual ~ModularEmitter()
            {
            }

            ModularEmitter&
            operator=( const ModularEmitter& )
            {
                return *this;
            }

            virtual void
            emitParticles( double dt );

        private:

            float                 _numParticleToCreateMovementCompensationRatio;
            osg::ref_ptr<Counter> _counter;
            osg::ref_ptr<Placer>  _placer;
            osg::ref_ptr<Shooter> _shooter;
    };

    // INLINE FUNCTIONS

    inline Counter*
    ModularEmitter::getCounter()
    {
        return _counter.get();
    }

    inline const Counter*
    ModularEmitter::getCounter() const
    {
        return _counter.get();
    }

    inline void
    ModularEmitter::setCounter( Counter* c )
    {
        _counter = c;
    }

    inline float
    ModularEmitter::getNumParticlesToCreateMovementCompensationRatio() const
    {
        return _numParticleToCreateMovementCompensationRatio;
    }

    inline void
    ModularEmitter::setNumParticlesToCreateMovementCompensationRatio( float r )
    {
        _numParticleToCreateMovementCompensationRatio = r;
    }

    inline Placer*
    ModularEmitter::getPlacer()
    {
        return _placer.get();
    }

    inline const Placer*
    ModularEmitter::getPlacer() const
    {
        return _placer.get();
    }

    inline void
    ModularEmitter::setPlacer( Placer* p )
    {
        _placer = p;
    }

    inline Shooter*
    ModularEmitter::getShooter()
    {
        return _shooter.get();
    }

    inline const Shooter*
    ModularEmitter::getShooter() const
    {
        return _shooter.get();
    }

    inline void
    ModularEmitter::setShooter( Shooter* s )
    {
        _shooter = s;
    }

}
