/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Weather precipitation (rain, snow). Renders falling particles
 * over a bounded volume relative to the camera.
 */
#include <osgParticle/PrecipitationEffect.hpp>

#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osg/core/Timer.hpp>
#include <osg/images/ImageUtils.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/maths/Math.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgUtil/culling/CullVisitor.hpp>
#include <osgUtil/utils/GLObjectsVisitor.hpp>
#include <stdlib.h>

using namespace osgParticle;

#define USE_LOCAL_SHADERS

static float
random( float min,
        float max )
{
    return min + ( max - min ) * ( float )rand() / ( float )RAND_MAX;
}

PrecipitationEffect::PrecipitationEffect() :
    _fogMode( FOG_EXP ),
    _fogDensity( 0.0F ),
    _fogColor( 0.0F,
               0.0F,
               0.0F,
               0.0F ),
    _fogStart( 0.0F ),
    _fogEnd( 1.0F ),
    _previousFrameTime( FLT_MAX )
{
    setNumChildrenRequiringUpdateTraversal( 1 );

    setUpGeometries( 1'024 );

    rain( 0.5 );
}

PrecipitationEffect::PrecipitationEffect( const PrecipitationEffect& copy,
                                          const osg::CopyOp&         copyop ) :
    osg::Node( copy,
               copyop ),
    _previousFrameTime( FLT_MAX )
{
    setNumChildrenRequiringUpdateTraversal( getNumChildrenRequiringUpdateTraversal() +
                                            1 );

    _wind                   = copy._wind;
    _particleSpeed          = copy._particleSpeed;
    _particleSize           = copy._particleSize;
    _particleColor          = copy._particleColor;
    _maximumParticleDensity = copy._maximumParticleDensity;
    _cellSize               = copy._cellSize;
    _nearTransition         = copy._nearTransition;
    _farTransition          = copy._farTransition;

    _fogMode                = copy._fogMode;
    _fogDensity             = copy._fogDensity;
    _fogColor               = copy._fogColor;
    _fogStart               = copy._fogStart;
    _fogEnd                 = copy._fogEnd;

    _useFarLineSegments     = copy._useFarLineSegments;

    _dirty                  = true;

    update();
}

void
PrecipitationEffect::rain( float intensity )
{
    _wind.set( 0.0F, 0.0F, 0.0F );
    _particleSpeed          = -2.0F + -5.0F * intensity;
    _particleSize           = 0.01F + 0.02F * intensity;
    _particleColor          = osg::vec4( 0.6F, 0.6F, 0.6F, 1.0F ) -
                              osg::vec4( 0.1F, 0.1F, 0.1F, 1.0F ) *
                              intensity;
    _maximumParticleDensity = intensity * 8.5F;
    _cellSize.set( 5.0F / ( 0.25F + intensity ), 5.0F / ( 0.25F + intensity ), 5.0F );
    _nearTransition = 25.F;
    _farTransition  = 100.0F - 60.0F * sqrtf( intensity );

    _fogMode        = FOG_EXP;
    _fogDensity     = 0.005F * intensity;
    _fogColor.set( 0.5, 0.5, 0.5, 1.0 );

    _useFarLineSegments = false;

    _dirty              = true;

    update();
}

void
PrecipitationEffect::snow( float intensity )
{
    _wind.set( 0.0F, 0.0F, 0.0F );
    _particleSpeed          = -0.75F - 0.25F * intensity;
    _particleSize           = 0.02F + 0.03F * intensity;
    _particleColor          = osg::vec4( 0.85F, 0.85F, 0.85F, 1.0F ) -
                              osg::vec4( 0.1F, 0.1F, 0.1F, 1.0F ) *
                              intensity;
    _maximumParticleDensity = intensity * 8.2F;
    _cellSize.set( 5.0F / ( 0.25F + intensity ), 5.0F / ( 0.25F + intensity ), 5.0F );
    _nearTransition = 25.F;
    _farTransition  = 100.0F - 60.0F * sqrtf( intensity );

    _fogMode        = FOG_EXP;
    _fogDensity     = 0.01F * intensity;
    _fogColor.set( 0.6F, 0.6F, 0.6F, 1.0F );

    _useFarLineSegments = false;

    _dirty              = true;

    update();
}

void
PrecipitationEffect::compileGLObjects( osg::RenderInfo& renderInfo ) const
{
    if( _quadGeometry.valid() )
    {
        _quadGeometry->compileGLObjects( renderInfo );
    }
    if( _lineGeometry.valid() )
    {
        _lineGeometry->compileGLObjects( renderInfo );
    }
    if( _pointGeometry.valid() )
    {
        _pointGeometry->compileGLObjects( renderInfo );
    }

    if( _quadStateSet.valid() )
    {
        _quadStateSet->compileGLObjects( *renderInfo.getState() );
    }
    if( _lineStateSet.valid() )
    {
        _lineStateSet->compileGLObjects( *renderInfo.getState() );
    }
    if( _pointStateSet.valid() )
    {
        _pointStateSet->compileGLObjects( *renderInfo.getState() );
    }

    for( ViewDrawableMap::const_iterator itr = _viewDrawableMap.begin();
         itr != _viewDrawableMap.end();
         ++itr )
    {
        const PrecipitationDrawableSet& pds = itr->second;
        if( pds._quadPrecipitationDrawable.valid() )
        {
            pds._quadPrecipitationDrawable->compileGLObjects( renderInfo );
        }
        if( pds._linePrecipitationDrawable.valid() )
        {
            pds._linePrecipitationDrawable->compileGLObjects( renderInfo );
        }
        if( pds._pointPrecipitationDrawable.valid() )
        {
            pds._pointPrecipitationDrawable->compileGLObjects( renderInfo );
        }
    }
}

void
PrecipitationEffect::resizeGLObjectBuffers( unsigned int maxSize )
{
    if( _quadGeometry.valid() )
    {
        _quadGeometry->resizeGLObjectBuffers( maxSize );
    }
    if( _lineGeometry.valid() )
    {
        _lineGeometry->resizeGLObjectBuffers( maxSize );
    }
    if( _pointGeometry.valid() )
    {
        _pointGeometry->resizeGLObjectBuffers( maxSize );
    }

    if( _quadStateSet.valid() )
    {
        _quadStateSet->resizeGLObjectBuffers( maxSize );
    }
    if( _lineStateSet.valid() )
    {
        _lineStateSet->resizeGLObjectBuffers( maxSize );
    }
    if( _pointStateSet.valid() )
    {
        _pointStateSet->resizeGLObjectBuffers( maxSize );
    }

    for( ViewDrawableMap::const_iterator itr = _viewDrawableMap.begin();
         itr != _viewDrawableMap.end();
         ++itr )
    {
        const PrecipitationDrawableSet& pds = itr->second;
        if( pds._quadPrecipitationDrawable.valid() )
        {
            pds._quadPrecipitationDrawable->resizeGLObjectBuffers( maxSize );
        }
        if( pds._linePrecipitationDrawable.valid() )
        {
            pds._linePrecipitationDrawable->resizeGLObjectBuffers( maxSize );
        }
        if( pds._pointPrecipitationDrawable.valid() )
        {
            pds._pointPrecipitationDrawable->resizeGLObjectBuffers( maxSize );
        }
    }
}

void
PrecipitationEffect::releaseGLObjects( osg::State* state ) const
{
    if( _quadGeometry.valid() )
    {
        _quadGeometry->releaseGLObjects( state );
    }
    if( _lineGeometry.valid() )
    {
        _lineGeometry->releaseGLObjects( state );
    }
    if( _pointGeometry.valid() )
    {
        _pointGeometry->releaseGLObjects( state );
    }

    if( _quadStateSet.valid() )
    {
        _quadStateSet->releaseGLObjects( state );
    }
    if( _lineStateSet.valid() )
    {
        _lineStateSet->releaseGLObjects( state );
    }
    if( _pointStateSet.valid() )
    {
        _pointStateSet->releaseGLObjects( state );
    }

    for( ViewDrawableMap::const_iterator itr = _viewDrawableMap.begin();
         itr != _viewDrawableMap.end();
         ++itr )
    {
        const PrecipitationDrawableSet& pds = itr->second;
        if( pds._quadPrecipitationDrawable.valid() )
        {
            pds._quadPrecipitationDrawable->releaseGLObjects( state );
        }
        if( pds._linePrecipitationDrawable.valid() )
        {
            pds._linePrecipitationDrawable->releaseGLObjects( state );
        }
        if( pds._pointPrecipitationDrawable.valid() )
        {
            pds._pointPrecipitationDrawable->releaseGLObjects( state );
        }
    }
}

void
PrecipitationEffect::traverse( osg::NodeVisitor& nv )
{
    if( nv.getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR )
    {
        if( _dirty )
        {
            update();
        }

        if( nv.getFrameStamp() )
        {
            double currentTime = nv.getFrameStamp()->getSimulationTime();
            if( _previousFrameTime == FLT_MAX )
            {
                _previousFrameTime = currentTime;
            }

            double delta        = currentTime - _previousFrameTime;
            _origin            += _wind * static_cast<float>( delta );
            _previousFrameTime  = currentTime;
        }

        return;
    }

    if( nv.getVisitorType() == osg::NodeVisitor::NODE_VISITOR )
    {
        if( _dirty )
        {
            update();
        }

        osgUtil::GLObjectsVisitor* globjVisitor =
            dynamic_cast<osgUtil::GLObjectsVisitor*>( &nv );
        if( globjVisitor )
        {
            if( globjVisitor->getMode() &
                osgUtil::GLObjectsVisitor::COMPILE_STATE_ATTRIBUTES )
            {
                compileGLObjects( globjVisitor->getRenderInfo() );
            }
        }

        return;
    }

    if( nv.getVisitorType() != osg::NodeVisitor::CULL_VISITOR )
    {
        return;
    }

    osgUtil::CullVisitor* cv = nv.asCullVisitor();
    if( !cv )
    {
        return;
    }

    ViewIdentifier viewIndentifier( cv, nv.getNodePath() );

    {
        PrecipitationDrawableSet* precipitationDrawableSet = 0;

        {
            std::lock_guard<std::mutex> lock( _mutex );
            precipitationDrawableSet = &( _viewDrawableMap[viewIndentifier] );

            if( !precipitationDrawableSet->_quadPrecipitationDrawable )
            {
                precipitationDrawableSet->_quadPrecipitationDrawable =
                    new PrecipitationDrawable;
                precipitationDrawableSet->_quadPrecipitationDrawable
                    ->setRequiresPreviousMatrix( true );
                precipitationDrawableSet->_quadPrecipitationDrawable->setGeometry(
                    _quadGeometry.get()
                );
                precipitationDrawableSet->_quadPrecipitationDrawable->setStateSet(
                    _quadStateSet.get()
                );
                precipitationDrawableSet->_quadPrecipitationDrawable->setDrawType(
                    GL_TRIANGLES
                );

                precipitationDrawableSet->_linePrecipitationDrawable =
                    new PrecipitationDrawable;
                precipitationDrawableSet->_linePrecipitationDrawable
                    ->setRequiresPreviousMatrix( true );
                precipitationDrawableSet->_linePrecipitationDrawable->setGeometry(
                    _lineGeometry.get()
                );
                precipitationDrawableSet->_linePrecipitationDrawable->setStateSet(
                    _lineStateSet.get()
                );
                precipitationDrawableSet->_linePrecipitationDrawable->setDrawType(
                    GL_LINES
                );

                precipitationDrawableSet->_pointPrecipitationDrawable =
                    new PrecipitationDrawable;
                precipitationDrawableSet->_pointPrecipitationDrawable
                    ->setRequiresPreviousMatrix( false );
                precipitationDrawableSet->_pointPrecipitationDrawable->setGeometry(
                    _pointGeometry.get()
                );
                precipitationDrawableSet->_pointPrecipitationDrawable->setStateSet(
                    _pointStateSet.get()
                );
                precipitationDrawableSet->_pointPrecipitationDrawable->setDrawType(
                    GL_POINTS
                );
            }
        }

        cull( *precipitationDrawableSet, cv );

        cv->pushStateSet( _stateset.get() );
        float depth = 0.0F;

        if( !precipitationDrawableSet->_quadPrecipitationDrawable
                 ->getCurrentCellMatrixMap()
                 .empty() )
        {
            cv->pushStateSet(
                precipitationDrawableSet->_quadPrecipitationDrawable->getStateSet()
            );
            cv->addDrawableAndDepth(
                precipitationDrawableSet->_quadPrecipitationDrawable.get(),
                cv->getModelViewMatrix(),
                depth
            );
            cv->popStateSet();
        }

        if( !precipitationDrawableSet->_linePrecipitationDrawable
                 ->getCurrentCellMatrixMap()
                 .empty() )
        {
            cv->pushStateSet(
                precipitationDrawableSet->_linePrecipitationDrawable->getStateSet()
            );
            cv->addDrawableAndDepth(
                precipitationDrawableSet->_linePrecipitationDrawable.get(),
                cv->getModelViewMatrix(),
                depth
            );
            cv->popStateSet();
        }

        if( !precipitationDrawableSet->_pointPrecipitationDrawable
                 ->getCurrentCellMatrixMap()
                 .empty() )
        {
            cv->pushStateSet(
                precipitationDrawableSet->_pointPrecipitationDrawable->getStateSet()
            );
            cv->addDrawableAndDepth(
                precipitationDrawableSet->_pointPrecipitationDrawable.get(),
                cv->getModelViewMatrix(),
                depth
            );
            cv->popStateSet();
        }

        cv->popStateSet();
    }
}

void
PrecipitationEffect::update()
{
    _dirty = false;

    OSG_INFO << "PrecipitationEffect::update()" << std::endl;

    float length_u = _cellSize.x;
    float length_v = _cellSize.y;
    float length_w = _cellSize.z;

    // time taken to get from start to the end of cycle
    _period = fabsf( _cellSize.z / _particleSpeed );

    _du.set( length_u, 0.0F, 0.0F );
    _dv.set( 0.0F, length_v, 0.0F );
    _dw.set( 0.0F, 0.0F, length_w );

    _inverse_du.set( 1.0F / length_u, 0.0F, 0.0F );
    _inverse_dv.set( 0.0F, 1.0F / length_v, 0.0F );
    _inverse_dw.set( 0.0F, 0.0F, 1.0F / length_w );

    OSG_INFO << "Cell size X=" << length_u << std::endl;
    OSG_INFO << "Cell size Y=" << length_v << std::endl;
    OSG_INFO << "Cell size Z=" << length_w << std::endl;

    {
        std::lock_guard<std::mutex> lock( _mutex );
        _viewDrawableMap.clear();
    }

    // set up state/
    {
        if( !_stateset )
        {
            _stateset = new osg::StateSet;
            _stateset->addUniform( new osg::Uniform( "baseTexture", 0 ) );

            _stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
            _stateset->setMode( GL_BLEND, osg::StateAttribute::ON );

            osg::Texture2D* texture = new osg::Texture2D(
                createSpotLightImage( osg::vec4( 1.0F, 1.0F, 1.0F, 1.0F ),
                                      osg::vec4( 1.0F, 1.0F, 1.0F, 0.0F ),
                                      32,
                                      1.0 )
            );
            _stateset->setTextureAttribute( 0, texture );
        }

        if( !_inversePeriodUniform )
        {
            _inversePeriodUniform = new osg::Uniform( "inversePeriod", 1.0F / _period );
            _stateset->addUniform( _inversePeriodUniform.get() );
        }
        else
        {
            _inversePeriodUniform->set( 1.0F / _period );
        }

        if( !_particleColorUniform )
        {
            _particleColorUniform = new osg::Uniform( "particleColour", _particleColor );
            _stateset->addUniform( _particleColorUniform.get() );
        }
        else
        {
            _particleColorUniform->set( _particleColor );
        }

        if( !_particleSizeUniform )
        {
            _particleSizeUniform = new osg::Uniform( "particleSize", _particleSize );
            _stateset->addUniform( _particleSizeUniform.get() );
        }
        else
        {
            _particleSizeUniform->set( _particleSize );
        }
    }
}

void
PrecipitationEffect::createGeometry( unsigned int   numParticles,
                                     osg::Geometry* quad_geometry,
                                     osg::Geometry* line_geometry,
                                     osg::Geometry* point_geometry )
{
    // particle corner offsets
    osg::vec2       offset00( 0.0F, 0.0F );
    osg::vec2       offset10( 1.0F, 0.0F );
    osg::vec2       offset01( 0.0F, 1.0F );
    osg::vec2       offset11( 1.0F, 1.0F );

    osg::vec2       offset0( 0.5F, 0.0F );
    osg::vec2       offset1( 0.5F, 1.0F );

    osg::vec2       offset( 0.5F, 0.5F );

    // configure quad_geometry;
    // 6 vertices per particle: 2 triangles replacing each quad
    osg::Vec3Array* quad_vertices = 0;
    osg::Vec2Array* quad_offsets  = 0;
    if( quad_geometry )
    {
        quad_geometry->setName( "quad" );

        quad_vertices = new osg::Vec3Array( numParticles * 6 );
        quad_offsets  = new osg::Vec2Array( numParticles * 6 );

        quad_geometry->setVertexArray( quad_vertices );
        quad_geometry->setTexCoordArray( 0, quad_offsets );
    }

    // configure line_geometry;
    osg::Vec3Array* line_vertices = 0;
    osg::Vec2Array* line_offsets  = 0;
    if( line_geometry )
    {
        line_geometry->setName( "line" );

        line_vertices = new osg::Vec3Array( numParticles * 2 );
        line_offsets  = new osg::Vec2Array( numParticles * 2 );

        line_geometry->setVertexArray( line_vertices );
        line_geometry->setTexCoordArray( 0, line_offsets );
    }

    // configure point_geometry;
    osg::Vec3Array* point_vertices = 0;
    osg::Vec2Array* point_offsets  = 0;
    if( point_geometry )
    {
        point_geometry->setName( "point" );

        point_vertices = new osg::Vec3Array( numParticles );
        point_offsets  = new osg::Vec2Array( numParticles );

        point_geometry->setVertexArray( point_vertices );
        point_geometry->setTexCoordArray( 0, point_offsets );
    }

    // set up vertex attribute data.
    for( unsigned int i = 0; i < numParticles; ++i )
    {
        osg::vec3 pos( random( 0.0F, 1.0F ),
                       random( 0.0F, 1.0F ),
                       random( 0.0F, 1.0F ) );

        // quad particles: 2 triangles per quad (6 vertices)
        // Original quad: v0(00), v1(01), v2(11), v3(10)
        // Triangle 1: v0, v1, v3  Triangle 2: v1, v2, v3
        if( quad_vertices )
        {
            ( *quad_vertices )[i * 6]     = pos;
            ( *quad_vertices )[i * 6 + 1] = pos;
            ( *quad_vertices )[i * 6 + 2] = pos;
            ( *quad_vertices )[i * 6 + 3] = pos;
            ( *quad_vertices )[i * 6 + 4] = pos;
            ( *quad_vertices )[i * 6 + 5] = pos;
            ( *quad_offsets )[i * 6]      = offset00;
            ( *quad_offsets )[i * 6 + 1]  = offset01;
            ( *quad_offsets )[i * 6 + 2]  = offset10;
            ( *quad_offsets )[i * 6 + 3]  = offset01;
            ( *quad_offsets )[i * 6 + 4]  = offset11;
            ( *quad_offsets )[i * 6 + 5]  = offset10;
        }

        // line particles
        if( line_vertices )
        {
            ( *line_vertices )[i * 2]     = pos;
            ( *line_vertices )[i * 2 + 1] = pos;
            ( *line_offsets )[i * 2]      = offset0;
            ( *line_offsets )[i * 2 + 1]  = offset1;
        }

        // point particles
        if( point_vertices )
        {
            ( *point_vertices )[i] = pos;
            ( *point_offsets )[i]  = offset;
        }
    }
}

void
PrecipitationEffect::setUpGeometries( unsigned int numParticles )
{
    unsigned int quadRenderBin  = 13;
    unsigned int lineRenderBin  = 12;
    unsigned int pointRenderBin = 11;

    OSG_INFO << "PrecipitationEffect::setUpGeometries(" << numParticles << ")"
             << std::endl;

    bool needGeometryRebuild = false;

    if( !_quadGeometry ||
        _quadGeometry->getVertexArray()->getNumElements() !=
        6 *
        numParticles )
    {
        _quadGeometry = new osg::Geometry;
        _quadGeometry->setUseVertexBufferObjects( true );
        needGeometryRebuild = true;
    }

    if( !_lineGeometry ||
        _lineGeometry->getVertexArray()->getNumElements() !=
        2 *
        numParticles )
    {
        _lineGeometry = new osg::Geometry;
        _lineGeometry->setUseVertexBufferObjects( true );
        needGeometryRebuild = true;
    }

    if( !_pointGeometry ||
        _pointGeometry->getVertexArray()->getNumElements() != numParticles )
    {
        _pointGeometry = new osg::Geometry;
        _pointGeometry->setUseVertexBufferObjects( true );
        needGeometryRebuild = true;
    }

    if( needGeometryRebuild )
    {
        createGeometry( numParticles,
                        _quadGeometry.get(),
                        _lineGeometry.get(),
                        _pointGeometry.get() );
    }

    if( !_quadStateSet )
    {
        _quadStateSet         = new osg::StateSet;

        osg::Program* program = new osg::Program;
        _quadStateSet->setAttribute( program );
        _quadStateSet->setRenderBinDetails( static_cast<int>( quadRenderBin ),
                                            "DepthSortedBin" );

#ifdef USE_LOCAL_SHADERS
        char vertexShaderSource[] =
            "#version 460 core\n"
            "layout(location = 0) in vec4 osg_Vertex;\n"
            "layout(location = 8) in vec4 osg_MultiTexCoord0;\n"
            "uniform float cellStartTime;\n"
            "uniform mat4 osg_ModelViewMatrix;\n"
            "uniform mat4 osg_ProjectionMatrix;\n"
            "uniform float inversePeriod;\n"
            "uniform vec4 particleColour;\n"
            "uniform float particleSize;\n"
            "\n"
            "uniform float osg_SimulationTime;\n"
            "uniform float osg_DeltaSimulationTime;\n"
            "uniform mat4 previousModelViewMatrix;\n"
            "\n"
            "out vec4 colour;\n"
            "out vec2 texCoord;\n"
            "\n"
            "void main(void)\n"
            "{\n"
            "    float offset = osg_Vertex.z;\n"
            "    float startTime = cellStartTime;\n"
            "    texCoord = osg_MultiTexCoord0.xy;\n"
            "\n"
            "    vec4 v_previous = osg_Vertex;\n"
            "    v_previous.z = fract( (osg_SimulationTime - startTime)*inversePeriod - "
            "offset);\n"
            "    \n"
            "    vec4 v_current =  v_previous;\n"
            "    v_current.z += (osg_DeltaSimulationTime*inversePeriod);\n"
            "    \n"
            "\n"
            "    colour = particleColour;\n"
            "    \n"
            "    vec4 v1 = osg_ModelViewMatrix * v_current;\n"
            "    vec4 v2 = previousModelViewMatrix * v_previous;\n"
            "    \n"
            "    vec3 dv = v2.xyz - v1.xyz;\n"
            "    \n"
            "    vec2 dv_normalized = normalize(dv.xy);\n"
            "    dv.xy += dv_normalized * particleSize;\n"
            "    vec2 dp = vec2( -dv_normalized.y, dv_normalized.x ) * particleSize;\n"
            "    \n"
            "    float area = length(dv.xy);\n"
            "    colour.a = 0.05+(particleSize)/area;\n"
            "    \n"
            "\n"
            "    v1.xyz += dv*texCoord.y;\n"
            "    v1.xy += dp*texCoord.x;\n"
            "    \n"
            "    gl_Position = osg_ProjectionMatrix * v1;\n"
            "}\n";

        char fragmentShaderSource[] =
            "#version 460 core\n"
            "uniform sampler2D baseTexture;\n"
            "in vec2 texCoord;\n"
            "in vec4 colour;\n"
            "out vec4 fragColor;\n"
            "\n"
            "void main (void)\n"
            "{\n"
            "    fragColor = colour * texture( baseTexture, texCoord);\n"
            "}\n";

        program->addShader( new osg::Shader( osg::Shader::VERTEX, vertexShaderSource ) );
        program->addShader( new osg::Shader( osg::Shader::FRAGMENT,
                                             fragmentShaderSource ) );
#else
        // get shaders from source
        program->addShader(
            osg::Shader::readShaderFile( osg::Shader::VERTEX,
                                         osgDB::findDataFile( "quad_rain.vert" ) )
        );
        program->addShader(
            osg::Shader::readShaderFile( osg::Shader::FRAGMENT,
                                         osgDB::findDataFile( "rain.frag" ) )
        );
#endif
    }

    if( !_lineStateSet )
    {
        _lineStateSet         = new osg::StateSet;

        osg::Program* program = new osg::Program;
        _lineStateSet->setAttribute( program );
        _lineStateSet->setRenderBinDetails( static_cast<int>( lineRenderBin ),
                                            "DepthSortedBin" );

#ifdef USE_LOCAL_SHADERS
        char vertexShaderSource[] =
            "#version 460 core\n"
            "layout(location = 0) in vec4 osg_Vertex;\n"
            "layout(location = 8) in vec4 osg_MultiTexCoord0;\n"
            "uniform float cellStartTime;\n"
            "uniform mat4 osg_ModelViewMatrix;\n"
            "uniform mat4 osg_ProjectionMatrix;\n"
            "uniform float inversePeriod;\n"
            "uniform vec4 particleColour;\n"
            "uniform float particleSize;\n"
            "\n"
            "uniform float osg_SimulationTime;\n"
            "uniform float osg_DeltaSimulationTime;\n"
            "uniform mat4 previousModelViewMatrix;\n"
            "\n"
            "out vec4 colour;\n"
            "out vec2 texCoord;\n"
            "\n"
            "void main(void)\n"
            "{\n"
            "    float offset = osg_Vertex.z;\n"
            "    float startTime = cellStartTime;\n"
            "    texCoord = osg_MultiTexCoord0.xy;\n"
            "\n"
            "    vec4 v_previous = osg_Vertex;\n"
            "    v_previous.z = fract( (osg_SimulationTime - startTime)*inversePeriod - "
            "offset);\n"
            "    \n"
            "    vec4 v_current =  v_previous;\n"
            "    v_current.z += (osg_DeltaSimulationTime*inversePeriod);\n"
            "    \n"
            "    colour = particleColour;\n"
            "    \n"
            "    vec4 v1 = osg_ModelViewMatrix * v_current;\n"
            "    vec4 v2 = previousModelViewMatrix * v_previous;\n"
            "    \n"
            "    vec3 dv = v2.xyz - v1.xyz;\n"
            "    \n"
            "    vec2 dv_normalized = normalize(dv.xy);\n"
            "    dv.xy += dv_normalized * particleSize;\n"
            "    \n"
            "    float area = length(dv.xy);\n"
            "    colour.a = (particleSize)/area;\n"
            "    \n"
            "    v1.xyz += dv*texCoord.y;\n"
            "    \n"
            "    gl_Position = osg_ProjectionMatrix * v1;\n"
            "}\n";

        char fragmentShaderSource[] =
            "#version 460 core\n"
            "uniform sampler2D baseTexture;\n"
            "in vec2 texCoord;\n"
            "in vec4 colour;\n"
            "out vec4 fragColor;\n"
            "\n"
            "void main (void)\n"
            "{\n"
            "    fragColor = colour * texture( baseTexture, texCoord);\n"
            "}\n";

        program->addShader( new osg::Shader( osg::Shader::VERTEX, vertexShaderSource ) );
        program->addShader( new osg::Shader( osg::Shader::FRAGMENT,
                                             fragmentShaderSource ) );
#else
        // get shaders from source
        program->addShader(
            osg::Shader::readShaderFile( osg::Shader::VERTEX,
                                         osgDB::findDataFile( "line_rain.vert" ) )
        );
        program->addShader(
            osg::Shader::readShaderFile( osg::Shader::FRAGMENT,
                                         osgDB::findDataFile( "rain.frag" ) )
        );
#endif
    }

    if( !_pointStateSet )
    {
        _pointStateSet        = new osg::StateSet;

        osg::Program* program = new osg::Program;
        _pointStateSet->setAttribute( program );

#ifdef USE_LOCAL_SHADERS
        char vertexShaderSource[] =
            "#version 460 core\n"
            "layout(location = 0) in vec4 osg_Vertex;\n"
            "uniform float cellStartTime;\n"
            "uniform mat4 osg_ModelViewProjectionMatrix;\n"
            "uniform mat4 osg_ModelViewMatrix;\n"
            "uniform float inversePeriod;\n"
            "uniform vec4 particleColour;\n"
            "uniform float particleSize;\n"
            "\n"
            "uniform float osg_SimulationTime;\n"
            "\n"
            "out vec4 colour;\n"
            "\n"
            "void main(void)\n"
            "{\n"
            "    float offset = osg_Vertex.z;\n"
            "    float startTime = cellStartTime;\n"
            "\n"
            "    vec4 v_current = osg_Vertex;\n"
            "    v_current.z = fract( (osg_SimulationTime - startTime)*inversePeriod - "
            "offset);\n"
            "   \n"
            "    colour = particleColour;\n"
            "\n"
            "    gl_Position = osg_ModelViewProjectionMatrix * v_current;\n"
            "\n"
            "    float pointSize = abs(1280.0*particleSize / gl_Position.w);\n"
            "\n"
            "    //gl_PointSize = max(ceil(pointSize),2);\n"
            "    gl_PointSize = ceil(pointSize);\n"
            "    \n"
            "    colour.a = 0.05+(pointSize*pointSize)/(gl_PointSize*gl_PointSize);\n"
            "}\n";

        char fragmentShaderSource[] =
            "#version 460 core\n"
            "uniform sampler2D baseTexture;\n"
            "in vec4 colour;\n"
            "out vec4 fragColor;\n"
            "\n"
            "void main (void)\n"
            "{\n"
            "    fragColor = colour * texture( baseTexture, gl_PointCoord.xy);\n"
            "}\n";

        program->addShader( new osg::Shader( osg::Shader::VERTEX, vertexShaderSource ) );
        program->addShader( new osg::Shader( osg::Shader::FRAGMENT,
                                             fragmentShaderSource ) );
#else
        // get shaders from source
        program->addShader(
            osg::Shader::readShaderFile( osg::Shader::VERTEX,
                                         osgDB::findDataFile( "point_rain.vert" ) )
        );
        program->addShader(
            osg::Shader::readShaderFile( osg::Shader::FRAGMENT,
                                         osgDB::findDataFile( "point_rain.frag" ) )
        );
#endif

        _pointStateSet->setMode( GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON );

        _pointStateSet->setRenderBinDetails( static_cast<int>( pointRenderBin ),
                                             "DepthSortedBin" );
    }
}

void
PrecipitationEffect::cull( PrecipitationDrawableSet& pds,
                           osgUtil::CullVisitor*     cv ) const
{
#ifdef DO_TIMING
    osg::Timer_t startTick = osg::Timer::instance()->tick();
#endif

    float cellVolume        = _cellSize.x * _cellSize.y * _cellSize.z;
    int   numberOfParticles = ( int )( _maximumParticleDensity * cellVolume );

    if( numberOfParticles == 0 )
    {
        return;
    }

    pds._quadPrecipitationDrawable->setNumberOfVertices(
        static_cast<unsigned int>( numberOfParticles * 6 )
    );
    pds._linePrecipitationDrawable->setNumberOfVertices(
        static_cast<unsigned int>( numberOfParticles * 2 )
    );
    pds._pointPrecipitationDrawable->setNumberOfVertices(
        static_cast<unsigned int>( numberOfParticles )
    );

    pds._quadPrecipitationDrawable->newFrame();
    pds._linePrecipitationDrawable->newFrame();
    pds._pointPrecipitationDrawable->newFrame();

    osg::dmat4 inverse_modelview;
    inverse_modelview  = osg::inverse( *( cv->getModelViewMatrix() ) );

    osg::vec3 eyeLocal = osg::vec3( osg::vec3( 0.0F, 0.0F, 0.0F ) * inverse_modelview );
    // OSG_NOTICE<<"  eyeLocal "<<eyeLocal<<std::endl;

    float     eye_k      = osg::dot( eyeLocal - _origin, _inverse_dw );
    osg::vec3 eye_kPlane = eyeLocal - _dw * eye_k - _origin;

    // OSG_NOTICE<<"  eye_kPlane "<<eye_kPlane<<std::endl;

    float     eye_i = osg::dot( eye_kPlane, _inverse_du );
    float     eye_j = osg::dot( eye_kPlane, _inverse_dv );

    osg::Polytope frustum;
    frustum.setToUnitFrustum( false, false );
    frustum.transformProvidingInverse( *( cv->getProjectionMatrix() ) );
    frustum.transformProvidingInverse( *( cv->getModelViewMatrix() ) );

    float        i_delta = _farTransition * _inverse_du.x;
    float        j_delta = _farTransition * _inverse_dv.y;
    float        k_delta = 1;    //_nearTransition * _inverse_dw.z;

    int          i_min   = ( int )floor( eye_i - i_delta );
    int          j_min   = ( int )floor( eye_j - j_delta );
    int          k_min   = ( int )floor( eye_k - k_delta );

    int          i_max   = ( int )ceil( eye_i + i_delta );
    int          j_max   = ( int )ceil( eye_j + j_delta );
    int          k_max   = ( int )ceil( eye_k + k_delta );

    // OSG_NOTICE<<"i_delta="<<i_delta<<" j_delta="<<j_delta<<"
    // k_delta="<<k_delta<<std::endl;

    unsigned int numTested = 0;
    ( void )numTested;
    unsigned int numInFrustum = 0;
    ( void )numInFrustum;

    float iCyle = 0.43F;
    float jCyle = 0.64F;

    for( int i = i_min; i <= i_max; ++i )
    {
        for( int j = j_min; j <= j_max; ++j )
        {
            for( int k = k_min; k <= k_max; ++k )
            {
                float startTime = ( float )( i )*iCyle + ( float )( j )*jCyle;
                startTime       = ( startTime - floor( startTime ) ) * _period;

                if( build( eyeLocal, i, j, k, startTime, pds, frustum, cv ) )
                {
                    ++numInFrustum;
                }
                ++numTested;
            }
        }
    }

#ifdef DO_TIMING
    osg::Timer_t endTick = osg::Timer::instance()->tick();

    OSG_NOTICE << "time for cull "
               << osg::Timer::instance()->delta_m( startTick, endTick )
               << "ms  numTested=" << numTested << " numInFrustum" << numInFrustum
               << std::endl;
    OSG_NOTICE << "     quads "
               << pds._quadPrecipitationDrawable->getCurrentCellMatrixMap().size()
               << "   lines "
               << pds._linePrecipitationDrawable->getCurrentCellMatrixMap().size()
               << "   points "
               << pds._pointPrecipitationDrawable->getCurrentCellMatrixMap().size()
               << std::endl;
#endif
}

bool
PrecipitationEffect::build( const osg::vec3           eyeLocal,
                            int                       i,
                            int                       j,
                            int                       k,
                            float                     startTime,
                            PrecipitationDrawableSet& pds,
                            osg::Polytope&            frustum,
                            osgUtil::CullVisitor*     cv ) const
{
    osg::vec3 position =
        _origin +
        osg::vec3( float( i ) * _du.x, float( j ) * _dv.y, float( k + 1 ) * _dw.z );
    osg::vec3 scale( _du.x, _dv.y, -_dw.z );

    osg::box  bb( position.x,
                  position.y,
                  position.z + scale.z,
                  position.x + scale.x,
                  position.y + scale.y,
                  position.z );

    if( !frustum.contains( bb ) )
    {
        return false;
    }

    osg::vec3   center      = position + scale * 0.5F;
    float       distance    = osg::length( center - eyeLocal );

    osg::dmat4* mymodelview = 0;
    if( distance < _nearTransition )
    {
        PrecipitationDrawable::DepthMatrixStartTime& mstp =
            pds._quadPrecipitationDrawable
                ->getCurrentCellMatrixMap()[PrecipitationDrawable::Cell( i, k, j )];
        mstp.depth     = distance;
        mstp.startTime = startTime;
        mymodelview    = &mstp.modelview;
    }
    else if( distance <= _farTransition )
    {
        if( _useFarLineSegments )
        {
            PrecipitationDrawable::DepthMatrixStartTime& mstp =
                pds._linePrecipitationDrawable
                    ->getCurrentCellMatrixMap()[PrecipitationDrawable::Cell( i, k, j )];
            mstp.depth     = distance;
            mstp.startTime = startTime;
            mymodelview    = &mstp.modelview;
        }
        else
        {
            PrecipitationDrawable::DepthMatrixStartTime& mstp =
                pds._pointPrecipitationDrawable
                    ->getCurrentCellMatrixMap()[PrecipitationDrawable::Cell( i, k, j )];
            mstp.depth     = distance;
            mstp.startTime = startTime;
            mymodelview    = &mstp.modelview;
        }
    }
    else
    {
        return false;
    }

    *mymodelview = *( cv->getModelViewMatrix() );
    osg::preMultTranslate( *mymodelview,
                           osg::dvec3( position.x, position.y, position.z ) );
    osg::preMultScale( *mymodelview, osg::dvec3( scale.x, scale.y, scale.z ) );

    cv->updateCalculatedNearFar( *( cv->getModelViewMatrix() ), bb );

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   Precipitation Drawable
//
////////////////////////////////////////////////////////////////////////////////////////////////////

PrecipitationEffect::PrecipitationDrawable::PrecipitationDrawable() :
    _requiresPreviousMatrix( true ),
    _drawType( GL_TRIANGLES ),
    _numberOfVertices( 0 )
{
}

PrecipitationEffect::PrecipitationDrawable::PrecipitationDrawable(
    const PrecipitationDrawable& copy,
    const osg::CopyOp&           copyop
) :
    Inherit( copy,
             copyop ),
    _requiresPreviousMatrix( copy._requiresPreviousMatrix ),
    _geometry( copy._geometry ),
    _drawType( copy._drawType ),
    _numberOfVertices( copy._numberOfVertices )
{
}

PrecipitationEffect::PrecipitationDrawable::~PrecipitationDrawable()
{
    OSG_NOTICE << "PrecipitationEffect::~PrecipitationDrawable() " << this << std::endl;
}

void
PrecipitationEffect::PrecipitationDrawable::resizeGLObjectBuffers( unsigned int maxSize )
{
    Drawable::resizeGLObjectBuffers( maxSize );

    if( _geometry )
    {
        _geometry->resizeGLObjectBuffers( maxSize );
    }
}

void
PrecipitationEffect::PrecipitationDrawable::releaseGLObjects( osg::State* state ) const
{
    Drawable::releaseGLObjects( state );

    if( _geometry )
    {
        _geometry->releaseGLObjects( state );
    }
}

void
PrecipitationEffect::PrecipitationDrawable::drawImplementation(
    osg::RenderInfo& renderInfo
) const
{
    if( !_geometry )
    {
        return;
    }

    osg::State&              state      = *renderInfo.getState();
    const osg::GLExtensions* extensions = state.get<osg::GLExtensions>();

    // save the original modelview matrix to restore later
    osg::dmat4               originalModelView = state.getModelViewMatrix();

    typedef std::vector<const CellMatrixMap::value_type*> DepthMatrixStartTimeVector;
    DepthMatrixStartTimeVector                            orderedEntries;
    orderedEntries.reserve( _currentCellMatrixMap.size() );

    for( CellMatrixMap::const_iterator citr = _currentCellMatrixMap.begin();
         citr != _currentCellMatrixMap.end();
         ++citr )
    {
        orderedEntries.push_back( &( *citr ) );
    }

    std::sort( orderedEntries.begin(), orderedEntries.end(), LessFunctor() );

    // look up uniform locations once before the draw loop
    GLint previousMVLoc = -1;
    if( _requiresPreviousMatrix )
    {
        previousMVLoc = state.getUniformLocation( "previousModelViewMatrix" );
    }
    GLint cellStartTimeLoc = state.getUniformLocation( "cellStartTime" );

    for( DepthMatrixStartTimeVector::reverse_iterator itr = orderedEntries.rbegin();
         itr != orderedEntries.rend();
         ++itr )
    {
        if( cellStartTimeLoc >= 0 )
        {
            extensions->glUniform( cellStartTimeLoc, ( *itr )->second.startTime );
        }

        // apply the cell's current modelview matrix via State (updates uniforms)
        state.applyModelViewMatrix( ( *itr )->second.modelview );

        if( _requiresPreviousMatrix && previousMVLoc >= 0 )
        {
            CellMatrixMap::const_iterator pitr =
                _previousCellMatrixMap.find( ( *itr )->first );
            if( pitr != _previousCellMatrixMap.end() )
            {
                // set previous frame modelview matrix uniform for motion blur effect
                extensions->glUniform( previousMVLoc,
                                       osg::mat4( pitr->second.modelview ) );
            }
            else
            {
                // use current modelview matrix as "previous" frame value, cancelling
                // motion blur effect
                extensions->glUniform( previousMVLoc,
                                       osg::mat4( ( *itr )->second.modelview ) );
            }
        }

        _geometry->draw( renderInfo );

        unsigned int numVertices =
            std::min( _geometry->getVertexArray()->getNumElements(), _numberOfVertices );
        glDrawArrays( _drawType, 0, static_cast<GLsizei>( numVertices ) );
    }

    // restore the original modelview matrix
    state.applyModelViewMatrix( originalModelView );
}
