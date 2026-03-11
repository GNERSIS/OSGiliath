/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Basic shadow mapping technique. Renders a depth map from the
 * light's view and projects it for shadow lookup.
 */
#include <osgShadow/ShadowMap>

#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osg/state/CullFace.hpp>
#include <osg/state/PolygonOffset.hpp>
#include <osg/traversal/ComputeBoundsVisitor.hpp>
#include <osgShadow/ShadowedScene>

using namespace osgShadow;

#include <iostream>
// for debug
#include <osg/geometry/Geometry.hpp>
#include <osg/lighting/LightSource.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/state/PolygonMode.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgText/Text>

//////////////////////////////////////////////////////////////////
// vertex shader (no base texture - shadow coords in texCoord_0)
//
static const char vertexShaderSource_noBaseTexture[] =
    "#version 460 core\n"
    "layout(location = 0) in vec4 osg_Vertex;\n"
    "layout(location = 2) in vec3 osg_Normal;\n"
    "layout(location = 3) in vec4 osg_Color;\n"
    "uniform mat4 osg_ModelViewProjectionMatrix;\n"
    "uniform mat4 osg_ModelViewMatrix;\n"
    "uniform mat3 osg_NormalMatrix;\n"
    "uniform mat4 osgShadow_shadowTextureMatrix;\n"
    "struct osg_LightSourceParameters {\n"
    "    vec4 ambient; vec4 diffuse; vec4 specular; vec4 position;\n"
    "    vec3 spotDirection; float spotExponent; float spotCutoff;\n"
    "    float spotCosCutoff; float constantAttenuation;\n"
    "    float linearAttenuation; float quadraticAttenuation;\n"
    "};\n"
    "uniform osg_LightSourceParameters osg_LightSource;\n"
    "struct osg_MaterialParameters {\n"
    "    vec4 ambient; vec4 diffuse; vec4 specular; vec4 emission;\n"
    "    float shininess;\n"
    "};\n"
    "uniform osg_MaterialParameters osg_FrontMaterial;\n"
    "out vec4 vertexColor;\n"
    "out vec4 texCoord_0;\n"
    "void main(void)\n"
    "{\n"
    "    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
    "    vec4 ecPos = osg_ModelViewMatrix * osg_Vertex;\n"
    "    vec3 normal = normalize(osg_NormalMatrix * osg_Normal);\n"
    "    vec3 L = normalize(osg_LightSource.position.xyz);\n"
    "    float NdotL = max(dot(normal, L), 0.0);\n"
    "    vertexColor.rgb = osg_FrontMaterial.emission.rgb +\n"
    "                      osg_FrontMaterial.ambient.rgb * osg_LightSource.ambient.rgb "
    "+\n"
    "                      osg_FrontMaterial.diffuse.rgb * osg_LightSource.diffuse.rgb "
    "* NdotL;\n"
    "    vertexColor.a = osg_FrontMaterial.diffuse.a;\n"
    "    texCoord_0 = osgShadow_shadowTextureMatrix * ecPos;\n"
    "}\n";

