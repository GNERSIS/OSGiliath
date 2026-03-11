/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Parallel-split (cascaded) shadow mapping. Divides the view
 * frustum into parallel slices with separate shadow maps.
 */
/* #####################################################################################################
 */
/* ParallelSplitShadowMap written by Adrian Egli (3dhelp (at) gmail.com) */
/* #####################################################################################################
 */
/*                                                                                                       */
/* the pssm main idea is based on: */
/*                                                                                                       */
/* Parallel-Split Shadow Maps for Large-scale Virtual Environments */
/*    Fan Zhang     Hanqiu Sun    Leilei Xu    Lee Kit Lun */
/*    The Chinese University of Hong Kong */
/*                                                                                                       */
/* Refer to our latest project webpage for "Parallel-Split Shadow Maps on Programmable
 * GPUs" in GPU Gems */
/*                                                                                                       */
/* #####################################################################################################
 */

#include <osgShadow/ParallelSplitShadowMap>

#include <iostream>
#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/state/CullFace.hpp>
#include <osg/state/Depth.hpp>
#include <osg/state/PolygonOffset.hpp>
#include <osg/textures/Texture1D.hpp>
#include <osg/traversal/ComputeBoundsVisitor.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgShadow/ShadowedScene>
#include <sstream>
using namespace osgShadow;

// split scheme
#define TEXTURE_RESOLUTION                            1'024

#define ZNEAR_MIN_FROM_LIGHT_SOURCE                   5.0
#define MOVE_VIRTUAL_CAMERA_BEHIND_REAL_CAMERA_FACTOR 0.0

// #define SHOW_SHADOW_TEXTURE_DEBUG    // DEPTH instead of color for debug information
// texture display in a rectangle #define SHADOW_TEXTURE_DEBUG         // COLOR instead
// of DEPTH

#ifndef SHADOW_TEXTURE_DEBUG
    #define SHADOW_TEXTURE_GLSL
#endif

