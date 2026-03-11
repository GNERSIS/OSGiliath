/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Automatic shader generation visitor. Creates GLSL shaders
 * matching the fixed-function state set on each Drawable.
 */
/**
 * \brief    Shader generator framework.
 * \author   Maciej Krol
 */

#pragma once

#include <osg/state/State.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osgUtil/Export>

namespace osgUtil
{

    class OSGUTIL_EXPORT ShaderGenVisitor : public osg::DualModeVisitor
    {
        public:

            ShaderGenVisitor();

            /// assign default uber program to specified StateSet - typically the root
            /// node of the scene graph or the view's Camera
            void
            assignUberProgram( osg::StateSet* stateSet );

            using osg::DualModeVisitor::apply;

            void
            apply( osg::Node& node );

            void
            remapStateSet( osg::StateSet* stateSet );

        protected:
    };

}