//////////////////////////////////////////////////////////////////
// vertex shader (with base texture - base in texCoord_0, shadow in texCoord_1)
//
static const char vertexShaderSource_withBaseTexture[] =
    "#version 460 core\n"
    "layout(location = 0) in vec4 osg_Vertex;\n"
    "layout(location = 2) in vec3 osg_Normal;\n"
    "layout(location = 3) in vec4 osg_Color;\n"
    "layout(location = 8) in vec4 osg_MultiTexCoord0;\n"
    "uniform mat4 osg_ModelViewProjectionMatrix;\n"
    "uniform mat4 osg_ModelViewMatrix;\n"
    "uniform mat3 osg_NormalMatrix;\n"
    "uniform mat4 osgShadow_shadowTextureMatrix;\n"
    "struct osg_LightSourceParameters {\n"
    "    vec4 ambient; vec4 diffuse; vec4 specular; vec4 position;\n"
    "    vec3 spotDirection; float spotExponent; float spotCutoff;\n"
    "    float spotCosCutoff; float constantAttenuation;\n"
    "    float linearAttenuation; float quadraticAttenuation;\n"
    "};\n"
    "uniform osg_LightSourceParameters osg_LightSource;\n"
    "struct osg_MaterialParameters {\n"
    "    vec4 ambient; vec4 diffuse; vec4 specular; vec4 emission;\n"
    "    float shininess;\n"
    "};\n"
    "uniform osg_MaterialParameters osg_FrontMaterial;\n"
    "out vec4 vertexColor;\n"
    "out vec4 texCoord_0;\n"
    "out vec4 texCoord_1;\n"
    "void main(void)\n"
    "{\n"
    "    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
    "    vec4 ecPos = osg_ModelViewMatrix * osg_Vertex;\n"
    "    vec3 normal = normalize(osg_NormalMatrix * osg_Normal);\n"
    "    vec3 L = normalize(osg_LightSource.position.xyz);\n"
    "    float NdotL = max(dot(normal, L), 0.0);\n"
    "    vertexColor.rgb = osg_FrontMaterial.emission.rgb +\n"
    "                      osg_FrontMaterial.ambient.rgb * osg_LightSource.ambient.rgb "
    "+\n"
    "                      osg_FrontMaterial.diffuse.rgb * osg_LightSource.diffuse.rgb "
    "* NdotL;\n"
    "    vertexColor.a = osg_FrontMaterial.diffuse.a;\n"
    "    texCoord_0 = osg_MultiTexCoord0;\n"
    "    texCoord_1 = osgShadow_shadowTextureMatrix * ecPos;\n"
    "}\n";

//////////////////////////////////////////////////////////////////
// vertex shader for debug HUD
//
static const char vertexShaderSource_debugHUD[] =
    "#version 460 core\n"
    "layout(location = 0) in vec4 osg_Vertex;\n"
    "layout(location = 8) in vec4 osg_MultiTexCoord0;\n"
    "uniform mat4 osg_ModelViewProjectionMatrix;\n"
    "out vec4 texCoord_0;\n"
    "void main(void)\n"
    "{\n"
    "    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
    "    texCoord_0 = osg_MultiTexCoord0;\n"
    "}\n";

