/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Light source parameters (position, direction, colors, attenuation,
 * spotlight). Uploads to osg_LightSource uniform struct.
 */
#include <osg/lighting/Light.hpp>

#include <osg/core/Notify.hpp>
#include <osg/state/State.hpp>
#include <osg/state/StateSet.hpp>

using namespace osg;

Light::Light( void )
{
    init();
}

Light::Light( unsigned int lightnum )
{
    init();
    _lightnum = static_cast<int>( lightnum );
}

Light::~Light( void )
{
}

void
Light::init( void )
{
    _lightnum = 0;
    _ambient.set( 0.05F, 0.05F, 0.05F, 1.0F );
    _diffuse.set( 0.8F, 0.8F, 0.8F, 1.0F );
    _specular.set( 0.05F, 0.05F, 0.05F, 1.0F );
    _position.set( 0.0F, 0.0F, 1.0F, 0.0F );
    _direction.set( 0.0F, 0.0F, -1.0F );
    _spot_exponent         = 0.0F;
    _spot_cutoff           = 180.0F;
    _constant_attenuation  = 1.0F;
    _linear_attenuation    = 0.0F;
    _quadratic_attenuation = 0.0F;

    // OSG_DEBUG << "_ambient "<<_ambient<<std::endl;
    // OSG_DEBUG << "_diffuse "<<_diffuse<<std::endl;
    // OSG_DEBUG << "_specular "<<_specular<<std::endl;
    // OSG_DEBUG << "_position "<<_position<<std::endl;
    // OSG_DEBUG << "_direction "<<_direction<<std::endl;
    // OSG_DEBUG << "_spot_exponent "<<_spot_exponent<<std::endl;
    // OSG_DEBUG << "_spot_cutoff "<<_spot_cutoff<<std::endl;
    // OSG_DEBUG << "_constant_attenuation "<<_constant_attenuation<<std::endl;
    // OSG_DEBUG << "_linear_attenuation "<<_linear_attenuation<<std::endl;
    // OSG_DEBUG << "_quadratic_attenuation "<<_quadratic_attenuation<<std::endl;
}

void
Light::setLightNum( int num )
{
    if( _lightnum == num )
    {
        return;
    }

    ReassignToParents needToReassingToParentsWhenMemberValueChanges( this );

    _lightnum = num;
}

void
Light::captureLightState()
{
    OSG_NOTICE << "Warning: Light::captureLightState() - not supported." << std::endl;
}

void
Light::apply( State& state ) const
{
    state.setLightUniforms( *this );
}
