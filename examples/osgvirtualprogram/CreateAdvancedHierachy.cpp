#include <iostream>
#include <osg/Geode>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osg/BlendFunc>
#include <osgText/Text>
#include <osgDB/ReadFile>

#include "VirtualProgram.h"

using osgCandidate::VirtualProgram;

////////////////////////////////////////////////////////////////////////////////
// Example shaders assume:
// one texture
// one directional light
// front face lighting
// color material mode not used (its not supported by GLSL anyway)
// diffuse/ambient/emissive/specular factors defined in material structure
// all coords and normal except gl_Position are in view space
////////////////////////////////////////////////////////////////////////////////

char MainVertexShaderSource[] =
"#version 460 core                                                          \n"
"layout(location = 0) in vec4 osg_Vertex;                                   \n"
"layout(location = 2) in vec3 osg_Normal;                                   \n"
"uniform mat4 osg_ModelViewProjectionMatrix;                                \n"
"uniform mat4 osg_ModelViewMatrix;                                          \n"
"uniform mat3 osg_NormalMatrix;                                             \n"
"out vec4 vTexCoord;                                                        \n"
"out vec4 vColor;                                                           \n"
"out vec4 vSecondaryColor;                                                  \n"
"out vec3 vPosition;                                                        \n"
"out vec3 vNormalVS;                                                        \n"
"                                                                           \n"
"vec4 texture_func( in vec3 position, in vec3 normal );                     \n"
"void lighting( in vec3 position, in vec3 normal );                         \n"
"                                                                           \n"
"void main ()                                                               \n"
"{                                                                          \n"
"    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;              \n"
"    vec4 position4 = osg_ModelViewMatrix * osg_Vertex;                     \n"
"    vec3 position = position4.xyz / position4.w;                           \n"
"    vec3 normal = normalize( osg_NormalMatrix * osg_Normal );              \n"
"    vTexCoord = texture_func( position, normal );                          \n"
"    vPosition = position;                                                  \n"
"    vNormalVS = normal;                                                    \n"
"    lighting( position, normal );                                          \n"
"}                                                                          \n";

char TexCoordTextureVertexShaderSource[] =
"#version 460 core                                                          \n"
"layout(location = 8) in vec4 osg_MultiTexCoord0;                           \n"
"vec4 texture_func( in vec3 position, in vec3 normal )                      \n"
"{                                                                          \n"
"    return osg_MultiTexCoord0;                                             \n"
"}                                                                          \n";

char SphereMapTextureVertexShaderSource[] =
"#version 460 core                                                          \n"
"vec4 texture_func( in vec3 position, in vec3 normal )                      \n"
"{                                                                          \n"
"    vec3 u = normalize( position );                                        \n"
"    vec3 r = reflect(u, normal);                                           \n"
"    float m = 2.0 * sqrt(r.x * r.x + r.y * r.y + (r.z+1.0) * (r.z+1.0));\n"
"    return vec4(r.x / m + 0.5, r.y / m + 0.5, 1.0, 1.0 );                \n"
"}                                                                          \n";

char PerVertexDirectionalLightingVertexShaderSource[] =
"#version 460 core                                                          \n"
"out vec4 vColor;                                                           \n"
"out vec4 vSecondaryColor;                                                  \n"
"                                                                           \n"
"void lighting( in vec3 position, in vec3 normal )                          \n"
"{                                                                          \n"
"    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.8));                        \n"
"    float NdotL = max( 0.0, dot( normal, lightDir ) );                     \n"
"    vec3 viewDir = normalize(-position);                                   \n"
"    vec3 halfVec = normalize(lightDir + viewDir);                          \n"
"    float NdotHV = max( 0.0, dot( normal, halfVec ) );                     \n"
"                                                                           \n"
"    vec4 ambient = vec4(0.2, 0.2, 0.2, 1.0);                               \n"
"    vec4 diffuse = vec4(0.8, 0.8, 0.8, 1.0);                               \n"
"    vec4 specular = vec4(0.7, 0.3, 0.3, 1.0);                              \n"
"    float shininess = 25.0;                                                 \n"
"                                                                           \n"
"    vColor = ambient + diffuse * NdotL;                                     \n"
"    vSecondaryColor = vec4(0.0);                                            \n"
"    if ( NdotL * NdotHV > 0.0 )                                            \n"
"        vSecondaryColor = specular * pow( NdotHV, shininess );              \n"
"}                                                                          \n";

