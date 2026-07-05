/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#include "SponzaOptions.hpp"
#include "SponzaTargets.hpp"

#include <osg/GL>

namespace sponza
{

    osg::ref_ptr<osg::Texture2D>
    createHdrColorTexture()
    {
        osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
        texture->setTextureSize( renderWidth, renderHeight );
        texture->setInternalFormat( GL_RGBA16F );
        texture->setSourceFormat( GL_RGBA );
        texture->setSourceType( GL_HALF_FLOAT );
        texture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
        texture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
        texture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE );
        texture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE );
        return texture;
    }

    osg::ref_ptr<osg::Texture2D>
    createSceneDepthTexture()
    {
        osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
        texture->setTextureSize( renderWidth, renderHeight );
        texture->setInternalFormat( GL_DEPTH_COMPONENT24 );
        texture->setSourceFormat( GL_DEPTH_COMPONENT );
        texture->setSourceType( GL_FLOAT );
        texture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST );
        texture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST );
        texture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE );
        texture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE );
        return texture;
    }

    osg::ref_ptr<osg::Texture2D>
    createAoTexture()
    {
        osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
        texture->setTextureSize( renderWidth, renderHeight );
        texture->setInternalFormat( GL_R8 );
        texture->setSourceFormat( GL_RED );
        texture->setSourceType( GL_UNSIGNED_BYTE );
        texture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
        texture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
        texture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE );
        texture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE );
        return texture;
    }

    SponzaTargets
    createSponzaTargets()
    {
        return SponzaTargets{
            createHdrColorTexture(),
            createSceneDepthTexture(),
            createAoTexture()
        };
    }

}
