/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Text rendering settings for UI widgets. Specifies
 * font, character size, and text color for widget text.
 */
#include <osgUI/TextSettings.hpp>

#include <osg/nodes/Geode.hpp>
#include <osgText/Text.hpp>

using namespace osgUI;

TextSettings::TextSettings() :
    _characterSize( 1.0 )
{
}

TextSettings::TextSettings( const TextSettings& textSettings,
                            const osg::CopyOp&  copyop ) :
    Inherit( textSettings,
             copyop ),
    _font( textSettings._font ),
    _characterSize( textSettings._characterSize )
{
}