//////////////////////////////////////////////////////////////////////////
// FragmentShaderGenerator
std::string
ParallelSplitShadowMap::FragmentShaderGenerator::generateGLSL_FragmentShader_BaseTex(
    bool         debug,
    unsigned int splitCount,
    double       textureRes,
    bool         filtered,
    unsigned int nbrSplits,
    unsigned int /*textureOffset*/
)
{
    std::stringstream sstr;

    sstr << "#version 460 core" << std::endl;
    sstr << "in vec4 vertexColor;" << std::endl;
    sstr << "in vec4 texCoord_0;" << std::endl;
    for( unsigned int i = 0; i < nbrSplits; i++ )
    {
        sstr << "in vec4 shadowTexCoord_" << i << ";" << std::endl;
    }
    sstr << "out vec4 fragColor;" << std::endl;

    /// base texture
    sstr << "uniform sampler2D baseTexture; " << std::endl;
    sstr << "uniform float enableBaseTexture; " << std::endl;
    sstr << "uniform vec2 ambientBias;" << std::endl;

    for( unsigned int i = 0; i < nbrSplits; i++ )
    {
        sstr << "uniform sampler2DShadow shadowTexture" << i << "; " << std::endl;
        sstr << "uniform float zShadow" << i << "; " << std::endl;
    }

    sstr << "void main(void)" << std::endl;
    sstr << "{" << std::endl;

    /// select the shadow map : split
    sstr << "float testZ = gl_FragCoord.z*2.0-1.0;" << std::endl;
    sstr << "float map0 = step(testZ, zShadow0);" << std::endl;    // DEBUG
    for( unsigned int i = 1; i < nbrSplits; i++ )
    {
        sstr << "float map" << i << "  = step(zShadow" << i -
            1 << ",testZ)*step(testZ, zShadow"
             << i << ");" << std::endl;    // DEBUG
    }

    if( filtered )
    {
        sstr << "          float fTexelSize=" << ( 1.41 / textureRes ) << ";"
             << std::endl;
        sstr << "          float fZOffSet  = -0.001954;"
             << std::endl;    // 2^-9 good value for ATI / NVidia
    }
    for( unsigned int i = 0; i < nbrSplits; i++ )
    {
        if( !filtered )
        {
            sstr << "    float shadow" << i << " = texture( shadowTexture" << i
                 << ",shadowTexCoord_" << i << ".xyz).r;" << std::endl;
            sstr << " shadow" << i << " = step(0.25,shadow" << i << ");"
                 << std::endl;    // reduce shadow artifacts
        }
        else
        {

            // filter the shadow (look up) 3x3
            //
            // 1 0 1
            // 0 2 0
            // 1 0 1
            //
            // / 6

            sstr << "    float shadowOrg" << i << " = texture( shadowTexture" << i
                 << ",shadowTexCoord_" << i << ".xyz+vec3(0.0,0.0,fZOffSet) ).r;"
                 << std::endl;
            sstr << "    float shadow0" << i << " = texture( shadowTexture" << i
                 << ",shadowTexCoord_" << i
                 << ".xyz+vec3(-fTexelSize,-fTexelSize,fZOffSet) ).r;" << std::endl;
            sstr << "    float shadow1" << i << " = texture( shadowTexture" << i
                 << ",shadowTexCoord_" << i
                 << ".xyz+vec3( fTexelSize,-fTexelSize,fZOffSet) ).r;" << std::endl;
            sstr << "    float shadow2" << i << " = texture( shadowTexture" << i
                 << ",shadowTexCoord_" << i
                 << ".xyz+vec3( fTexelSize, fTexelSize,fZOffSet) ).r;" << std::endl;
            sstr << "    float shadow3" << i << " = texture( shadowTexture" << i
                 << ",shadowTexCoord_" << i
                 << ".xyz+vec3(-fTexelSize, fTexelSize,fZOffSet) ).r;" << std::endl;

            sstr << "    float shadow" << i << " = ( 2.0*shadowOrg" << i << " + shadow0"
                 << i << " + shadow1" << i << " + shadow2" << i << " + shadow3" << i
                 << ")/6.0;" << std::endl;

            // sstr << " shadow"    <<    i    <<" = shadow"    <<    i    <<" *
            // step(0.025,shadow"    <<    i    <<");" << std::endl; // reduce shadow
            // artifacts

            // sstr << "    float shadow02"    <<    i    <<" = (shadow0"    <<    i
            // <<"+shadow2"    <<    i    <<")*0.5;"<< std::endl; sstr << "    float
            // shadow13"    <<    i    <<" = (shadow1"    <<    i    <<"+shadow3"    <<
            // i    <<")*0.5;"<< std::endl; sstr << "    float shadowSoft"    <<    i <<"
            // = (shadow02"    <<    i    <<"+shadow13"    <<    i    <<")*0.5;"<<
            // std::endl; sstr << "    float shadow"    <<    i    <<" = (shadowSoft" <<
            // i    <<"+shadowOrg"    <<    i    <<")*0.5;"<< std::endl; sstr << "
            // shadow"    <<    i    <<" = step(0.25,shadow"    <<    i    <<");" <<
            // std::endl; // reduce shadow artifacts
        }
    }

    sstr << "    float term0 = (1.0-shadow0)*map0; " << std::endl;
    for( unsigned int i = 1; i < nbrSplits; i++ )
    {
        sstr << "    float term" << i << " = map" << i << "*(1.0-shadow" << i << ");"
             << std::endl;
    }

    /// build shadow factor value v
    sstr << "    float v = clamp(";
    for( unsigned int i = 0; i < nbrSplits; i++ )
    {
        sstr << "term" << i;
        if( i + 1 < nbrSplits )
        {
            sstr << "+";
        }
    }
    sstr << ",0.0,1.0);" << std::endl;

    if( debug )
    {

        sstr << "    float c0=0.0;" << std::endl;
        sstr << "    float c1=0.0;" << std::endl;
        sstr << "    float c2=0.0;" << std::endl;

        sstr << "    float sumTerm=0.0;" << std::endl;

        for( unsigned int i = 0; i < nbrSplits; i++ )
        {
            if( i < 3 )
            {
                sstr << "    c" << i << "=term" << i << ";" << std::endl;
            }
            sstr << "    sumTerm=sumTerm+term" << i << ";" << std::endl;
        }

        sstr << "    vec4 color    = vertexColor*( 1.0 - sumTerm ) + (sumTerm)* "
                "vertexColor*vec4(c0,(1.0-c0)*c1,(1.0-c0)*(1.0-c1)*c2,1.0); "
             << std::endl;

        switch( nbrSplits )
        {
            case 1 :
                sstr << "    color    =  color*0.75 + vec4(map0,0,0,1.0)*0.25; "
                     << std::endl;
                break;
            case 2 :
                sstr << "    color    =  color*0.75 + vec4(map0,map1,0,1.0)*0.25; "
                     << std::endl;
                break;
            case 3 :
                sstr << "    color    =  color*0.75 + vec4(map0,map1,map2,1.0)*0.25; "
                     << std::endl;
                break;
            case 4 :
                sstr << "    color    =  color*0.75 + "
                        "vec4(map0+map3,map1+map3,map2,1.0)*0.25; "
                     << std::endl;
                break;
            case 5 :
                sstr << "    color    =  color*0.75 + "
                        "vec4(map0+map3,map1+map3+map4,map2+map4,1.0)*0.25; "
                     << std::endl;
                break;
            case 6 :
                sstr << "    color    =  color*0.75 + "
                        "vec4(map0+map3+map5,map1+map3+map4,map2+map4+map5,1.0)*0.25; "
                     << std::endl;
                break;
            default :
                break;
        }
    }
    else
    {
        sstr << "    vec4 color    = vertexColor; " << std::endl;
    }

    sstr << "    vec4 texcolor = texture(baseTexture,texCoord_0.st); " << std::endl;

    sstr << "    float enableBaseTextureFilter = enableBaseTexture*(1.0 - "
            "step(texcolor.x+texcolor.y+texcolor.z+texcolor.a,0.0)); "
         << std::endl;    // 18
    sstr << "    vec4 colorTex = color*texcolor;" << std::endl;
    sstr << "    fragColor.rgb = "
            "(((color*(ambientBias.x+1.0)*(1.0-enableBaseTextureFilter)) + "
            "colorTex*(1.0+ambientBias.x)*enableBaseTextureFilter)*(1.0-ambientBias.y*v)"
            ").rgb; "
         << std::endl;
    sstr << "    fragColor.a = (color*(1.0-enableBaseTextureFilter) + "
            "colorTex*enableBaseTextureFilter).a; "
         << std::endl;

    sstr << "}" << std::endl;

    // std::cout << sstr.str() << std::endl;
    if( splitCount == nbrSplits - 1 )
    {
        OSG_INFO << std::endl
                 << "ParallelSplitShadowMap: GLSL shader code:" << std::endl
                 << "-------------------------------------------------------------------"
                 << std::endl
                 << sstr.str() << std::endl;
    }

    return sstr.str();
}

//////////////////////////////////////////////////////////////////////////
// clamp variables of any type
template<class Type>
inline Type
Clamp( Type A,
       Type Min,
       Type Max )
{
    if( A < Min )
    {
        return Min;
    }
    if( A > Max )
    {
        return Max;
    }
    return A;
}

#define min( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#define max( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )

//////////////////////////////////////////////////////////////////////////
ParallelSplitShadowMap::ParallelSplitShadowMap( osg::Geode** gr,
                                                int          icountplanes ) :
    _textureUnitOffset( 1 ),
    _debug_color_in_GLSL( false ),
    _user_polgyonOffset_set( false ),
    _resolution( TEXTURE_RESOLUTION ),
    _setMaxFarDistance( 1000.0 ),
    _isSetMaxFarDistance( false ),
    _split_min_near_dist( ZNEAR_MIN_FROM_LIGHT_SOURCE ),
    _move_vcam_behind_rcam_factor( MOVE_VIRTUAL_CAMERA_BEHIND_REAL_CAMERA_FACTOR ),
    _userLight( NULL ),
    _GLSL_shadow_filtered( true ),
    _ambientBiasUniform( NULL ),
    _ambientBias( 0.1F,
                  0.3F )
{
    _displayTexturesGroupingNode = gr;
    _number_of_splits            = static_cast<unsigned int>( icountplanes );

    _polgyonOffset.set( 0.0F, 0.0F );
    setFragmentShaderGenerator( new FragmentShaderGenerator() );
    setSplitCalculationMode( SPLIT_EXP );
}

ParallelSplitShadowMap::ParallelSplitShadowMap( const ParallelSplitShadowMap& copy,
                                                const osg::CopyOp&            copyop ) :
    Inherit( copy,
             copyop ),
    _displayTexturesGroupingNode( 0 ),
    _textureUnitOffset( copy._textureUnitOffset ),
    _number_of_splits( copy._number_of_splits ),
    _debug_color_in_GLSL( copy._debug_color_in_GLSL ),
    _polgyonOffset( copy._polgyonOffset ),
    _user_polgyonOffset_set( copy._user_polgyonOffset_set ),
    _resolution( copy._resolution ),
    _setMaxFarDistance( copy._setMaxFarDistance ),
    _isSetMaxFarDistance( copy._isSetMaxFarDistance ),
    _split_min_near_dist( copy._split_min_near_dist ),
    _move_vcam_behind_rcam_factor( copy._move_vcam_behind_rcam_factor ),
    _userLight( copy._userLight ),
    _FragmentShaderGenerator( copy._FragmentShaderGenerator ),
    _GLSL_shadow_filtered( copy._GLSL_shadow_filtered ),
    _SplitCalcMode( copy._SplitCalcMode ),
    _ambientBiasUniform( NULL ),
    _ambientBias( copy._ambientBias )
{
}

void
ParallelSplitShadowMap::resizeGLObjectBuffers( unsigned int maxSize )
{
    for( PSSMShadowSplitTextureMap::iterator itr = _PSSMShadowSplitTextureMap.begin();
         itr != _PSSMShadowSplitTextureMap.end();
         ++itr )
    {
        itr->second.resizeGLObjectBuffers( maxSize );
    }
}

void
ParallelSplitShadowMap::releaseGLObjects( osg::State* state ) const
{
    for( PSSMShadowSplitTextureMap::const_iterator itr =
             _PSSMShadowSplitTextureMap.begin();
         itr != _PSSMShadowSplitTextureMap.end();
         ++itr )
    {
        itr->second.releaseGLObjects( state );
    }
}

void
ParallelSplitShadowMap::PSSMShadowSplitTexture::resizeGLObjectBuffers(
    unsigned int maxSize
)
{
    osg::resizeGLObjectBuffers( _camera, maxSize );
    osg::resizeGLObjectBuffers( _texture, maxSize );
    osg::resizeGLObjectBuffers( _stateset, maxSize );
    osg::resizeGLObjectBuffers( _debug_camera, maxSize );
    osg::resizeGLObjectBuffers( _debug_texture, maxSize );
    osg::resizeGLObjectBuffers( _debug_stateset, maxSize );
}

void
ParallelSplitShadowMap::PSSMShadowSplitTexture::releaseGLObjects(
    osg::State* state
) const
{
    osg::releaseGLObjects( _camera, state );
    osg::releaseGLObjects( _texture, state );
    osg::releaseGLObjects( _stateset, state );
    osg::releaseGLObjects( _debug_camera, state );
    osg::releaseGLObjects( _debug_texture, state );
    osg::releaseGLObjects( _debug_stateset, state );
}

void
ParallelSplitShadowMap::setAmbientBias( const osg::vec2& ambientBias )
{
    _ambientBias = ambientBias;
    if( _ambientBiasUniform )
    {
        _ambientBiasUniform->set( osg::vec2( _ambientBias.x, _ambientBias.y ) );
    }
}