//////////////////////////////////////////////////////////////////
// fragment shader
//
static const char fragmentShaderSource_noBaseTexture[] =
    "#version 460 core\n"
    "in vec4 vertexColor; \n"
    "in vec4 texCoord_0; \n"
    "out vec4 fragColor; \n"
    "uniform sampler2DShadow osgShadow_shadowTexture; \n"
    "uniform vec2 osgShadow_ambientBias; \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "    fragColor = vertexColor * (osgShadow_ambientBias.x + textureProj( "
    "osgShadow_shadowTexture, texCoord_0 ) * osgShadow_ambientBias.y); \n"
    "}\n";

//////////////////////////////////////////////////////////////////
// fragment shader
//
static const char fragmentShaderSource_withBaseTexture[] =
    "#version 460 core\n"
    "in vec4 vertexColor; \n"
    "in vec4 texCoord_0; \n"
    "in vec4 texCoord_1; \n"
    "out vec4 fragColor; \n"
    "uniform sampler2D osgShadow_baseTexture; \n"
    "uniform sampler2DShadow osgShadow_shadowTexture; \n"
    "uniform vec2 osgShadow_ambientBias; \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "    vec4 color = vertexColor * texture( osgShadow_baseTexture, texCoord_0.xy ); \n"
    "    fragColor = color * (osgShadow_ambientBias.x + textureProj( "
    "osgShadow_shadowTexture, texCoord_1 ) * osgShadow_ambientBias.y); \n"
    "}\n";

//////////////////////////////////////////////////////////////////
// fragment shader
//
static const char fragmentShaderSource_debugHUD[] =
    "#version 460 core\n"
    "in vec4 texCoord_0; \n"
    "out vec4 fragColor; \n"
    "uniform sampler2D osgShadow_shadowTexture; \n"
    " \n"
    "void main(void) \n"
    "{ \n"
    "   vec4 texResult = texture(osgShadow_shadowTexture, texCoord_0.st ); \n"
    "   float value = texResult.r; \n"
    "   fragColor = vec4( value, value, value, 0.8 ); \n"
    "} \n";

ShadowMap::ShadowMap() :
    _baseTextureUnit( 0 ),
    _shadowTextureUnit( 1 ),
    _polyOffset( 1.0,
                 1.0 ),
    _ambientBias( 0.5F,
                  0.5F ),
    _textureSize( 1'024,
                  1'024 )
{
}

ShadowMap::ShadowMap( const ShadowMap&   copy,
                      const osg::CopyOp& copyop ) :
    Inherit( copy,
             copyop ),
    _baseTextureUnit( copy._baseTextureUnit ),
    _shadowTextureUnit( copy._shadowTextureUnit ),
    _polyOffset( copy._polyOffset ),
    _ambientBias( copy._ambientBias ),
    _textureSize( copy._textureSize )
{
}

void
ShadowMap::resizeGLObjectBuffers( unsigned int maxSize )
{
    osg::resizeGLObjectBuffers( _camera, maxSize );
    osg::resizeGLObjectBuffers( _texture, maxSize );
    osg::resizeGLObjectBuffers( _stateset, maxSize );
    osg::resizeGLObjectBuffers( _program, maxSize );

    osg::resizeGLObjectBuffers( _ls, maxSize );

    for( ShaderList::iterator itr = _shaderList.begin(); itr != _shaderList.end();
         ++itr )
    {
        osg::resizeGLObjectBuffers( *itr, maxSize );
    }
}

void
ShadowMap::releaseGLObjects( osg::State* state ) const
{
    osg::releaseGLObjects( _camera, state );
    osg::releaseGLObjects( _texture, state );
    osg::releaseGLObjects( _stateset, state );
    osg::releaseGLObjects( _program, state );

    osg::releaseGLObjects( _ls, state );

    for( ShaderList::const_iterator itr = _shaderList.begin(); itr != _shaderList.end();
         ++itr )
    {
        osg::releaseGLObjects( *itr, state );
    }
}

void
ShadowMap::setTextureUnit( unsigned int unit )
{
    _shadowTextureUnit = unit;
}

void
ShadowMap::setPolygonOffset( const osg::vec2& polyOffset )
{
    _polyOffset = polyOffset;
}

void
ShadowMap::setAmbientBias( const osg::vec2& ambientBias )
{
    _ambientBias = ambientBias;
    if( _ambientBiasUniform.valid() )
    {
        _ambientBiasUniform->set( _ambientBias );
    }
}

void
ShadowMap::setTextureSize( const osg::svec2& textureSize )
{
    _textureSize = textureSize;
    dirty();
}

void
ShadowMap::setLight( osg::Light* light )
{
    _light = light;
}

void
ShadowMap::setLight( osg::LightSource* ls )
{
    _ls    = ls;
    _light = _ls->getLight();
}

void
ShadowMap::createUniforms()
{
    _uniformList.clear();

    osg::Uniform* baseTextureSampler =
        new osg::Uniform( "osgShadow_baseTexture", ( int )_baseTextureUnit );
    _uniformList.push_back( baseTextureSampler );

    osg::Uniform* shadowTextureSampler =
        new osg::Uniform( "osgShadow_shadowTexture", ( int )_shadowTextureUnit );
    _uniformList.push_back( shadowTextureSampler );

    _ambientBiasUniform = new osg::Uniform( "osgShadow_ambientBias", _ambientBias );
    _uniformList.push_back( _ambientBiasUniform.get() );

    _shadowTextureMatrixUniform =
        new osg::Uniform( osg::Uniform::FLOAT_MAT4, "osgShadow_shadowTextureMatrix" );
    _shadowTextureMatrixUniform->set( osg::mat4() );
    _uniformList.push_back( _shadowTextureMatrixUniform.get() );
}

void
ShadowMap::createShaders()
{
    // if we are not given shaders, use the default
    if( _shaderList.empty() )
    {
        if( _shadowTextureUnit == 0 )
        {
            osg::Shader* vertex_shader =
                new osg::Shader( osg::Shader::VERTEX, vertexShaderSource_noBaseTexture );
            _shaderList.push_back( vertex_shader );
            osg::Shader* fragment_shader =
                new osg::Shader( osg::Shader::FRAGMENT,
                                 fragmentShaderSource_noBaseTexture );
            _shaderList.push_back( fragment_shader );
        }
        else
        {
            osg::Shader* vertex_shader =
                new osg::Shader( osg::Shader::VERTEX,
                                 vertexShaderSource_withBaseTexture );
            _shaderList.push_back( vertex_shader );
            osg::Shader* fragment_shader =
                new osg::Shader( osg::Shader::FRAGMENT,
                                 fragmentShaderSource_withBaseTexture );
            _shaderList.push_back( fragment_shader );
        }
    }
}

void
ShadowMap::init()
{
    if( !_shadowedScene )
    {
        return;
    }

    _texture = new osg::Texture2D;
    _texture->setTextureSize( _textureSize.x, _textureSize.y );
    _texture->setInternalFormat( GL_DEPTH_COMPONENT );
    _texture->setShadowComparison( true );
    _texture->setShadowTextureMode( osg::Texture2D::LUMINANCE );
    _texture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
    _texture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );

    // the shadow comparison should fail if object is outside the texture
    _texture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_BORDER );
    _texture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_BORDER );
    _texture->setBorderColor( osg::dvec4( 1.0, 1.0, 1.0, 1.0 ) );

    // set up the render to texture camera.
    {
        // create the camera
        _camera = new osg::Camera;

        _camera->setReferenceFrame( osg::Camera::ABSOLUTE_RF_INHERIT_VIEWPOINT );

        _camera->setCullCallback( new CameraCullCallback( this ) );

        _camera->setClearMask( GL_DEPTH_BUFFER_BIT );
        //_camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        _camera->setClearColor( osg::vec4( 1.0F, 1.0F, 1.0F, 1.0F ) );
        _camera->setComputeNearFarMode( osg::Camera::DO_NOT_COMPUTE_NEAR_FAR );

        // set viewport
        _camera->setViewport( 0, 0, _textureSize.x, _textureSize.y );

        // set the camera to render before the main camera.
        _camera->setRenderOrder( osg::Camera::PRE_RENDER );

        // tell the camera to use OpenGL frame buffer object where supported.
        _camera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
        //_camera->setRenderTargetImplementation(osg::Camera::SEPERATE_WINDOW);

        // attach the texture and use it as the color buffer.
        _camera->attach( osg::Camera::DEPTH_BUFFER, _texture.get() );

        osg::StateSet* stateset = _camera->getOrCreateStateSet();

#if 1
        // cull front faces so that only backfaces contribute to depth map

        osg::ref_ptr<osg::CullFace> cull_face = new osg::CullFace;
        cull_face->setMode( osg::CullFace::Mode::FRONT );
        stateset->setAttribute( cull_face.get(),
                                osg::StateAttribute::ON |
                                    osg::StateAttribute::OVERRIDE );
        stateset->setMode( GL_CULL_FACE,
                           osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

        // negative polygonoffset - move the backface nearer to the eye point so that
        // backfaces shadow themselves
        float                            factor         = -_polyOffset[0];
        float                            units          = -_polyOffset[1];

        osg::ref_ptr<osg::PolygonOffset> polygon_offset = new osg::PolygonOffset;
        polygon_offset->setFactor( factor );
        polygon_offset->setUnits( units );
        stateset->setAttribute( polygon_offset.get(),
                                osg::StateAttribute::ON |
                                    osg::StateAttribute::OVERRIDE );
        stateset->setMode( GL_POLYGON_OFFSET_FILL,
                           osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
#else
        // disabling cull faces so that only front and backfaces contribute to depth map
        stateset->setMode( GL_CULL_FACE,
                           osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE );

        // negative polygonoffset - move the backface nearer to the eye point
        // so that front faces do not shadow themselves.
        float                            factor         = _polyOffset[0];
        float                            units          = _polyOffset[1];

        osg::ref_ptr<osg::PolygonOffset> polygon_offset = new osg::PolygonOffset;
        polygon_offset->setFactor( factor );
        polygon_offset->setUnits( units );
        stateset->setAttribute( polygon_offset.get(),
                                osg::StateAttribute::ON |
                                    osg::StateAttribute::OVERRIDE );
        stateset->setMode( GL_POLYGON_OFFSET_FILL,
                           osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
#endif
    }

    {
        _stateset = new osg::StateSet;
        _stateset->setTextureAttributeAndModes( _shadowTextureUnit,
                                                _texture.get(),
                                                osg::StateAttribute::ON |
                                                    osg::StateAttribute::OVERRIDE );

        // add Program, when empty of Shaders then we are using fixed functionality
        _program = new osg::Program;
        _stateset->setAttribute( _program.get() );

        // create default shaders if needed
        createShaders();

        // add the shader list to the program
        for( ShaderList::const_iterator itr = _shaderList.begin();
             itr != _shaderList.end();
             ++itr )
        {
            _program->addShader( itr->get() );
        }

        // create own uniforms
        createUniforms();

        // add the uniform list to the stateset
        for( UniformList::const_iterator itr = _uniformList.begin();
             itr != _uniformList.end();
             ++itr )
        {
            _stateset->addUniform( itr->get() );
        }

        {
            // fake texture for baseTexture, add a fake texture
            // we support by default at least one texture layer
            // without this fake texture we can not support
            // textured and not textured scene

            // TODO: at the moment the PSSM supports just one texture layer in the GLSL
            // shader, multitexture are
            //       not yet supported !

            osg::Image* image = new osg::Image;
            // allocate the image data, noPixels x 1 x 1 with 4 rgba floats - equivalent
            // to a vec4!
            int         noPixels = 1;
            image->allocateImage( noPixels, 1, 1, GL_RGBA, GL_FLOAT );
            image->setInternalTextureFormat( GL_RGBA );
            // fill in the image data.
            osg::vec4* dataPtr = ( osg::vec4* )image->data();
            osg::vec4  color( 1, 1, 1, 1 );
            *dataPtr = color;
            // make fake texture
            osg::Texture2D* fakeTex = new osg::Texture2D;
            fakeTex->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE );
            fakeTex->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE );
            fakeTex->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
            fakeTex->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
            fakeTex->setImage( image );
            // add fake texture
            _stateset->setTextureAttribute( _baseTextureUnit,
                                            fakeTex,
                                            osg::StateAttribute::ON );
        }
    }

    _dirty = false;
}

void
ShadowMap::update( osg::NodeVisitor& nv )
{
    _shadowedScene->osg::Group::traverse( nv );
}

void
ShadowMap::cull( osgUtil::CullVisitor& cv )
{
    // record the traversal mask on entry so we can reapply it later.
    unsigned int          traversalMask = cv.getTraversalMask();

    osgUtil::RenderStage* orig_rs       = cv.getRenderStage();

    // do traversal of shadow receiving scene which does need to be decorated by the
    // shadow map
    {
        cv.pushStateSet( _stateset.get() );

        _shadowedScene->osg::Group::traverse( cv );

        cv.popStateSet();
    }

    // need to compute view frustum for RTT camera.
    // 1) get the light position
    // 2) get the center and extents of the view frustum

    const osg::Light*                                  selectLight = 0;
    osg::vec4                                          lightpos;
    osg::vec3                                          lightDir;

    // MR testing giving a specific light
    osgUtil::PositionalStateContainer::AttrMatrixList& aml =
        orig_rs->getPositionalStateContainer()->getAttrMatrixList();
    for( osgUtil::PositionalStateContainer::AttrMatrixList::iterator itr = aml.begin();
         itr != aml.end();
         ++itr )
    {
        const osg::Light* light = dynamic_cast<const osg::Light*>( itr->first.get() );
        if( light )
        {
            if( _light.valid() )
            {
                if( _light.get() == light )
                {
                    selectLight = light;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                selectLight = light;
            }

            osg::RefMatrix* matrix = itr->second.get();
            if( matrix )
            {
                lightpos = light->getPosition() * ( *matrix );
                lightDir = osg::transform3x3( light->getDirection(), *matrix );
            }
            else
            {
                lightpos = light->getPosition();
                lightDir = light->getDirection();
            }
        }
    }

    osg::dmat4 eyeToWorld;
    eyeToWorld = osg::inverse( *cv.getModelViewMatrix() );

    lightpos   = lightpos * eyeToWorld;
    lightDir   = osg::transform3x3( lightDir, eyeToWorld );
    lightDir   = osg::normalize( lightDir );

    if( selectLight )
    {

        // set to ambient on light to black so that the ambient bias uniform can take
        // it's affect
        const_cast<osg::Light*>( selectLight )
            ->setAmbient( osg::vec4( 0.0F, 0.0F, 0.0F, 1.0F ) );

        // std::cout<<"----- VxOSG::ShadowMap selectLight spot cutoff
        // "<<selectLight->getSpotCutoff()<<std::endl;

        float fov = selectLight->getSpotCutoff() * 2;
        if( fov < 180.0F )    // spotlight, then we don't need the bounding box
        {
            osg::vec3 position( lightpos.x, lightpos.y, lightpos.z );
            _camera->setProjectionMatrixAsPerspective( fov, 1.0, 0.1, 1000.0 );
            _camera->setViewMatrixAsLookAt(
                osg::dvec3( position ),
                osg::dvec3( position + lightDir ),
                osg::dvec3( computeOrthogonalVector( lightDir ) )
            );
        }
        else
        {
            // get the bounds of the model.
            osg::ComputeBoundsVisitor cbbv( osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN );
            cbbv.setTraversalMask( getShadowedScene()->getCastsShadowTraversalMask() );

            _shadowedScene->osg::Group::traverse( cbbv );

            osg::box bb = cbbv.getBoundingBox();

            if( lightpos[3] != 0.0 )    // point light
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

                _camera->setProjectionMatrixAsFrustum( -right,
                                                       right,
                                                       -top,
                                                       top,
                                                       znear,
                                                       zfar );
                _camera->setViewMatrixAsLookAt(
                    osg::dvec3( position ),
                    osg::dvec3( bb.center() ),
                    osg::dvec3( computeOrthogonalVector( bb.center() - position ) )
                );
            }
            else    // directional light
            {
                // make an orthographic projection
                osg::vec3 ortho_lightDir( lightpos.x, lightpos.y, lightpos.z );
                ortho_lightDir = osg::normalize( ortho_lightDir );

                // set the position far away along the light direction
                osg::vec3 position = bb.center() + ortho_lightDir * bb.radius() * 2.0F;

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

                _camera->setProjectionMatrixAsOrtho( -right,
                                                     right,
                                                     -top,
                                                     top,
                                                     znear,
                                                     zfar );
                _camera->setViewMatrixAsLookAt(
                    osg::dvec3( position ),
                    osg::dvec3( bb.center() ),
                    osg::dvec3( computeOrthogonalVector( ortho_lightDir ) )
                );
            }
        }

        // Compute shadow texture matrix:
        // Maps eye-space position to shadow map texture coordinates.
        // shadowTextureMatrix = inverseCameraView * shadowView * shadowProj * bias
        {
            osg::dmat4 inverseCameraView = osg::inverse( *cv.getModelViewMatrix() );

            osg::dmat4 bias( 0.5,
                             0.0,
                             0.0,
                             0.0,
                             0.0,
                             0.5,
                             0.0,
                             0.0,
                             0.0,
                             0.0,
                             0.5,
                             0.0,
                             0.5,
                             0.5,
                             0.5,
                             1.0 );

            osg::mat4  shadowTextureMatrix = osg::mat4( bias *
                                                        _camera->getProjectionMatrix() *
                                                        _camera->getViewMatrix() *
                                                        inverseCameraView );

            _shadowTextureMatrixUniform->set( shadowTextureMatrix );
        }

        cv.setTraversalMask( traversalMask &
                             getShadowedScene()->getCastsShadowTraversalMask() );

        // do RTT camera traversal
        _camera->accept( cv );

    }    // if(selectLight)

    // reapply the original traversal mask
    cv.setTraversalMask( traversalMask );
}

void
ShadowMap::cleanSceneGraph()
{
}

///////////////////// Debug Methods

////////////////////////////////////////////////////////////////////////////////
// Callback used by debugging hud to display Shadow Map in color buffer
// OSG does not allow to use the same GL Texture Id with different glTexParams.
// Callback simply turns shadow compare mode off via GL while rendering hud and
// restores it afterwards.
////////////////////////////////////////////////////////////////////////////////
class ShadowMap::DrawableDrawWithDepthShadowComparisonOffCallback
    : public osg::Drawable::DrawCallback
{
    public:

        //
        DrawableDrawWithDepthShadowComparisonOffCallback( osg::Texture2D* texture,
                                                          unsigned        stage = 0 ) :
            _texture( texture ),
            _stage( stage )
        {
        }

        virtual void
        drawImplementation( osg::RenderInfo&     ri,
                            const osg::Drawable* drawable ) const
        {
            if( _texture.valid() )
            {
                // make sure proper texture is currently applied
                ri.getState()->applyTextureAttribute( _stage, _texture.get() );

                // Turn off depth comparison mode
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE );
            }

            drawable->drawImplementation( ri );

            if( _texture.valid() )
            {
                // Turn it back on
                glTexParameteri( GL_TEXTURE_2D,
                                 GL_TEXTURE_COMPARE_MODE,
                                 GL_COMPARE_REF_TO_TEXTURE );
            }
        }

        osg::ref_ptr<osg::Texture2D> _texture;
        unsigned                     _stage;
};

