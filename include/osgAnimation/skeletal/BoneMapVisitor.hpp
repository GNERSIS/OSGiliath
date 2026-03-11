/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Visitor that collects all Bone nodes into a name-indexed map.
 * Used to resolve bone references during skeleton setup.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osgAnimation/core/Export.hpp>
#include <osgAnimation/skeletal/Bone.hpp>

namespace osgAnimation
{

    class OSGANIMATION_EXPORT BoneMapVisitor : public osg::DualModeVisitor
    {
        public:

            using osg::DualModeVisitor::apply;

            OSG_REGISTER_TYPE(osgAnimation, BoneMapVisitor)BoneMapVisitor();

            void
            apply( osg::Node& );
            void
            apply( osg::Transform& node );
            const BoneMap&
            getBoneMap() const;

        protected:

            BoneMap _map;
    };

}
