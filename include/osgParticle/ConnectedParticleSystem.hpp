/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Linked particle system that renders particles as a connected
 * trail. Used for contrails, ribbons, and fluid streams.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osgParticle/ParticleSystem.hpp>

namespace osgParticle
{

    /** ConnectConnectedParticleSystem is a specialise ConnectedParticleSystem for
     * effects like missile trails, where the individual particles are rendered as single
     * ribbon.
     */
    class OSGPARTICLE_EXPORT ConnectedParticleSystem
        : public osg::Inherit<osgParticle::ParticleSystem, ConnectedParticleSystem>
    {
        public:

            ConnectedParticleSystem();
            ConnectedParticleSystem( const ConnectedParticleSystem& copy,
                                     const osg::CopyOp&             copyop =
                                         osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgParticle,
                               ConnectedParticleSystem )

            /// Create a new particle from the specified template (or the default one if
            /// <CODE>ptemplate</CODE> is null).
            virtual Particle*
            createParticle( const Particle* ptemplate );

            /// Reuse the i-th particle.
            virtual void
            reuseParticle( int i );

            /// Draw the connected particles as either a line or a quad strip, depending
            /// upon viewing distance. .
            virtual void
            drawImplementation( osg::RenderInfo& renderInfo ) const;

            /// Get the (const) particle from where the line or quadstrip starts to be
            /// drawn
            const osgParticle::Particle*
            getStartParticle() const
            {
                return ( _startParticle != Particle::INVALID_INDEX )
                         ? &_particles[static_cast<std::size_t>( _startParticle )]
                         : 0;
            }

            /// Get the particle from where the line or quadstrip starts to be drawn
            osgParticle::Particle*
            getStartParticle()
            {
                return ( _startParticle != Particle::INVALID_INDEX )
                         ? &_particles[static_cast<std::size_t>( _startParticle )]
                         : 0;
            }

            /// Set the maximum numbers of particles to be skipped during the predraw
            /// filtering
            void
            setMaxNumberOfParticlesToSkip( unsigned int maxNumberofParticlesToSkip )
            {
                _maxNumberOfParticlesToSkip = maxNumberofParticlesToSkip;
            }

            /// Get the maximum numbers of particles to be skipped during the predraw
            /// filtering
            unsigned int
            getMaxNumberOfParticlesToSkip()
            {
                return _maxNumberOfParticlesToSkip;
            }

        protected:

            virtual ~ConnectedParticleSystem();

            ConnectedParticleSystem&
            operator=( const ConnectedParticleSystem& )
            {
                return *this;
            }

            int          _lastParticleCreated;
            unsigned int _maxNumberOfParticlesToSkip;

            int          _startParticle;
    };

}