void
ParallelSplitShadowMap::init()
{
    if( !_shadowedScene )
    {
        return;
    }

    osg::ref_ptr<osg::StateSet> sharedStateSet = new osg::StateSet;
    sharedStateSet->setDataVariance( osg::Object::DataVariance::DYNAMIC );

    unsigned int iCamerasMax = _number_of_splits;
    for( unsigned int iCameras = 0; iCameras < iCamerasMax; iCameras++ )
    {
        PSSMShadowSplitTexture pssmShadowSplitTexture;
        pssmShadowSplitTexture._splitID     = iCameras;
        pssmShadowSplitTexture._textureUnit = iCameras + _textureUnitOffset;

        pssmShadowSplitTexture._resolution  = _resolution;

        OSG_DEBUG << "ParallelSplitShadowMap : Texture ID=" << iCameras
                  << " Resolution=" << pssmShadowSplitTexture._resolution << std::endl;
        // set up the texture to render into
        {
            pssmShadowSplitTexture._texture = new osg::Texture2D;
            pssmShadowSplitTexture._texture->setTextureSize(
                static_cast<int>( pssmShadowSplitTexture._resolution ),
                static_cast<int>( pssmShadowSplitTexture._resolution )
            );
#ifndef SHADOW_TEXTURE_DEBUG
            pssmShadowSplitTexture._texture->setInternalFormat( GL_DEPTH_COMPONENT );
            pssmShadowSplitTexture._texture->setShadowComparison( true );
            pssmShadowSplitTexture._texture->setShadowTextureMode(
                osg::Texture2D::LUMINANCE
            );
#else
            pssmShadowSplitTexture._texture->setInternalFormat( GL_RGBA );
#endif
            pssmShadowSplitTexture._texture->setFilter( osg::Texture2D::MIN_FILTER,
                                                        osg::Texture2D::NEAREST );
            pssmShadowSplitTexture._texture->setFilter( osg::Texture2D::MAG_FILTER,
                                                        osg::Texture2D::NEAREST );
            pssmShadowSplitTexture._texture->setBorderColor(
                osg::dvec4( 1.0, 1.0, 1.0, 1.0 )
            );
            pssmShadowSplitTexture._texture->setWrap( osg::Texture2D::WRAP_S,
                                                      osg::Texture2D::CLAMP_TO_BORDER );
            pssmShadowSplitTexture._texture->setWrap( osg::Texture2D::WRAP_T,
                                                      osg::Texture2D::CLAMP_TO_BORDER );
        }
        // set up the render to texture camera.
        {
            // create the camera
            pssmShadowSplitTexture._camera = new osg::Camera;
            pssmShadowSplitTexture._camera->setReadBuffer( GL_BACK );
            pssmShadowSplitTexture._camera->setDrawBuffer( GL_BACK );
            pssmShadowSplitTexture._camera->setCullCallback(
                new CameraCullCallback( this )
            );

#ifndef SHADOW_TEXTURE_DEBUG
            pssmShadowSplitTexture._camera->setClearMask( GL_DEPTH_BUFFER_BIT );
            pssmShadowSplitTexture._camera->setClearColor(
                osg::vec4( 1.0, 1.0, 1.0, 1.0 )
            );
#else
            pssmShadowSplitTexture._camera->setClearMask( GL_DEPTH_BUFFER_BIT |
                                                          GL_COLOR_BUFFER_BIT );
            switch( iCameras )
            {
                case 0 :
                    pssmShadowSplitTexture._camera->setClearColor(
                        osg::vec4( 1.0, 0.0, 0.0, 1.0 )
                    );
                    break;
                case 1 :
                    pssmShadowSplitTexture._camera->setClearColor(
                        osg::vec4( 0.0, 1.0, 0.0, 1.0 )
                    );
                    break;
                case 2 :
                    pssmShadowSplitTexture._camera->setClearColor(
                        osg::vec4( 0.0, 0.0, 1.0, 1.0 )
                    );
                    break;
                default :
                    pssmShadowSplitTexture._camera->setClearColor(
                        osg::vec4( 1.0, 1.0, 1.0, 1.0 )
                    );
                    break;
            }
#endif
            pssmShadowSplitTexture._camera->setComputeNearFarMode(
                osg::Camera::DO_NOT_COMPUTE_NEAR_FAR
            );
            pssmShadowSplitTexture._camera->setReferenceFrame(
                osg::Camera::ABSOLUTE_RF_INHERIT_VIEWPOINT
            );

            // set viewport
            pssmShadowSplitTexture._camera->setViewport(
                0,
                0,
                static_cast<int>( pssmShadowSplitTexture._resolution ),
                static_cast<int>( pssmShadowSplitTexture._resolution )
            );

            // set the camera to render before the main camera.
            pssmShadowSplitTexture._camera->setRenderOrder( osg::Camera::PRE_RENDER );

            // tell the camera to use OpenGL frame buffer object where supported.
            pssmShadowSplitTexture._camera->setRenderTargetImplementation(
                osg::Camera::FRAME_BUFFER_OBJECT
            );

            // attach the texture and use it as the color buffer.
#ifndef SHADOW_TEXTURE_DEBUG
            pssmShadowSplitTexture._camera->attach(
                osg::Camera::DEPTH_BUFFER,
                pssmShadowSplitTexture._texture.get()
            );
#else
            pssmShadowSplitTexture._camera->attach(
                osg::Camera::COLOR_BUFFER,
                pssmShadowSplitTexture._texture.get()
            );
#endif
            osg::StateSet* stateset =
                pssmShadowSplitTexture._camera->getOrCreateStateSet();

            //////////////////////////////////////////////////////////////////////////
            if( _user_polgyonOffset_set )
            {
                float                            factor         = _polgyonOffset.x;
                float                            units          = _polgyonOffset.y;
                osg::ref_ptr<osg::PolygonOffset> polygon_offset = new osg::PolygonOffset;
                polygon_offset->setFactor( factor );
                polygon_offset->setUnits( units );
                stateset->setAttribute( polygon_offset.get(),
                                        osg::StateAttribute::ON |
                                            osg::StateAttribute::OVERRIDE );
                stateset->setMode( GL_POLYGON_OFFSET_FILL,
                                   osg::StateAttribute::ON |
                                       osg::StateAttribute::OVERRIDE );
            }

            //////////////////////////////////////////////////////////////////////////
            if( !_GLSL_shadow_filtered )
            {
                // if not glsl filtering enabled then we should force front face culling
                // to reduce the number of shadow artifacts.
                osg::ref_ptr<osg::CullFace> cull_face = new osg::CullFace;
                cull_face->setMode( osg::CullFace::Mode::FRONT );
                stateset->setAttribute( cull_face.get(),
                                        osg::StateAttribute::ON |
                                            osg::StateAttribute::OVERRIDE );
                stateset->setMode( GL_CULL_FACE,
                                   osg::StateAttribute::ON |
                                       osg::StateAttribute::OVERRIDE );
            }

            // GL_LIGHTING removed: not in core profile
        }

        //////////////////////////////////////////////////////////////////////////
        // set up stateset and append texture
        {
            pssmShadowSplitTexture._stateset =
                sharedStateSet.get();    // new osg::StateSet;
            pssmShadowSplitTexture._stateset->setTextureAttributeAndModes(
                pssmShadowSplitTexture._textureUnit,
                pssmShadowSplitTexture._texture.get(),
                osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE
            );
        }

        //////////////////////////////////////////////////////////////////////////
        // set up shader (GLSL)
#ifdef SHADOW_TEXTURE_GLSL

        osg::Program* program = new osg::Program;
        pssmShadowSplitTexture._stateset->setAttribute( program );

        //////////////////////////////////////////////////////////////////////////
        // GLSL PROGRAMS
        osg::Shader* fragment_shader =
            new osg::Shader( osg::Shader::FRAGMENT,
                             _FragmentShaderGenerator
                                 ->generateGLSL_FragmentShader_BaseTex(
                                     _debug_color_in_GLSL,
                                     iCameras,
                                     pssmShadowSplitTexture._resolution,
                                     _GLSL_shadow_filtered,
                                     _number_of_splits,
                                     _textureUnitOffset
                                 )
                                 .c_str() );
        program->addShader( fragment_shader );

        //////////////////////////////////////////////////////////////////////////
        // UNIFORMS
        std::stringstream strST;
        strST << "shadowTexture"
              << ( pssmShadowSplitTexture._textureUnit - _textureUnitOffset );
        osg::Uniform* shadowTextureSampler =
            new osg::Uniform( strST.str().c_str(),
                              ( int )( pssmShadowSplitTexture._textureUnit ) );
        pssmShadowSplitTexture._stateset->addUniform( shadowTextureSampler );

        // TODO: NOT YET SUPPORTED in the current version of the shader
        if( !_ambientBiasUniform )
        {
            _ambientBiasUniform = new osg::Uniform( "ambientBias", _ambientBias );
            pssmShadowSplitTexture._stateset->addUniform( _ambientBiasUniform );
        }

        std::stringstream strzShadow;
        strzShadow << "zShadow"
                   << ( pssmShadowSplitTexture._textureUnit - _textureUnitOffset );
        pssmShadowSplitTexture._farDistanceSplit =
            new osg::Uniform( strzShadow.str().c_str(), 1.0F );
        pssmShadowSplitTexture._stateset->addUniform(
            pssmShadowSplitTexture._farDistanceSplit
        );

        osg::Uniform* baseTextureSampler = new osg::Uniform( "baseTexture", 0 );
        pssmShadowSplitTexture._stateset->addUniform( baseTextureSampler );

        osg::Uniform* randomTextureSampler =
            new osg::Uniform( "randomTexture",
                              ( int )( _textureUnitOffset + _number_of_splits ) );
        pssmShadowSplitTexture._stateset->addUniform( randomTextureSampler );

        if( _textureUnitOffset > 0 )
        {
            osg::Uniform* enableBaseTexture =
                new osg::Uniform( "enableBaseTexture", 1.0F );
            pssmShadowSplitTexture._stateset->addUniform( enableBaseTexture );
        }
        else
        {
            osg::Uniform* enableBaseTexture =
                new osg::Uniform( "enableBaseTexture", 0.0F );
            pssmShadowSplitTexture._stateset->addUniform( enableBaseTexture );
        }

        for( unsigned int textLoop( 0 ); textLoop < _textureUnitOffset; textLoop++ )
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
            osg::vec4  color( 1.0F, 1.0F, 1.0F, 0.0F );
            *dataPtr = color;
            // make fake texture
            osg::Texture2D* texture = new osg::Texture2D;
            texture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_BORDER );
            texture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_BORDER );
            texture->setBorderColor( osg::dvec4( 1.0, 1.0, 1.0, 1.0 ) );
            texture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
            texture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
            texture->setImage( image );
            // add fake texture
            pssmShadowSplitTexture._stateset
                ->setTextureAttribute( textLoop, texture, osg::StateAttribute::ON );
        }
