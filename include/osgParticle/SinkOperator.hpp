/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Removes particles that enter a defined domain region.
 * Used for ground collision and boundary conditions.
 */
// Written by Wang Rui, (C) 2010

#pragma once

#include <osg/core/Inherit.hpp>
#include <osgParticle/DomainOperator.hpp>
#include <osgParticle/Particle.hpp>

namespace osgParticle
{

    /** A sink operator kills particles if positions or velocities inside/outside the
       specified domain. Refer to David McAllister's Particle System API
       (http://www.particlesystems.org)
    */
    class OSGPARTICLE_EXPORT SinkOperator
        : public osg::Inherit<DomainOperator, SinkOperator>
    {
        public:

            enum SinkTarget
            {
                SINK_POSITION,
                SINK_VELOCITY,
                SINK_ANGULAR_VELOCITY
            };

            enum SinkStrategy
            {
                SINK_INSIDE,
                SINK_OUTSIDE
            };

            SinkOperator() :
                _sinkTarget( SINK_POSITION ),
                _sinkStrategy( SINK_INSIDE )
            {
            }

            SinkOperator( const SinkOperator& copy,
                          const osg::CopyOp&  copyop = osg::CopyOp::SHALLOW_COPY ) :
                Inherit( copy,
                         copyop ),
                _sinkTarget( copy._sinkTarget ),
                _sinkStrategy( copy._sinkStrategy )
            {
            }

            OSG_REGISTER_TYPE( osgParticle,
                               SinkOperator )

            /// Set the sink strategy
            void
            setSinkTarget( SinkTarget so )
            {
                _sinkTarget = so;
            }

            /// Get the sink strategy
            SinkTarget
            getSinkTarget() const
            {
                return _sinkTarget;
            }

            /// Set the sink strategy
            void
            setSinkStrategy( SinkStrategy ss )
            {
                _sinkStrategy = ss;
            }

            /// Get the sink strategy
            SinkStrategy
            getSinkStrategy() const
            {
                return _sinkStrategy;
            }

            /// Perform some initializations. Do not call this method manually.
            void
            beginOperate( Program* prg );

        protected:

            virtual ~SinkOperator()
            {
            }

            SinkOperator&
            operator=( const SinkOperator& )
            {
                return *this;
            }

            virtual void
            handlePoint( const Domain& domain,
                         Particle*     P,
                         double        dt );
            virtual void
            handleLineSegment( const Domain& domain,
                               Particle*     P,
                               double        dt );
            virtual void
            handleTriangle( const Domain& domain,
                            Particle*     P,
                            double        dt );
            virtual void
            handleRectangle( const Domain& domain,
                             Particle*     P,
                             double        dt );
            virtual void
            handlePlane( const Domain& domain,
                         Particle*     P,
                         double        dt );
            virtual void
            handleSphere( const Domain& domain,
                          Particle*     P,
                          double        dt );
            virtual void
            handleBox( const Domain& domain,
                       Particle*     P,
                       double        dt );
            virtual void
            handleDisk( const Domain& domain,
                        Particle*     P,
                        double        dt );

            inline const osg::vec3&
            getValue( Particle* P );
            inline void
                         kill( Particle* P,
                               bool      insideDomain );

            SinkTarget   _sinkTarget;
            SinkStrategy _sinkStrategy;
    };

    // INLINE METHODS

    inline const osg::vec3&
    SinkOperator::getValue( Particle* P )
    {
        switch( _sinkTarget )
        {
            case SINK_VELOCITY :
                return P->getVelocity();
            case SINK_ANGULAR_VELOCITY :
                return P->getAngularVelocity();
            case SINK_POSITION :
            default :
                return P->getPosition();
        }
    }

    inline void
    SinkOperator::kill( Particle* P,
                        bool      insideDomain )
    {
        if( !( ( _sinkStrategy == SINK_INSIDE ) ^ insideDomain ) )
        {
            P->kill();
        }
    }

}