////////////////////////////////////////////////////////////////////////////////
osg::ref_ptr<osg::Camera>
ShadowMap::makeDebugHUD()
{
    // Make sure we attach initialized texture to HUD
    if( !_texture.valid() )
    {
        init();
    }

    osg::ref_ptr<osg::Camera> camera = new osg::Camera;

    osg::vec2                 size( 1'280, 1'024 );
    // set the projection matrix
    camera->setProjectionMatrix(
        osg::orthographic( 0.0, ( double )size.x, 0.0, ( double )size.y, -1.0, 1.0 )
    );

    // set the view matrix
    camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    camera->setViewMatrix( osg::dmat4() );

    // only clear the depth buffer
    camera->setClearMask( GL_DEPTH_BUFFER_BIT );
    camera->setClearColor( osg::vec4( 0.2F, 0.3F, 0.5F, 0.2F ) );
    // camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // draw subgraph after main camera view.
    camera->setRenderOrder( osg::Camera::POST_RENDER );

    // we don't want the camera to grab event focus from the viewers main camera(s).
    camera->setAllowEventFocus( false );

    osg::Geode* geode = new osg::Geode;

    osg::vec3   position( 10.0F, size.y - 100.0F, 0.0F );
    osg::vec3   delta( 0.0F, -120.0F, 0.0F );
    float       length = 300.0F;

    // turn the text off to avoid linking with osgText
#if 0
    std::string timesFont("fonts/arial.ttf");

    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );

        text->setFont(timesFont);
        text->setPosition(position);
        text->setText("Shadow Map HUD");

        position += delta;
    }
