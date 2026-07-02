/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Scene-level volume rendering manager. Coordinates multiple
 * VolumeTiles with proper back-to-front ordering.
 */
#include <osgVolume/VolumeScene.hpp>

#include <limits>
#include <osg/core/io_utils.hpp>
#include <osg/core/ValueObject.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/PrimitiveSet.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/state/Depth.hpp>
#include <osgDB/io/ReadFile.hpp>

using namespace osgVolume;

class RTTCameraCullCallback : public osg::NodeCallback
{
    public:

        RTTCameraCullCallback( VolumeScene* vs ) :
            _volumeScene( vs )
        {
        }

        virtual void
        operator()( osg::Node*        node,
                    osg::NodeVisitor* nv )
        {
            osgUtil::CullVisitor* cv = nv->asCullVisitor();

            _volumeScene->osg::Group::traverse( *nv );

            node->setUserValue( "CalculatedNearPlane",
                                double( cv->getCalculatedNearPlane() ) );
            node->setUserValue( "CalculatedFarPlane",
                                double( cv->getCalculatedFarPlane() ) );
        }

    protected:

        virtual ~RTTCameraCullCallback()
        {
        }

        osgVolume::VolumeScene* _volumeScene;
};

////////////////////////////////////////////////////////////////////////
//
// VolumeScene::ViewData
//
VolumeScene::ViewData::ViewData()
{
}

void
VolumeScene::ViewData::clearTiles()
{
    for( Tiles::iterator itr = _tiles.begin(); itr != _tiles.end(); ++itr )
    {
        if( itr->second.valid() )
        {
            itr->second->active = false;
        }
    }
}

void
VolumeScene::ViewData::visitTile( VolumeTile* /*tile*/ )
{
}

////////////////////////////////////////////////////////////////////////
//
// VolumeScene
//
VolumeScene::VolumeScene()
{
}

VolumeScene::VolumeScene( const VolumeScene& vs,
                          const osg::CopyOp& copyop ) :
    Inherit( vs,
             copyop )
{
}

VolumeScene::~VolumeScene()
{
}

TileData*
VolumeScene::tileVisited( osgUtil::CullVisitor*  cv,
                          osgVolume::VolumeTile* tile )
{
    osg::ref_ptr<ViewData> viewData;

    {
        std::lock_guard<std::mutex> lock( _viewDataMapMutex );
        viewData = _viewDataMap[cv];
    }

    // osg::box bb(0.0f,0.0f,0.0f,1.0f,1.0f,1.0f);
    // cv->updateCalculatedNearFar(*(cv->getModelViewMatrix()),bb);

    if( viewData.valid() )
    {
        osg::ref_ptr<TileData>& tileData = viewData->_tiles[tile];
        if( !tileData )
        {
            tileData = tile->getVolumeTechnique()
                         ? tile->getVolumeTechnique()->createTileData( cv )
                         : 0;
        }

        if( tileData )
        {
            tileData->update( cv );
        }

        return tileData.get();
    }
    return 0;
}

TileData*
VolumeScene::getTileData( osgUtil::CullVisitor*  cv,
                          osgVolume::VolumeTile* tile )
{
    osg::ref_ptr<ViewData> viewData;
    {
        std::lock_guard<std::mutex> lock( _viewDataMapMutex );
        viewData = _viewDataMap[cv];
    }

    if( !viewData )
    {
        return 0;
    }

    Tiles::iterator itr = viewData->_tiles.find( tile );
    return ( itr != viewData->_tiles.end() ) ? itr->second.get() : 0;
}