char MainFragmentShaderSource[] =
"#version 460 core                                                          \n"
"out vec4 fragColor;                                                        \n"
"                                                                           \n"
"vec4 texture_func( void );                                                 \n"
"void lighting( inout vec4 color );                                         \n"
"                                                                           \n"
"void main ()                                                               \n"
"{                                                                          \n"
"    vec4 color = texture_func();                                           \n"
"    lighting( color );                                                     \n"
"    fragColor = color;                                                     \n"
"}                                                                          \n";

char TextureFragmentShaderSource[] =
"#version 460 core                                                          \n"
"uniform sampler2D baseTexture;                                             \n"
"in vec4 vTexCoord;                                                         \n"
"vec4 texture_func( void )                                                  \n"
"{                                                                          \n"
"    return texture( baseTexture, vTexCoord.xy );                           \n"
"}                                                                          \n";

char ProceduralBlueTextureFragmentShaderSource[] =
"#version 460 core                                                          \n"
"vec4 texture_func( void )                                                  \n"
"{                                                                          \n"
"    return vec4( 0.3, 0.3, 1.0, 1.0 );                                     \n"
"}                                                                          \n";

char PerVertexLightingFragmentShaderSource[] =
"#version 460 core                                                          \n"
"in vec4 vColor;                                                            \n"
"in vec4 vSecondaryColor;                                                   \n"
"void lighting( inout vec4 color )                                          \n"
"{                                                                          \n"
"    color = color * vColor + vSecondaryColor;                              \n"
"}                                                                          \n";

char PerFragmentLightingVertexShaderSource[] =
"#version 460 core                                                          \n"
"out vec3 vNormalPF;                                                        \n"
"out vec3 vPositionPF;                                                      \n"
"out vec4 vColor;                                                           \n"
"out vec4 vSecondaryColor;                                                  \n"
"                                                                           \n"
"void lighting( in vec3 position, in vec3 normal )                          \n"
"{                                                                          \n"
"    vNormalPF = normal;                                                    \n"
"    vPositionPF = position;                                                \n"
"    vColor = vec4(1.0);                                                    \n"
"    vSecondaryColor = vec4(0.0);                                           \n"
"}                                                                          \n";

char PerFragmentDirectionalLightingFragmentShaderSource[] =
"#version 460 core                                                          \n"
"in vec3 vNormalPF;                                                         \n"
"in vec3 vPositionPF;                                                       \n"
"in vec4 vColor;                                                            \n"
"in vec4 vSecondaryColor;                                                   \n"
"                                                                           \n"
"void lighting( inout vec4 color )                                          \n"
"{                                                                          \n"
"    vec3 n = normalize( vNormalPF );                                       \n"
"    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.8));                        \n"
"    float NdotL = max( 0.0, dot( n, lightDir ) );                          \n"
"    vec3 viewDir = normalize(-vPositionPF);                                \n"
"    vec3 halfVec = normalize(lightDir + viewDir);                          \n"
"    float NdotHV = max( 0.0, dot( n, halfVec ) );                          \n"
"                                                                           \n"
"    vec4 ambient = vec4(0.2, 0.2, 0.2, 1.0);                               \n"
"    vec4 diffuse = vec4(0.8, 0.8, 0.8, 1.0);                               \n"
"    vec4 specular = vec4(0.7, 0.3, 0.3, 1.0);                              \n"
"    float shininess = 25.0;                                                 \n"
"                                                                           \n"
"    color *= ambient + diffuse * NdotL;                                     \n"
"    if ( NdotL * NdotHV > 0.0 )                                            \n"
"        color += specular * pow( NdotHV, shininess );                       \n"
"}                                                                          \n";

