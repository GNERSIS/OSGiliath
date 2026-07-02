/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Applies a constant force to all particles.
 * Similar to AccelOperator but specified as force, not acceleration.
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

    /** An operator that applies a constant force to the particles.
     * Remember that if the mass of particles is expressed in kg and the lengths are
     * expressed in meters, then the force should be expressed in Newtons.
     */
    class ForceOperator : public osg::Inherit<Operator, ForceOperator>
    {
        public:

            inline ForceOperator();
            inline ForceOperator( const ForceOperator& copy,
                                  const osg::CopyOp&   copyop =
                                      osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgParticle,
                               ForceOperator )

            /// Get the force vector.
            inline const osg::vec3&
            getForce() const;

            /// Set the force vector.
            inline void
            setForce( const osg::vec3& f );

            /// Apply the force to a particle. Do not call this method manually.
            inline void
            operate( Particle* P,
                     double    dt );

            /// Perform some initialization. Do not call this method manually.
            inline void
            beginOperate( Program* prg );

        protected:

            virtual ~ForceOperator() {};

            ForceOperator&
            operator=( const ForceOperator& )
            {
                return *this;
            }

        private:

            osg::vec3 _force;
            osg::vec3 _xf_force;
    };

    // INLINE FUNCTIONS

    inline ForceOperator::ForceOperator() :
        _force( 0,
                0,
                0 )
    {
    }

    inline ForceOperator::ForceOperator( const ForceOperator& copy,
                                         const osg::CopyOp&   copyop ) :
        Inherit( copy,
                 copyop ),
        _force( copy._force )
    {
    }

    inline const osg::vec3&
    ForceOperator::getForce() const
    {
        return _force;
    }

    inline void
    ForceOperator::setForce( const osg::vec3& v )
    {
        _force = v;
    }

    inline void
    ForceOperator::operate( Particle* P,
                            double    dt )
    {
        P->addVelocity( _xf_force * ( P->getMassInv() * dt ) );
    }

    inline void
    ForceOperator::beginOperate( Program* prg )
    {
        if( prg->getReferenceFrame() == ModularProgram::RELATIVE_RF )
        {
            _xf_force = prg->rotateLocalToWorld( _force );
        }
        else
        {
            _xf_force = _force;
        }
    }

}
