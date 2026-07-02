#include <osgParticle/FluidFrictionOperator.hpp>

#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osgParticle/ModularProgram.hpp>
#include <osgParticle/Operator.hpp>
#include <osgParticle/Particle.hpp>

osgParticle::FluidFrictionOperator::FluidFrictionOperator() :
    _coeff_A( 0 ),
    _coeff_B( 0 ),
    _density( 0 ),
    _viscosity( 0 ),
    _ovr_rad( 0 ),
    _current_program( 0 )
{
    setFluidToAir();
}

osgParticle::FluidFrictionOperator::FluidFrictionOperator(
    const FluidFrictionOperator& copy,
    const osg::CopyOp&           copyop
) :
    Inherit( copy,
             copyop ),
    _coeff_A( copy._coeff_A ),
    _coeff_B( copy._coeff_B ),
    _density( copy._density ),
    _viscosity( copy._viscosity ),
    _ovr_rad( copy._ovr_rad ),
    _current_program( 0 )
{
}

void
osgParticle::FluidFrictionOperator::operate( Particle* P,
                                             double    dt )
{
    float     r  = ( _ovr_rad > 0 ) ? _ovr_rad : P->getRadius();
    osg::vec3 v  = P->getVelocity() - _wind;

    float     vm = osg::length( v );
    if( vm > 0.0F )
    {
        v = osg::normalize( v );
    }
    float     R = _coeff_A * r * vm + _coeff_B * r * r * vm * vm;

    osg::vec3 Fr( -R * v.x, -R * v.y, -R * v.z );

#if 0
    // Commenting out rotation of force vector rotation from local to world as the particle velocity itself
    // should already be in world coords so shouldn't need rotating.
    if (_current_program->getReferenceFrame() == ModularProgram::RELATIVE_RF) {
        Fr = _current_program->rotateLocalToWorld(Fr);
    }
#endif

    // correct unwanted velocity increments
    osg::vec3 dv  = Fr * P->getMassInv() * static_cast<float>( dt );
    float     dvl = osg::length( dv );
    if( dvl > vm )
    {
        dv *= vm / dvl;
    }

    P->addVelocity( dv );
}