#endif

        //////////////////////////////////////////////////////////////////////////
        // DEBUG
        if( _displayTexturesGroupingNode )
        {
            {
                pssmShadowSplitTexture._debug_textureUnit = 1;
                pssmShadowSplitTexture._debug_texture     = new osg::Texture2D;
                pssmShadowSplitTexture._debug_texture->setTextureSize(
                    TEXTURE_RESOLUTION,
                    TEXTURE_RESOLUTION
                );
#ifdef SHOW_SHADOW_TEXTURE_DEBUG
                pssmShadowSplitTexture._debug_texture->setInternalFormat(
                    GL_DEPTH_COMPONENT
                );
                pssmShadowSplitTexture._debug_texture->setShadowTextureMode(
                    osg::Texture2D::LUMINANCE
                );
#else
                pssmShadowSplitTexture._debug_texture->setInternalFormat( GL_RGBA );
#endif
                pssmShadowSplitTexture._debug_texture->setFilter(
                    osg::Texture2D::MIN_FILTER,
                    osg::Texture2D::LINEAR
                );
                pssmShadowSplitTexture._debug_texture->setFilter(
                    osg::Texture2D::MAG_FILTER,
                    osg::Texture2D::LINEAR
                );
                // create the camera
                pssmShadowSplitTexture._debug_camera = new osg::Camera;
                pssmShadowSplitTexture._debug_camera->setCullCallback(
                    new CameraCullCallback( this )
                );
                pssmShadowSplitTexture._debug_camera->setClearMask(
                    GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT
                );
                pssmShadowSplitTexture._debug_camera->setClearColor(
                    osg::vec4( 1.0, 1.0, 1.0, 1.0 )
                );
                pssmShadowSplitTexture._debug_camera->setComputeNearFarMode(
                    osg::Camera::DO_NOT_COMPUTE_NEAR_FAR
                );

                // set viewport
                pssmShadowSplitTexture._debug_camera->setViewport( 0,
                                                                   0,
                                                                   TEXTURE_RESOLUTION,
                                                                   TEXTURE_RESOLUTION );
                // set the camera to render before the main camera.
                pssmShadowSplitTexture._debug_camera->setRenderOrder(
                    osg::Camera::PRE_RENDER
                );
                // tell the camera to use OpenGL frame buffer object where supported.
                pssmShadowSplitTexture._debug_camera->setRenderTargetImplementation(
                    osg::Camera::FRAME_BUFFER_OBJECT
                );
                // attach the texture and use it as the color buffer.
#ifdef SHOW_SHADOW_TEXTURE_DEBUG
                pssmShadowSplitTexture._debug_camera->attach(
                    osg::Camera::DEPTH_BUFFER,
                    pssmShadowSplitTexture._debug_texture.get()
                );
#else
                pssmShadowSplitTexture._debug_camera->attach(
                    osg::Camera::COLOR_BUFFER,
                    pssmShadowSplitTexture._debug_texture.get()
                );
#endif
                // osg::StateSet* stateset =
                // pssmShadowSplitTexture._debug_camera->getOrCreateStateSet();

                pssmShadowSplitTexture._debug_stateset = new osg::StateSet;
                pssmShadowSplitTexture._debug_stateset->setTextureAttributeAndModes(
                    pssmShadowSplitTexture._debug_textureUnit,
                    pssmShadowSplitTexture._debug_texture.get(),
                    osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE
                );
            }

            osg::Geode* geode = _displayTexturesGroupingNode[iCameras];
            geode->getOrCreateStateSet()->setTextureAttributeAndModes(
                0,
                pssmShadowSplitTexture._debug_texture.get(),
                osg::StateAttribute::ON
            );
        }
        //////////////////////////////////////////////////////////////////////////

        _PSSMShadowSplitTextureMap.insert(
            PSSMShadowSplitTextureMap::value_type( iCameras, pssmShadowSplitTexture )
        );
    }

    _dirty = false;
}

