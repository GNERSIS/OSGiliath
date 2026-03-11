/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract base for scene graph transform nodes. Defines the
 * interface for computing local-to-world and world-to-local matrices.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/nodes/Group.hpp>

namespace osg
{

    /** Compute the matrix which transforms objects in local coords to world coords,
     * by accumulating the Transform local to world matrices along the specified node
     * path.
     */
    extern OSG_EXPORT dmat4
    computeLocalToWorld( const NodePath& nodePath,
                         bool            ignoreCameras = true );

    /** Compute the matrix which transforms objects in world coords to local coords,
     * by accumulating the Transform world to local matrices along the specified node
     * path.
     */
    extern OSG_EXPORT dmat4
    computeWorldToLocal( const NodePath& nodePath,
                         bool            ignoreCameras = true );

    /** Compute the matrix which transforms objects in local coords to eye coords,
     * by accumulating the Transform local to world matrices along the specified node
     * path and multiplying by the supplied initial camera modelview.
     */
    extern OSG_EXPORT dmat4
    computeLocalToEye( const dmat4&    modelview,
                       const NodePath& nodePath,
                       bool            ignoreCameras = true );

    /** Compute the matrix which transforms objects in eye coords to local coords,
     * by accumulating the Transform world to local matrices along the specified node
     * path and multiplying by the inverse of the supplied initial camera modelview.
     */
    extern OSG_EXPORT dmat4
    computeEyeToLocal( const dmat4&    modelview,
                       const NodePath& nodePath,
                       bool            ignoreCameras = true );

    /** A Transform is a group node for which all children are transformed by
     * a 4x4 matrix. It is often used for positioning objects within a scene,
     * producing trackball functionality or for animation.
     *
     * Transform itself does not provide set/get functions, only the interface
     * for defining what the 4x4 transformation is.  Subclasses, such as
     * MatrixTransform and PositionAttitudeTransform support the use of an
     * osg::dmat4 or a vec3 and quat respectively.
     *
     * Note: If the transformation matrix scales the subgraph then the normals
     * of the underlying geometry will need to be renormalized to be unit
     * vectors once more.  This can be done transparently through OpenGL's
     * use of either GL_NORMALIZE and GL_RESCALE_NORMAL modes. For further
     * background reading see the glNormalize documentation in the OpenGL
     * Reference Guide (the blue book). To enable it in the OSG, you simply
     * need to attach a local osg::StateSet to the osg::Transform, and set
     * the appropriate mode to ON via
     *   stateset->setMode(GL_NORMALIZE, osg::StateAttribute::ON);
     */
    class OSG_EXPORT Transform : public osg::Inherit<Group, Transform>
    {
        public:

            OSG_REGISTER_TYPE( osg,
                               Transform )

            Transform();

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            Transform( const Transform&,
                       const CopyOp& copyop = CopyOp::SHALLOW_COPY );

            virtual Transform*
            asTransform()
            {
                return this;
            }

            virtual const Transform*
            asTransform() const
            {
                return this;
            }

            virtual MatrixTransform*
            asMatrixTransform()
            {
                return 0;
            }

            virtual const MatrixTransform*
            asMatrixTransform() const
            {
                return 0;
            }

            virtual PositionAttitudeTransform*
            asPositionAttitudeTransform()
            {
                return 0;
            }

            virtual const PositionAttitudeTransform*
            asPositionAttitudeTransform() const
            {
                return 0;
            }

            virtual AutoTransform*
            asAutoTransform()
            {
                return 0;
            }

            virtual const AutoTransform*
            asAutoTransform() const
            {
                return 0;
            }

            enum ReferenceFrame
            {
                RELATIVE_RF,
                ABSOLUTE_RF,
                ABSOLUTE_RF_INHERIT_VIEWPOINT,
            };

            /** Set the transform's ReferenceFrame, either to be relative to its
             * parent reference frame, or relative to an absolute coordinate
             * frame. RELATIVE_RF is the default.
             * Note: Setting the ReferenceFrame to be ABSOLUTE_RF will
             * also set the CullingActive flag on the transform, and hence all
             * of its parents, to false, thereby disabling culling of it and
             * all its parents.  This is necessary to prevent inappropriate
             * culling, but may impact cull times if the absolute transform is
             * deep in the scene graph.  It is therefore recommended to only use
             * absolute Transforms at the top of the scene, for such things as
             * heads up displays.
             * ABSOLUTE_RF_INHERIT_VIEWPOINT is the same as ABSOLUTE_RF except it
             * adds the ability to use the parents view points position in world
             * coordinates as its local viewpoint in the new coordinates frame.  This is
             * useful for Render to texture Cameras that wish to use the main views LOD
             * range computation (which uses the viewpoint rather than the eye point)
             * rather than use the local eye point defined by the this Transforms'
             * absolute view matrix.
             */
            void
            setReferenceFrame( ReferenceFrame rf );

            ReferenceFrame
            getReferenceFrame() const
            {
                return _referenceFrame;
            }

            virtual bool
            computeLocalToWorldMatrix( dmat4& matrix,
                                       NodeVisitor* ) const
            {
                if( _referenceFrame == RELATIVE_RF )
                {
                    return false;
                }
                else    // absolute
                {
                    matrix = osg::dmat4();
                    return true;
                }
            }

            virtual bool
            computeWorldToLocalMatrix( dmat4& matrix,
                                       NodeVisitor* ) const
            {
                if( _referenceFrame == RELATIVE_RF )
                {
                    return false;
                }
                else    // absolute
                {
                    matrix = osg::dmat4();
                    return true;
                }
            }

            /** Overrides Group's computeBound.
             * There is no need to override in subclasses from osg::Transform
             * since this computeBound() uses the underlying matrix (calling
             * computeMatrix if required).
             */
            virtual sphere
            computeBound() const;

        protected:

            virtual ~Transform();

            ReferenceFrame _referenceFrame;
    };

}
