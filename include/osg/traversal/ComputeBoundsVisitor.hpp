/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Traversal visitor that computes the aggregate bounding box
 * of a subgraph in world coordinates.
 */
#pragma once

#include <osg/maths/box.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osg/traversal/Polytope.hpp>

namespace osg
{

    class OSG_EXPORT ComputeBoundsVisitor : public osg::DualModeVisitor
    {
        public:

            ComputeBoundsVisitor( TraversalMode traversalMode = TRAVERSE_ALL_CHILDREN );

            OSG_REGISTER_TYPE( osg,
                               ComputeBoundsVisitor )

            virtual void
            reset();

            osg::box&
            getBoundingBox()
            {
                return _bb;
            }

            void
            getPolytope( osg::Polytope& polytope,
                         float          margin = 0.1F ) const;

            void
            getBase( osg::Polytope& polytope,
                     float          margin = 0.1F ) const;

            using ConstNodeVisitor::apply;
            using NodeVisitor::apply;

            void
            apply( osg::Drawable& drawable );

            void
            apply( osg::Transform& transform );

            inline void
            pushMatrix( osg::dmat4& matrix )
            {
                _matrixStack.push_back( matrix );
            }

            inline void
            popMatrix()
            {
                _matrixStack.pop_back();
            }

            void
                                            applyBoundingBox( const osg::box& );

            typedef std::vector<osg::dmat4> MatrixStack;

            const MatrixStack&
            getMatrixStack() const
            {
                return _matrixStack;
            }

        protected:

            MatrixStack _matrixStack;
            osg::box    _bb;
    };

}
