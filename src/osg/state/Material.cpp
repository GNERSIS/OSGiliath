/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Surface material properties (ambient, diffuse, specular, emission,
 * shininess). Applied as a state attribute, uploads to osg_FrontMaterial.
 */
#include <osg/state/Material.hpp>

#include <osg/core/BoundsChecking.hpp>
#include <osg/core/Notify.hpp>
#include <osg/state/State.hpp>

using namespace osg;

Material::Material()
{
    _colorMode           = OFF;

    _ambientFrontAndBack = true;
    _ambientFront.set( 0.2F, 0.2F, 0.2F, 1.0F );
    _ambientBack.set( 0.2F, 0.2F, 0.2F, 1.0F );

    _diffuseFrontAndBack = true;
    _diffuseFront.set( 0.8F, 0.8F, 0.8F, 1.0F );
    _diffuseBack.set( 0.8F, 0.8F, 0.8F, 1.0F );

    _specularFrontAndBack = true;
    _specularFront.set( 0.0F, 0.0F, 0.0F, 1.0F );
    _specularBack.set( 0.0F, 0.0F, 0.0F, 1.0F );

    _emissionFrontAndBack = true;
    _emissionFront.set( 0.0F, 0.0F, 0.0F, 1.0F );
    _emissionBack.set( 0.0F, 0.0F, 0.0F, 1.0F );

    _shininessFrontAndBack = true;
    _shininessFront        = 0.0F;
    _shininessBack         = 0.0F;
}

Material::~Material()
{
}

Material&
Material::operator=( const Material& rhs )
{
    if( &rhs == this )
    {
        return *this;
    }

    _colorMode             = rhs._colorMode;
    _ambientFrontAndBack   = rhs._ambientFrontAndBack;
    _ambientFront          = rhs._ambientFront;
    _ambientBack           = rhs._ambientBack;
    _diffuseFrontAndBack   = rhs._diffuseFrontAndBack;
    _diffuseFront          = rhs._diffuseFront;
    _diffuseBack           = rhs._diffuseBack;
    _specularFrontAndBack  = rhs._specularFrontAndBack;
    _specularFront         = rhs._specularFront;
    _specularBack          = rhs._specularBack;
    _emissionFrontAndBack  = rhs._emissionFrontAndBack;
    _emissionFront         = rhs._emissionFront;
    _emissionBack          = rhs._emissionBack;
    _shininessFrontAndBack = rhs._shininessFrontAndBack;
    _shininessFront        = rhs._shininessFront;
    _shininessBack         = rhs._shininessBack;

    return *this;
}

void
Material::setAmbient( Face        face,
                      const vec4& ambient )
{
    switch( face )
    {
        case( FRONT ) :
            _ambientFrontAndBack = false;
            _ambientFront        = ambient;
            // clampArray4BetweenRange(_ambientFront,0.0f,1.0f,"osg::Material::setAmbient(..)");
            break;
        case( BACK ) :
            _ambientFrontAndBack = false;
            _ambientBack         = ambient;
            // clampArray4BetweenRange(_ambientBack,0.0f,1.0f,"Material::setAmbient(..)");
            break;
        case( FRONT_AND_BACK ) :
            _ambientFrontAndBack = true;
            _ambientFront        = ambient;
            // clampArray4BetweenRange(_ambientFront,0.0f,1.0f,"Material::setAmbient(..)");
            _ambientBack = _ambientFront;
            break;
        default :
            OSG_NOTICE << "Notice: invalid Face passed to Material::setAmbient()."
                       << std::endl;
    }
}

const vec4&
Material::getAmbient( Face face ) const
{
    switch( face )
    {
        case( FRONT ) :
            return _ambientFront;
        case( BACK ) :
            return _ambientBack;
        case( FRONT_AND_BACK ) :
            if( !_ambientFrontAndBack )
            {
                OSG_NOTICE
                    << "Notice: Material::getAmbient(FRONT_AND_BACK) called on material "
                    << std::endl;
                OSG_NOTICE << "        with separate FRONT and BACK ambient colors."
                           << std::endl;
            }
            return _ambientFront;
    }
    OSG_NOTICE << "Notice: invalid Face passed to Material::getAmbient()." << std::endl;
    return _ambientFront;
}

