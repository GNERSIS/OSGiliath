/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Cull callback using a cluster normal and deviation angle.
 * Efficiently rejects back-facing terrain patches and tile clusters.
 */
#pragma once

#include <osg/core/Callback.hpp>
#include <osg/core/Inherit.hpp>
#include <osg/geometry/Drawable.hpp>

namespace osg
{

    /** @class ClusterCullingCallback
        @brief Implements cluster culling to cull back facing subgraphs and drawables.
       Derived from Drawable::CullCallback and osg::NodeCallback.

        This culling callback is intended to be attached to a node using the
       setCullCallback method. If the node is a drawable cull(osg::NodeVisitor*,
       osg::Drawable*, osg::State*) otherwise operator()(Node*, NodeVisitor*) will be
       called during the cull traversal.

        To decide whether the node (in case of a drawable) or its children (in case of
       any other node type) are to be culled depends on four parameters:
         - a control point,
         - a normal specified at the control point,
         - a deviation value representing the cosinus of an enclosed angle and
         - a radius describing a sphere around the control point.

        The node is culled if the following two conditions are fulfilled:
         - the distance between the current eye/view point to the control point is larger
       or equal to radius,
         - the cosinus of the enclosed angle between the normal and the vector from the
       control point to the eye/view point is smaller(!) than the specified deviation
       value (normally this value is negative meaning that the enclosed angle between the
       control point and the eye/view point is larger than the angle indirectly specified
       by the deviation value).

        @remark As the deviation is representing the cosine of an enclosed angle its
       value should be within the the interval [-1; 1]. A value of one will cull all
       nodes while a value of -1 will never cull a node. The deviation will normally have
       negative values because then the enclosed angle between the normal and the
                eye/view point is larger than 90 degrees (and therefore the eye sees the
       "back" from the control point).
    */
    class OSG_EXPORT ClusterCullingCallback
        : public osg::Inherit<DrawableCullCallback, ClusterCullingCallback>,
          public NodeCallback
    {
        public:

            ClusterCullingCallback();
            ClusterCullingCallback( const ClusterCullingCallback& ccc,
                                    const CopyOp&                 copyop );
            ClusterCullingCallback( const osg::vec3& controlPoint,
                                    const osg::vec3& normal,
                                    float            deviation,
                                    float            radius = -1.0F );
            ClusterCullingCallback( const osg::Drawable* drawable );

            OSG_REGISTER_TYPE( osg,
                               ClusterCullingCallback )

            // Explicit overrides to resolve diamond ambiguity between
            // Inherit<DrawableCullCallback,CCC> and NodeCallback
            osg::Object*
            cloneType() const override
            {
                return new ClusterCullingCallback();
            }

            osg::Object*
            clone( const osg::CopyOp& copyop ) const override
            {
                return new ClusterCullingCallback( *this, copyop );
            }

            bool
            isSameKindAs( const osg::Object* obj ) const override
            {
                return dynamic_cast<const ClusterCullingCallback*>( obj ) != NULL;
            }

            const char*
            libraryName() const override
            {
                return "osg";
            }

            const char*
            className() const override
            {
                return "ClusterCullingCallback";
            }

            NodeCallback*
            asNodeCallback() override
            {
                return osg::NodeCallback::asNodeCallback();
            }

            const NodeCallback*
            asNodeCallback() const override
            {
                return osg::NodeCallback::asNodeCallback();
            }

            DrawableCullCallback*
            asDrawableCullCallback() override
            {
                return osg::DrawableCullCallback::asDrawableCullCallback();
            }

            const DrawableCullCallback*
            asDrawableCullCallback() const override
            {
                return osg::DrawableCullCallback::asDrawableCullCallback();
            }

            // use the NodeCallbacks implementation of run.
            bool
            run( osg::Object* object,
                 osg::Object* data ) override
            {
                return NodeCallback::run( object, data );
            }

            /** Computes the control point, normal, and deviation from the
             * given drawable contents. */
            void
            computeFrom( const osg::Drawable* drawable );

            /** Transform the ClusterCullingCallback's positional members to a new
             * coordinate frame.*/
            void
            transform( const osg::dmat4& matrix );

            void
            set( const osg::vec3& controlPoint,
                 const osg::vec3& normal,
                 float            deviation,
                 float            radius );

            void
            setControlPoint( const osg::vec3& controlPoint )
            {
                _controlPoint = controlPoint;
            }

            const osg::vec3&
            getControlPoint() const
            {
                return _controlPoint;
            }

            void
            setNormal( const osg::vec3& normal )
            {
                _normal = normal;
            }

            const osg::vec3&
            getNormal() const
            {
                return _normal;
            }

            void
            setRadius( float radius )
            {
                _radius = radius;
            }

            float
            getRadius() const
            {
                return _radius;
            }

            void
            setDeviation( float deviation )
            {
                _deviation = deviation;
            }

            float
            getDeviation() const
            {
                return _deviation;
            }

            bool
            cull( osg::NodeVisitor*,
                  osg::Drawable*,
                  osg::State* ) const override;

            /** Callback method called by the NodeVisitor when visiting a node.*/
            void
            operator()( Node*        node,
                        NodeVisitor* nv ) override;

        protected:

            virtual ~ClusterCullingCallback()
            {
            }

            osg::vec3 _controlPoint;
            osg::vec3 _normal;
            float     _radius;
            float     _deviation;
    };

}