void
ParallelSplitShadowMap::update( osg::NodeVisitor& nv )
{
    getShadowedScene()->osg::Group::traverse( nv );
}

void
ParallelSplitShadowMap::cull( osgUtil::CullVisitor& cv )
{
    // record the traversal mask on entry so we can reapply it later.
    unsigned int          traversalMask = cv.getTraversalMask();
    osgUtil::RenderStage* orig_rs       = cv.getRenderStage();

#ifdef SHADOW_TEXTURE_GLSL
    PSSMShadowSplitTextureMap::iterator tm_itr = _PSSMShadowSplitTextureMap.begin();
#else
    // do traversal of shadow receiving scene which does need to be decorated by the
    // shadow map
    for( PSSMShadowSplitTextureMap::iterator tm_itr = _PSSMShadowSplitTextureMap.begin();
         it != _PSSMShadowSplitTextureMap.end();
         it++ )
#endif
    {
        PSSMShadowSplitTexture pssmShadowSplitTexture = tm_itr->second;
        cv.pushStateSet( pssmShadowSplitTexture._stateset.get() );

        //////////////////////////////////////////////////////////////////////////
        // DEBUG
        if( _displayTexturesGroupingNode )
        {
            cv.pushStateSet( pssmShadowSplitTexture._debug_stateset.get() );
        }
        //////////////////////////////////////////////////////////////////////////

        _shadowedScene->osg::Group::traverse( cv );

        cv.popStateSet();
    }

    //////////////////////////////////////////////////////////////////////////
    const osg::Light* selectLight = 0;

    /// light pos and light direction
    osg::vec4         lightpos;
    osg::vec3         lightDirection;

    if( !_userLight )
    {
        // try to find a light in the scene
        osgUtil::PositionalStateContainer::AttrMatrixList& aml =
            orig_rs->getPositionalStateContainer()->getAttrMatrixList();
        for( osgUtil::PositionalStateContainer::AttrMatrixList::iterator itr =
                 aml.begin();
             itr != aml.end();
             ++itr )
        {
            const osg::Light* light =
                dynamic_cast<const osg::Light*>( itr->first.get() );
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
                if( matrix )
                {
                    lightDirection = light->getDirection() * ( *matrix );
                }
                else
                {
                    lightDirection = light->getDirection();
                }

                selectLight = light;
            }
        }

        osg::dmat4 eyeToWorld;
        eyeToWorld     = osg::inverse( *cv.getModelViewMatrix() );

        lightpos       = lightpos * eyeToWorld;
        lightDirection = lightDirection * eyeToWorld;
    }
    else
    {
        // take the user light as light source
        lightpos       = _userLight->getPosition();
        lightDirection = _userLight->getDirection();
        selectLight    = _userLight.get();
    }

    if( selectLight )
    {

        // do traversal of shadow receiving scene which does need to be decorated by the
        // shadow map
        // unsigned int iMaxSplit = _PSSMShadowSplitTextureMap.size();

        for( PSSMShadowSplitTextureMap::iterator it = _PSSMShadowSplitTextureMap.begin();
             it != _PSSMShadowSplitTextureMap.end();
             it++ )
        {
            PSSMShadowSplitTexture pssmShadowSplitTexture = it->second;

            //////////////////////////////////////////////////////////////////////////
            // SETUP pssmShadowSplitTexture for rendering
            //
            lightDirection                         = osg::normalize( lightDirection );
            pssmShadowSplitTexture._lightDirection = lightDirection;
            pssmShadowSplitTexture._cameraView =
                cv.getRenderInfo().getView()->getCamera()->getViewMatrix();
            pssmShadowSplitTexture._cameraProj =
                cv.getRenderInfo().getView()->getCamera()->getProjectionMatrix();

            //////////////////////////////////////////////////////////////////////////
            // CALCULATE

            // Calculate corner points of frustum split
            //
            // To avoid edge problems, scale the frustum so
            // that it's at least a few pixels larger
            //
            osg::dvec3 pCorners[8];
            calculateFrustumCorners( pssmShadowSplitTexture, pCorners );

            // Init Light (Directional Light)
            //
            calculateLightInitialPosition( pssmShadowSplitTexture, pCorners );

            // Calculate near and far for light view
            //
            calculateLightNearFarFormFrustum( pssmShadowSplitTexture, pCorners );

            // Calculate view and projection matrices
            //
            calculateLightViewProjectionFormFrustum( pssmShadowSplitTexture, pCorners );

            //////////////////////////////////////////////////////////////////////////
            // set up shadow rendering camera
            pssmShadowSplitTexture._camera->setReferenceFrame(
                osg::Camera::ABSOLUTE_RF
            );

            //////////////////////////////////////////////////////////////////////////
            // DEBUG
            if( _displayTexturesGroupingNode )
            {
                pssmShadowSplitTexture._debug_camera->setViewMatrix(
                    pssmShadowSplitTexture._camera->getViewMatrix()
                );
                pssmShadowSplitTexture._debug_camera->setProjectionMatrix(
                    pssmShadowSplitTexture._camera->getProjectionMatrix()
                );
                pssmShadowSplitTexture._debug_camera->setReferenceFrame(
                    osg::Camera::ABSOLUTE_RF
                );
            }

            //////////////////////////////////////////////////////////////////////////
            cv.setTraversalMask( traversalMask &
                                 getShadowedScene()->getCastsShadowTraversalMask() );

            // do RTT camera traversal
            pssmShadowSplitTexture._camera->accept( cv );

            //////////////////////////////////////////////////////////////////////////
            // DEBUG
            if( _displayTexturesGroupingNode )
            {
                pssmShadowSplitTexture._debug_camera->accept( cv );
            }
        }
    }    // if light

    // reapply the original traversal mask
    cv.setTraversalMask( traversalMask );
}