void
Material::setDiffuse( Face        face,
                      const vec4& diffuse )
{
    switch( face )
    {
        case( FRONT ) :
            _diffuseFrontAndBack = false;
            _diffuseFront        = diffuse;
            // clampArray4BetweenRange(_diffuseFront,0.0f,1.0f,"Material::setDiffuse(..)");
            break;
        case( BACK ) :
            _diffuseFrontAndBack = false;
            _diffuseBack         = diffuse;
            // clampArray4BetweenRange(_diffuseBack,0.0f,1.0f,"Material::setDiffuse(..)");
            break;
        case( FRONT_AND_BACK ) :
            _diffuseFrontAndBack = true;
            _diffuseFront        = diffuse;
            // clampArray4BetweenRange(_diffuseFront,0.0f,1.0f,"Material::setDiffuse(..)");
            _diffuseBack = _diffuseFront;
            break;
        default :
            OSG_NOTICE << "Notice: invalid Face passed to Material::setDiffuse()."
                       << std::endl;
            break;
    }
}

const vec4&
Material::getDiffuse( Face face ) const
{
    switch( face )
    {
        case( FRONT ) :
            return _diffuseFront;
        case( BACK ) :
            return _diffuseBack;
        case( FRONT_AND_BACK ) :
            if( !_diffuseFrontAndBack )
            {
                OSG_NOTICE
                    << "Notice: Material::getDiffuse(FRONT_AND_BACK) called on material "
                    << std::endl;
                OSG_NOTICE << "        with separate FRONT and BACK diffuse colors."
                           << std::endl;
            }
            return _diffuseFront;
    }
    OSG_NOTICE << "Notice: invalid Face passed to Material::getDiffuse()." << std::endl;
    return _diffuseFront;
}

void
Material::setSpecular( Face        face,
                       const vec4& specular )
{
    switch( face )
    {
        case( FRONT ) :
            _specularFrontAndBack = false;
            _specularFront        = specular;
            // clampArray4BetweenRange(_specularFront,0.0f,1.0f,"Material::setSpecular(..)");
            break;
        case( BACK ) :
            _specularFrontAndBack = false;
            _specularBack         = specular;
            // clampArray4BetweenRange(_specularBack,0.0f,1.0f,"Material::setSpecular(..)");
            break;
        case( FRONT_AND_BACK ) :
            _specularFrontAndBack = true;
            _specularFront        = specular;
            // clampArray4BetweenRange(_specularFront,0.0f,1.0f,"Material::setSpecular(..)");
            _specularBack = _specularFront;
            break;
        default :
            OSG_NOTICE << "Notice: invalid Face passed to Material::setSpecular()."
                       << std::endl;
            break;
    }
}

const vec4&
Material::getSpecular( Face face ) const
{
    switch( face )
    {
        case( FRONT ) :
            return _specularFront;
        case( BACK ) :
            return _specularBack;
        case( FRONT_AND_BACK ) :
            if( !_specularFrontAndBack )
            {
                OSG_NOTICE << "Notice: Material::getSpecular(FRONT_AND_BACK) called on "
                              "material "
                           << std::endl;
                OSG_NOTICE << "        with separate FRONT and BACK specular colors."
                           << std::endl;
            }
            return _specularFront;
    }
    OSG_NOTICE << "Notice: invalid Face passed to Material::getSpecular()." << std::endl;
    return _specularFront;
}

void
Material::setEmission( Face        face,
                       const vec4& emission )
{
    switch( face )
    {
        case( FRONT ) :
            _emissionFrontAndBack = false;
            _emissionFront        = emission;
            // clampArray4BetweenRange(_emissionFront,0.0f,1.0f,"Material::setEmission(..)");
            break;
        case( BACK ) :
            _emissionFrontAndBack = false;
            _emissionBack         = emission;
            // clampArray4BetweenRange(_emissionBack,0.0f,1.0f,"Material::setEmission(..)");
            break;
        case( FRONT_AND_BACK ) :
            _emissionFrontAndBack = true;
            _emissionFront        = emission;
            // clampArray4BetweenRange(_emissionFront,0.0f,1.0f,"Material::setEmission(..)");
            _emissionBack = _emissionFront;
            break;
        default :
            OSG_NOTICE << "Notice: invalid Face passed to Material::setEmission()."
                       << std::endl;
            break;
    }
}

