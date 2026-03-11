/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Update callback animating Material properties (diffuse,
 * ambient, etc.) from animation channels.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/state/Material.hpp>
#include <osg/state/StateAttribute.hpp>
#include <osgAnimation/core/AnimationUpdateCallback.hpp>
#include <osgAnimation/core/Export.hpp>

namespace osgAnimation
{

    class OSGANIMATION_EXPORT UpdateMaterial
        : public osg::Inherit<AnimationUpdateCallback<osg::StateAttributeCallback>,
                              UpdateMaterial>
    {
        protected:

            osg::ref_ptr<Vec4Target> _diffuse;

        public:

            OSG_REGISTER_TYPE( osgAnimation,
                               UpdateMaterial )

            UpdateMaterial( const std::string& name = "" );
            UpdateMaterial( const UpdateMaterial& apc,
                            const osg::CopyOp&    copyop );

            /** Callback method called by the NodeVisitor when visiting a node.*/
            virtual void
            operator()( osg::StateAttribute*,
                        osg::NodeVisitor* );
            void
            update( osg::Material& material );
            bool
            link( Channel* channel );
            Vec4Target*
            getDiffuse();
    };

}
