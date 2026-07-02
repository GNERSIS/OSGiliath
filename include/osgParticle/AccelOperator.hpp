/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Applies constant acceleration (e.g., gravity) to particles.
 * Configurable 3D acceleration vector.
 */
// osgParticle - Copyright (C) 2002 Marco Jez

#pragma once

#include <osg/core/CopyOp.hpp>
#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>
#include <osg/maths/vec3.hpp>
#include <osgParticle/ModularProgram.hpp>
#include <osgParticle/Operator.hpp>
#include <osgParticle/Particle.hpp>

namespace osgParticle
{

    /**    An operator class that applies a constant acceleration to the particles.
     */
    class AccelOperator : public osg::Inherit<Operator, AccelOperator>
    {
        public:

            inline AccelOperator();
            inline AccelOperator( const AccelOperator& copy,
                                  const osg::CopyOp&   copyop =
                                      osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgParticle,
                               AccelOperator )

            /// Get the acceleration vector.
            inline const osg::vec3&
            getAcceleration() const;

            /// Set the acceleration vector.
            inline void
            setAcceleration( const osg::vec3& v );

            /** Quickly set the acceleration vector to the gravity on earth (0, 0,
               -9.81). The acceleration will be multiplied by the <CODE>scale</CODE>
               parameter.
            */
            inline void
            setToGravity( float scale = 1 );

            /// Apply the acceleration to a particle. Do not call this method manually.
            inline void
            operate( Particle* P,
                     double    dt );

            /// Perform some initializations. Do not call this method manually.
            inline void
            beginOperate( Program* prg );

        protected:

            virtual ~AccelOperator()
            {
            }

            AccelOperator&
            operator=( const AccelOperator& )
            {
                return *this;
            }

        private:

            osg::vec3 _accel;
            osg::vec3 _xf_accel;
    };

    // INLINE FUNCTIONS

    inline AccelOperator::AccelOperator() :
        _accel( 0,
                0,
                0 )
    {
    }

    inline AccelOperator::AccelOperator( const AccelOperator& copy,
                                         const osg::CopyOp&   copyop ) :
        Inherit( copy,
                 copyop ),
        _accel( copy._accel )
    {
    }

    inline const osg::vec3&
    AccelOperator::getAcceleration() const
    {
        return _accel;
    }

    inline void
    AccelOperator::setAcceleration( const osg::vec3& v )
    {
        _accel = v;
    }

    inline void
    AccelOperator::setToGravity( float scale )
    {
        _accel.set( 0, 0, -9.80665F * scale );
    }

    inline void
    AccelOperator::operate( Particle* P,
                            double    dt )
    {
        P->addVelocity( _xf_accel * static_cast<float>( dt ) );
    }

    inline void
    AccelOperator::beginOperate( Program* prg )
    {
        if( prg->getReferenceFrame() == ModularProgram::RELATIVE_RF )
        {
            _xf_accel = prg->rotateLocalToWorld( _accel );
        }
        else
        {
            _xf_accel = _accel;
        }
    }

}
