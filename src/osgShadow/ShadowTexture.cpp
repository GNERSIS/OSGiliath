/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Projective shadow texture technique. Projects a shadow texture
 * from the light source onto receiver geometry.
 */
#include <osgShadow/ShadowTexture>

#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/traversal/ComputeBoundsVisitor.hpp>
#include <osgShadow/ShadowedScene>

using namespace osgShadow;

ShadowTexture::ShadowTexture() :
    _textureUnit( 1 )
{
}

ShadowTexture::ShadowTexture( const ShadowTexture& copy,
                              const osg::CopyOp&   copyop ) :
    Inherit( copy,
             copyop ),
    _textureUnit( copy._textureUnit )
{
}

void
ShadowTexture::resizeGLObjectBuffers( unsigned int maxSize )
{
    osg::resizeGLObjectBuffers( _camera, maxSize );
    osg::resizeGLObjectBuffers( _texture, maxSize );
    osg::resizeGLObjectBuffers( _stateset, maxSize );
}

void
ShadowTexture::releaseGLObjects( osg::State* state ) const
{
    osg::releaseGLObjects( _camera, state );
    osg::releaseGLObjects( _texture, state );
    osg::releaseGLObjects( _stateset, state );
}

void
ShadowTexture::setTextureUnit( unsigned int unit )
{
    _textureUnit = unit;
}

void
ShadowTexture::init()
{
    if( !_shadowedScene )
    {
        return;
    }

    unsigned int tex_width  = 512;
    unsigned int tex_height = 512;

    _texture                = new osg::Texture2D;
    _texture->setTextureSize( static_cast<int>( tex_width ),
                              static_cast<int>( tex_height ) );
    _texture->setInternalFormat( GL_RGB );
    _texture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
    _texture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
    _texture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_BORDER );
    _texture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_BORDER );
    _texture->setBorderColor( osg::dvec4( 1.0, 1.0, 1.0, 1.0 ) );

    // set up the render to texture camera.
    {
        // create the camera
        _camera = new osg::Camera;

        _camera->setClearColor( osg::vec4( 1.0F, 1.0F, 1.0F, 1.0F ) );

        _camera->setCullCallback( new CameraCullCallback( this ) );

        // set viewport
        _camera->setViewport( 0,
                              0,
                              static_cast<int>( tex_width ),
                              static_cast<int>( tex_height ) );

        // set the camera to render before the main camera.
        _camera->setRenderOrder( osg::Camera::PRE_RENDER );

        // tell the camera to use OpenGL frame buffer object where supported.
        _camera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
        //_camera->setRenderTargetImplementation(osg::Camera::SEPERATE_WINDOW);

        // attach the texture and use it as the color buffer.
        _camera->attach( osg::Camera::COLOR_BUFFER, _texture.get() );

        _material = new osg::Material;
        _material->setAmbient( osg::Material::FRONT_AND_BACK,
                               osg::vec4( 0.0F, 0.0F, 0.0F, 1.0F ) );
        _material->setDiffuse( osg::Material::FRONT_AND_BACK,
                               osg::vec4( 0.0F, 0.0F, 0.0F, 1.0F ) );
        _material->setEmission( osg::Material::FRONT_AND_BACK,
                                osg::vec4( 0.0F, 0.0F, 0.0F, 1.0F ) );
        _material->setShininess( osg::Material::FRONT_AND_BACK, 0.0F );

        osg::StateSet* stateset = _camera->getOrCreateStateSet();
        stateset->setAttribute( _material.get(), osg::StateAttribute::OVERRIDE );
    }

    {
        _stateset = new osg::StateSet;
        _stateset->setTextureAttributeAndModes( _textureUnit,
                                                _texture.get(),
                                                osg::StateAttribute::ON );
    }

    _dirty = false;
}

void
ShadowTexture::update( osg::NodeVisitor& nv )
{
    _shadowedScene->osg::Group::traverse( nv );
}

