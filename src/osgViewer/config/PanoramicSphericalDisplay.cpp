/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osgViewer: center, eye, xAxis, yAxis, texcoord.
 */
#include <osgViewer/config/PanoramicSphericalDisplay.hpp>

#include <osg/core/io_utils.hpp>
#include <osg/core/ValueObject.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/state/Stencil.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osg/textures/TextureRectangle.hpp>
#include <osgViewer/core/Renderer.hpp>
#include <osgViewer/core/View.hpp>
#include <osgViewer/platform/GraphicsWindow.hpp>

using namespace osgViewer;

osg::Geometry*
PanoramicSphericalDisplay::createParoramicSphericalDisplayDistortionMesh(
    const osg::vec3&  origin,
    const osg::vec3&  widthVector,
    const osg::vec3&  heightVector,
    double            sphere_radius,
    double            collar_radius,
    osg::Image*       intensityMap,
    const osg::dmat4& projectorMatrix
) const
{
    osg::dvec3 eye( 0.0, 0.0, 0.0 );

    double     distance =
        sqrt( sphere_radius * sphere_radius - collar_radius * collar_radius );
    bool       flip          = false;
    bool       texcoord_flip = false;

    osg::dvec3 projector     = eye - osg::dvec3( 0.0, 0.0, distance );

    OSG_INFO << "createParoramicSphericalDisplayDistortionMesh : Projector position = "
             << projector << std::endl;
    OSG_INFO << "createParoramicSphericalDisplayDistortionMesh : distance = " << distance
             << std::endl;

    // create the quad to visualize.
    osg::Geometry* geometry = new osg::Geometry();

    osg::vec3      xAxis( widthVector );
    float          width  = osg::length( widthVector );
    xAxis                /= width;

    osg::vec3 yAxis( heightVector );
    float     height              = osg::length( heightVector );
    yAxis                        /= height;

    int             noSteps       = 160;

    osg::Vec3Array* vertices      = new osg::Vec3Array;
    osg::Vec2Array* texcoords0    = new osg::Vec2Array;
    osg::Vec2Array* texcoords1    = intensityMap == 0 ? new osg::Vec2Array : 0;
    osg::Vec4Array* colors        = new osg::Vec4Array;

    osg::vec3       top           = origin + yAxis * height;

    osg::vec3       screenCenter  = origin + widthVector * 0.5F + heightVector * 0.5F;
    float           screenRadius  = osg::length( heightVector ) * 0.5F;

    geometry->getOrCreateStateSet()->setMode( GL_CULL_FACE,
                                              osg::StateAttribute::OFF |
                                                  osg::StateAttribute::PROTECTED );

    for( int i = 0; i < noSteps; ++i )
    {
        for( int j = 0; j < noSteps; ++j )
        {
            osg::vec2 texcoord(
                static_cast<float>( double( i ) / double( noSteps - 1 ) ),
                static_cast<float>( double( j ) / double( noSteps - 1 ) )
            );
            double theta = texcoord.x * 2.0 * osg::PI;
            double phi   = ( 1.0 - texcoord.y ) * osg::PI;

            if( texcoord_flip )
            {
                texcoord.y = 1.0F - texcoord.y;
            }

            osg::vec3 pos( static_cast<float>( sin( phi ) * sin( theta ) ),
                           static_cast<float>( sin( phi ) * cos( theta ) ),
                           static_cast<float>( cos( phi ) ) );
            pos          = pos * projectorMatrix;

            double alpha = atan2( pos.x, pos.y );
            if( alpha < 0.0 )
            {
                alpha += 2.0 * osg::PI;
            }

            double beta = atan2( sqrt( pos.x * pos.x + pos.y * pos.y ), pos.z );
            if( beta < 0.0 )
            {
                beta += 2.0 * osg::PI;
            }

            double gamma = atan2( sqrt( double( pos.x * pos.x + pos.y * pos.y ) ),
                                  double( pos.z + distance ) );
            if( gamma < 0.0 )
            {
                gamma += 2.0 * osg::PI;
            }

            osg::vec3 v =
                screenCenter +
                osg::vec3( static_cast<float>( sin( alpha ) * gamma * 2.0 / osg::PI ),
                           static_cast<float>( -cos( alpha ) * gamma * 2.0 / osg::PI ),
                           0.0F ) *
                screenRadius;

            if( flip )
            {
                vertices->push_back( osg::vec3( v.x, top.y - ( v.y - origin.y ), v.z ) );
            }
            else
            {
                vertices->push_back( v );
            }

            texcoords0->push_back( texcoord );

            osg::vec2 texcoord1( static_cast<float>( alpha / ( 2.0 * osg::PI ) ),
                                 static_cast<float>( 1.0 - beta / osg::PI ) );
            if( intensityMap )
            {
                colors->push_back( intensityMap->getColor( texcoord1 ) );
            }
            else
            {
                colors->push_back( osg::vec4( 1.0F, 1.0F, 1.0F, 1.0F ) );
                if( texcoords1 )
                {
                    texcoords1->push_back( texcoord1 );
                }
            }
        }
    }

    // pass the created vertex array to the points geometry object.
    geometry->setVertexArray( vertices );

    geometry->setColorArray( colors, osg::Array::BIND_PER_VERTEX );

    geometry->setTexCoordArray( 0, texcoords0 );
    if( texcoords1 )
    {
        geometry->setTexCoordArray( 1, texcoords1 );
    }

    osg::DrawElementsUShort* elements =
        new osg::DrawElementsUShort( osg::PrimitiveSet::TRIANGLES );
    geometry->addPrimitiveSet( elements );

    for( int i = 0; i < noSteps - 1; ++i )
    {
        for( int j = 0; j < noSteps - 1; ++j )
        {
            int        i1 = j + ( i + 1 ) * noSteps;
            int        i2 = j + ( i )*noSteps;
            int        i3 = j + 1 + ( i )*noSteps;
            int        i4 = j + 1 + ( i + 1 ) * noSteps;

            osg::vec3& v1 = ( *vertices )[static_cast<std::size_t>( i1 )];
            osg::vec3& v2 = ( *vertices )[static_cast<std::size_t>( i2 )];
            osg::vec3& v3 = ( *vertices )[static_cast<std::size_t>( i3 )];
            osg::vec3& v4 = ( *vertices )[static_cast<std::size_t>( i4 )];

            if( osg::length( v1 - screenCenter ) > screenRadius )
            {
                continue;
            }
            if( osg::length( v2 - screenCenter ) > screenRadius )
            {
                continue;
            }
            if( osg::length( v3 - screenCenter ) > screenRadius )
            {
                continue;
            }
            if( osg::length( v4 - screenCenter ) > screenRadius )
            {
                continue;
            }

            elements->push_back( static_cast<unsigned short>( i1 ) );
            elements->push_back( static_cast<unsigned short>( i2 ) );
            elements->push_back( static_cast<unsigned short>( i3 ) );

            elements->push_back( static_cast<unsigned short>( i1 ) );
            elements->push_back( static_cast<unsigned short>( i3 ) );
            elements->push_back( static_cast<unsigned short>( i4 ) );
        }
    }

    return geometry;
}

