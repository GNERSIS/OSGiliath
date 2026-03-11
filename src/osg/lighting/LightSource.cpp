/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Positional node that places a Light in the scene graph.
 * Provides local/absolute reference frame for light positioning.
 */
#include <osg/lighting/LightSource.hpp>

using namespace osg;

LightSource::LightSource() :
    _value( StateAttribute::ON ),
    _referenceFrame( RELATIVE_RF )
{
    // switch off culling of light source nodes by default.
    setCullingActive( false );
    setStateSet( new StateSet );
    _light = new Light;
}

LightSource::~LightSource()
{
    // ref_ptr<> automactially decrements the reference count of attached lights.
}

void
LightSource::setReferenceFrame( ReferenceFrame rf )
{
    _referenceFrame = rf;
}

void
LightSource::setLight( Light* light )
{
    _light = light;
    setLocalStateSetModes( _value );
}

// Set the GLModes on StateSet associated with the LightSource.
// Also adds Light as a StateAttribute so Light::apply() is called during
// state application, delivering actual light parameters to shader uniforms.
void
LightSource::setStateSetModes( StateSet&                   stateset,
                               StateAttribute::GLModeValue value ) const
{
    if( _light.valid() )
    {
        stateset.setAttributeAndModes( _light.get(), value );
    }
}

void
LightSource::setLocalStateSetModes( StateAttribute::GLModeValue value )
{
    if( !_stateset )
    {
        setStateSet( new StateSet );
    }

    _stateset->clear();
    setStateSetModes( *_stateset, value );
}

sphere
LightSource::computeBound() const
{
    sphere bsphere( Group::computeBound() );

    if( _light.valid() && _referenceFrame == RELATIVE_RF )
    {
        const vec4& pos = _light->getPosition();
        if( pos[3] != 0.0F )
        {
            float div = 1.0F / pos[3];
            bsphere.expandBy( vec3( pos[0] * div, pos[1] * div, pos[2] * div ) );
        }
    }

    return bsphere;
}

void
LightSource::setThreadSafeRefUnref( bool threadSafe )
{
    Group::setThreadSafeRefUnref( threadSafe );

    if( _light.valid() )
    {
        _light->setThreadSafeRefUnref( threadSafe );
    }
}
