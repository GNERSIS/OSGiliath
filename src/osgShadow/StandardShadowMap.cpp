/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
 *
 * ViewDependentShadow codes Copyright (C) 2008 Wojciech Lewandowski
 * Thanks to to my company http://www.ai.com.pl for allowing me free this work.
*/

#include <osgShadow/StandardShadowMap>
#include <osg/PolygonOffset>
#include <osg/ComputeBoundsVisitor>
#include <osgShadow/ShadowedScene>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/CullFace>
#include <osg/Point>

#include <stdio.h>

using namespace osgShadow;

#define DISPLAY_SHADOW_TEXEL_TO_PIXEL_ERROR 0


StandardShadowMap::StandardShadowMap():
    BaseClass(),
    _polygonOffsetFactor( 1.1f ),
    _polygonOffsetUnits( 4.0f ),
    _textureSize( 1024, 1024 ),
    _baseTextureUnit( 0 ),
    _shadowTextureUnit( 1 ),
    _baseTextureCoordIndex( 0 ),
    _shadowTextureCoordIndex( 1 )

{
    _mainFragmentShader = new osg::Shader( osg::Shader::FRAGMENT,
        "#version 460 core\n"
        "                                                                       \n"
        "out vec4 fragColor;                                                    \n"
        "float DynamicShadow( );                                                \n"
        "                                                                       \n"
        "in vec4 colorAmbientEmissive;                                          \n"
        "in vec4 vertexColor;                                                   \n"
        "in vec4 baseTexCoord;                                                  \n"
        "                                                                       \n"
        "uniform sampler2D baseTexture;                                         \n"
        "                                                                       \n"
        "void main(void)                                                        \n"
        "{                                                                      \n"
        "  vec4 color = texture( baseTexture, baseTexCoord.xy );                \n"
        "  color *= mix( colorAmbientEmissive, vertexColor, DynamicShadow() );  \n"
        "  fragColor = color;                                                   \n"
        "} \n" );

    _shadowFragmentShader = new osg::Shader( osg::Shader::FRAGMENT,
        "#version 460 core\n"
        "                                                                       \n"
        "uniform sampler2DShadow shadowTexture;                                 \n"
        "in vec4 shadowTexCoord;                                                \n"
        "                                                                       \n"
        "float DynamicShadow( )                                                 \n"
        "{                                                                      \n"
        "    return textureProj( shadowTexture, shadowTexCoord ).r;             \n"
        "} \n" );

    _shadowVertexShader = new osg::Shader( osg::Shader::VERTEX,
        "#version 460 core\n"
        "                                                                       \n"
        "uniform mat4 shadowTextureMatrix;                                      \n"
        "out vec4 shadowTexCoord;                                               \n"
        "                                                                       \n"
        "void DynamicShadow( in vec4 ecPosition )                               \n"
        "{                                                                      \n"
        "    shadowTexCoord = shadowTextureMatrix * ecPosition;                 \n"
        "} \n" );

    _mainVertexShader = new osg::Shader( osg::Shader::VERTEX,
        "#version 460 core\n"
        "layout(location = 0) in vec4 osg_Vertex;                              \n"
        "layout(location = 2) in vec3 osg_Normal;                              \n"
        "in vec4 osg_MultiTexCoord0;                                           \n"
        "uniform mat4 osg_ModelViewProjectionMatrix;                            \n"
        "uniform mat4 osg_ModelViewMatrix;                                      \n"
        "uniform mat3 osg_NormalMatrix;                                         \n"
        "                                                                       \n"
        "struct osg_LightSourceParameters {                                     \n"
        "    vec4 ambient; vec4 diffuse; vec4 specular; vec4 position;          \n"
        "    vec3 spotDirection; float spotExponent; float spotCutoff;          \n"
        "    float spotCosCutoff; float constantAttenuation;                    \n"
        "    float linearAttenuation; float quadraticAttenuation;              \n"
        "};                                                                     \n"
        "uniform osg_LightSourceParameters osg_LightSource;                     \n"
        "struct osg_MaterialParameters {                                        \n"
        "    vec4 ambient; vec4 diffuse; vec4 specular; vec4 emission;          \n"
        "    float shininess;                                                   \n"
        "};                                                                     \n"
        "uniform osg_MaterialParameters osg_FrontMaterial;                      \n"
        "                                                                       \n"
        "void DynamicShadow( in vec4 ecPosition );                              \n"
        "                                                                       \n"
        "out vec4 colorAmbientEmissive;                                         \n"
        "out vec4 baseTexCoord;                                                 \n"
        "out vec4 vertexColor;                                                  \n"
        "out vec4 vertexSecondaryColor;                                         \n"
        "                                                                       \n"
        "void SpotLight(in vec3 eye,                                            \n"
        "               in vec3 ecPosition3,                                    \n"
        "               in vec3 normal,                                         \n"
        "               inout vec4 ambient,                                     \n"
        "               inout vec4 diffuse,                                     \n"
        "               inout vec4 specular)                                    \n"
        "{                                                                      \n"
        "    float nDotVP;          // normal . light direction                 \n"
        "    float nDotHV;          // normal . light half vector               \n"
        "    float pf;              // power factor                             \n"
        "    float spotDot;         // cosine of angle between spotlight        \n"
        "    float spotAttenuation; // spotlight attenuation factor             \n"
        "    float attenuation;     // computed attenuation factor              \n"
        "    float d;               // distance from surface to light source    \n"
        "    vec3 VP;               // direction from surface to light position \n"
        "    vec3 halfVector;       // direction of maximum highlights          \n"
        "                                                                       \n"
        "    // Compute vector from surface to light position                   \n"
        "    VP = vec3(osg_LightSource.position) - ecPosition3;                 \n"
        "                                                                       \n"
        "    // Compute distance between surface and light position             \n"
        "    d = length(VP);                                                    \n"
        "                                                                       \n"
        "    // Normalize the vector from surface to light position             \n"
        "    VP = normalize(VP);                                                \n"
        "                                                                       \n"
        "    // Compute attenuation                                             \n"
        "    attenuation = 1.0 / (osg_LightSource.constantAttenuation +         \n"
        "                         osg_LightSource.linearAttenuation * d +       \n"
        "                         osg_LightSource.quadraticAttenuation *d*d);   \n"
        "                                                                       \n"
        "    // See if point on surface is inside cone of illumination          \n"
        "    spotDot = dot(-VP, normalize(osg_LightSource.spotDirection));      \n"
        "                                                                       \n"
        "    if (spotDot < osg_LightSource.spotCosCutoff)                       \n"
        "        spotAttenuation = 0.0; // light adds no contribution           \n"
        "    else                                                               \n"
        "        spotAttenuation = pow(spotDot, osg_LightSource.spotExponent);  \n"
        "                                                                       \n"
        "    // Combine the spotlight and distance attenuation.                 \n"
        "    attenuation *= spotAttenuation;                                    \n"
        "                                                                       \n"
        "    halfVector = normalize(VP + eye);                                  \n"
        "                                                                       \n"
        "    nDotVP = max(0.0, dot(normal, VP));                                \n"
        "    nDotHV = max(0.0, dot(normal, halfVector));                        \n"
        "                                                                       \n"
        "    if (nDotVP == 0.0)                                                 \n"
        "        pf = 0.0;                                                      \n"
        "    else                                                               \n"
        "        pf = pow(nDotHV, osg_FrontMaterial.shininess);                 \n"
        "                                                                       \n"
        "    ambient  += osg_LightSource.ambient * attenuation;                 \n"
        "    diffuse  += osg_LightSource.diffuse * nDotVP * attenuation;        \n"
        "    specular += osg_LightSource.specular * pf * attenuation;           \n"
        "}                                                                      \n"
        "                                                                       \n"
        "void PointLight(in vec3 eye,                                           \n"
        "                in vec3 ecPosition3,                                   \n"
        "                in vec3 normal,                                        \n"
        "                inout vec4 ambient,                                    \n"
        "                inout vec4 diffuse,                                    \n"
        "                inout vec4 specular)                                   \n"
        "{                                                                      \n"
        "    float nDotVP;      // normal . light direction                     \n"
        "    float nDotHV;      // normal . light half vector                   \n"
        "    float pf;          // power factor                                 \n"
        "    float attenuation; // computed attenuation factor                  \n"
        "    float d;           // distance from surface to light source        \n"
        "    vec3  VP;          // direction from surface to light position     \n"
        "    vec3  halfVector;  // direction of maximum highlights              \n"
        "                                                                       \n"
        "    // Compute vector from surface to light position                   \n"
        "    VP = vec3(osg_LightSource.position) - ecPosition3;                 \n"
        "                                                                       \n"
        "    // Compute distance between surface and light position             \n"
        "    d = length(VP);                                                    \n"
        "                                                                       \n"
        "    // Normalize the vector from surface to light position             \n"
        "    VP = normalize(VP);                                                \n"
        "                                                                       \n"
        "    // Compute attenuation                                             \n"
        "    attenuation = 1.0 / (osg_LightSource.constantAttenuation +         \n"
        "                         osg_LightSource.linearAttenuation * d +       \n"
        "                         osg_LightSource.quadraticAttenuation * d*d);  \n"
        "                                                                       \n"
        "    halfVector = normalize(VP + eye);                                  \n"
        "                                                                       \n"
        "    nDotVP = max(0.0, dot(normal, VP));                                \n"
        "    nDotHV = max(0.0, dot(normal, halfVector));                        \n"
        "                                                                       \n"
        "    if (nDotVP == 0.0)                                                 \n"
        "        pf = 0.0;                                                      \n"
        "    else                                                               \n"
        "        pf = pow(nDotHV, osg_FrontMaterial.shininess);                 \n"
        "                                                                       \n"
        "    ambient += osg_LightSource.ambient * attenuation;                  \n"
        "    diffuse += osg_LightSource.diffuse * nDotVP * attenuation;         \n"
        "    specular += osg_LightSource.specular * pf * attenuation;           \n"
        "}                                                                      \n"
        "                                                                       \n"
        "void DirectionalLight(in vec3 normal,                                  \n"
        "                      inout vec4 ambient,                              \n"
        "                      inout vec4 diffuse,                              \n"
        "                      inout vec4 specular)                             \n"
        "{                                                                      \n"
        "     float nDotVP;         // normal . light direction                 \n"
        "     float nDotHV;         // normal . light half vector               \n"
        "     float pf;             // power factor                             \n"
        "                                                                       \n"
        "     nDotVP = max(0.0, dot(normal,                                     \n"
        "                normalize(vec3(osg_LightSource.position))));           \n"
        "     vec3 halfVector = normalize(normalize(osg_LightSource.position.xyz) + vec3(0.0, 0.0, 1.0)); \n"
        "     nDotHV = max(0.0, dot(normal, halfVector));                       \n"
        "                                                                       \n"
        "     if (nDotVP == 0.0)                                                \n"
        "         pf = 0.0;                                                     \n"
        "     else                                                              \n"
        "         pf = pow(nDotHV, osg_FrontMaterial.shininess);                \n"
        "                                                                       \n"
        "     ambient  += osg_LightSource.ambient;                              \n"
        "     diffuse  += osg_LightSource.diffuse * nDotVP;                     \n"
        "     specular += osg_LightSource.specular * pf;                        \n"
        "}                                                                      \n"
        "                                                                       \n"
        "void main( )                                                           \n"
        "{                                                                      \n"
        "    // Transform vertex to clip space                                  \n"
        "    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;          \n"
        "    vec3 normal = normalize( osg_NormalMatrix * osg_Normal );          \n"
        "                                                                       \n"
        "    vec4  ecPos  = osg_ModelViewMatrix * osg_Vertex;                   \n"
        "    float ecLen  = length( ecPos );                                    \n"
        "    vec3  ecPosition3 = ecPos.xyz / ecPos.w;                           \n"
        "                                                                       \n"
        "    vec3  eye = vec3( 0.0, 0.0, 1.0 );                                 \n"
        "    //vec3  eye = -normalize(ecPosition3);                             \n"
        "                                                                       \n"
        "    DynamicShadow( ecPos );                                            \n"
        "                                                                       \n"
        "    baseTexCoord = osg_MultiTexCoord0;                                 \n"
        "                                                                       \n"
        "    // Front Face lighting                                             \n"
        "                                                                       \n"
        "    // Clear the light intensity accumulators                          \n"
        "    vec4 amb  = vec4(0.0);                                             \n"
        "    vec4 diff = vec4(0.0);                                             \n"
        "    vec4 spec = vec4(0.0);                                             \n"
        "                                                                       \n"
        "    // Compute lighting from the single light source                   \n"
        "    if (osg_LightSource.position.w == 0.0)                             \n"
        "        DirectionalLight(normal, amb, diff, spec);                     \n"
        "    else if (osg_LightSource.spotCutoff == 180.0)                      \n"
        "        PointLight(eye, ecPosition3, normal, amb, diff, spec);         \n"
        "    else                                                               \n"
        "        SpotLight(eye, ecPosition3, normal, amb, diff, spec);          \n"
        "                                                                       \n"
        "    vec4 sceneColor = osg_FrontMaterial.emission + osg_FrontMaterial.ambient * osg_LightSource.ambient; \n"
        "    colorAmbientEmissive = sceneColor +                                \n"
        "                           amb * osg_FrontMaterial.ambient;            \n"
        "                                                                       \n"
        "    vertexColor = colorAmbientEmissive +                               \n"
        "                    diff * osg_FrontMaterial.diffuse;                  \n"
        "                                                                       \n"
        "    vertexSecondaryColor = vec4(spec*osg_FrontMaterial.specular);      \n"
        "} \n" );
}