void
PanoramicSphericalDisplay::configure( osgViewer::View& view ) const
{
    OSG_INFO << "PanoramicSphericalDisplay::configure(rad=" << _radius
             << ", cllr=" << _collar << ", sn=" << _screenNum
             << ", im=" << _intensityMap.get() << ")" << std::endl;

    osg::GraphicsContext::WindowingSystemInterface* wsi =
        osg::GraphicsContext::getWindowingSystemInterface();
    if( !wsi )
    {
        OSG_NOTICE << "Error, no WindowSystemInterface available, cannot create windows."
                   << std::endl;
        return;
    }

    osg::GraphicsContext::ScreenIdentifier si;
    si.readDISPLAY();

    // displayNum has not been set so reset it to 0.
    if( si.displayNum < 0 )
    {
        si.displayNum = 0;
    }

    si.screenNum = static_cast<int>( _screenNum );

    unsigned int width, height;
    wsi->getScreenResolution( si, width, height );

    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->hostName                                  = si.hostName;
    traits->displayNum                                = si.displayNum;
    traits->screenNum                                 = si.screenNum;
    traits->x                                         = 0;
    traits->y                                         = 0;
    traits->width                                     = static_cast<int>( width );
    traits->height                                    = static_cast<int>( height );
    traits->windowDecoration                          = false;
    traits->doubleBuffer                              = true;
    traits->sharedContext                             = 0;

    bool                               applyIntensityMapAsColours = true;

    osg::ref_ptr<osg::GraphicsContext> gc =
        osg::GraphicsContext::createGraphicsContext( traits.get() );
    if( !gc )
    {
        OSG_NOTICE << "GraphicsWindow has not been created successfully." << std::endl;
        return;
    }

    int                    tex_width     = static_cast<int>( width );
    int                    tex_height    = static_cast<int>( height );

    int                    camera_width  = tex_width;
    int                    camera_height = tex_height;

    osg::TextureRectangle* texture       = new osg::TextureRectangle;

    texture->setTextureSize( tex_width, tex_height );
    texture->setInternalFormat( GL_RGB );
    texture->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR );
    texture->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );
    texture->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
    texture->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );

    // front face
    {
#if 0
            osg::Camera::RenderTargetImplementation renderTargetImplementation = osg::Camera::SEPERATE_WINDOW;
            GLenum buffer = GL_FRONT;
#else
        osg::Camera::RenderTargetImplementation renderTargetImplementation =
            osg::Camera::FRAME_BUFFER_OBJECT;
        GLenum buffer = GL_FRONT;
#endif

        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setName( "Front face camera" );
        camera->setGraphicsContext( gc.get() );
        camera->setViewport( new osg::Viewport( 0, 0, camera_width, camera_height ) );
        camera->setDrawBuffer( buffer );
        camera->setReadBuffer( buffer );
        camera->setAllowEventFocus( false );
        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation( renderTargetImplementation );

        // attach the texture and use it as the color buffer.
        camera->attach( osg::Camera::COLOR_BUFFER, texture );

        view.addSlave( camera.get(), osg::dmat4(), osg::dmat4() );
    }

    // distortion correction set up.
    {
        osg::Geode* geode = new osg::Geode();
        geode->addDrawable( createParoramicSphericalDisplayDistortionMesh(
            osg::vec3( 0.0F, 0.0F, 0.0F ),
            osg::vec3( static_cast<float>( width ), 0.0F, 0.0F ),
            osg::vec3( 0.0F, static_cast<float>( height ), 0.0F ),
            _radius,
            _collar,
            applyIntensityMapAsColours ? _intensityMap.get() : 0,
            _projectorMatrix
        ) );

        // new we need to add the texture to the mesh, we do so by creating a
        // StateSet to contain the Texture StateAttribute.
        osg::StateSet* stateset = geode->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes( 0, texture, osg::StateAttribute::ON );
        stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

        if( !applyIntensityMapAsColours && _intensityMap.valid() )
        {
            stateset->setTextureAttributeAndModes(
                1,
                new osg::Texture2D( _intensityMap.get() ),
                osg::StateAttribute::ON
            );
        }

        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext( gc.get() );
        camera->setClearMask( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
        camera->setClearColor( osg::vec4( 0.0, 0.0, 0.0, 1.0 ) );
        camera->setViewport( new osg::Viewport( 0, 0, width, height ) );

        GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer( buffer );
        camera->setReadBuffer( buffer );
        camera->setReferenceFrame( osg::Camera::ABSOLUTE_RF );
        camera->setAllowEventFocus( false );
        camera->setInheritanceMask( camera->getInheritanceMask() &
                                    ~osg::CullSettings::CLEAR_COLOR &
                                    ~osg::CullSettings::COMPUTE_NEAR_FAR_MODE );
        // camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

        camera->setProjectionMatrixAsOrtho2D( 0, width, 0, height );
        camera->setViewMatrix( osg::dmat4() );

        // add subgraph to render
        camera->addChild( geode );

        camera->setName( "DistortionCorrectionCamera" );

        view.addSlave( camera.get(), osg::dmat4(), osg::dmat4(), false );
    }
}
