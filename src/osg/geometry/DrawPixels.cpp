/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Legacy glDrawPixels wrapper. Retained for API compat but
 * non-functional in Core Profile (use textured quads instead).
 */
#include <osg/geometry/DrawPixels.hpp>

using namespace osg;

DrawPixels::DrawPixels()
{
    // turn off display lists right now, just in case we want to modify the projection
    // matrix along the way.

    _position.set( 0.0F, 0.0F, 0.0F );

    _useSubImage = false;
    _offsetX     = 0;
    _offsetY     = 0;
    _width       = 0;
    _height      = 0;
}

DrawPixels::DrawPixels( const DrawPixels& drawimage,
                        const CopyOp&     copyop ) :
    Drawable( drawimage,
              copyop ),
    _position( drawimage._position ),
    _image( drawimage._image ),
    _useSubImage( drawimage._useSubImage ),
    _offsetX( drawimage._offsetX ),
    _offsetY( drawimage._offsetY ),
    _width( drawimage._width ),
    _height( drawimage._height )
{
}

DrawPixels::~DrawPixels()
{
    // image will delete itself thanks to ref_ptr :-)
}

void
DrawPixels::setPosition( const osg::vec3& position )
{
    _position = position;
    dirtyBound();
}

void
DrawPixels::setSubImageDimensions( unsigned int offsetX,
                                   unsigned int offsetY,
                                   unsigned int width,
                                   unsigned int height )
{
    _useSubImage = true;
    _offsetX     = offsetX;
    _offsetY     = offsetY;
    _width       = width;
    _height      = height;
}

void
DrawPixels::getSubImageDimensions( unsigned int& offsetX,
                                   unsigned int& offsetY,
                                   unsigned int& width,
                                   unsigned int& height ) const
{
    offsetX = _offsetX;
    offsetY = _offsetY;
    width   = _width;
    height  = _height;
}

box
DrawPixels::computeBoundingBox() const
{
    // really needs to be dependent of view position and projection... will implement
    // simple version right now.
    box   bbox;
    float diagonal = 0.0F;
    if( _useSubImage )
    {
        diagonal = sqrtf( static_cast<float>( _width * _width + _height * _height ) );
    }
    else
    {
        diagonal = sqrtf(
            static_cast<float>( _image->s() * _image->s() + _image->t() * _image->t() )
        );
    }

    bbox.expandBy( _position - osg::vec3( diagonal, diagonal, diagonal ) );
    bbox.expandBy( _position + osg::vec3( diagonal, diagonal, diagonal ) );
    return bbox;
}

void
DrawPixels::drawImplementation( RenderInfo& ) const
{
    OSG_NOTICE << "Warning: DrawPixels::drawImplementation(RenderInfo&) - not supported."
               << std::endl;
}