const vec4&
Material::getEmission( Face face ) const
{
    switch( face )
    {
        case( FRONT ) :
            return _emissionFront;
        case( BACK ) :
            return _emissionBack;
        case( FRONT_AND_BACK ) :
            if( !_emissionFrontAndBack )
            {
                OSG_NOTICE << "Notice: Material::getEmission(FRONT_AND_BACK) called on "
                              "material "
                           << std::endl;
                OSG_NOTICE << "        with separate FRONT and BACK emission colors."
                           << std::endl;
            }
            return _emissionFront;
    }
    OSG_NOTICE << "Notice: invalid Face passed to Material::getEmission()." << std::endl;
    return _emissionFront;
}

void
Material::setShininess( Face  face,
                        float shininess )
{
    clampBetweenRange( shininess, 0.0F, 128.0F, "Material::setShininess()" );

    switch( face )
    {
        case( FRONT ) :
            _shininessFrontAndBack = false;
            _shininessFront        = shininess;
            break;
        case( BACK ) :
            _shininessFrontAndBack = false;
            _shininessBack         = shininess;
            break;
        case( FRONT_AND_BACK ) :
            _shininessFrontAndBack = true;
            _shininessFront        = shininess;
            _shininessBack         = shininess;
            break;
        default :
            OSG_NOTICE << "Notice: invalid Face passed to Material::setShininess()."
                       << std::endl;
            break;
    }
}

float
Material::getShininess( Face face ) const
{
    switch( face )
    {
        case( FRONT ) :
            return _shininessFront;
        case( BACK ) :
            return _shininessBack;
        case( FRONT_AND_BACK ) :
            if( !_shininessFrontAndBack )
            {
                OSG_NOTICE << "Notice: Material::getShininess(FRONT_AND_BACK) called on "
                              "material "
                           << std::endl;
                OSG_NOTICE << "        with separate FRONT and BACK shininess colors."
                           << std::endl;
            }
            return _shininessFront;
    }
    OSG_NOTICE << "Notice: invalid Face passed to Material::getShininess()."
               << std::endl;
    return _shininessFront;
}

void
Material::setTransparency( Face  face,
                           float transparency )
{
    // clampBetweenRange(transparency,0.0f,1.0f,"Material::setTransparency()");

    if( face == FRONT || face == FRONT_AND_BACK )
    {
        _ambientFront[3]  = 1.0F - transparency;
        _diffuseFront[3]  = 1.0F - transparency;
        _specularFront[3] = 1.0F - transparency;
        _emissionFront[3] = 1.0F - transparency;
    }

    if( face == BACK || face == FRONT_AND_BACK )
    {
        _ambientBack[3]  = 1.0F - transparency;
        _diffuseBack[3]  = 1.0F - transparency;
        _specularBack[3] = 1.0F - transparency;
        _emissionBack[3] = 1.0F - transparency;
    }
}

void
Material::setAlpha( Face  face,
                    float alpha )
{
    clampBetweenRange( alpha, 0.0F, 1.0F, "Material::setAlpha()" );

    if( face == FRONT || face == FRONT_AND_BACK )
    {
        _ambientFront[3]  = alpha;
        _diffuseFront[3]  = alpha;
        _specularFront[3] = alpha;
        _emissionFront[3] = alpha;
    }

    if( face == BACK || face == FRONT_AND_BACK )
    {
        _ambientBack[3]  = alpha;
        _diffuseBack[3]  = alpha;
        _specularBack[3] = alpha;
        _emissionBack[3] = alpha;
    }
}

void
Material::apply( State& state ) const
{
    state.Color( _diffuseFront.r, _diffuseFront.g, _diffuseFront.b, _diffuseFront.a );
    state.setMaterialUniforms( *this );
}
