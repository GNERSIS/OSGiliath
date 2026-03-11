/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Web browser widget embedding (platform-specific).
 * Displays HTML content within the widget system.
 */
#include <osgWidget/Browser>

#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

using namespace osgWidget;

osg::ref_ptr<BrowserManager>&
BrowserManager::instance()
{
    static osg::ref_ptr<BrowserManager> s_BrowserManager = new BrowserManager;
    return s_BrowserManager;
}

BrowserManager::BrowserManager()
{
    OSG_INFO << "Constructing base BrowserManager" << std::endl;
}

BrowserManager::~BrowserManager()
{
    OSG_INFO << "Destructing base BrowserManager" << std::endl;
}

void
BrowserManager::init( const std::string& application )
{
    _application = application;
}

BrowserImage*
BrowserManager::createBrowserImage( const std::string& /*url*/,
                                    int /*width*/,
                                    int /*height*/ )
{
    OSG_NOTICE << "Cannot create browser" << std::endl;
    return 0;
}

Browser::Browser( const std::string&   url,
                  const GeometryHints& hints )
{
    open( url, hints );
}

bool
Browser::assign( BrowserImage*        browserImage,
                 const GeometryHints& hints )
{
    if( !browserImage )
    {
        return false;
    }

    _browserImage         = browserImage;

    bool      flip        = _browserImage->getOrigin() == osg::Image::TOP_LEFT;

    float     aspectRatio = ( _browserImage->t() > 0 && _browserImage->s() > 0 )
                              ? float( _browserImage->t() ) / float( _browserImage->s() )
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

    osg::Texture2D* texture     = new osg::Texture2D( _browserImage.get() );
    texture->setResizeNonPowerOfTwoHint( false );
    texture->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR );
    texture->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
    texture->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );

    pictureQuad->getOrCreateStateSet()
        ->setTextureAttributeAndModes( 0, texture, osg::StateAttribute::ON );

    osg::ref_ptr<osgViewer::InteractiveImageHandler> handler =
        new osgViewer::InteractiveImageHandler( _browserImage.get() );

    pictureQuad->setEventCallback( handler.get() );
    pictureQuad->setCullCallback( handler.get() );

    addDrawable( pictureQuad );

    return true;
}

bool
Browser::open( const std::string&   hostname,
               const GeometryHints& hints )
{
    osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile( hostname + ".gecko" );
    return assign( dynamic_cast<BrowserImage*>( image.get() ), hints );
}

void
Browser::navigateTo( const std::string& url )
{
    if( _browserImage.valid() )
    {
        _browserImage->navigateTo( url );
    }
}
