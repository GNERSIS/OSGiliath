/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Linked particle system that renders particles as a connected
 * trail. Used for contrails, ribbons, and fluid streams.
 */
#include <osgParticle/ConnectedParticleSystem>

#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/traversal/CullingSet.hpp>

using namespace osgParticle;

ConnectedParticleSystem::ConnectedParticleSystem() :
    _lastParticleCreated( Particle::INVALID_INDEX ),
    _maxNumberOfParticlesToSkip( 200 ),
    _startParticle( Particle::INVALID_INDEX )
{
}

ConnectedParticleSystem::ConnectedParticleSystem( const ConnectedParticleSystem& copy,
                                                  const osg::CopyOp& copyop ) :
    Inherit( copy,
             copyop ),
    _lastParticleCreated( copy._lastParticleCreated ),
    _maxNumberOfParticlesToSkip( 200 ),
    _startParticle( copy._startParticle )
{
}

ConnectedParticleSystem::~ConnectedParticleSystem()
{
}

Particle*
ConnectedParticleSystem::createParticle( const Particle* ptemplate )
{
    // OSG_NOTICE<<this<< " Creating particle "<<std::endl;

    Particle* particle      = ParticleSystem::createParticle( ptemplate );
    int       particleIndex = ( int )( particle - &_particles[0] );

    if( particle )
    {

        if( _startParticle == Particle::INVALID_INDEX )
        {
            // we are the first particle create, so start the connect particle list
            _startParticle = particleIndex;
        }

        if( _lastParticleCreated != Particle::INVALID_INDEX )
        {
            // OSG_NOTICE<<this<< " Connecting "<<_lastParticleCreated<<" to
            // "<<particleIndex<<std::endl;

            // write up the last created particle to this new particle
            _particles[static_cast<std::size_t>( _lastParticleCreated )].setNextParticle(
                particleIndex
            );
            particle->setPreviousParticle( _lastParticleCreated );
        }

        // set the new particle as the last particle created.
        _lastParticleCreated = particleIndex;
    }

    return particle;
}

void
ConnectedParticleSystem::reuseParticle( int particleIndex )
{
    // OSG_NOTICE<<this<< " Reusing particle "<<particleIndex<<std::endl;

    if( particleIndex < 0 || particleIndex >= ( int )_particles.size() )
    {
        return;
    }

    Particle* particle = &_particles[static_cast<std::size_t>( particleIndex )];
    int       previous = particle->getPreviousParticle();
    int       next     = particle->getNextParticle();

    // update start and last entries
    if( _startParticle == particleIndex )
    {
        _startParticle = particle->getNextParticle();
    }

    if( _lastParticleCreated == particleIndex )
    {
        _lastParticleCreated = Particle::INVALID_INDEX;
    }

    // join up the previous and next particles to account for
    // the deletion of the this particle
    if( previous != Particle::INVALID_INDEX )
    {
        _particles[static_cast<std::size_t>( previous )].setNextParticle( next );
    }

    if( next != Particle::INVALID_INDEX )
    {
        _particles[static_cast<std::size_t>( next )].setPreviousParticle( previous );
    }

    // reset the next and previous particle entries of this particle
    particle->setPreviousParticle( Particle::INVALID_INDEX );
    particle->setNextParticle( Particle::INVALID_INDEX );

    // put the particle on the death stack
    ParticleSystem::reuseParticle( particleIndex );
}

