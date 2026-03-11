/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Overrides the projection matrix for its subgraph.
 * Used for HUD overlays and orthographic sub-scenes.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/nodes/Group.hpp>

namespace osg
{

    /** Projection nodes set up the frustum/orthographic projection used when rendering
     * the scene.
     */
    class OSG_EXPORT Projection : public osg::Inherit<Group, Projection>
    {
        public:

            OSG_REGISTER_TYPE( osg,
                               Projection )

            Projection();

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            Projection( const Projection&,
                        const CopyOp& copyop = CopyOp::SHALLOW_COPY );

            Projection( const dmat4& matix );

            /** Set the transform's matrix.*/
            void
            setMatrix( const dmat4& mat )
            {
                _matrix = mat;
            }

            /** Get the transform's matrix. */
            inline const dmat4&
            getMatrix() const
            {
                return _matrix;
            }

            /** preMult transform.*/
            void
            preMult( const dmat4& mat )
            {
                _matrix = _matrix * mat;
            }

            /** postMult transform.*/
            void
            postMult( const dmat4& mat )
            {
                _matrix = mat * _matrix;
            }

        protected:

            virtual ~Projection();

            dmat4 _matrix;
    };

}