StandardShadowMap::StandardShadowMap(const StandardShadowMap& copy, const osg::CopyOp& copyop) :
    BaseClass(copy,copyop),
    _polygonOffsetFactor( copy._polygonOffsetFactor ),
    _polygonOffsetUnits( copy._polygonOffsetUnits ),
    _textureSize( copy._textureSize ),
    _baseTextureUnit( copy._baseTextureUnit ),
    _shadowTextureUnit( copy._shadowTextureUnit ),
    _baseTextureCoordIndex( copy._baseTextureCoordIndex ),
    _shadowTextureCoordIndex( copy._shadowTextureCoordIndex )
{
    if( copy._mainVertexShader.valid() )
        _mainVertexShader = dynamic_cast<osg::Shader*>
            ( copy._mainVertexShader->clone(copyop) );

    if( copy._mainFragmentShader.valid() )
        _mainFragmentShader = dynamic_cast<osg::Shader*>
            ( copy._mainFragmentShader->clone(copyop) );

    if( copy._shadowVertexShader.valid() )
        _shadowVertexShader = dynamic_cast<osg::Shader*>
            ( copy._shadowVertexShader->clone(copyop) );

    if( copy._shadowFragmentShader.valid() )
        _shadowFragmentShader = dynamic_cast<osg::Shader*>
            ( copy._shadowFragmentShader->clone(copyop) );
}

