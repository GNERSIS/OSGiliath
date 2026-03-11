/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Scene graph node holding an occluder for occlusion culling.
 * Placed in the scene to define occluding geometry.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/geometry/ConvexPlanarOccluder.hpp>
#include <osg/nodes/Group.hpp>

namespace osg
{

    /**
     * OccluderNode is a Group node which provides hooks for adding
     * ConvexPlanarOccluders to the scene.
     */
    class OSG_EXPORT OccluderNode : public osg::Inherit<Group, OccluderNode>
    {
        public:

            OSG_REGISTER_TYPE( osg,
                               OccluderNode )

            OccluderNode();

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            OccluderNode( const OccluderNode&,
                          const CopyOp& copyop = CopyOp::SHALLOW_COPY );

            /** Convert 'this' into an OccluderNode pointer if Node is an OccluderNode,
             * otherwise return 0. Equivalent to dynamic_cast<OccluderNode*>(this).*/
            virtual OccluderNode*
            asOccluderNode()
            {
                return this;
            }

            /** Convert 'const this' into a const OccluderNode pointer if Node is an
             * OccluderNode, otherwise return 0. Equivalent to dynamic_cast<const
             * OccluderNode*>(this).*/
            virtual const OccluderNode*
            asOccluderNode() const
            {
                return this;
            }

            /** Attach a ConvexPlanarOccluder to an OccluderNode.*/
            void
            setOccluder( ConvexPlanarOccluder* occluder )
            {
                _occluder = occluder;
            }

            /** Get the ConvexPlanarOccluder* attached to a OccluderNode. */
            ConvexPlanarOccluder*
            getOccluder()
            {
                return _occluder.get();
            }

            /** Get the const ConvexPlanarOccluder* attached to a OccluderNode.*/
            const ConvexPlanarOccluder*
            getOccluder() const
            {
                return _occluder.get();
            }

            /** Overrides Group's computeBound.*/
            virtual sphere
            computeBound() const;

        protected:

            virtual ~OccluderNode()
            {
            }

            ref_ptr<ConvexPlanarOccluder> _occluder;
    };

}
