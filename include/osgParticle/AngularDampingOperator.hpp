/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Damps particle angular velocity over time.
 * Reduces spin rate for realistic tumbling decay.
 */
// Written by Wang Rui, (C) 2010

#pragma once

#include <osg/core/Inherit.hpp>
#include <osgParticle/Operator.hpp>
#include <osgParticle/Particle.hpp>

namespace osgParticle
{

    /** A angular damping operator applies damping constant to particle's angular
       velocity. Refer to David McAllister's Particle System API
       (http://www.particlesystems.org)
    */
    class AngularDampingOperator : public osg::Inherit<Operator, AngularDampingOperator>
    {
        public:

            AngularDampingOperator() :
                _cutoffLow( 0.0F ),
                _cutoffHigh( FLT_MAX )
            {
            }

            AngularDampingOperator( const AngularDampingOperator& copy,
                                    const osg::CopyOp&            copyop =
                                        osg::CopyOp::SHALLOW_COPY ) :
                Inherit( copy,
                         copyop ),
                _damping( copy._damping ),
                _cutoffLow( copy._cutoffLow ),
                _cutoffHigh( copy._cutoffHigh )
            {
            }

            OSG_REGISTER_TYPE( osgParticle,
                               AngularDampingOperator )

            /// Set the damping factors
            void
            setDamping( float x,
                        float y,
                        float z )
            {
                _damping.set( x, y, z );
            }

            void
            setDamping( const osg::vec3& damping )
            {
                _damping = damping;
            }

            /// Set the damping factors to one value
            void
            setDamping( float x )
            {
                _damping.set( x, x, x );
            }

            /// Get the damping factors
            void
            getDamping( float& x,
                        float& y,
                        float& z ) const
            {
                x = _damping.x;
                y = _damping.y;
                z = _damping.z;
            }

            const osg::vec3&
            getDamping() const
            {
                return _damping;
            }

            /// Set the velocity cutoff factors
            void
            setCutoff( float low,
                       float high )
            {
                _cutoffLow  = low;
                _cutoffHigh = high;
            }

            void
            setCutoffLow( float low )
            {
                _cutoffLow = low;
            }

            void
            setCutoffHigh( float low )
            {
                _cutoffHigh = low;
            }

            /// Get the velocity cutoff factors
            void
            getCutoff( float& low,
                       float& high ) const
            {
                low  = _cutoffLow;
                high = _cutoffHigh;
            }

            float
            getCutoffLow() const
            {
                return _cutoffLow;
            }

            float
            getCutoffHigh() const
            {
                return _cutoffHigh;
            }

            /// Apply the acceleration to a particle. Do not call this method manually.
            inline void
            operate( Particle* P,
                     double    dt );

        protected:

            virtual ~AngularDampingOperator()
            {
            }

            AngularDampingOperator&
            operator=( const AngularDampingOperator& )
            {
                return *this;
            }

            osg::vec3 _damping;
            float     _cutoffLow;
            float     _cutoffHigh;
    };

    // INLINE METHODS

    inline void
    AngularDampingOperator::operate( Particle* P,
                                     double    dt )
    {
        const osg::vec3& vel     = P->getAngularVelocity();
        float            length2 = osg::length2( vel );
        if( length2 >= _cutoffLow && length2 <= _cutoffHigh )
        {
            osg::vec3 newvel( vel.x * ( 1.0F - ( 1.0F - _damping.x ) * dt ),
                              vel.y * ( 1.0F - ( 1.0F - _damping.y ) * dt ),
                              vel.z * ( 1.0F - ( 1.0F - _damping.z ) * dt ) );
            P->setAngularVelocity( newvel );
        }
    }

}