StandardShadowMap::~StandardShadowMap(void)
{

}

void StandardShadowMap::resizeGLObjectBuffers(unsigned int maxSize)
{
    osg::resizeGLObjectBuffers(_mainVertexShader, maxSize);
    osg::resizeGLObjectBuffers(_mainFragmentShader, maxSize);
    osg::resizeGLObjectBuffers(_shadowVertexShader, maxSize);
    osg::resizeGLObjectBuffers(_shadowFragmentShader, maxSize);

    DebugShadowMap::resizeGLObjectBuffers(maxSize);
}

void StandardShadowMap::releaseGLObjects(osg::State* state) const
{
    osg::releaseGLObjects(_mainVertexShader, state);
    osg::releaseGLObjects(_mainFragmentShader, state);
    osg::releaseGLObjects(_shadowVertexShader, state);
    osg::releaseGLObjects(_shadowFragmentShader, state);

    DebugShadowMap::releaseGLObjects(state);
}

void StandardShadowMap::ViewData::resizeGLObjectBuffers(unsigned int maxSize)
{
    osg::resizeGLObjectBuffers(_stateset, maxSize);
}

void StandardShadowMap::ViewData::releaseGLObjects(osg::State* state) const
{
    osg::releaseGLObjects(_stateset, state);
}


