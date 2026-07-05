/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#pragma once

#include <array>
#include <cstddef>
#include <osg/core/ref_ptr.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/maths/MatrixTemplate.hpp>
#include <osg/maths/vec3.hpp>

namespace osg
{

    class Image;
    class Node;
    class StateSet;
    class Texture2D;

}

namespace sponza
{

    struct SponzaOptions;

    constexpr std::size_t irradianceShCount = 9U;

    using IrradianceShCoefficients          = std::array<osg::vec3, irradianceShCount>;

    struct IrradianceShResult
    {
            IrradianceShCoefficients coefficients{};
            osg::vec3                upNormal;
            int                      excludedSunPixels = 0;
            bool                     valid             = false;
    };

    osg::vec3
    scaledColor( const osg::vec3& color,
                 float            scale );

    float
    computeMaxMipLevel( const osg::Image& image );

    osg::ref_ptr<osg::Image>
    loadEnvironmentImage();

    osg::ref_ptr<osg::Texture2D>
    createEnvironmentTexture( osg::Image* image );

    osg::Matrix3
    makeViewToWorldRotation( const osg::dmat4& viewMatrix );

    osg::dvec3
    computeSunDirectionWorld( const SponzaOptions& options );

    IrradianceShResult
    computeSunExcludedIrradianceSh( const osg::Image& image );

    osg::vec3
    evaluateIrradianceShEOverPi( const IrradianceShCoefficients& coefficients,
                                 const osg::vec3&                directionWorld );

    osg::ref_ptr<osg::Texture2D>
    applySunAndIbl( osg::Node*                model,
                    const SponzaOptions&      options,
                    const osg::dmat4&         rttView,
                    osg::Image*               preloadedEnvImage       = nullptr,
                    const IrradianceShResult* precomputedIrradianceSh = nullptr );

    void
    applyShadowReceiverState( osg::StateSet*       stateSet,
                              const SponzaOptions& options,
                              osg::Texture2D*      shadowTexture,
                              const osg::mat4&     shadowMatrix,
                              float                lightSpaceExtent,
                              bool                 hasShadow );

}
