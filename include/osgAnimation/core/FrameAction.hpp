/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Single-frame callback action. Fired at a specific frame
 * number during timeline playback.
 */
#pragma once

#include <map>
#include <osg/core/ref_ptr.hpp>

namespace osgAnimation
{

    class Action;
    typedef std::pair<unsigned int, osg::ref_ptr<Action>> FrameAction;

}
