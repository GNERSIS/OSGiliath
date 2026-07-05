/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#include "ShadowPass.hpp"
#include "SponzaFrameContext.hpp"
#include "SponzaLighting.hpp"
#include "SponzaOptions.hpp"
#include "SponzaPassOrder.hpp"
#include "SponzaTargets.hpp"

#include <algorithm>
#include <osg/GL>
#include <osg/maths/box.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/transform.hpp>
#include <osg/nodes/Camera.hpp>
#include <osg/nodes/Node.hpp>
#include <osg/state/PolygonOffset.hpp>
#include <osg/state/Program.hpp>
#include <osg/state/Shader.hpp>
#include <osg/state/StateSet.hpp>
#include <osg/traversal/ComputeBoundsVisitor.hpp>

namespace
{

    constexpr double shadowFrustumMargin         = 2.0;
    constexpr double lightDistanceScale          = 1.5;
    constexpr float  polygonOffsetFactor         = 2.5F;
    constexpr float  polygonOffsetUnits          = 8.0F;

    constexpr char   shadowDepthVertexShader[]   = R"glsl(
#version 460 core

layout(location = 0) in vec4 osg_Vertex;
uniform mat4 osg_ModelViewProjectionMatrix;

void main()
{
    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;
}
)glsl";

    constexpr char   shadowDepthFragmentShader[] = R"glsl(
#version 460 core

