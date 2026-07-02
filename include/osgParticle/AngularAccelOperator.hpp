/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Applies constant angular acceleration to particles.
 * Adds spin around particle axes over time.
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

    /**    An operator class that applies a constant angular acceleration to
     *     the particles.
     */
    class AngularAccelOperator : public osg::Inherit<Operator, AngularAccelOperator>
    {
        public:

            inline AngularAccelOperator();
            inline AngularAccelOperator( const AngularAccelOperator& copy,
                                         const osg::CopyOp&          copyop =
                                             osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgParticle,
                               AngularAccelOperator )

            /// Get the angular acceleration vector.
            inline const osg::vec3&
            getAngularAcceleration() const;

            /// Set the angular acceleration vector.
            inline void
            setAngularAcceleration( const osg::vec3& v );

            /// Apply the angular acceleration to a particle. Do not call this method
            /// manually.
            inline void
            operate( Particle* P,
                     double    dt );

            /// Perform some initializations. Do not call this method manually.
            inline void
            beginOperate( Program* prg );

        protected:

            virtual ~AngularAccelOperator()
            {
            }

            AngularAccelOperator&
            operator=( const AngularAccelOperator& )
            {
                return *this;
            }

        private:

            osg::vec3 _angul_araccel;
            osg::vec3 _xf_angul_araccel;
    };

    // INLINE FUNCTIONS

    inline AngularAccelOperator::AngularAccelOperator() :
        _angul_araccel( 0,
                        0,
                        0 )
    {
    }

    inline AngularAccelOperator::AngularAccelOperator( const AngularAccelOperator& copy,
                                                       const osg::CopyOp& copyop ) :
        Inherit( copy,
                 copyop ),
        _angul_araccel( copy._angul_araccel )
    {
    }

    inline const osg::vec3&
    AngularAccelOperator::getAngularAcceleration() const
    {
        return _angul_araccel;
    }

    inline void
    AngularAccelOperator::setAngularAcceleration( const osg::vec3& v )
    {
        _angul_araccel = v;
    }

    inline void
    AngularAccelOperator::operate( Particle* P,
                                   double    dt )
    {
        P->addAngularVelocity( _xf_angul_araccel * dt );
    }

    inline void
    AngularAccelOperator::beginOperate( Program* prg )
    {
        if( prg->getReferenceFrame() == ModularProgram::RELATIVE_RF )
        {
            _xf_angul_araccel = prg->rotateLocalToWorld( _angul_araccel );
        }
        else
        {
            _xf_angul_araccel = _angul_araccel;
        }
    }

}
