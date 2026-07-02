/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Alignment configuration for UI widgets. Specifies
 * horizontal and vertical alignment within a container.
 */
#include <osg/nodes/Geode.hpp>
#include <osgText/Text.hpp>
#include <osgUI/Style.hpp>

using namespace osgUI;

AlignmentSettings::AlignmentSettings( AlignmentSettings::Alignment alignment ) :
    _alignment( alignment )
{
}

AlignmentSettings::AlignmentSettings( const AlignmentSettings& alingmentSettings,
                                      const osg::CopyOp&       copyop ) :
    Inherit( alingmentSettings,
             copyop ),
    _alignment( alingmentSettings._alignment )
{
}
