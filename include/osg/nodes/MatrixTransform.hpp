/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Transform node that positions children using an explicit 4x4 matrix.
 * Used for static transforms, animation callbacks, and scene positioning.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/maths/transform.hpp>
#include <osg/nodes/Transform.hpp>

namespace osg
{

    /** MatrixTransform - is a subclass of Transform which has an osg::dmat4
     * which represents a 4x4 transformation of its children from local coordinates
     * into the Transform's parent coordinates.
     */
    class OSG_EXPORT MatrixTransform : public osg::Inherit<Transform, MatrixTransform>
    {
        public:

            OSG_REGISTER_TYPE( osg,
                               MatrixTransform )

            MatrixTransform();

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            MatrixTransform( const MatrixTransform&,
                             const CopyOp& copyop = CopyOp::SHALLOW_COPY );

            MatrixTransform( const dmat4& matix );

            virtual MatrixTransform*
            asMatrixTransform()
            {
                return this;
            }

            virtual const MatrixTransform*
            asMatrixTransform() const
            {
                return this;
            }

            /** Set the transform's matrix.*/
            void
            setMatrix( const dmat4& mat )
            {
                _matrix       = mat;
                _inverseDirty = true;
                dirtyBound();
            }

            /** Get the matrix. */
            inline const dmat4&
            getMatrix() const
            {
                return _matrix;
            }

            /** pre multiply the transform's matrix.*/
            void
            preMult( const dmat4& mat )
            {
                _matrix       = _matrix * mat;
                _inverseDirty = true;
                dirtyBound();
            }

            /** post multiply the transform's matrix.*/
            void
            postMult( const dmat4& mat )
            {
                _matrix       = mat * _matrix;
                _inverseDirty = true;
                dirtyBound();
            }

            /** Get the inverse matrix. */
            inline const dmat4&
            getInverseMatrix() const
            {
                if( _inverseDirty )
                {
                    _inverse      = osg::inverse( _matrix );
                    _inverseDirty = false;
                }
                return _inverse;
            }

            virtual bool
            computeLocalToWorldMatrix( dmat4& matrix,
                                       NodeVisitor* ) const;

            virtual bool
            computeWorldToLocalMatrix( dmat4& matrix,
                                       NodeVisitor* ) const;

        protected:

            virtual ~MatrixTransform();

            dmat4         _matrix;
            mutable dmat4 _inverse;
            mutable bool  _inverseDirty;
    };

}
