/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Removes particles that enter a defined domain region.
 * Used for ground collision and boundary conditions.
 */
// Written by Wang Rui, (C) 2010

#include <osgParticle/SinkOperator.hpp>

#include <osg/core/Notify.hpp>
#include <osgParticle/ModularProgram.hpp>

#define SINK_EPSILON 1E-3

using namespace osgParticle;

void
SinkOperator::beginOperate( Program* prg )
{
    // Don't transform domains if they are used for sinking velocities
    if( _sinkTarget == SINK_POSITION )
    {
        DomainOperator::beginOperate( prg );
    }
}

void
SinkOperator::handlePoint( const Domain& domain,
                           Particle*     P,
                           double /*dt*/ )
{
    const osg::vec3& value = getValue( P );
    kill( P, domain.v1 == value );
}

void
SinkOperator::handleLineSegment( const Domain& domain,
                                 Particle*     P,
                                 double /*dt*/ )
{
    const osg::vec3& value  = getValue( P );
    osg::vec3        offset = value - domain.v1, normal = domain.v2 - domain.v1;
    normal     = osg::normalize( normal );

    float diff = static_cast<float>( fabs( osg::dot( normal, offset ) -
                                           osg::length( offset ) ) ) /
                 domain.r1;
    kill( P, diff < SINK_EPSILON );
}

void
SinkOperator::handleTriangle( const Domain& domain,
                              Particle*     P,
                              double /*dt*/ )
{
    bool             insideDomain = false;
    const osg::vec3& value        = getValue( P );
    osg::vec3        offset       = value - domain.v1;
    if( osg::dot( offset, osg::vec3( domain.plane.getNormal() ) ) > SINK_EPSILON )
    {
        insideDomain = false;
    }
    else
    {
        float upos   = osg::dot( offset, domain.s1 );
        float vpos   = osg::dot( offset, domain.s2 );
        insideDomain = !( upos < 0.0F || vpos < 0.0F || ( upos + vpos ) > 1.0F );
    }
    kill( P, insideDomain );
}

void
SinkOperator::handleRectangle( const Domain& domain,
                               Particle*     P,
                               double /*dt*/ )
{
    bool             insideDomain = false;
    const osg::vec3& value        = getValue( P );
    osg::vec3        offset       = value - domain.v1;
    if( osg::dot( offset, osg::vec3( domain.plane.getNormal() ) ) > SINK_EPSILON )
    {
        insideDomain = false;
    }
    else
    {
        float upos   = osg::dot( offset, domain.s1 );
        float vpos   = osg::dot( offset, domain.s2 );
        insideDomain = !( upos < 0.0F || upos > 1.0F || vpos < 0.0F || vpos > 1.0F );
    }
    kill( P, insideDomain );
}

void
SinkOperator::handlePlane( const Domain& domain,
                           Particle*     P,
                           double /*dt*/ )
{
    const osg::vec3& value = getValue( P );
    bool             insideDomain =
        ( osg::dot( osg::vec3( domain.plane.getNormal() ), value ) >= -domain.plane[3] );
    kill( P, insideDomain );
}

void
SinkOperator::handleSphere( const Domain& domain,
                            Particle*     P,
                            double /*dt*/ )
{
    const osg::vec3& value = getValue( P );
    float            r     = osg::length( value - domain.v1 );
    kill( P, r <= domain.r1 );
}

void
SinkOperator::handleBox( const Domain& domain,
                         Particle*     P,
                         double /*dt*/ )
{
    const osg::vec3& value        = getValue( P );
    bool             insideDomain = !( ( value.x < domain.v1.x ) ||
                                       ( value.x > domain.v2.x ) ||
                                       ( value.y < domain.v1.y ) ||
                                       ( value.y > domain.v2.y ) ||
                                       ( value.z < domain.v1.z ) ||
                                       ( value.z > domain.v2.z ) );
    kill( P, insideDomain );
}

void
SinkOperator::handleDisk( const Domain& domain,
                          Particle*     P,
                          double /*dt*/ )
{
    bool             insideDomain = false;
    const osg::vec3& value        = getValue( P );
    osg::vec3        offset       = value - domain.v1;
    if( osg::dot( offset, domain.v2 ) > SINK_EPSILON )
    {
        insideDomain = false;
    }
    else
    {
        float length = osg::length( offset );
        insideDomain = ( length <= domain.r1 && length >= domain.r2 );
    }
    kill( P, insideDomain );
}
