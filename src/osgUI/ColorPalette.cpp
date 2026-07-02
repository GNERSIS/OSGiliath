/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Color palette widget providing a grid of selectable colors.
 * Used for color picker UIs in 3D scenes.
 */
#include <osgUI/ColorPalette.hpp>

using namespace osgUI;

ColorPalette::ColorPalette()
{
}

ColorPalette::ColorPalette( const ColorPalette& cp,
                            const osg::CopyOp&  copyop ) :
    Inherit( cp,
             copyop ),
    _colors( cp._colors )
{
}
