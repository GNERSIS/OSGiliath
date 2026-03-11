/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * VNC remote desktop client widget. Displays a VNC
 * session as a texture within the widget system.
 */
#include <osgWidget/VncClient>

#include <osgDB/io/ReadFile.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

using namespace osgWidget;

VncClient::VncClient( const std::string&   hostname,
                      const GeometryHints& hints )
{
    connect( hostname, hints );
}

bool
VncClient::assign( VncImage*            vncImage,
                   const GeometryHints& hints )
{
    if( !vncImage )
    {
        return false;
    }

    _vncImage             = vncImage;

    bool      flip        = _vncImage->getOrigin() == osg::Image::TOP_LEFT;

    float     aspectRatio = ( _vncImage->t() > 0 && _vncImage->s() > 0 )
                              ? float( _vncImage->t() ) / float( _vncImage->s() )
                              : 1.0;

    osg::vec3 widthVec( hints.widthVec );
    osg::vec3 heightVec( hints.heightVec );

    switch( hints.aspectRatioPolicy )
    {
        case( GeometryHints::RESIZE_HEIGHT_TO_MAINTAINCE_ASPECT_RATIO ) :
            heightVec *= aspectRatio;
            break;
        case( GeometryHints::RESIZE_WIDTH_TO_MAINTAINCE_ASPECT_RATIO ) :
            widthVec /= aspectRatio;
            break;
        default :
            // no need to adjust aspect ratio
            break;
    }

    osg::Geometry*  pictureQuad = osg::createTexturedQuadGeometry( hints.position,
                                                                   widthVec,
                                                                   heightVec,
                                                                   0.0F,
                                                                   flip ? 1.0F : 0.0F,
                                                                   1.0F,
                                                                   flip ? 0.0F : 1.0F );

    osg::Texture2D* texture     = new osg::Texture2D( _vncImage.get() );
    texture->setResizeNonPowerOfTwoHint( false );
    texture->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR );
    texture->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
    texture->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );

    pictureQuad->getOrCreateStateSet()
        ->setTextureAttributeAndModes( 0, texture, osg::StateAttribute::ON );

    osg::ref_ptr<osgViewer::InteractiveImageHandler> iih =
        new osgViewer::InteractiveImageHandler( _vncImage.get() );

    pictureQuad->setEventCallback( iih.get() );
    pictureQuad->setCullCallback( iih.get() );

    addDrawable( pictureQuad );

    return true;
}

bool
VncClient::connect( const std::string&   hostname,
                    const GeometryHints& hints )
{
    osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile( hostname + ".vnc" );
    return assign( dynamic_cast<VncImage*>( image.get() ), hints );
}

void
VncClient::close()
{
    if( !_vncImage )
    {
        return;
    }

    _vncImage->close();
}