void main()
{
}
)glsl";

    osg::dvec3
    toDVec3( const osg::vec3& value )
    {
        return osg::dvec3( static_cast<double>( value.x ),
                           static_cast<double>( value.y ),
                           static_cast<double>( value.z ) );
    }

    osg::dvec3
    computeOrthogonalVector( const osg::dvec3& direction )
    {
        const double length = osg::length( direction );
        osg::dvec3   orthogonalVector =
            osg::cross( direction, osg::dvec3( 0.0, 1.0, 0.0 ) );
        const double orthoLength = osg::length( orthogonalVector );
        if( orthoLength > 1.0E-12 )
        {
            orthogonalVector = osg::normalize( orthogonalVector );
        }
        if( orthoLength < length * 0.5 )
        {
            orthogonalVector = osg::cross( direction, osg::dvec3( 0.0, 0.0, 1.0 ) );
            orthogonalVector = osg::normalize( orthogonalVector );
        }
        return orthogonalVector;
    }

    osg::ref_ptr<osg::Program>
    createShadowDepthProgram()
    {
        osg::ref_ptr<osg::Program> program = new osg::Program;
        program->addBindAttribLocation( "osg_Vertex", 0U );
        program->addShader( new osg::Shader( osg::Shader::VERTEX,
                                             shadowDepthVertexShader ) );
        program->addShader( new osg::Shader( osg::Shader::FRAGMENT,
                                             shadowDepthFragmentShader ) );
        return program;
    }

    osg::box
    computeWorldBounds( osg::Node* model )
    {
        osg::ComputeBoundsVisitor boundsVisitor;
        model->accept( boundsVisitor );
        osg::box bounds = boundsVisitor.getBoundingBox();
        if( !bounds.valid() )
        {
            bounds.set( -1.0F, -1.0F, -1.0F, 1.0F, 1.0F, 1.0F );
        }
        return bounds;
    }

    struct LightFit
    {
            osg::dmat4 lightView;
            osg::dmat4 lightProjection;
            osg::mat4  shadowMatrix;
            float      lightSpaceExtent = 1.0F;
    };

    LightFit
    fitLightSpace( osg::Node*                        model,
                   const sponza::SponzaOptions&      options,
                   const sponza::SponzaFrameContext& frame )
    {
        const osg::box   worldBounds = computeWorldBounds( model );
        const osg::dvec3 center      = toDVec3( worldBounds.center() );
        const double     lightDistance =
            std::max( static_cast<double>( worldBounds.radius() ) * lightDistanceScale,
                      1.0 );
        const osg::dvec3 dirWorld = sponza::computeSunDirectionWorld( options );
        const osg::dvec3 up       = computeOrthogonalVector( dirWorld );
        const osg::dmat4 lightView =
            osg::lookAt( center + dirWorld * lightDistance, center, up );

        osg::dbox lightBounds;
        for( unsigned int cornerIndex = 0U; cornerIndex < 8U; ++cornerIndex )
        {
            lightBounds.expandBy( lightView *
                                  toDVec3( worldBounds.corner( cornerIndex ) ) );
        }

        const double     lightMinX = lightBounds.xMin();
        const double     lightMaxX = lightBounds.xMax();
        const double     lightMinY = lightBounds.yMin();
        const double     lightMaxY = lightBounds.yMax();
        const double     lightMinZ = lightBounds.zMin();
        const double     lightMaxZ = lightBounds.zMax();

        const osg::dmat4 lightProjection =
            osg::orthographic( lightMinX,
                               lightMaxX,
                               lightMinY,
                               lightMaxY,
                               -lightMaxZ - shadowFrustumMargin,
                               -lightMinZ + shadowFrustumMargin );
        const osg::dmat4 shadowMatrix = osg::translate( osg::dvec3( 0.5, 0.5, 0.5 ) ) *
                                        osg::scale( osg::dvec3( 0.5, 0.5, 0.5 ) ) *
                                        lightProjection *
                                        lightView *
                                        osg::inverse( frame.view );

        const double extent = std::max( lightMaxX - lightMinX, lightMaxY - lightMinY );

        return LightFit{
            lightView,
            lightProjection,
            osg::mat4( shadowMatrix ),
            static_cast<float>( std::max( extent, 1.0 ) )
        };
    }

    osg::ref_ptr<osg::Camera>
    createShadowCamera( osg::Node*        model,
                        osg::Texture2D*   shadowTexture,
                        int               textureSize,
                        const osg::dmat4& lightView,
                        const osg::dmat4& lightProjection )
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
        camera->attach( osg::Camera::DEPTH_BUFFER, shadowTexture );
        camera->setViewport( 0, 0, textureSize, textureSize );
        camera->setRenderOrder( osg::Camera::PRE_RENDER, sponza::shadowPassOrder );
        camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
        camera->setComputeNearFarMode( osg::Camera::DO_NOT_COMPUTE_NEAR_FAR );
        camera->setClearMask( GL_DEPTH_BUFFER_BIT );
        camera->setProjectionMatrix( lightProjection );
        camera->setViewMatrix( lightView );
        camera->addChild( model );

        osg::StateSet*             stateSet = camera->getOrCreateStateSet();
        osg::ref_ptr<osg::Program> program  = createShadowDepthProgram();
        stateSet->setAttributeAndModes( program.get(),
                                        osg::StateAttribute::ON |
                                            osg::StateAttribute::OVERRIDE );
        stateSet->setAttributeAndModes(
            new osg::PolygonOffset( polygonOffsetFactor, polygonOffsetUnits ),
            osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE
        );
        stateSet->setMode( GL_POLYGON_OFFSET_FILL,
                           osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
        stateSet->setMode( GL_DEPTH_CLAMP,
                           osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

        return camera;
    }

}

namespace sponza
{

    ShadowPassResult
    createShadowPass( osg::Node*                model,
                      const SponzaOptions&      options,
                      const SponzaFrameContext& frame )
    {
        if( !options.shadowEnabled )
        {
            return ShadowPassResult{
                nullptr,
                createShadowDepthTexture( 1 ),
                osg::mat4(),
                1.0F,
                false
            };
        }

        const int                    textureSize = std::max( options.shadowMapSize, 1 );
        LightFit                     lightFit = fitLightSpace( model, options, frame );
        osg::ref_ptr<osg::Texture2D> shadowTexture =
            createShadowDepthTexture( textureSize );

        return ShadowPassResult{
            createShadowCamera( model,
                                shadowTexture.get(),
                                textureSize,
                                lightFit.lightView,
                                lightFit.lightProjection ),
            shadowTexture,
            lightFit.shadowMatrix,
            lightFit.lightSpaceExtent,
            true
        };
    }

}