void StandardShadowMap::updateTextureCoordIndices( unsigned int fromTextureCoordIndex, unsigned int toTextureCoordIndex )
{

    if( fromTextureCoordIndex == toTextureCoordIndex ) return;

    const char *expressions[] = {
        "osg_MultiTexCoord","",
    };

    for( unsigned int i = 0;
         i < sizeof( expressions ) / sizeof( expressions[0] );
         i+=2 )
    {
        char acFrom[ 32 ], acTo[32];

        // its not elegant to mix stdio & stl strings
        // but in this context I do an exception for cleaner code

        sprintf( acFrom, "%s%d%s", expressions[i], fromTextureCoordIndex, expressions[i+1]);
        sprintf( acTo, "%s%d%s", expressions[i], toTextureCoordIndex, expressions[i+1]);

        std::string from( acFrom ), to( acTo );

        searchAndReplaceShaderSource( getShadowVertexShader(), from, to );
        searchAndReplaceShaderSource( getShadowFragmentShader(), from, to );
        searchAndReplaceShaderSource( getMainVertexShader(), from, to );
        searchAndReplaceShaderSource( getMainFragmentShader(), from, to );
    }

    dirty();
}

void StandardShadowMap::searchAndReplaceShaderSource
           ( osg::Shader* shader, std::string fromString, std::string toString )
{
    if( !shader || fromString == toString ) return;

    const std::string & srceString = shader->getShaderSource();
    std::string destString;

    std::string::size_type fromLength = fromString.length();
    std::string::size_type srceLength = srceString.length();

    for( std::string::size_type pos = 0; pos < srceLength; )
    {
        std::string::size_type end = srceString.find( fromString, pos );

        if( end == std::string::npos )
            end = srceLength;

        destString.append( srceString, pos, end - pos );

        if( end == srceLength )
            break;

        destString.append( toString );
        pos = end + fromLength;
    }

    shader->setShaderSource( destString );
}

