/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Traversal visitor that collects occluder nodes from the scene.
 * Gathers ConvexPlanarOccluders for software occlusion culling.
 */
#pragma once

#include <osg/traversal/CullStack.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <set>

namespace osg
{

    class OSG_EXPORT CollectOccludersVisitor : public osg::DualModeVisitor,
                                               public osg::CullStack
    {
        public:

            typedef std::set<ShadowVolumeOccluder> ShadowVolumeOccluderSet;

            CollectOccludersVisitor();
            virtual ~CollectOccludersVisitor();

            OSG_REGISTER_TYPE( osg,
                               CollectOccludersVisitor )

            virtual Object*
            cloneType() const
            {
                return new CollectOccludersVisitor();
            }

            virtual void
            reset();

            virtual float
            getDistanceToEyePoint( const vec3& pos,
                                   bool        withLODScale ) const;
            virtual float
            getDistanceToViewPoint( const vec3& pos,
                                    bool        withLODScale ) const;

            virtual float
            getDistanceFromEyePoint( const vec3& pos,
                                     bool        withLODScale ) const;

            using ConstNodeVisitor::apply;
            using NodeVisitor::apply;

            virtual void
            apply( osg::Node& );
            virtual void
            apply( osg::Transform& node );
            virtual void
            apply( osg::Projection& node );

            virtual void
            apply( osg::Switch& node );
            virtual void
            apply( osg::LOD& node );
            virtual void
            apply( osg::OccluderNode& node );

            /** Sets the minimum shadow occluder volume that an active occluder
             * must have. vol is units relative the clip space volume where 1.0
             * is the whole clip space. */
            void
            setMinimumShadowOccluderVolume( float vol )
            {
                _minimumShadowOccluderVolume = vol;
            }

            float
            getMinimumShadowOccluderVolume() const
            {
                return _minimumShadowOccluderVolume;
            }

            /** Sets the maximum number of occluders to have active for culling
             * purposes. */
            void
            setMaximumNumberOfActiveOccluders( unsigned int num )
            {
                _maximumNumberOfActiveOccluders = num;
            }

            unsigned int
            getMaximumNumberOfActiveOccluders() const
            {
                return _maximumNumberOfActiveOccluders;
            }

            void
            setCreateDrawablesOnOccludeNodes( bool flag )
            {
                _createDrawables = flag;
            }

            bool
            getCreateDrawablesOnOccludeNodes() const
            {
                return _createDrawables;
            }

            void
            setCollectedOccluderSet( const ShadowVolumeOccluderSet& svol )
            {
                _occluderSet = svol;
            }

            ShadowVolumeOccluderSet&
            getCollectedOccluderSet()
            {
                return _occluderSet;
            }

            const ShadowVolumeOccluderSet&
            getCollectedOccluderSet() const
            {
                return _occluderSet;
            }

            /** Removes occluded occluders for the collected occluders list, then
             * discards all but MaximumNumberOfActiveOccluders of occluders,
             * discarding the occluders with the lowest shadow occluder volume. */
            void
            removeOccludedOccluders();

        protected:

            /** Prevents unwanted copy construction. */
            // CollectOccludersVisitor(const
            // CollectOccludersVisitor&):osg::NodeVisitor(),osg::CullStack() {}

            /** Prevents unwanted copy operator. */
            CollectOccludersVisitor&
            operator=( const CollectOccludersVisitor& )
            {
                return *this;
            }

            inline void
            handle_cull_callbacks_and_traverse( osg::Node& node )
            {
                /*osg::NodeCallback* callback = node.getCullCallback();
                if (callback) (*callback)(&node,this);
                else*/
                if( node.getNumChildrenWithOccluderNodes() > 0 )
                {
                    traverse( node );
                }
            }

            inline void
            handle_cull_callbacks_and_accept( osg::Node& node,
                                              osg::Node* acceptNode )
            {
                /*osg::NodeCallback* callback = node.getCullCallback();
                if (callback) (*callback)(&node,this);
                else*/
                if( node.getNumChildrenWithOccluderNodes() > 0 )
                {
                    acceptNode->accept( *this );
                }
            }

            float                   _minimumShadowOccluderVolume;
            unsigned                _maximumNumberOfActiveOccluders;
            bool                    _createDrawables;
            ShadowVolumeOccluderSet _occluderSet;
    };

}
