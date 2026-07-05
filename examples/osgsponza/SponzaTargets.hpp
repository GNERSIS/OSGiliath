/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#pragma once

#include <osg/core/ref_ptr.hpp>
#include <osg/textures/Texture2D.hpp>

namespace sponza
{

    struct SponzaTargets
    {
            osg::ref_ptr<osg::Texture2D> hdrColor;
            osg::ref_ptr<osg::Texture2D> sceneDepth;
            osg::ref_ptr<osg::Texture2D> aoTexture;
    };

    osg::ref_ptr<osg::Texture2D>
    createHdrColorTexture();

    osg::ref_ptr<osg::Texture2D>
    createSceneDepthTexture();

    osg::ref_ptr<osg::Texture2D>
    createAoTexture();

    SponzaTargets
    createSponzaTargets();

}