void
VolumeScene::traverse( osg::NodeVisitor& nv )
{
    osgUtil::CullVisitor* cv = nv.asCullVisitor();
    if( !cv )
    {
        Group::traverse( nv );
        return;
    }

    osg::ref_ptr<ViewData> viewData;
    bool                   initializeViewData = false;
    {
        std::lock_guard<std::mutex> lock( _viewDataMapMutex );
        if( !_viewDataMap[cv] )
        {
            _viewDataMap[cv]   = new ViewData;
            initializeViewData = true;
        }

        viewData = _viewDataMap[cv];
    }

    if( initializeViewData )
    {
        OSG_NOTICE << "Creating ViewData" << std::endl;

        int            textureWidth  = 512;
        int            textureHeight = 512;

        osg::Viewport* viewport      = cv->getCurrentRenderStage()->getViewport();
        if( viewport )
        {
            textureWidth  = static_cast<int>( static_cast<float>( viewport->width() ) );
            textureHeight = static_cast<int>( static_cast<float>( viewport->height() ) );
        }

        // set up depth texture
        viewData->_depthTexture = new osg::Texture2D;

        viewData->_depthTexture->setTextureSize( textureWidth, textureHeight );
        viewData->_depthTexture->setInternalFormat( GL_DEPTH_COMPONENT );
        viewData->_depthTexture->setFilter( osg::Texture2D::MIN_FILTER,
                                            osg::Texture2D::LINEAR );
        viewData->_depthTexture->setFilter( osg::Texture2D::MAG_FILTER,
                                            osg::Texture2D::LINEAR );

        viewData->_depthTexture->setWrap( osg::Texture2D::WRAP_S,
                                          osg::Texture2D::CLAMP_TO_BORDER );
        viewData->_depthTexture->setWrap( osg::Texture2D::WRAP_T,
                                          osg::Texture2D::CLAMP_TO_BORDER );
        viewData->_depthTexture->setBorderColor( osg::dvec4( 1.0F, 1.0F, 1.0F, 1.0F ) );

        // set up color texture
        viewData->_colorTexture = new osg::Texture2D;

        viewData->_colorTexture->setTextureSize( textureWidth, textureHeight );
        viewData->_colorTexture->setInternalFormat( GL_RGBA );
        viewData->_colorTexture->setFilter( osg::Texture2D::MIN_FILTER,
                                            osg::Texture2D::LINEAR );
        viewData->_colorTexture->setFilter( osg::Texture2D::MAG_FILTER,
                                            osg::Texture2D::LINEAR );

        viewData->_colorTexture->setWrap( osg::Texture2D::WRAP_S,
                                          osg::Texture2D::CLAMP_TO_EDGE );
        viewData->_colorTexture->setWrap( osg::Texture2D::WRAP_T,
                                          osg::Texture2D::CLAMP_TO_EDGE );

        // set up the RTT Camera to capture the main scene to a color and depth texture
        // that can be used in post processing
        viewData->_rttCamera = new osg::Camera;
        viewData->_rttCamera->setName( "viewData->_rttCamera" );
        viewData->_rttCamera->attach( osg::Camera::DEPTH_BUFFER,
                                      viewData->_depthTexture.get() );
        viewData->_rttCamera->attach( osg::Camera::COLOR_BUFFER,
                                      viewData->_colorTexture.get() );
        viewData->_rttCamera->setCullCallback( new RTTCameraCullCallback( this ) );
        viewData->_rttCamera->setViewport( 0, 0, textureWidth, textureHeight );

        // clear the depth and colour bufferson each clear.
        viewData->_rttCamera->setClearMask( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

        // set the camera to render before the main camera.
        viewData->_rttCamera->setRenderOrder( osg::Camera::PRE_RENDER );

        // tell the camera to use OpenGL frame buffer object where supported.
        viewData->_rttCamera->setRenderTargetImplementation(
            osg::Camera::FRAME_BUFFER_OBJECT
        );

        viewData->_rttCamera->setReferenceFrame( osg::Transform::RELATIVE_RF );
        viewData->_rttCamera->setProjectionMatrix( osg::dmat4() );
        viewData->_rttCamera->setViewMatrix( osg::dmat4() );

        // create mesh for rendering the RTT textures onto the screen
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        geode->setCullingActive( false );

        viewData->_backdropSubgraph = geode;
        // geode->addDrawable(osg::createTexturedQuadGeometry(osg::vec3(-1.0f,-1.0f,-1.0f),osg::vec3(2.0f,0.0f,-1.0f),osg::vec3(0.0f,2.0f,-1.0f)));

        viewData->_geometry = new osg::Geometry;
        geode->addDrawable( viewData->_geometry.get() );

        viewData->_geometry->setUseVertexBufferObjects( false );

        viewData->_vertices = new osg::Vec3Array( 4 );
        viewData->_geometry->setVertexArray( viewData->_vertices.get() );

        osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array( 1 );
        ( *colors )[0].set( 1.0F, 1.0F, 1.0F, 1.0F );
        viewData->_geometry->setColorArray( colors.get(), osg::Array::BIND_OVERALL );

        osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array( 4 );
        ( *texcoords )[0].set( 0.0F, 1.0F );
        ( *texcoords )[1].set( 0.0F, 0.0F );
        ( *texcoords )[2].set( 1.0F, 1.0F );
        ( *texcoords )[3].set( 1.0F, 0.0F );
        viewData->_geometry->setTexCoordArray( 0,
                                               texcoords.get(),
                                               osg::Array::BIND_PER_VERTEX );

        viewData->_geometry->addPrimitiveSet(
            new osg::DrawArrays( GL_TRIANGLE_STRIP, 0, 4 )
        );

        osg::ref_ptr<osg::StateSet> stateset =
            viewData->_geometry->getOrCreateStateSet();

        stateset->setMode( GL_DEPTH_TEST, osg::StateAttribute::ON );
        // GL_LIGHTING removed: not in core profile
        stateset->setMode( GL_BLEND, osg::StateAttribute::OFF );
        stateset->setAttribute( new osg::Depth( osg::Depth::Function::LEQUAL ) );
        stateset->setRenderBinDetails( 10, "DepthSortedBin" );

        osg::ref_ptr<osg::Program> program = new osg::Program;
        stateset->setAttribute( program.get() );

        // get vertex shaders from source
        osg::ref_ptr<osg::Shader> vertexShader =
            osgDB::readRefShaderFile( osg::Shader::VERTEX,
                                      "shaders/volume_color_depth.vert" );
        if( vertexShader.valid() )
        {
            program->addShader( vertexShader.get() );
        }
#if 0
        else
        {
    #include "Shaders/volume_color_depth_vert.cpp"
            program->addShader(new osg::Shader(osg::Shader::VERTEX, volume_color_depth_vert));
        }
#endif
        // get fragment shaders from source
        osg::ref_ptr<osg::Shader> fragmentShader =
            osgDB::readRefShaderFile( osg::Shader::FRAGMENT,
                                      "shaders/volume_color_depth.frag" );
        if( fragmentShader.valid() )
        {
            program->addShader( fragmentShader.get() );
        }
#if 0
        else
        {
    #include "Shaders/volume_color_depth_frag.cpp"
            program->addShader(new osg::Shader(osg::Shader::FRAGMENT, volume_color_depth_frag));
        }
#endif
        viewData->_stateset = new osg::StateSet;
        viewData->_stateset->addUniform( new osg::Uniform( "colorTexture", 0 ) );
        viewData->_stateset->addUniform( new osg::Uniform( "depthTexture", 1 ) );

        viewData->_stateset->setTextureAttributeAndModes(
            0,
            viewData->_colorTexture.get(),
            osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE
        );
        viewData->_stateset->setTextureAttributeAndModes(
            1,
            viewData->_depthTexture.get(),
            osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE
        );

        viewData->_viewportDimensionsUniform =
            new osg::Uniform( "viewportDimensions",
                              osg::vec4( 0.0, 0.0, 1280.0, 1024.0 ) );
        viewData->_stateset->addUniform( viewData->_viewportDimensionsUniform.get() );

        geode->setStateSet( viewData->_stateset.get() );
    }
    else
    {
        // OSG_NOTICE<<"Reusing ViewData"<<std::endl;
    }

    osg::dmat4 projectionMatrix = *( cv->getProjectionMatrix() );
    osg::dmat4 modelviewMatrix  = *( cv->getModelViewMatrix() );

    // new frame so need to clear last frames log of VolumeTiles
    viewData->clearTiles();

    osg::Viewport* viewport = cv->getCurrentRenderStage()->getViewport();
    if( viewport )
    {
        viewData->_viewportDimensionsUniform->set(
            osg::vec4( static_cast<float>( viewport->x() ),
                       static_cast<float>( viewport->y() ),
                       static_cast<float>( viewport->width() ),
                       static_cast<float>( viewport->height() ) )
        );

        int textureWidth  = static_cast<int>( static_cast<float>( viewport->width() ) );
        int textureHeight = static_cast<int>( static_cast<float>( viewport->height() ) );

        if( textureWidth !=
            viewData->_colorTexture->getTextureWidth() ||
            textureHeight != viewData->_colorTexture->getTextureHeight() )
        {
            OSG_NOTICE << "Need to change texture size to " << textureWidth << ", "
                       << textureHeight << std::endl;
            viewData->_colorTexture->setTextureSize( textureWidth, textureHeight );
            viewData->_colorTexture->dirtyTextureObject();
            viewData->_depthTexture->setTextureSize( textureWidth, textureHeight );
            viewData->_depthTexture->dirtyTextureObject();
            viewData->_rttCamera->setViewport( 0, 0, textureWidth, textureHeight );
            if( viewData->_rttCamera->getRenderingCache() )
            {
                viewData->_rttCamera->getRenderingCache()->releaseGLObjects( 0 );
            }
        }
    }

    cv->setUserValue( "VolumeSceneTraversal", std::string( "RenderToTexture" ) );

    // OSG_NOTICE<<"Ready to traverse RTT Camera"<<std::endl;
    // OSG_NOTICE<<"   RTT Camera ProjectionMatrix Before
    // "<<viewData->_rttCamera->getProjectionMatrix()<<std::endl;
    viewData->_rttCamera->accept( nv );

    // OSG_NOTICE<<"   RTT Camera ProjectionMatrix After
    // "<<viewData->_rttCamera->getProjectionMatrix()<<std::endl; OSG_NOTICE<<"   cv
    // ProjectionMatrix After "<<*(cv->getProjectionMatrix())<<std::endl;

    // OSG_NOTICE<<"  after RTT near ="<<cv->getCalculatedNearPlane()<<std::endl;
    // OSG_NOTICE<<"  after RTT far ="<<cv->getCalculatedFarPlane()<<std::endl;

    // OSG_NOTICE<<"tileVisited()"<<viewData->_tiles.size()<<std::endl;

    typedef osgUtil::CullVisitor::value_type NearFarValueType;
    NearFarValueType calculatedNearPlane = std::numeric_limits<NearFarValueType>::max();
    NearFarValueType calculatedFarPlane  = -std::numeric_limits<NearFarValueType>::max();
    if( viewData->_rttCamera->getUserValue( "CalculatedNearPlane",
                                            calculatedNearPlane ) &&
        viewData->_rttCamera->getUserValue( "CalculatedFarPlane", calculatedFarPlane ) )
    {
        calculatedNearPlane *= 0.5;
        calculatedFarPlane  *= 2.0;

        // OSG_NOTICE<<"Got from RTTCamera
        // CalculatedNearPlane="<<calculatedNearPlane<<std::endl; OSG_NOTICE<<"Got from
        // RTTCamera CalculatedFarPlane="<<calculatedFarPlane<<std::endl;
        if( calculatedNearPlane < cv->getCalculatedNearPlane() )
        {
            cv->setCalculatedNearPlane( calculatedNearPlane );
        }
        if( calculatedFarPlane > cv->getCalculatedFarPlane() )
        {
            cv->setCalculatedFarPlane( calculatedFarPlane );
        }
    }

    if( calculatedFarPlane > calculatedNearPlane )
    {
        cv->clampProjectionMatrix( projectionMatrix,
                                   calculatedNearPlane,
                                   calculatedFarPlane );
    }

    osg::dmat4 inv_projectionModelViewMatrix;
    inv_projectionModelViewMatrix = osg::inverse( modelviewMatrix * projectionMatrix );

    double     depth              = 1.0;
    osg::dvec3 v00 = osg::dvec3( -1.0, -1.0, depth ) * inv_projectionModelViewMatrix;
    osg::dvec3 v01 = osg::dvec3( -1.0, 1.0, depth ) * inv_projectionModelViewMatrix;
    osg::dvec3 v10 = osg::dvec3( 1.0, -1.0, depth ) * inv_projectionModelViewMatrix;
    osg::dvec3 v11 = osg::dvec3( 1.0, 1.0, depth ) * inv_projectionModelViewMatrix;

    // OSG_NOTICE<<"v00= "<<v00<<std::endl;
    // OSG_NOTICE<<"v01= "<<v01<<std::endl;
    // OSG_NOTICE<<"v10= "<<v10<<std::endl;
    // OSG_NOTICE<<"v11= "<<v11<<std::endl;

    ( *( viewData->_vertices ) )[0] = v01;
    ( *( viewData->_vertices ) )[1] = v00;
    ( *( viewData->_vertices ) )[2] = v11;
    ( *( viewData->_vertices ) )[3] = v10;
    viewData->_geometry->dirtyBound();

    // OSG_NOTICE<<"  new after RTT near ="<<cv->getCalculatedNearPlane()<<std::endl;
    // OSG_NOTICE<<"  new after RTT far ="<<cv->getCalculatedFarPlane()<<std::endl;

    viewData->_backdropSubgraph->accept( *cv );

    osg::NodePath nodePathPriorToTraversingSubgraph = cv->getNodePath();
    cv->setUserValue( "VolumeSceneTraversal", std::string( "Post" ) );

    // for each tile that needs post rendering we need to add it into current
    // RenderStage.
    Tiles& tiles = viewData->_tiles;
    for( Tiles::iterator itr = tiles.begin(); itr != tiles.end(); ++itr )
    {
        TileData* tileData = itr->second.get();
        if( !tileData || !( tileData->active ) )
        {
            OSG_INFO << "Skipping TileData that is inactive : " << tileData << std::endl;
            continue;
        }

        unsigned int   numStateSetPushed = 0;

        // OSG_NOTICE<<"VolumeTile to add "<<tileData->projectionMatrix.get()<<",
        // "<<tileData->modelviewMatrix.get()<<std::endl;

        osg::NodePath& nodePath = tileData->nodePath;

        cv->getNodePath()       = nodePath;
        cv->pushProjectionMatrix( tileData->projectionMatrix.get() );
        cv->pushModelViewMatrix( tileData->modelviewMatrix.get(),
                                 osg::Transform::ABSOLUTE_RF_INHERIT_VIEWPOINT );

        cv->pushStateSet( viewData->_stateset.get() );
        ++numStateSetPushed;

        cv->pushStateSet( tileData->stateset.get() );
        ++numStateSetPushed;

        osg::NodePath::iterator np_itr = nodePath.begin();

        // skip over all nodes above VolumeScene as this will have already been traversed
        // by CullVisitor
        while( np_itr != nodePath.end() && ( *np_itr ) != viewData->_rttCamera.get() )
        {
            ++np_itr;
        }
        if( np_itr != nodePath.end() )
        {
            ++np_itr;
        }

        // push the stateset on the nodes between this VolumeScene and the VolumeTile
        for( osg::NodePath::iterator ss_itr = np_itr; ss_itr != nodePath.end();
             ++ss_itr )
        {
            if( ( *ss_itr )->getStateSet() )
            {
                numStateSetPushed++;
                cv->pushStateSet( ( *ss_itr )->getStateSet() );
                // OSG_NOTICE<<"  pushing StateSet"<<std::endl;
            }
        }
        cv->traverse( *( tileData->nodePath.back() ) );

        // pop the StateSet's
        for( unsigned int i = 0; i < numStateSetPushed; ++i )
        {
            cv->popStateSet();
            // OSG_NOTICE<<"  popping StateSet"<<std::endl;
        }

        cv->popModelViewMatrix();
        cv->popProjectionMatrix();
    }

    // need to synchronize projection matrices:
    //    current CV projection matrix
    //    main scene RTT Camera projection matrix
    //    each tile RTT Camera
    //    each tile final render.

    cv->getNodePath() = nodePathPriorToTraversingSubgraph;
}
