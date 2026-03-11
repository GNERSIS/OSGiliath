/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Reference-counted matrix (RefMatrix). Extends mat4 with
 * ref_ptr support for matrix stacks during cull traversal.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/maths/mat4.hpp>

namespace osg
{

    /// RefMatrix — a ref-counted (Object) wrapper around dmat4.
    /// This replaces the old RefMatrixd class.  Old code used bare 'RefMatrix'
    /// which was always the double-precision variant.
    class RefMatrix : public Inherit<Object, RefMatrix>,
                      public dmat4
    {
            using InheritBase = Inherit<Object, RefMatrix>;

        public:

            OSG_REGISTER_TYPE( osg,
                               RefMatrix )

            RefMatrix() :
                InheritBase(),
                dmat4()
            {
            }

            RefMatrix( const dmat4& m ) :
                InheritBase(),
                dmat4( m )
            {
            }

            RefMatrix( const mat4& m ) :
                InheritBase(),
                dmat4( m )
            {
            }

            RefMatrix( const RefMatrix& rm,
                       const CopyOp&    copyop = CopyOp::SHALLOW_COPY ) :
                InheritBase( rm,
                             copyop ),
                dmat4( static_cast<const dmat4&>( rm ) )
            {
            }

            /// Forward to any multi-argument dmat4 constructor (e.g. 16-value ctor).
            template<typename... Args,
                     std::enable_if_t<( sizeof...( Args ) > 1 ),
                                      int> = 0>
            explicit RefMatrix( Args&&... args ) :
                InheritBase(),
                dmat4( std::forward<Args>( args )... )
            {
            }

            using dmat4::operator=;
            using dmat4::operator();
            using dmat4::operator[];

        protected:

            virtual ~RefMatrix()
            {
            }
    };

    // Legacy aliases
    using RefDMat4 = RefMatrix;

}    // namespace osg
