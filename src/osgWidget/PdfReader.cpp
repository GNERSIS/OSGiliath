/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * PDF document viewer widget. Renders PDF pages as
 * textures within the widget system.
 */
#include <osgWidget/PdfReader.hpp>

#include <osg/core/io_utils.hpp>
#include <osg/nodes/Geode.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

using namespace osgWidget;

PdfReader::PdfReader( const std::string&   filename,
                      const GeometryHints& hints )
{
    open( filename, hints );
}

bool
PdfReader::assign( PdfImage*            pdfImage,
                   const GeometryHints& hints )
{
    if( !pdfImage )
    {
        return false;
    }

    _pdfImage = pdfImage;
    _pdfImage->setBackgroundColor( hints.backgroundColor );

    bool      flip        = _pdfImage->getOrigin() == osg::Image::TOP_LEFT;

    float     aspectRatio = ( _pdfImage->t() > 0 && _pdfImage->s() > 0 )
                              ? float( _pdfImage->t() ) / float( _pdfImage->s() )
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

    osg::Texture2D* texture     = new osg::Texture2D( _pdfImage.get() );
    texture->setResizeNonPowerOfTwoHint( false );
    texture->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR );
    texture->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
    texture->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );

    pictureQuad->getOrCreateStateSet()
        ->setTextureAttributeAndModes( 0, texture, osg::StateAttribute::ON );

    osg::ref_ptr<osgViewer::InteractiveImageHandler> iih =
        new osgViewer::InteractiveImageHandler( _pdfImage.get() );

    pictureQuad->setEventCallback( iih.get() );
    pictureQuad->setCullCallback( iih.get() );

    addDrawable( pictureQuad );

    return true;
}

bool
PdfReader::open( const std::string&   filename,
                 const GeometryHints& hints )
{
    osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile( filename );
    return assign( dynamic_cast<PdfImage*>( image.get() ), hints );
}

bool
PdfReader::page( int pageNum )
{
    if( !_pdfImage )
    {
        return false;
    }

    return _pdfImage->page( pageNum );
}

bool
PdfReader::previous()
{
    if( !_pdfImage )
    {
        return false;
    }

    return _pdfImage->previous();
}

bool
PdfReader::next()
{
    if( !_pdfImage )
    {
        return false;
    }

    return _pdfImage->next();
}