#endif

    osg::vec3 widthVec( length, 0.0F, 0.0F );
    osg::vec3 depthVec( 0.0F, length, 0.0F );
    osg::vec3 centerBase( 10.0F + length / 2, size.y - length / 2, 0.0F );
    centerBase += delta;

    osg::Geometry* geometry =
        osg::createTexturedQuadGeometry( centerBase - widthVec * 0.5F - depthVec * 0.5F,
                                         widthVec,
                                         depthVec );

    geode->addDrawable( geometry );

    geometry->setDrawCallback(
        new DrawableDrawWithDepthShadowComparisonOffCallback( _texture.get() )
    );

    osg::StateSet* stateset = geode->getOrCreateStateSet();

    // GL_LIGHTING removed: not in core profile
    stateset->setMode( GL_BLEND, osg::StateAttribute::ON );
    stateset->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

    // test with regular texture
    // stateset->setTextureAttributeAndModes(0, new
    // osg::Texture2D(osgDB::readImageFile("Images/lz.rgb")));

    stateset->setTextureAttributeAndModes( 0, _texture.get(), osg::StateAttribute::ON );

    // shader for correct display

    osg::ref_ptr<osg::Program> program = new osg::Program;
    stateset->setAttribute( program.get() );

    osg::Shader* vertex_shader =
        new osg::Shader( osg::Shader::VERTEX, vertexShaderSource_debugHUD );
    program->addShader( vertex_shader );
    osg::Shader* fragment_shader =
        new osg::Shader( osg::Shader::FRAGMENT, fragmentShaderSource_debugHUD );
    program->addShader( fragment_shader );

    camera->addChild( geode );

    return camera;
}

//////////////////////// End Debug Section