void
ConnectedParticleSystem::drawImplementation( osg::RenderInfo& renderInfo ) const
{
    ScopedReadLock  lock( _readWriteMutex );

    osg::State&     state = *renderInfo.getState();

    const Particle* particle =
        ( _startParticle != Particle::INVALID_INDEX )
            ? &_particles[static_cast<std::size_t>( _startParticle )]
            : 0;
    if( !particle )
    {
        return;
    }

    ArrayData& ad = _bufferedArrayData[state.getContextID()];
    if( !ad.vertices.valid() )
    {
        ad.init();
        ad.reserve( static_cast<unsigned int>( _particles.capacity() ) * 2 );
    }

    ad.clear();
    ad.dirty();

    // set up arrays and primitives ready to fill in
    osg::Vec3Array&        vertices   = *ad.vertices;
    osg::Vec4Array&        colors     = *ad.colors;
    osg::Vec2Array&        texcoords  = *ad.texcoords2;
    ArrayData::Primitives& primitives = ad.primitives;

    osg::vec4              pixelSizeVector =
        osg::CullingSet::computePixelSizeVector( *state.getCurrentViewport(),
                                                 state.getProjectionMatrix(),
                                                 state.getModelViewMatrix() );
    float unitPixelSize =
        fabs( 1.0F / ( osg::dot( osg::vec4( particle->getPosition(), 1.0F ),
                                 pixelSizeVector ) ) );
    float pixelSizeOfFirstParticle = unitPixelSize * particle->getCurrentSize();
    // float desiredGapBetweenDrawnParticles = 50.0f/unitPixelSize;
    // float desiredGapBetweenDrawnParticles2 =
    // desiredGapBetweenDrawnParticles*desiredGapBetweenDrawnParticles;

    float maxPixelError2 = osg::square( 1.0F / unitPixelSize );

    if( pixelSizeOfFirstParticle < 1.0 )
    {
        // draw the connected particles as a line
        while( particle != 0 )
        {

            const osg::vec4& color = particle->getCurrentColor();
            const osg::vec3& pos   = particle->getPosition();
            colors.push_back( osg::vec4( color.r,
                                         color.g,
                                         color.b,
                                         color.a * particle->getCurrentAlpha() ) );
            texcoords.push_back( osg::vec2( particle->getSTexCoord(), 0.5F ) );
            vertices.push_back( pos );

            const Particle* nextParticle =
                ( particle->getNextParticle() != Particle::INVALID_INDEX )
                    ? &_particles[static_cast<std::size_t>( particle
                                                                ->getNextParticle() )]
                    : 0;
            if( nextParticle )
            {
                osg::vec3 startDelta =
                    osg::normalize( nextParticle->getPosition() - pos );
                float distance2 = 0.0;

                // now skip particles of required
                for( unsigned int i = 0;
                     i <
                     _maxNumberOfParticlesToSkip &&
                     ( ( distance2 < maxPixelError2 ) &&
                       ( nextParticle->getNextParticle() != Particle::INVALID_INDEX ) );
                     ++i )
                {
                    nextParticle =
                        &_particles[static_cast<std::size_t>( nextParticle
                                                                  ->getNextParticle() )];
                    osg::vec3 delta = nextParticle->getPosition() - pos;
                    distance2       = osg::length2( osg::cross( delta, startDelta ) );
                }
            }
            particle = nextParticle;
        }

        primitives.push_back( ArrayData::ModeCount( GL_LINE_STRIP, vertices.size() ) );
    }
    else
    {
        // draw the connected particles as a quad stripped aligned to be orthogonal to
        // the eye
        osg::dmat4 eyeToLocalTransform = osg::inverse( state.getModelViewMatrix() );
        osg::vec3  eyeLocal =
            osg::vec3( osg::dvec3( 0.0, 0.0, 0.0 ) * eyeToLocalTransform );

        osg::vec3 delta( 0.0F, 0.0F, 1.0F );

        while( particle != 0 )
        {
            const osg::vec4& color = particle->getCurrentColor();
            const osg::vec3& pos   = particle->getPosition();

            const Particle*  nextParticle =
                ( particle->getNextParticle() != Particle::INVALID_INDEX )
                    ? &_particles[static_cast<std::size_t>( particle
                                                                ->getNextParticle() )]
                    : 0;

            if( nextParticle )
            {
                osg::vec3 startDelta = nextParticle->getPosition() - pos;
                delta                = startDelta;
                startDelta           = osg::normalize( startDelta );
                float distance2      = 0.0;

                // now skip particles of required
                for( unsigned int i = 0;
                     i <
                     _maxNumberOfParticlesToSkip &&
                     ( ( distance2 < maxPixelError2 ) &&
                       ( nextParticle->getNextParticle() != Particle::INVALID_INDEX ) );
                     ++i )
                {
                    nextParticle =
                        &_particles[static_cast<std::size_t>( nextParticle
                                                                  ->getNextParticle() )];
                    delta     = nextParticle->getPosition() - pos;
                    distance2 = osg::length2( osg::cross( delta, startDelta ) );
                }
            }

            osg::vec3 normal  = osg::normalize( osg::cross( delta, pos - eyeLocal ) );
            normal           *= particle->getCurrentSize();

            osg::vec3 bottom( pos - normal );
            osg::vec3 top( pos + normal );

            colors.push_back( osg::vec4( color.r,
                                         color.g,
                                         color.b,
                                         color.a * particle->getCurrentAlpha() ) );
            texcoords.push_back( osg::vec2( particle->getSTexCoord(), 0.0F ) );
            vertices.push_back( bottom );

            colors.push_back( colors.back() );
            texcoords.push_back( osg::vec2( particle->getSTexCoord(), 1.0F ) );
            vertices.push_back( top );

            particle = nextParticle;
        }

        primitives.push_back( ArrayData::ModeCount( GL_TRIANGLE_STRIP,
                                                    vertices.size() ) );
    }

    ad.dispatchArrays( state );
    ad.dispatchPrimitives();
}
