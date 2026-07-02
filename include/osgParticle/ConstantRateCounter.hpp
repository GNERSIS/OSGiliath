/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Emits a fixed number of particles per second.
 * Provides steady-state emission for continuous effects.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>
#include <osg/maths/Math.hpp>
#include <osgParticle/Counter.hpp>

namespace osgParticle
{

    class ConstantRateCounter : public osg::Inherit<Counter, ConstantRateCounter>
    {
        public:

            ConstantRateCounter() :
                _minimumNumberOfParticlesToCreate( 0 ),
                _numberOfParticlesPerSecondToCreate( 0 ),
                _carryOver( 0 )
            {
            }

            ConstantRateCounter( const ConstantRateCounter& copy,
                                 const osg::CopyOp&         copyop =
                                     osg::CopyOp::SHALLOW_COPY ) :
                Inherit( copy,
                         copyop ),
                _minimumNumberOfParticlesToCreate(
                    copy._minimumNumberOfParticlesToCreate
                ),
                _numberOfParticlesPerSecondToCreate(
                    copy._numberOfParticlesPerSecondToCreate
                ),
                _carryOver( copy._carryOver )
            {
            }

            OSG_REGISTER_TYPE( osgParticle,
                               ConstantRateCounter )

            void
            setMinimumNumberOfParticlesToCreate( int minNumToCreate )
            {
                _minimumNumberOfParticlesToCreate = minNumToCreate;
            }

            int
            getMinimumNumberOfParticlesToCreate() const
            {
                return _minimumNumberOfParticlesToCreate;
            }

            void
            setNumberOfParticlesPerSecondToCreate( double numPerSecond )
            {
                _numberOfParticlesPerSecondToCreate = numPerSecond;
            }

            double
            getNumberOfParticlesPerSecondToCreate() const
            {
                return _numberOfParticlesPerSecondToCreate;
            }

            /// Return the number of particles to be created in this frame
            virtual int
            numParticlesToCreate( double dt ) const
            {
                double v    = ( dt * _numberOfParticlesPerSecondToCreate );
                int    i    = ( int )( v );
                _carryOver += ( v - ( double )i );
                if( _carryOver > 1.0 )
                {
                    ++i;
                    _carryOver -= 1.0;
                }
                return std::max( _minimumNumberOfParticlesToCreate, i );
            }

            virtual int
            getEstimatedMaxNumOfParticles( double lifeTime ) const
            {
                int minNumParticles = static_cast<int>(
                    static_cast<float>( _minimumNumberOfParticlesToCreate ) *
                    60.0F *
                    static_cast<float>( lifeTime )
                );
                int baseNumPartciles =
                    static_cast<int>( _numberOfParticlesPerSecondToCreate * lifeTime );
                return std::max( minNumParticles, baseNumPartciles );
            }

        protected:

            virtual ~ConstantRateCounter()
            {
            }

            int            _minimumNumberOfParticlesToCreate;
            double         _numberOfParticlesPerSecondToCreate;
            mutable double _carryOver;
    };

}