void
ParallelSplitShadowMap::cleanSceneGraph()
{
}

//////////////////////////////////////////////////////////////////////////
// Computes corner points of a frustum
//
//
// unit box representing frustum in clip space
const osg::dvec3 const_pointFarTR( 1.0,
                                   1.0,
                                   1.0 );
const osg::dvec3 const_pointFarBR( 1.0,
                                   -1.0,
                                   1.0 );
const osg::dvec3 const_pointFarTL( -1.0,
                                   1.0,
                                   1.0 );
const osg::dvec3 const_pointFarBL( -1.0,
                                   -1.0,
                                   1.0 );
const osg::dvec3 const_pointNearTR( 1.0,
                                    1.0,
                                    -1.0 );
const osg::dvec3 const_pointNearBR( 1.0,
                                    -1.0,
                                    -1.0 );
const osg::dvec3 const_pointNearTL( -1.0,
                                    1.0,
                                    -1.0 );
const osg::dvec3 const_pointNearBL( -1.0,
                                    -1.0,
                                    -1.0 );

//////////////////////////////////////////////////////////////////////////

void
ParallelSplitShadowMap::calculateFrustumCorners(
    PSSMShadowSplitTexture& pssmShadowSplitTexture,
    osg::dvec3*             frustumCorners
)
{
    // get user cameras
    double fovy, aspectRatio, camNear, camFar;
    osg::getPerspective( pssmShadowSplitTexture._cameraProj,
                         fovy,
                         aspectRatio,
                         camNear,
                         camFar );

    // force to max far distance to show shadow, for some scene it can be solve
    // performance problems.
    if( ( _isSetMaxFarDistance ) && ( _setMaxFarDistance < camFar ) )
    {
        camFar = _setMaxFarDistance;
    }

    // build camera matrix with some offsets (the user view camera)
    osg::dmat4 viewMat;
    osg::dvec3 camEye, camCenter, camUp;
    osg::getLookAt( pssmShadowSplitTexture._cameraView, camEye, camCenter, camUp );
    osg::dvec3 viewDir = camCenter - camEye;
    // viewDir = osg::normalize(viewDir); //we can assume that viewDir is still
    // normalized in the viewMatrix
    camEye   = camEye - viewDir * _move_vcam_behind_rcam_factor;
    camFar  += _move_vcam_behind_rcam_factor * osg::length( viewDir );
    viewMat  = osg::lookAt( camEye, camCenter, camUp );

    //////////////////////////////////////////////////////////////////////////
    /// CALCULATE SPLIT
    double maxFar = camFar;
    // double minNear = camNear;
    double camNearFar_Dist = maxFar - camNear;
    if( _SplitCalcMode == SPLIT_LINEAR )
    {
        camFar  = camNear +
                  ( camNearFar_Dist ) *
                  ( ( double )( pssmShadowSplitTexture._splitID + 1 ) ) /
                  ( ( double )( _number_of_splits ) );
        camNear = camNear +
                  ( camNearFar_Dist ) *
                  ( ( double )( pssmShadowSplitTexture._splitID ) ) /
                  ( ( double )( _number_of_splits ) );
    }
    else
    {
        // Exponential split scheme:
        //
        // Ci = (n - f)*(i/numsplits)^(bias+1) + n;
        //
        static double fSplitSchemeBias[2] = { 0.25F, 0.66F };
        fSplitSchemeBias[1]               = Clamp( fSplitSchemeBias[1], 0.0, 3.0 );
        double* pSplitDistances           = new double[_number_of_splits + 1];

        for( int i = 0; i < ( int )_number_of_splits; i++ )
        {
            double fIDM = ( double )( i ) / ( double )( _number_of_splits );
            pSplitDistances[i] =
                camNearFar_Dist * ( pow( fIDM, fSplitSchemeBias[1] + 1 ) ) + camNear;
        }
        // make sure border values are right
        pSplitDistances[0]                 = camNear;
        pSplitDistances[_number_of_splits] = camFar;

        camNear = pSplitDistances[pssmShadowSplitTexture._splitID];
        camFar  = pSplitDistances[pssmShadowSplitTexture._splitID + 1];

        delete[] pSplitDistances;
    }

    pssmShadowSplitTexture._split_far = camFar;

    //////////////////////////////////////////////////////////////////////////
    /// TRANSFORM frustum corners (Optimized for Orthogonal)

    osg::dmat4 projMat;
    projMat = osg::perspective( fovy, aspectRatio, camNear, camFar );
    osg::dmat4 projViewMat( viewMat * projMat );
    osg::dmat4 invProjViewMat;
    invProjViewMat = osg::inverse( projViewMat );

    // transform frustum vertices to world space
    frustumCorners[0] = const_pointFarBR * invProjViewMat;
    frustumCorners[1] = const_pointNearBR * invProjViewMat;
    frustumCorners[2] = const_pointNearTR * invProjViewMat;
    frustumCorners[3] = const_pointFarTR * invProjViewMat;
    frustumCorners[4] = const_pointFarTL * invProjViewMat;
    frustumCorners[5] = const_pointFarBL * invProjViewMat;
    frustumCorners[6] = const_pointNearBL * invProjViewMat;
    frustumCorners[7] = const_pointNearTL * invProjViewMat;

    // std::cout << "camFar : "<<pssmShadowSplitTexture._splitID << " / " << camNear <<
    // "," << camFar << std::endl;
}

