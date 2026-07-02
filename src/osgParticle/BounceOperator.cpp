/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Bounces particles off a set of domain surfaces.
 * Applies reflection and energy loss on collision.
 */
// Written by Wang Rui, (C) 2010

#include <osgParticle/BounceOperator.hpp>

#include <osg/core/Notify.hpp>
#include <osgParticle/ModularProgram.hpp>

using namespace osgParticle;

void
BounceOperator::handleTriangle( const Domain& domain,
                                Particle*     P,
                                double        dt )
{
    osg::vec3 nextpos  = P->getPosition() + P->getVelocity() * static_cast<float>( dt );
    float     distance = domain.plane.distance( P->getPosition() );
    if( distance * domain.plane.distance( nextpos ) >= 0 )
    {
        return;
    }

    osg::vec3 normal   = osg::vec3( domain.plane.getNormal() );
    float     nv       = osg::dot( normal, P->getVelocity() );
    osg::vec3 hitPoint = P->getPosition() - P->getVelocity() * ( distance / nv );

    float     upos     = osg::dot( hitPoint - domain.v1, domain.s1 );
    float     vpos     = osg::dot( hitPoint - domain.v1, domain.s2 );
    if( upos < 0.0F || vpos < 0.0F || ( upos + vpos ) > 1.0F )
    {
        return;
    }

    // Compute tangential and normal components of velocity
    osg::vec3 vn = normal * nv;
    osg::vec3 vt = P->getVelocity() - vn;

    // Compute new velocity
    if( osg::length2( vt ) <= _cutoff )
    {
        P->setVelocity( vt - vn * _resilience );
    }
    else
    {
        P->setVelocity( vt * ( 1.0F - _friction ) - vn * _resilience );
    }
}

void
BounceOperator::handleRectangle( const Domain& domain,
                                 Particle*     P,
                                 double        dt )
{
    osg::vec3 nextpos  = P->getPosition() + P->getVelocity() * static_cast<float>( dt );
    float     distance = domain.plane.distance( P->getPosition() );
    if( distance * domain.plane.distance( nextpos ) >= 0 )
    {
        return;
    }

    osg::vec3 normal   = osg::vec3( domain.plane.getNormal() );
    float     nv       = osg::dot( normal, P->getVelocity() );
    osg::vec3 hitPoint = P->getPosition() - P->getVelocity() * ( distance / nv );

    float     upos     = osg::dot( hitPoint - domain.v1, domain.s1 );
    float     vpos     = osg::dot( hitPoint - domain.v1, domain.s2 );
    if( upos < 0.0F || upos > 1.0F || vpos < 0.0F || vpos > 1.0F )
    {
        return;
    }

    // Compute tangential and normal components of velocity
    osg::vec3 vn = normal * nv;
    osg::vec3 vt = P->getVelocity() - vn;

    // Compute new velocity
    if( osg::length2( vt ) <= _cutoff )
    {
        P->setVelocity( vt - vn * _resilience );
    }
    else
    {
        P->setVelocity( vt * ( 1.0F - _friction ) - vn * _resilience );
    }
}

void
BounceOperator::handlePlane( const Domain& domain,
                             Particle*     P,
                             double        dt )
{
    osg::vec3 nextpos  = P->getPosition() + P->getVelocity() * static_cast<float>( dt );
    float     distance = domain.plane.distance( P->getPosition() );
    if( distance * domain.plane.distance( nextpos ) >= 0 )
    {
        return;
    }

    osg::vec3 normal = osg::vec3( domain.plane.getNormal() );
    float     nv     = osg::dot( normal, P->getVelocity() );

    // Compute tangential and normal components of velocity
    osg::vec3 vn = normal * nv;
    osg::vec3 vt = P->getVelocity() - vn;

    // Compute new velocity
    if( osg::length2( vt ) <= _cutoff )
    {
        P->setVelocity( vt - vn * _resilience );
    }
    else
    {
        P->setVelocity( vt * ( 1.0F - _friction ) - vn * _resilience );
    }
}

void
BounceOperator::handleSphere( const Domain& domain,
                              Particle*     P,
                              double        dt )
{
    osg::vec3 nextpos   = P->getPosition() + P->getVelocity() * static_cast<float>( dt );
    float     distance1 = osg::length( P->getPosition() - domain.v1 );
    if( distance1 <= domain.r1 )    // Within the sphere
    {
        float distance2 = osg::length( nextpos - domain.v1 );
        if( distance2 <= domain.r1 )
        {
            return;
        }

        // Bounce back in if going outside
        osg::vec3 normal = domain.v1 - P->getPosition();
        normal           = osg::normalize( normal );
        float     nmag   = osg::dot( P->getVelocity(), normal );

        // Compute tangential and normal components of velocity
        osg::vec3 vn = normal * nmag;
        osg::vec3 vt = P->getVelocity() - vn;
        if( nmag < 0 )
        {
            vn = -vn;
        }

        // Compute new velocity
        float tanscale = ( osg::length2( vt ) <= _cutoff ) ? 1.0F : ( 1.0F - _friction );
        P->setVelocity( vt * tanscale + vn * _resilience );

        // Make sure the particle is fixed to stay inside
        nextpos   = P->getPosition() + P->getVelocity() * static_cast<float>( dt );
        distance2 = osg::length( nextpos - domain.v1 );
        if( distance2 > domain.r1 )
        {
            normal              = domain.v1 - nextpos;
            normal              = osg::normalize( normal );

            osg::vec3 wishPoint = domain.v1 - normal * ( 0.999F * domain.r1 );
            P->setVelocity( ( wishPoint - P->getPosition() ) /
                            static_cast<float>( dt ) );
        }
    }
    else    // Outside the sphere
    {
        float distance2 = osg::length( nextpos - domain.v1 );
        if( distance2 > domain.r1 )
        {
            return;
        }

        // Bounce back out if going inside
        osg::vec3 normal = P->getPosition() - domain.v1;
        normal           = osg::normalize( normal );
        float     nmag   = osg::dot( P->getVelocity(), normal );

        // Compute tangential and normal components of velocity
        osg::vec3 vn = normal * nmag;
        osg::vec3 vt = P->getVelocity() - vn;
        if( nmag < 0 )
        {
            vn = -vn;
        }

        // Compute new velocity
        float tanscale = ( osg::length2( vt ) <= _cutoff ) ? 1.0F : ( 1.0F - _friction );
        P->setVelocity( vt * tanscale + vn * _resilience );
    }
}

void
BounceOperator::handleDisk( const Domain& domain,
                            Particle*     P,
                            double        dt )
{
    osg::vec3 nextpos  = P->getPosition() + P->getVelocity() * static_cast<float>( dt );
    float     distance = domain.plane.distance( P->getPosition() );
    if( distance * domain.plane.distance( nextpos ) >= 0 )
    {
        return;
    }

    osg::vec3 normal   = osg::vec3( domain.plane.getNormal() );
    float     nv       = osg::dot( normal, P->getVelocity() );
    osg::vec3 hitPoint = P->getPosition() - P->getVelocity() * ( distance / nv );

    float     radius   = osg::length( hitPoint - domain.v1 );
    if( radius > domain.r1 || radius < domain.r2 )
    {
        return;
    }

    // Compute tangential and normal components of velocity
    osg::vec3 vn = normal * nv;
    osg::vec3 vt = P->getVelocity() - vn;

    // Compute new velocity
    if( osg::length2( vt ) <= _cutoff )
    {
        P->setVelocity( vt - vn * _resilience );
    }
    else
    {
        P->setVelocity( vt * ( 1.0F - _friction ) - vn * _resilience );
    }
}
