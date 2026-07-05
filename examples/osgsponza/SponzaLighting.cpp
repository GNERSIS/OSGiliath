/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#include "SponzaLighting.hpp"
#include "SponzaOptions.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <osg/GL>
#include <osg/images/Image.hpp>
#include <osg/lighting/Light.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/maths/transform.hpp>
#include <osg/nodes/Node.hpp>
#include <osg/state/StateSet.hpp>
#include <osg/state/Uniform.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/ReadFile.hpp>

namespace sponza
{

    osg::vec3
    scaledColor( const osg::vec3& color,
                 float            scale )
    {
        return osg::vec3( color.r * scale, color.g * scale, color.b * scale );
    }

    float
    computeMaxMipLevel( const osg::Image& image )
    {
        const int maxDimension = std::max( image.s(), image.t() );
        return maxDimension > 0
                 ? std::floor( std::log2( static_cast<float>( maxDimension ) ) )
                 : 0.0F;
    }

    osg::ref_ptr<osg::Image>
    loadEnvironmentImage()
    {
        osg::ref_ptr<osg::Image> image =
            osgDB::readRefImageFile( "textures/kloppenheim_05_4k.hdr" );
        if( !image )
        {
            image = osgDB::readRefImageFile( "kloppenheim_05_4k.hdr" );
        }
        return image;
    }

    osg::ref_ptr<osg::Texture2D>
    createEnvironmentTexture( osg::Image* image )
    {
        osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
        texture->setImage( image );
        texture->setInternalFormat( GL_RGB16F );
        texture->setFilter( osg::Texture2D::MIN_FILTER,
                            osg::Texture2D::LINEAR_MIPMAP_LINEAR );
        texture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
        texture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT );
        texture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE );
        texture->setUseHardwareMipMapGeneration( true );
        texture->setMaxAnisotropy( 4.0F );
        return texture;
    }

    osg::Matrix3
    makeViewToWorldRotation( const osg::dmat4& viewMatrix )
    {
        return osg::Matrix3( static_cast<float>( viewMatrix[0][0] ),
                             static_cast<float>( viewMatrix[1][0] ),
                             static_cast<float>( viewMatrix[2][0] ),
                             static_cast<float>( viewMatrix[0][1] ),
                             static_cast<float>( viewMatrix[1][1] ),
                             static_cast<float>( viewMatrix[2][1] ),
                             static_cast<float>( viewMatrix[0][2] ),
                             static_cast<float>( viewMatrix[1][2] ),
                             static_cast<float>( viewMatrix[2][2] ) );
    }

    osg::ref_ptr<osg::Texture2D>
    applySunAndIbl( osg::Node*           model,
                    const SponzaOptions& options,
                    const osg::dmat4&    rttView )
    {
        const double     azimuthRad   = osg::DegreesToRadians( options.sunAzimuthDeg );
        const double     elevationRad = osg::DegreesToRadians( options.sunElevationDeg );
        const osg::dvec3 dirWorld     = osg::normalize(
            osg::dvec3( std::cos( elevationRad ) * std::cos( azimuthRad ),
                        std::sin( elevationRad ),
                        std::cos( elevationRad ) * std::sin( azimuthRad ) )
        );
        const osg::dvec3 dirView =
            osg::normalize( osg::transform3x3( rttView, dirWorld ) );

        osg::ref_ptr<osg::Light> sun = new osg::Light;
        const osg::vec3          sunRadiance =
            scaledColor( options.sunColor, options.sunIntensity );
        sun->setLightNum( 0 );
        sun->setPosition( osg::vec4( static_cast<float>( dirView.x ),
                                     static_cast<float>( dirView.y ),
                                     static_cast<float>( dirView.z ),
                                     0.0F ) );
        sun->setDiffuse(
            osg::vec4( sunRadiance.r, sunRadiance.g, sunRadiance.b, 1.0F )
        );
        sun->setSpecular(
            osg::vec4( sunRadiance.r, sunRadiance.g, sunRadiance.b, 1.0F )
        );
        sun->setAmbient( osg::vec4( 0.0F, 0.0F, 0.0F, 1.0F ) );

        osg::StateSet* modelStateSet = model->getOrCreateStateSet();
        modelStateSet->setAttributeAndModes( sun.get(), osg::StateAttribute::ON );
        modelStateSet->setMode( GL_LIGHTING, osg::StateAttribute::ON );

        const osg::vec3 ambientRadiance =
            scaledColor( options.ambientColor, options.ambientLevel );
        modelStateSet->addUniform( new osg::Uniform(
            "osg_LightModel_ambient",
            osg::vec4( ambientRadiance.r, ambientRadiance.g, ambientRadiance.b, 1.0F )
        ) );
        modelStateSet->addUniform(
            new osg::Uniform( "uViewToWorldRot", makeViewToWorldRotation( rttView ) )
        );
        modelStateSet->addUniform(
            new osg::Uniform( "uEnvMap", static_cast<int>( environmentTextureUnit ) )
        );
        modelStateSet->addUniform( new osg::Uniform( "uEnvMaxLod", 0.0F ) );
        modelStateSet->addUniform( new osg::Uniform( "uEnvClamp", options.iblClamp ) );
        modelStateSet->addUniform( new osg::Uniform( "uIblIntensity",
                                                     options.iblIntensity ) );
        modelStateSet->addUniform( new osg::Uniform( "uIblDiffuse",
                                                     options.iblDiffuse ) );
        modelStateSet->addUniform( new osg::Uniform( "uIblSpecular",
                                                     options.iblSpecular ) );
        modelStateSet->addUniform( new osg::Uniform( "uEnvRotation",
                                                     options.envRotation ) );
        modelStateSet->addUniform( new osg::Uniform( "uHasEnv", false ) );

        osg::ref_ptr<osg::Image> envImage = loadEnvironmentImage();
        if( envImage )
        {
            osg::ref_ptr<osg::Texture2D> envTexture =
                createEnvironmentTexture( envImage.get() );
            modelStateSet->setTextureAttributeAndModes( environmentTextureUnit,
                                                        envTexture.get(),
                                                        osg::StateAttribute::ON );
            modelStateSet->getUniform( "uEnvMaxLod" )
                ->set( computeMaxMipLevel( *envImage ) );
            modelStateSet->getUniform( "uHasEnv" )->set( true );
            return envTexture;
        }

        std::cerr << "Warning: failed to load textures/kloppenheim_05_4k.hdr "
                     "or kloppenheim_05_4k.hdr; IBL disabled"
                  << std::endl;
        return nullptr;
    }

}
