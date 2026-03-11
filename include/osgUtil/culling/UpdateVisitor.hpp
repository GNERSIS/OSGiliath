/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Default update traversal visitor. Runs update callbacks on
 * nodes and statesets each frame before culling.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/lighting/LightSource.hpp>
#include <osg/nodes/Billboard.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/LOD.hpp>
#include <osg/nodes/OccluderNode.hpp>
#include <osg/nodes/Projection.hpp>
#include <osg/nodes/Switch.hpp>
#include <osg/nodes/Transform.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osg/traversal/ScriptEngine.hpp>
#include <osgUtil/Export>

namespace osgUtil
{

    /**
     * Basic UpdateVisitor implementation for animating a scene.
     * This visitor traverses the scene graph, calling each nodes appCallback if
     * it exists.
     */
    class OSGUTIL_EXPORT UpdateVisitor : public osg::DualModeVisitor
    {
        public:

            UpdateVisitor();
            virtual ~UpdateVisitor();

            OSG_REGISTER_TYPE( osgUtil,
                               UpdateVisitor )

            /** Convert 'this' into a osgUtil::UpdateVisitor pointer if Object is a
             * osgUtil::UpdateVisitor, otherwise return 0. Equivalent to
             * dynamic_cast<osgUtil::UpdateVisitor*>(this).*/
            virtual osgUtil::UpdateVisitor*
            asUpdateVisitor()
            {
                return this;
            }

            /** convert 'const this' into a const osgUtil::UpdateVisitor pointer if
             * Object is a osgUtil::UpdateVisitor, otherwise return 0. Equivalent to
             * dynamic_cast<const osgUtil::UpdateVisitor*>(this).*/
            virtual const osgUtil::UpdateVisitor*
            asUpdateVisitor() const
            {
                return this;
            }

            virtual void
            reset();

            using osg::ConstNodeVisitor::apply;
            using osg::NodeVisitor::apply;

            /** During traversal each type of node calls its callbacks and its children
             * traversed. */
            virtual void
            apply( osg::Node& node )
            {
                handle_callbacks_and_traverse( node );
            }

            virtual void
            apply( osg::Drawable& drawable )
            {
                osg::Callback* callback = drawable.getUpdateCallback();
                if( callback )
                {
                    osg::DrawableUpdateCallback* drawable_callback =
                        callback->asDrawableUpdateCallback();
                    osg::NodeCallback* node_callback = callback->asNodeCallback();

                    if( drawable_callback )
                    {
                        drawable_callback->update( this, &drawable );
                    }
                    if( node_callback )
                    {
                        ( *node_callback )( &drawable, this );
                    }

                    if( !drawable_callback && !node_callback )
                    {
                        callback->run( &drawable, this );
                    }
                }

                handle_callbacks( drawable.getStateSet() );
            }

            // The following overrides are technically redundant as the default
            // implementation would eventually trickle down to apply(osg::Node&); -
            // however defining these explicitly should save a couple of virtual function
            // calls
            virtual void
            apply( osg::Geode& node )
            {
                handle_callbacks_and_traverse( node );
            }

            virtual void
            apply( osg::Billboard& node )
            {
                handle_callbacks_and_traverse( node );
            }

            virtual void
            apply( osg::LightSource& node )
            {
                handle_callbacks_and_traverse( node );
            }

            virtual void
            apply( osg::Group& node )
            {
                handle_callbacks_and_traverse( node );
            }

            virtual void
            apply( osg::Transform& node )
            {
                handle_callbacks_and_traverse( node );
            }

            virtual void
            apply( osg::Projection& node )
            {
                handle_callbacks_and_traverse( node );
            }

            virtual void
            apply( osg::Switch& node )
            {
                handle_callbacks_and_traverse( node );
            }

            virtual void
            apply( osg::LOD& node )
            {
                handle_callbacks_and_traverse( node );
            }

            virtual void
            apply( osg::OccluderNode& node )
            {
                handle_callbacks_and_traverse( node );
            }

        protected:

            // /** Prevent unwanted copy construction.*/
            // UpdateVisitor(const UpdateVisitor&):osg::NodeVisitor() {}

            /** Prevent unwanted copy operator.*/
            UpdateVisitor&
            operator=( const UpdateVisitor& )
            {
                return *this;
            }

            inline void
            handle_callbacks( osg::StateSet* stateset )
            {
                if( stateset && stateset->requiresUpdateTraversal() )
                {
                    stateset->runUpdateCallbacks( this );
                }
            }

            inline void
            handle_callbacks_and_traverse( osg::Node& node )
            {
                handle_callbacks( node.getStateSet() );

                osg::Callback* callback = node.getUpdateCallback();
                if( callback )
                {
                    callback->run( &node, this );
                }
                else if( node.getNumChildrenRequiringUpdateTraversal() > 0 )
                {
                    traverse( node );
                }
            }
    };

}