////////////////////////////////////////////////////////////////////////////////
// Convenience method to simplify code a little ...
void SetVirtualProgramShader( VirtualProgram * virtualProgram,
                              std::string shader_semantics,
                              osg::Shader::Type shader_type,
                              std::string shader_name,
                              std::string shader_source )
{
    osg::Shader * shader = new osg::Shader( shader_type );
    shader->setName( shader_name );
    shader->setShaderSource( shader_source );
    virtualProgram->setShader( shader_semantics, shader );
}
///////////////////////////////////////////////////////////////////////////////
void AddLabel( osg::Group * group, const std::string & label, float offset )
{
    osg::Vec3 center( 0, 0, offset * 0.5 );
    osg::Geode * geode = new osg::Geode;

    // Make sure no program breaks text outputs
    geode->getOrCreateStateSet()->setAttribute
      ( new osg::Program, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

    group->addChild( geode );

    osgText::Text* text = new osgText::Text;
    geode->addDrawable( text );
    text->setFont("fonts/times.ttf");
    text->setCharacterSize( offset * 0.1 );
    text->setPosition(center);
    text->setAlignment( osgText::TextBase::CENTER_CENTER );
    text->setAxisAlignment(osgText::Text::SCREEN);

    osg::Vec4 characterSizeModeColor(1.0f,0.0f,0.5f,1.0f);
#if 1
    // reproduce outline bounding box compute problem with backdrop on.
    text->setBackdropType(osgText::Text::OUTLINE);
    text->setDrawMode(osgText::Text::TEXT | osgText::Text::BOUNDINGBOX);
#endif

    text->setText( label );
}
////////////////////////////////////////////////////////////////////////////////
osg::Node * CreateAdvancedHierarchy( osg::Node * model )
{
    if( !model ) return NULL;
    float offset = model->getBound().radius() * 1.3; // diameter

    // Create transforms for translated instances of the model
    osg::MatrixTransform * transformCenterMiddle  = new osg::MatrixTransform( );
    transformCenterMiddle->setMatrix( osg::Matrix::translate( 0,0, offset * 0.5 ) );
    transformCenterMiddle->addChild( model );

    osg::MatrixTransform * transformCenterTop  = new osg::MatrixTransform( );
    transformCenterMiddle->addChild( transformCenterTop );
    transformCenterTop->setMatrix( osg::Matrix::translate( 0,0,offset ) );
    transformCenterTop->addChild( model );

    osg::MatrixTransform * transformCenterBottom  = new osg::MatrixTransform( );
    transformCenterMiddle->addChild( transformCenterBottom );
    transformCenterBottom->setMatrix( osg::Matrix::translate( 0,0,-offset ) );
    transformCenterBottom->addChild( model );

    osg::MatrixTransform * transformLeftBottom  = new osg::MatrixTransform( );
    transformCenterBottom->addChild( transformLeftBottom );
    transformLeftBottom->setMatrix( osg::Matrix::translate( -offset * 0.8,0, -offset * 0.8 ) );
    transformLeftBottom->addChild( model );

    osg::MatrixTransform * transformRightBottom  = new osg::MatrixTransform( );
    transformCenterBottom->addChild( transformRightBottom );
    transformRightBottom->setMatrix( osg::Matrix::translate( offset * 0.8,0, -offset * 0.8 ) );
    transformRightBottom->addChild( model );

    // Set default VirtualProgram in root StateSet
    // With main vertex and main fragment shaders calling
    // lighting and texture functions defined in additional shaders
    // Lighting is done per vertex using simple directional light
    // Texture uses stage 0 TexCoords and TexMap

    if( 1 )
    {
        // NOTE:
        // duplicating the same semantics name in virtual program
        // is only possible if its used for shaders of differing types
        // here for VERTEX and FRAGMENT

        VirtualProgram * vp = new VirtualProgram( );
        transformCenterMiddle->getOrCreateStateSet()->setAttribute( vp );
        AddLabel( transformCenterMiddle, "Per Vertex Lighting Virtual Program", offset );

        SetVirtualProgramShader( vp, "main", osg::Shader::VERTEX,
            "Vertex Main", MainVertexShaderSource );

        SetVirtualProgramShader( vp, "main", osg::Shader::FRAGMENT,
            "Fragment Main", MainFragmentShaderSource );

        SetVirtualProgramShader( vp, "texture",osg::Shader::VERTEX,
            "Vertex Texture Coord 0", TexCoordTextureVertexShaderSource );

        SetVirtualProgramShader( vp, "texture",osg::Shader::FRAGMENT,
            "Fragment Texture", TextureFragmentShaderSource );

        SetVirtualProgramShader( vp, "lighting",osg::Shader::VERTEX,
            "Vertex Lighting", PerVertexDirectionalLightingVertexShaderSource );

        SetVirtualProgramShader( vp, "lighting",osg::Shader::FRAGMENT,
            "Fragment Lighting", PerVertexLightingFragmentShaderSource );

        transformCenterMiddle->getOrCreateStateSet()->
            addUniform( new osg::Uniform( "baseTexture", 0 ) );

    }

    // Override default vertex ligting with pixel lighting shaders
    // For three bottom models
    if( 1 )
    {
        AddLabel( transformCenterBottom, "Per Pixel Lighting VP", offset );
        VirtualProgram * vp = new VirtualProgram( );
        transformCenterBottom->getOrCreateStateSet()->setAttribute( vp );

        SetVirtualProgramShader( vp, "lighting",osg::Shader::VERTEX,
            "Vertex Shader For Per Pixel Lighting",
            PerFragmentLightingVertexShaderSource );

        SetVirtualProgramShader( vp, "lighting",osg::Shader::FRAGMENT,
            "Fragment Shader For Per Pixel Lighting",
            PerFragmentDirectionalLightingFragmentShaderSource );
    }

    // Additionally set bottom left model texture to procedural blue to
    // better observe smooth speculars done through per pixel lighting
    if( 1 )
    {
        AddLabel( transformLeftBottom, "Blue Tex VP", offset );
        VirtualProgram * vp = new VirtualProgram( );
        transformLeftBottom->getOrCreateStateSet()->setAttribute( vp );

        SetVirtualProgramShader( vp, "texture",osg::Shader::FRAGMENT,
            "Fragment Shader Procedural Blue Tex",
            ProceduralBlueTextureFragmentShaderSource );
    }

    // Additionally change texture mapping to SphereMAp in bottom right model
    if( 1 )
    {
        AddLabel( transformRightBottom, "EnvMap Sphere VP", offset );

        osg::StateSet * ss = transformRightBottom->getOrCreateStateSet();
        VirtualProgram * vp = new VirtualProgram( );
        ss->setAttribute( vp );
        SetVirtualProgramShader( vp, "texture",osg::Shader::VERTEX,
            "Vertex Texture Sphere Map", SphereMapTextureVertexShaderSource );

        osg::Texture2D * texture =  new osg::Texture2D( osgDB::readRefImageFile("Images/skymap.jpg") );

        // Texture is set on stage 1 to not interfere with label text
        // The same could be achieved with texture override
        // but such approach also turns off label texture
        ss->setTextureAttributeAndModes( 1, texture, osg::StateAttribute::ON );
        ss->addUniform( new osg::Uniform( "baseTexture", 1 ) );


    }


    // Top center model usues osg::Program overriding VirtualProgram in model
    if( 1 )
    {
        AddLabel( transformCenterTop, "Fixed Vertex + Simple Fragment osg::Program", offset );
        osg::Program * program = new osg::Program;
        program->setName( "Trivial Fragment + Fixed Vertex Program" );

        transformCenterTop->getOrCreateStateSet( )->setAttributeAndModes
            ( program, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

        osg::Shader * vertShader = new osg::Shader( osg::Shader::VERTEX );
        vertShader->setName( "Trivial Vertex Shader" );
        vertShader->setShaderSource(
            "#version 460 core\n"
            "layout(location = 0) in vec4 osg_Vertex;\n"
            "layout(location = 3) in vec4 osg_Color;\n"
            "layout(location = 8) in vec4 osg_MultiTexCoord0;\n"
            "uniform mat4 osg_ModelViewProjectionMatrix;\n"
            "out vec4 vColor;\n"
            "out vec2 vTexCoord;\n"
            "void main(void)\n"
            "{\n"
            "    vColor = osg_Color;\n"
            "    vTexCoord = osg_MultiTexCoord0.xy;\n"
            "    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
            "}\n"
            );
        program->addShader( vertShader );

        osg::Shader * shader = new osg::Shader( osg::Shader::FRAGMENT );
        shader->setName( "Trivial Fragment Shader" );
        shader->setShaderSource(
            "#version 460 core\n"
            "uniform sampler2D baseTexture;\n"
            "in vec4 vColor;\n"
            "in vec2 vTexCoord;\n"
            "out vec4 fragColor;\n"
            "void main(void)\n"
            "{\n"
            "    fragColor = vColor * texture( baseTexture, vTexCoord);\n"
            "}\n"
            );

        program->addShader( shader );
    }

    return transformCenterMiddle;
}

////////////////////////////////////////////////////////////////////////////////
// Shaders not used in the example but left for reference - modernized to 460 core
char LightingVertexShaderSource[] =
"#version 460 core                                                          \n"
"// Forward declarations                                                    \n"
"                                                                           \n"
"void DirectionalLight( in vec3 normal,                                     \n"
"            inout vec4 ambient, inout vec4 diffuse, inout vec4 specular ); \n"
"                                                                           \n"
"out vec4 vColor;                                                           \n"
"out vec4 vSecondaryColor;                                                  \n"
"                                                                           \n"
"void lighting( in vec3 position, in vec3 normal )                          \n"
"{                                                                          \n"
"    // Clear the light intensity accumulators                              \n"
"    vec4 amb  = vec4(0.0);                                                 \n"
"    vec4 diff = vec4(0.0);                                                 \n"
"    vec4 spec = vec4(0.0);                                                 \n"
"                                                                           \n"
"    DirectionalLight(normal, amb, diff, spec);                              \n"
"                                                                           \n"
"    vec4 matAmbient = vec4(0.9, 0.9, 0.9, 1.0);                            \n"
"    vec4 matDiffuse = vec4(0.9, 0.9, 0.9, 1.0);                            \n"
"    vec4 matSpecular = vec4(0.7, 0.3, 0.3, 1.0);                           \n"
"                                                                           \n"
"    vColor = vec4(0.1, 0.1, 0.1, 1.0) +                                    \n"
"             amb * matAmbient + diff * matDiffuse;                          \n"
"    vSecondaryColor = vec4(spec * matSpecular);                             \n"
"}                                                                          \n";

char DirectionalLightShaderSource[] =
"#version 460 core                                                          \n"
"void DirectionalLight( in vec3 normal,                                     \n"
"                       inout vec4 ambient,                                 \n"
"                       inout vec4 diffuse,                                 \n"
"                       inout vec4 specular)                                \n"
"{                                                                          \n"
"     vec3 lightDir = normalize(vec3(0.5, 1.0, 0.8));                       \n"
"     float nDotVP = max(0.0, dot(normal, lightDir));                       \n"
"     vec3 halfVec = normalize(lightDir + vec3(0.0, 0.0, 1.0));             \n"
"     float nDotHV = max(0.0, dot(normal, halfVec));                         \n"
"     float shininess = 25.0;                                                \n"
"     float pf = (nDotVP == 0.0) ? 0.0 : pow(nDotHV, shininess);            \n"
"                                                                           \n"
"     ambient  += vec4(0.2, 0.2, 0.2, 1.0);                                 \n"
"     diffuse  += vec4(0.8, 0.8, 0.8, 1.0) * nDotVP;                        \n"
"     specular += vec4(1.0, 1.0, 1.0, 1.0) * pf;                            \n"
"}                                                                          \n";