void
ShadowTexture::cull( osgUtil::CullVisitor& cv )
{
    // record the traversal mask on entry so we can reapply it later.
    unsigned int          traversalMask = cv.getTraversalMask();

    osgUtil::RenderStage* orig_rs       = cv.getRenderStage();

    // do traversal of shadow casting scene which does not need to be decorated by the
    // shadow texture
    {
        cv.setTraversalMask( traversalMask &
                             getShadowedScene()->getCastsShadowTraversalMask() );

        _shadowedScene->osg::Group::traverse( cv );
    }

    // do traversal of shadow receiving scene which does need to be decorated by the
    // shadow texture
    {
        cv.pushStateSet( _stateset.get() );

        cv.setTraversalMask( traversalMask &
                             getShadowedScene()->getReceivesShadowTraversalMask() );

        _shadowedScene->osg::Group::traverse( cv );

        cv.popStateSet();
    }

    // need to compute view frustum for RTT camera.
    // 1) get the light position
    // 2) get the center and extents of the view frustum

    const osg::Light*                                  selectLight = 0;
    osg::vec4                                          lightpos;

    osgUtil::PositionalStateContainer::AttrMatrixList& aml =
        orig_rs->getPositionalStateContainer()->getAttrMatrixList();
    for( osgUtil::PositionalStateContainer::AttrMatrixList::iterator itr = aml.begin();
         itr != aml.end();
         ++itr )
    {
        const osg::Light* light = dynamic_cast<const osg::Light*>( itr->first.get() );
        if( light )
        {
            osg::RefMatrix* matrix = itr->second.get();
            if( matrix )
            {
                lightpos = light->getPosition() * ( *matrix );
            }
            else
            {
                lightpos = light->getPosition();
            }

            selectLight = light;
        }
    }

    osg::dmat4 eyeToWorld;
    eyeToWorld = osg::inverse( *cv.getModelViewMatrix() );

    lightpos   = lightpos * eyeToWorld;

    if( selectLight )
    {

        // get the bounds of the model.
        osg::ComputeBoundsVisitor cbbv( osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN );
        cbbv.setTraversalMask( getShadowedScene()->getCastsShadowTraversalMask() );

        _shadowedScene->osg::Group::traverse( cbbv );

        osg::box bb = cbbv.getBoundingBox();

        if( lightpos[3] != 0.0 )
        {
            osg::vec3 position( lightpos.x, lightpos.y, lightpos.z );

            float     centerDistance = osg::length( position - bb.center() );

            float     znear          = centerDistance - bb.radius();
            float     zfar           = centerDistance + bb.radius();
            float     zNearRatio     = 0.001F;
            if( znear < zfar * zNearRatio )
            {
                znear = zfar * zNearRatio;
            }

            float top   = ( bb.radius() / centerDistance ) * znear;
            float right = top;

            _camera->setReferenceFrame( osg::Camera::ABSOLUTE_RF );
            _camera
                ->setProjectionMatrixAsFrustum( -right, right, -top, top, znear, zfar );
            _camera->setViewMatrixAsLookAt(
                osg::dvec3( position ),
                osg::dvec3( bb.center() ),
                osg::dvec3( computeOrthogonalVector( bb.center() - position ) )
            );
        }
        else
        {
            // make an orthographic projection
            osg::vec3 lightDir( lightpos.x, lightpos.y, lightpos.z );
            lightDir = osg::normalize( lightDir );

            // set the position far away along the light direction
            osg::vec3 position       = bb.center() + lightDir * bb.radius() * 2.0F;

            float     centerDistance = osg::length( position - bb.center() );

            float     znear          = centerDistance - bb.radius();
            float     zfar           = centerDistance + bb.radius();
            float     zNearRatio     = 0.001F;
            if( znear < zfar * zNearRatio )
            {
                znear = zfar * zNearRatio;
            }

            float top   = bb.radius();
            float right = top;

            _camera->setReferenceFrame( osg::Camera::ABSOLUTE_RF );
            _camera->setProjectionMatrixAsOrtho( -right, right, -top, top, znear, zfar );
            _camera->setViewMatrixAsLookAt(
                osg::dvec3( position ),
                osg::dvec3( bb.center() ),
                osg::dvec3( computeOrthogonalVector( lightDir ) )
            );
        }

        cv.setTraversalMask( traversalMask &
                             getShadowedScene()->getCastsShadowTraversalMask() );

        // do RTT camera traversal
        _camera->accept( cv );
    }

    // reapply the original traversal mask
    cv.setTraversalMask( traversalMask );
}

void
ShadowTexture::cleanSceneGraph()
{
}
