/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#pragma once

#include <osg/core/ref_ptr.hpp>
#include <osg/textures/Texture2D.hpp>

namespace sponza
{

    struct SponzaOptions;
    enum class IndirectTargetFormat;

    struct SponzaTargets
    {
            osg::ref_ptr<osg::Texture2D> hdrColor;
            osg::ref_ptr<osg::Texture2D> indirectColor;
            osg::ref_ptr<osg::Texture2D> sceneDepth;
            osg::ref_ptr<osg::Texture2D> aoTexture;
    };

    osg::ref_ptr<osg::Texture2D>
    createHdrColorTexture( int width,
                           int height );

    osg::ref_ptr<osg::Texture2D>
    createIndirectColorTexture( int                  width,
                                int                  height,
                                IndirectTargetFormat format );

    osg::ref_ptr<osg::Texture2D>
    createSceneDepthTexture( int width,
                             int height );

    osg::ref_ptr<osg::Texture2D>
    createAoTexture( int width,
                     int height );

    osg::ref_ptr<osg::Texture2D>
    createShadowDepthTexture( int size );

    SponzaTargets
    createSponzaTargets( const SponzaOptions& options );

}