//////////////////////////////////////////////////////////////////////////
//
// compute directional light initial position;
void
ParallelSplitShadowMap::calculateLightInitialPosition(
    PSSMShadowSplitTexture& pssmShadowSplitTexture,
    osg::dvec3*             frustumCorners
)
{
    pssmShadowSplitTexture._frustumSplitCenter = frustumCorners[0];
    for( int i = 1; i < 8; i++ )
    {
        pssmShadowSplitTexture._frustumSplitCenter += frustumCorners[i];
    }
    // pssmShadowSplitTexture._frustumSplitCenter /= 8.0;
    pssmShadowSplitTexture._frustumSplitCenter *= 0.125;
}

void
ParallelSplitShadowMap::calculateLightNearFarFormFrustum(
    PSSMShadowSplitTexture& pssmShadowSplitTexture,
    osg::dvec3*             frustumCorners
)
{

    // calculate near, far
    double zFar( -DBL_MAX );

    // calculate zFar (as longest distance)
    for( int i = 0; i < 8; i++ )
    {
        double dist_z_from_light = fabs(
            osg::dot( pssmShadowSplitTexture._lightDirection,
                      frustumCorners[i] - pssmShadowSplitTexture._frustumSplitCenter )
        );
        if( zFar < dist_z_from_light )
        {
            zFar = dist_z_from_light;
        }
    }

    // update camera position and look at center
    pssmShadowSplitTexture._lightCameraSource =
        pssmShadowSplitTexture._frustumSplitCenter -
        pssmShadowSplitTexture._lightDirection *
        ( zFar + _split_min_near_dist );
    pssmShadowSplitTexture._lightCameraTarget =
        pssmShadowSplitTexture._frustumSplitCenter +
        pssmShadowSplitTexture._lightDirection *
        ( zFar );

    // calculate [zNear,zFar]
    zFar = ( -DBL_MAX );
    double zNear( DBL_MAX );
    for( int i = 0; i < 8; i++ )
    {
        double dist_z_from_light = fabs(
            osg::dot( pssmShadowSplitTexture._lightDirection,
                      frustumCorners[i] - pssmShadowSplitTexture._lightCameraSource )
        );
        if( zFar < dist_z_from_light )
        {
            zFar = dist_z_from_light;
        }
        if( zNear > dist_z_from_light )
        {
            zNear = dist_z_from_light;
        }
    }
    // update near - far plane
    pssmShadowSplitTexture._lightNear = max( zNear - _split_min_near_dist - 0.01, 0.01 );
    pssmShadowSplitTexture._lightFar  = zFar;
}

void
ParallelSplitShadowMap::calculateLightViewProjectionFormFrustum(
    PSSMShadowSplitTexture& pssmShadowSplitTexture,
    osg::dvec3*             frustumCorners
)
{

    // calculate the camera's coordinate system
    osg::dvec3 camEye, camCenter, camUp;
    osg::getLookAt( pssmShadowSplitTexture._cameraView, camEye, camCenter, camUp );
    osg::dvec3 viewDir( camCenter - camEye );
    osg::dvec3 camRight( viewDir ^ camUp );

    // we force to have normalized vectors (camera's view)
    camUp    = osg::normalize( camUp );
    viewDir  = osg::normalize( viewDir );
    camRight = osg::normalize( camRight );

    // use quaternion -> numerical more robust
    osg::dquat qRot( viewDir, pssmShadowSplitTexture._lightDirection );
    osg::dvec3 top   = qRot * camUp;
    osg::dvec3 right = qRot * camRight;

    // calculate the camera's frustum right,right,bottom,top parameters
    double     maxRight( -DBL_MAX ), maxTop( -DBL_MAX );
    double     minRight( DBL_MAX ), minTop( DBL_MAX );

    for( int i( 0 ); i < 8; i++ )
    {

        osg::dvec3 diffCorner( frustumCorners[i] -
                               pssmShadowSplitTexture._frustumSplitCenter );
        double     lright( osg::dot( diffCorner, right ) );
        double     lTop( osg::dot( diffCorner, top ) );

        if( lright > maxRight )
        {
            maxRight = lright;
        }
        if( lTop > maxTop )
        {
            maxTop = lTop;
        }

        if( lright < minRight )
        {
            minRight = lright;
        }
        if( lTop < minTop )
        {
            minTop = lTop;
        }
    }

    // make the camera view matrix
    pssmShadowSplitTexture._camera->setViewMatrixAsLookAt(
        pssmShadowSplitTexture._lightCameraSource,
        pssmShadowSplitTexture._lightCameraTarget,
        top
    );

    // use ortho projection for light (directional light only supported)
    pssmShadowSplitTexture._camera->setProjectionMatrixAsOrtho(
        minRight,
        maxRight,
        minTop,
        maxTop,
        pssmShadowSplitTexture._lightNear,
        pssmShadowSplitTexture._lightFar
    );

#ifdef SHADOW_TEXTURE_GLSL
    // get user cameras
    osg::dvec3 vProjCamFraValue =
        ( camEye + viewDir * pssmShadowSplitTexture._split_far ) *
        ( pssmShadowSplitTexture._cameraView * pssmShadowSplitTexture._cameraProj );
    pssmShadowSplitTexture._farDistanceSplit->set( ( float )vProjCamFraValue.z );
#endif
}
