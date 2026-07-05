/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#include "SponzaLighting.hpp"
#include "SponzaOptions.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <osg/core/Notify.hpp>
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

    namespace
    {

        constexpr unsigned int shadowTextureUnit = 6U;
        constexpr size_t       irradianceShCount = 9U;
        constexpr double       pi                = 3.14159265358979323846;
        constexpr double       sunDiscLuminance  = 500.0;

        using IrradianceShCoefficients = std::array<osg::vec3, irradianceShCount>;

        struct IrradianceShResult
        {
                IrradianceShCoefficients coefficients{};
                osg::vec3                upNormal;
                int                      excludedSunPixels = 0;
                bool                     valid             = false;
        };

        std::array<double,
                   irradianceShCount>
        evaluateShBasis( double x,
                         double y,
                         double z )
        {
            return {
                0.28209479177387814,
                0.4886025119029199 * y,
                0.4886025119029199 * z,
                0.4886025119029199 * x,
                1.0925484305920792 * x * y,
                1.0925484305920792 * y * z,
                0.31539156525252005 * ( 3.0 * y * y - 1.0 ),
                1.0925484305920792 * x * z,
                0.5462742152960396 * ( x * x - z * z )
            };
        }

        double
        luminance( const osg::dvec3& color )
        {
            return color.r * 0.2126 + color.g * 0.7152 + color.b * 0.0722;
        }

        bool
        canReadFloatRgb( const osg::Image& image )
        {
            return image.valid() &&
                   image.getDataType() ==
                   GL_FLOAT &&
                   ( image.getPixelFormat() ==
                     GL_RGB ||
                     image.getPixelFormat() == GL_RGBA );
        }

        osg::dvec3
        readFloatRgb( const osg::Image& image,
                      int               x,
                      int               y )
        {
            const auto* pixel = reinterpret_cast<const float*>(
                image.data( static_cast<unsigned int>( x ),
                            static_cast<unsigned int>( y ) )
            );
            return osg::dvec3( static_cast<double>( pixel[0] ),
                               static_cast<double>( pixel[1] ),
                               static_cast<double>( pixel[2] ) );
        }

        IrradianceShResult
        computeSunExcludedIrradianceSh( const osg::Image& image )
        {
            IrradianceShResult result;
            for( osg::vec3& coefficient : result.coefficients )
            {
                coefficient.set( 0.0F, 0.0F, 0.0F );
            }

            if( !canReadFloatRgb( image ) )
            {
                OSG_WARN << "Sponza: environment SH requires GL_RGB/GL_RGBA float HDR "
                         << "image data; SH irradiance disabled" << std::endl;
                return result;
            }

            const int width  = image.s();
            const int height = image.t();
            if( width <= 0 || height <= 0 )
            {
                return result;
            }

            std::array<osg::dvec3, irradianceShCount> radianceCoefficients{};
            for( osg::dvec3& coefficient : radianceCoefficients )
            {
                coefficient.set( 0.0, 0.0, 0.0 );
            }

            const double invWidth          = 1.0 / static_cast<double>( width );
            const double invHeight         = 1.0 / static_cast<double>( height );
            const double thetaStep         = pi * invHeight;
            const double phiStep           = 2.0 * pi * invWidth;
            const double solidAnglePerStep = thetaStep * phiStep;

            for( int y = 0; y < height; ++y )
            {
                const double theta =
                    ( static_cast<double>( height - 1 - y ) + 0.5 ) * thetaStep;
                const double sinTheta = std::sin( theta );
                const double cosTheta = std::cos( theta );
                const double weight   = sinTheta * solidAnglePerStep;

                for( int x = 0; x < width; ++x )
                {
                    const osg::dvec3 color = readFloatRgb( image, x, y );
                    if( luminance( color ) > sunDiscLuminance )
                    {
                        ++result.excludedSunPixels;
                        continue;
                    }

                    const double phi = ( static_cast<double>( x ) + 0.5 ) * phiStep - pi;
                    const double dirX = sinTheta * std::cos( phi );
                    const double dirY = cosTheta;
                    const double dirZ = sinTheta * std::sin( phi );

                    const std::array<double, irradianceShCount> basis =
                        evaluateShBasis( dirX, dirY, dirZ );
                    for( size_t i = 0; i < irradianceShCount; ++i )
                    {
                        radianceCoefficients[i] += color * ( basis[i] * weight );
                    }
                }
            }

            constexpr std::array<double, irradianceShCount> convolutionByPi{
                pi / pi,
                ( 2.0 * pi / 3.0 ) / pi,
                ( 2.0 * pi / 3.0 ) / pi,
                ( 2.0 * pi / 3.0 ) / pi,
                ( pi / 4.0 ) / pi,
                ( pi / 4.0 ) / pi,
                ( pi / 4.0 ) / pi,
                ( pi / 4.0 ) / pi,
                ( pi / 4.0 ) / pi
            };

            std::array<osg::dvec3, irradianceShCount> irradianceCoefficients{};
            for( size_t i = 0; i < irradianceShCount; ++i )
            {
                irradianceCoefficients[i] = radianceCoefficients[i] * convolutionByPi[i];
                result.coefficients[i].set(
                    static_cast<float>( irradianceCoefficients[i].r ),
                    static_cast<float>( irradianceCoefficients[i].g ),
                    static_cast<float>( irradianceCoefficients[i].b )
                );
            }

            const std::array<double, irradianceShCount> upBasis =
                evaluateShBasis( 0.0, 1.0, 0.0 );
            osg::dvec3 upNormal( 0.0, 0.0, 0.0 );
            for( size_t i = 0; i < irradianceShCount; ++i )
            {
                upNormal += irradianceCoefficients[i] * upBasis[i];
            }

            result.upNormal.set( static_cast<float>( upNormal.r ),
                                 static_cast<float>( upNormal.g ),
                                 static_cast<float>( upNormal.b ) );
            result.valid = true;
            return result;
        }

        osg::ref_ptr<osg::Uniform>
        createIrradianceShUniform( const IrradianceShCoefficients& coefficients )
        {
            osg::ref_ptr<osg::Uniform> uniform =
                new osg::Uniform( osg::Uniform::FLOAT_VEC3,
                                  "uIrradianceSH",
                                  static_cast<int>( irradianceShCount ) );
            for( size_t i = 0; i < coefficients.size(); ++i )
            {
                uniform->setElement( static_cast<unsigned int>( i ), coefficients[i] );
            }
            return uniform;
        }

    }

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

    osg::dvec3
    computeSunDirectionWorld( const SponzaOptions& options )
    {
        const double azimuthRad   = osg::DegreesToRadians( options.sunAzimuthDeg );
        const double elevationRad = osg::DegreesToRadians( options.sunElevationDeg );
        return osg::normalize(
            osg::dvec3( std::cos( elevationRad ) * std::cos( azimuthRad ),
                        std::sin( elevationRad ),
                        std::cos( elevationRad ) * std::sin( azimuthRad ) )
        );
    }

    osg::ref_ptr<osg::Texture2D>
    applySunAndIbl( osg::Node*           model,
                    const SponzaOptions& options,
                    const osg::dmat4&    rttView )
    {
        const osg::dvec3 dirWorld = computeSunDirectionWorld( options );
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

        IrradianceShCoefficients zeroIrradianceSh{};
        for( osg::vec3& coefficient : zeroIrradianceSh )
        {
            coefficient.set( 0.0F, 0.0F, 0.0F );
        }
        osg::ref_ptr<osg::Uniform> irradianceShUniform =
            createIrradianceShUniform( zeroIrradianceSh );
        osg::ref_ptr<osg::Uniform> useShUniform =
            new osg::Uniform( "uUseShIrradiance", false );
        const float bounceScale =
            options.bounceStrength * ( options.sunIntensity / 6.5F );
        const osg::vec3 bounceRadiance = scaledColor( options.bounceColor, bounceScale );

        modelStateSet->addUniform( irradianceShUniform.get(),
                                   osg::StateAttribute::OVERRIDE );
        modelStateSet->addUniform( useShUniform.get(), osg::StateAttribute::OVERRIDE );
        modelStateSet->addUniform( new osg::Uniform( "uBounceRadiance", bounceRadiance ),
                                   osg::StateAttribute::OVERRIDE );

        osg::ref_ptr<osg::Image> envImage = loadEnvironmentImage();
        if( envImage )
        {
            const IrradianceShResult irradianceSh =
                computeSunExcludedIrradianceSh( *envImage );
            if( irradianceSh.valid )
            {
                for( size_t i = 0; i < irradianceSh.coefficients.size(); ++i )
                {
                    irradianceShUniform->setElement( static_cast<unsigned int>( i ),
                                                     irradianceSh.coefficients[i] );
                }
                useShUniform->set( options.iblShEnabled );
                OSG_NOTICE << "Sponza SH irradiance up-normal reconstruction: ("
                           << irradianceSh.upNormal.r << ", " << irradianceSh.upNormal.g
                           << ", " << irradianceSh.upNormal.b << "), excluded "
                           << irradianceSh.excludedSunPixels << " sun-disc pixels"
                           << std::endl;
            }

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

    void
    applyShadowReceiverState( osg::StateSet*       stateSet,
                              const SponzaOptions& options,
                              osg::Texture2D*      shadowTexture,
                              const osg::mat4&     shadowMatrix,
                              float                lightSpaceExtent,
                              bool                 hasShadow )
    {
        const float uvSoftness =
            options.shadowSoftness / std::max( lightSpaceExtent, 1.0E-6F );

        stateSet->setTextureAttributeAndModes( shadowTextureUnit,
                                               shadowTexture,
                                               osg::StateAttribute::ON );
        stateSet->addUniform(
            new osg::Uniform( "uShadowMap", static_cast<int>( shadowTextureUnit ) )
        );
        stateSet->addUniform( new osg::Uniform( "uShadowMatrix", shadowMatrix ) );
        stateSet->addUniform( new osg::Uniform( "uHasShadow", hasShadow ) );
        stateSet->addUniform( new osg::Uniform( "uShadowSoftness", uvSoftness ) );
        stateSet->addUniform( new osg::Uniform( "uShadowBias", options.shadowBias ) );
        stateSet->addUniform( new osg::Uniform( "uShadowNormalOffset",
                                                options.shadowNormalOffset ) );
    }

}