void StandardShadowMap::ViewData::cull()
{
    // step 1:
    // cull shadowed scene ie put into render bins and states into stage graphs
    cullShadowReceivingScene( );

    // step 2:
    // find the light casting our shadows
    osg::Vec4 lightPos;
    osg::Vec3 lightDir;
    osg::Vec3 lightUp( 0,0,0 ); // force computing most approprate dir
    const osg::Light *light = selectLight( lightPos, lightDir );

    if ( !light )
        return;// bail out - no shadowing needed in darkest night

    // step 3:
    // compute shadow casting matrices and apply them to shadow map camera
    aimShadowCastingCamera( light, lightPos, lightDir, lightUp );

    // step 3b:
    // compute shadow texture matrix = bias * shadowProj * shadowView * inverseCameraView
    {
        osg::Matrix inverseCameraView = osg::Matrix::inverse( *_cv->getModelViewMatrix() );
        // Note: getModelViewMatrix at this point returns just the camera view (no model transform)
        // since we haven't pushed any model matrices yet

        osg::Matrix shadowView = _camera->getViewMatrix();
        osg::Matrix shadowProj = _camera->getProjectionMatrix();

        // Bias matrix: maps from [-1,1] clip space to [0,1] texture space
        osg::Matrix bias(
            0.5, 0.0, 0.0, 0.0,
            0.0, 0.5, 0.0, 0.0,
            0.0, 0.0, 0.5, 0.0,
            0.5, 0.5, 0.5, 1.0
        );

        osg::Matrixf shadowTextureMatrix =
            inverseCameraView * shadowView * shadowProj * bias;

        _shadowTextureMatrixUniform->set( shadowTextureMatrix );
    }

    // step 4:
    // cull scene casting shadow and generate render
    cullShadowCastingScene( );

    BaseClass::ViewData::cull();
}

