/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#pragma once

#include <osg/core/ref_ptr.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/textures/Texture2D.hpp>

namespace osg
{

    class Camera;
    class Node;

}

namespace sponza
{

    struct SponzaFrameContext;
    struct SponzaOptions;

    struct ShadowPassResult
    {
            osg::ref_ptr<osg::Camera>    camera;
            osg::ref_ptr<osg::Texture2D> shadowTexture;
            osg::mat4                    shadowMatrix;
            float                        lightSpaceExtent = 1.0F;
            bool                         hasShadow        = false;
    };

    ShadowPassResult
    createShadowPass( osg::Node*                model,
                      const SponzaOptions&      options,
                      const SponzaFrameContext& frame );

}
