/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Applies an outward radial force from a center point.
 * Simulates explosion shockwave pushing particles away.
 */
// Written by Wang Rui, (C) 2010

#pragma once

#include <osg/core/Inherit.hpp>
#include <osgParticle/ModularProgram.hpp>
#include <osgParticle/Operator.hpp>
#include <osgParticle/Particle.hpp>

namespace osgParticle
{

    /** An explosion operator exerts force on each particle away from the explosion
       center. Refer to David McAllister's Particle System API
       (http://www.particlesystems.org)
    */
    class ExplosionOperator : public osg::Inherit<Operator, ExplosionOperator>
    {
        public:

            ExplosionOperator() :
                Operator(),
                _radius( 1.0F ),
                _magnitude( 1.0F ),
                _epsilon( 1E-3 ),
                _sigma( 1.0F ),
                _inexp( 0.0F ),
                _outexp( 0.0F )
            {
            }

            ExplosionOperator( const ExplosionOperator& copy,
                               const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY ) :
                Operator( copy,
                          copyop ),
                _center( copy._center ),
                _radius( copy._radius ),
                _magnitude( copy._magnitude ),
                _epsilon( copy._epsilon ),
                _sigma( copy._sigma ),
                _inexp( copy._inexp ),
                _outexp( copy._outexp )
            {
            }

            OSG_REGISTER_TYPE( osgParticle,
                               ExplosionOperator )

            /// Set the center of shock wave
            void
            setCenter( const osg::vec3& c )
            {
                _center = c;
            }

            /// Get the center of shock wave
            const osg::vec3&
            getCenter() const
            {
                return _center;
            }

            /// Set the radius of wave peak
            void
            setRadius( float r )
            {
                _radius = r;
            }

            /// Get the radius of wave peak
            float
            getRadius() const
            {
                return _radius;
            }

            /// Set the acceleration scale
            void
            setMagnitude( float mag )
            {
                _magnitude = mag;
            }

            /// Get the acceleration scale
            float
            getMagnitude() const
            {
                return _magnitude;
            }

            /// Set the acceleration epsilon
            void
            setEpsilon( float eps )
            {
                _epsilon = eps;
            }

            /// Get the acceleration epsilon
            float
            getEpsilon() const
            {
                return _epsilon;
            }

            /// Set broadness of the strength of the wave
            void
            setSigma( float s )
            {
                _sigma = s;
            }

            /// Get broadness of the strength of the wave
            float
            getSigma() const
            {
                return _sigma;
            }

            /// Apply the acceleration to a particle. Do not call this method manually.
            inline void
            operate( Particle* P,
                     double    dt );

            /// Perform some initializations. Do not call this method manually.
            inline void
            beginOperate( Program* prg );

        protected:

            virtual ~ExplosionOperator()
            {
            }

            ExplosionOperator&
            operator=( const ExplosionOperator& )
            {
                return *this;
            }

            osg::vec3 _center;
            osg::vec3 _xf_center;
            float     _radius;
            float     _magnitude;
            float     _epsilon;
            float     _sigma;
            float     _inexp;
            float     _outexp;
    };

    // INLINE METHODS

    inline void
    ExplosionOperator::operate( Particle* P,
                                double    dt )
    {
        osg::vec3 dir               = P->getPosition() - _xf_center;
        float     length            = osg::length( dir );
        float     distanceFromWave2 = ( _radius - length ) * ( _radius - length );
        float     Gd                = exp( distanceFromWave2 * _inexp ) * _outexp;
        float factor = ( _magnitude * dt ) / ( length * ( _epsilon + length * length ) );
        P->addVelocity( dir * ( Gd * factor ) );
    }

    inline void
    ExplosionOperator::beginOperate( Program* prg )
    {
        if( prg->getReferenceFrame() == ModularProgram::RELATIVE_RF )
        {
            _xf_center = prg->transformLocalToWorld( _center );
        }
        else
        {
            _xf_center = _center;
        }

        float oneOverSigma = ( _sigma != 0.0F ? ( 1.0F / _sigma ) : 1.0F );
        _inexp             = -0.5F * oneOverSigma * oneOverSigma;
        _outexp            = oneOverSigma / sqrt( osg::PI * 2.0F );
    }

}