void StandardShadowMap::ViewData::init( ThisClass *st, osgUtil::CullVisitor *cv )
{
    BaseClass::ViewData::init( st, cv );

    _lightPtr             = &st->_light;
    _shadowTextureUnitPtr = &st->_shadowTextureUnit;
    _baseTextureUnitPtr   = &st->_baseTextureUnit;

    { // Setup shadow texture
        osg::Texture2D * texture = new osg::Texture2D;
        texture->setTextureSize( st->_textureSize.x(), st->_textureSize.y());
        texture->setInternalFormat(GL_DEPTH_COMPONENT);
        texture->setShadowComparison(true);
        texture->setShadowTextureMode(osg::Texture2D::LUMINANCE);
        texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
        texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

        // the shadow comparison should fail if object is outside the texture
        texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
        texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
        texture->setBorderColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
        _texture = texture;
    }

    _camera = new osg::Camera;
    { // Setup shadow map camera
        _camera->setName( "ShadowCamera" );
#if 0  // Absolute reference frame INHERIT_VIEWPOINT works better than this
        _camera->setCullingMode
                ( _camera->getCullingMode() & ~osg::CullSettings::SMALL_FEATURE_CULLING );
#endif
        _camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF_INHERIT_VIEWPOINT);
        _camera->setCullCallback(new CameraCullCallback( st ));
        _camera->setClearMask(GL_DEPTH_BUFFER_BIT);

#if 0   // Left in case of some debug testing
        _camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        _camera->setClearColor( osg::Vec4(1.0f,1.0f,1.0f,1.0f) );
#endif
        _camera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);
        _camera->setViewport(0,0, st->_textureSize.x(), st->_textureSize.y() );
        _camera->setRenderOrder(osg::Camera::PRE_RENDER);
        _camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
        _camera->attach(osg::Camera::DEPTH_BUFFER, _texture.get());
    }

    _stateset = new osg::StateSet;
    { // Create and add fake texture for use with nodes without any texture
        osg::Image * image = new osg::Image;
        image->allocateImage( 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE );
        *(osg::Vec4ub*)image->data() = osg::Vec4ub( 0xFF, 0xFF, 0xFF, 0xFF );

        osg::Texture2D* fakeTex = new osg::Texture2D( image );
        fakeTex->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::REPEAT);
        fakeTex->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::REPEAT);
        fakeTex->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST);
        fakeTex->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::NEAREST);

        _stateset->setTextureAttribute(st->_baseTextureUnit,fakeTex,osg::StateAttribute::ON);
    }

    { // Add shadow texture
        _stateset->setTextureAttributeAndModes(st->_shadowTextureUnit,_texture.get(),osg::StateAttribute::ON);
    }

    {  // Setup shaders used in shadow casting
        osg::Program * program = new osg::Program();
        _stateset->setAttribute( program );

        if( st->_shadowFragmentShader.valid() )
            program->addShader( st->_shadowFragmentShader.get() );

        if( st->_mainFragmentShader.valid() )
            program->addShader( st->_mainFragmentShader.get() );

        if( st->_shadowVertexShader.valid() )
            program->addShader( st->_shadowVertexShader.get() );

        if( st->_mainVertexShader.valid() )
            program->addShader( st->_mainVertexShader.get() );

        _stateset->addUniform
            ( new osg::Uniform( "baseTexture", int( st->_baseTextureUnit ) ) );
        _stateset->addUniform
            ( new osg::Uniform( "shadowTexture", int( st->_shadowTextureUnit ) ) );

        // Shadow texture matrix uniform (updated each frame in cull())
        _shadowTextureMatrixUniform = new osg::Uniform( osg::Uniform::FLOAT_MAT4, "shadowTextureMatrix" );
        _shadowTextureMatrixUniform->set( osg::Matrixf::identity() );
        _stateset->addUniform( _shadowTextureMatrixUniform.get() );
    }

    { // Setup states used for shadow map generation
        osg::StateSet * stateset = _camera->getOrCreateStateSet();

        stateset->setAttribute(
            new osg::PolygonOffset( st->_polygonOffsetFactor, st->_polygonOffsetUnits ),
                osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

        stateset->setMode( GL_POLYGON_OFFSET_FILL,
              osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

        // aggressive optimization
        stateset->setRenderBinDetails( 0, "RenderBin",
                            osg::StateSet::OVERRIDE_RENDERBIN_DETAILS );

        // aggressive optimization
        stateset->setAttributeAndModes
            ( new osg::ColorMask( false, false, false, false ),
            osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

        // note soft (attribute only no mode override) setting. When this works ?
        // 1. for objects prepared for backface culling
        //    because they usually also set CullFace and CullMode on in their state
        //    For them we override CullFace but CullMode remains set by them
        // 2. For one faced, trees, and similar objects which cannot use
        //    backface nor front face so they usually use CullMode off set here.
        //    In this case we will draw them in their entirety.

        stateset->setAttribute( new osg::CullFace( osg::CullFace::FRONT ),
              osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

        // make sure GL_CULL_FACE is off by default
        // we assume that if object has cull face attribute set to back
        // it will also set cull face mode ON so no need for override
        stateset->setMode( GL_CULL_FACE, osg::StateAttribute::OFF );

        // optimization attributes
        osg::Program* program = new osg::Program;
        stateset->setAttribute( program, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON );
        stateset->setMode
            ( GL_LIGHTING, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF );
        stateset->setMode
            ( GL_BLEND, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF );

#if 0 // fixed pipeline seems faster (at least on my 7800)
        program->addShader( new osg::Shader( osg::Shader::FRAGMENT,
            "#version 460 core\n"
            "in vec4 texCoord_0;                                              \n"
            "out vec4 fragColor;                                              \n"
            "uniform sampler2D texture;                                       \n"
            "void main(void)                                                  \n"
            "{                                                                \n"
            " fragColor = texture( texture, texCoord_0.xy );                  \n"
            "}                                                                \n"
        ) ); // program->addShader Fragment

        program->addShader( new osg::Shader( osg::Shader::VERTEX,
            "#version 460 core\n"
            "layout(location = 0) in vec4 osg_Vertex;                         \n"
            "layout(location = 8) in vec4 osg_MultiTexCoord0;                 \n"
            "uniform mat4 osg_ModelViewProjectionMatrix;                      \n"
            "out vec4 texCoord_0;                                             \n"
            "void main(void)                                                  \n"
            "{                                                                \n"
            "   gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;     \n"
            "   texCoord_0 = osg_MultiTexCoord0;                              \n"
            "}                                                                \n"
        ) ); // program->addShader Vertex
#endif

        // GL_TEXTURE_1D/2D/3D texture mode removed: no-op in Core Profile
    }
}

const osg::Light* StandardShadowMap::ViewData::selectLight
                                ( osg::Vec4 & lightPos, osg::Vec3 & lightDir )
{
    const osg::Light* light = 0;

    //MR testing giving a specific light
    osgUtil::RenderStage * rs = _cv->getRenderStage();

    osgUtil::PositionalStateContainer::AttrMatrixList& aml =
        rs->getPositionalStateContainer()->getAttrMatrixList();

    osg::RefMatrix* matrix = 0;

    for(osgUtil::PositionalStateContainer::AttrMatrixList::iterator itr = aml.begin();
        itr != aml.end();
        ++itr)
    {
        const osg::Light* found = dynamic_cast<const osg::Light*>(itr->first.get());
        if( found )
        {
            if( _lightPtr->valid() && _lightPtr->get() != found )
                continue; // continue search for the right one

            light = found;
            matrix = itr->second.get();
        }
    }

    if( light ) { // transform light to world space

        osg::Matrix localToWorld = osg::Matrix::inverse( *_cv->getModelViewMatrix() );
        if( matrix ) localToWorld.preMult( *matrix );

        lightPos = light->getPosition();

        if( lightPos[3] == 0 )
            lightDir.set( -lightPos[0], -lightPos[1], -lightPos[2] );
        else
            lightDir = light->getDirection();

        lightPos = lightPos * localToWorld;
        lightDir = osg::Matrix::transform3x3( lightDir, localToWorld );
        lightDir.normalize();
    }

    return light;
}

void StandardShadowMap::ViewData::aimShadowCastingCamera( const osg::Light *light,
                                                  const osg::Vec4 &lightPos,
                                                  const osg::Vec3 &lightDir,
                                                  const osg::Vec3 &lightUp
                                        /* by default = osg::Vec3( 0, 1 0 )*/ )
{
#if 0 // less precise but faster
    osg::BoundingSphere bs =_st->getShadowedScene()->getBound();
#else
    // get the bounds of the model.
    osg::ComputeBoundsVisitor cbbv(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);
    cbbv.setTraversalMask(_st->getShadowedScene()->getCastsShadowTraversalMask());
    _st->getShadowedScene()->osg::Group::traverse(cbbv);
    osg::BoundingSphere bs( cbbv.getBoundingBox() );
#endif

    aimShadowCastingCamera
        ( bs, light, lightPos, lightDir, lightUp );
}

void StandardShadowMap::ViewData::aimShadowCastingCamera(
                                        const osg::BoundingSphere &bs,
                                        const osg::Light *light,
                                        const osg::Vec4 &lightPos,
                                        const osg::Vec3 &lightDir,
                                        const osg::Vec3 &lightUpVector
                                        /* by default = osg::Vec3( 0, 1 0 )*/ )
{
    osg::Matrixd & view = _camera->getViewMatrix();
    osg::Matrixd & projection = _camera->getProjectionMatrix();

    osg::Vec3 up = lightUpVector;
    if( up.length2() <= 0 )  up.set( 0,1,0 );

    osg::Vec3d position(lightPos.x(), lightPos.y(), lightPos.z());
    if (lightPos[3]==0.0)   // infinite directional light
    {
        // make an orthographic projection
        // set the position far away along the light direction
        position = bs.center() - lightDir * bs.radius() * 2;
    }

    float centerDistance = (position-bs.center()).length();
    float znear = centerDistance-bs.radius();
    float zfar  = centerDistance+bs.radius();
    float zNearRatio = 0.001f;
    if (znear<zfar*zNearRatio)
        znear = zfar*zNearRatio;

    if ( lightPos[3]!=0.0 ) {  // positional light
        if( light->getSpotCutoff() < 180.0f) // also needs znear zfar estimates
        {
            float spotAngle = light->getSpotCutoff();
            projection.makePerspective( spotAngle * 2, 1.0, znear, zfar);
            view.makeLookAt(position,position+lightDir,up);
        } else { // standard omnidirectional positional light
            float top   = (bs.radius()/centerDistance)*znear;
            float right = top;

            projection.makeFrustum(-right,right,-top,top,znear,zfar);
            view.makeLookAt(position,bs.center(),up );
        }
    }
    else    // directional light
    {
            float top   = bs.radius();
            float right = top;
            projection.makeOrtho(-right, right, -top, top, znear, zfar);
            view.makeLookAt(position,bs.center(),up);
    }
}

void StandardShadowMap::ViewData::cullShadowReceivingScene( )
{
    _cv->pushStateSet( _stateset.get() );

    _st->getShadowedScene()->osg::Group::traverse( *_cv );

    _cv->popStateSet();
}

void StandardShadowMap::ViewData::cullShadowCastingScene( )
{
    // record the traversal mask on entry so we can reapply it later.
    unsigned int traversalMask = _cv->getTraversalMask();

    _cv->setTraversalMask( traversalMask &
         _st->getShadowedScene()->getCastsShadowTraversalMask() );

    // do RTT camera traversal
    _camera->accept(*_cv);

    // reapply the original traversal mask
    _cv->setTraversalMask( traversalMask );
}


