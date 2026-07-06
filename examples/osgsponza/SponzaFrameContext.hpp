/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#pragma once

#include <osg/core/ref_ptr.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/maths/MatrixTemplate.hpp>
#include <osg/textures/Texture2D.hpp>

namespace sponza
{

    struct SponzaFrameContext
    {
            osg::dmat4                   view;
            osg::dmat4                   proj;
            osg::dmat4                   invProj;
            osg::Matrix3                 viewToWorldRot;
            osg::ref_ptr<osg::Texture2D> envTexture;
            float                        envRotation = 0.0F;
    };

}
