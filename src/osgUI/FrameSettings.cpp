/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Frame decoration settings for dialog/window widgets.
 * Controls title bar visibility, border style, and close button.
 */
#include <osgUI/FrameSettings>

using namespace osgUI;

FrameSettings::FrameSettings() :
    _shape( FrameSettings::NO_FRAME ),
    _shadow( FrameSettings::PLAIN ),
    _lineWidth( 0.01F )
{
}

FrameSettings::FrameSettings( const FrameSettings& frameSettings,
                              const osg::CopyOp&   copyop ) :
    Inherit( frameSettings,
             copyop ),
    _shape( frameSettings._shape ),
    _shadow( frameSettings._shadow ),
    _lineWidth( frameSettings._lineWidth )
{
}
