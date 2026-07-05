/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#pragma once

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

    osg::ref_ptr<osg::Texture2D>
    applySunAndIbl( osg::Node*           model,
                    const SponzaOptions& options,
                    const osg::dmat4&    rttView );

    void
    applyShadowReceiverState( osg::StateSet*       stateSet,
                              const SponzaOptions& options,
                              osg::Texture2D*      shadowTexture,
                              const osg::mat4&     shadowMatrix,
                              float                lightSpaceExtent,
                              bool                 hasShadow );

}
