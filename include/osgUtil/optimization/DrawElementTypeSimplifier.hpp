/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Downcasts DrawElements index types (uint32 to uint16/uint8)
 * when possible to reduce index buffer memory.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osgUtil/Export.hpp>

namespace osgUtil
{

    class OSGUTIL_EXPORT DrawElementTypeSimplifier
    {
        public:

            void
            simplify( osg::Geometry& geometry ) const;
    };

    class OSGUTIL_EXPORT DrawElementTypeSimplifierVisitor : public osg::DualModeVisitor
    {
        public:

            using osg::DualModeVisitor::apply;

            DrawElementTypeSimplifierVisitor() :
                osg::DualModeVisitor( osg::DualModeVisitor::TRAVERSE_ALL_CHILDREN )
            {
            }
            OSG_REGISTER_TYPE( osgUtil,
                               DrawElementTypeSimplifierVisitor )

            void
            apply( osg::Geometry& geom );
    };

}
