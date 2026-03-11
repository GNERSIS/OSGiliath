/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Traversal visitor that computes the aggregate bounding box
 * of a subgraph in world coordinates.
 */
#include <osg/traversal/ComputeBoundsVisitor.hpp>

#include <osg/geometry/Drawable.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/Transform.hpp>

using namespace osg;

ComputeBoundsVisitor::ComputeBoundsVisitor( TraversalMode traversalMode ) :
    osg::DualModeVisitor( traversalMode )
{
}

void
ComputeBoundsVisitor::reset()
{
    _matrixStack.clear();
    _bb.init();
}

void
ComputeBoundsVisitor::getPolytope( osg::Polytope& polytope,
                                   float          margin ) const
{
    float delta = _bb.radius() * margin;
    polytope.add( osg::Plane( 0.0, 0.0, 1.0, -( _bb.zMin() - delta ) ) );
    polytope.add( osg::Plane( 0.0, 0.0, -1.0, _bb.zMax() + delta ) );

    polytope.add( osg::Plane( 1.0, 0.0, 0.0, -( _bb.xMin() - delta ) ) );
    polytope.add( osg::Plane( -1.0, 0.0, 0.0, _bb.xMax() + delta ) );

    polytope.add( osg::Plane( 0.0, 1.0, 0.0, -( _bb.yMin() - delta ) ) );
    polytope.add( osg::Plane( 0.0, -1.0, 0.0, _bb.yMax() + delta ) );
}

void
ComputeBoundsVisitor::getBase( osg::Polytope& polytope,
                               float          margin ) const
{
    float delta = _bb.radius() * margin;
    polytope.add( osg::Plane( 0.0, 0.0, 1.0, -( _bb.zMin() - delta ) ) );
}

void
ComputeBoundsVisitor::apply( osg::Transform& transform )
{
    osg::dmat4 matrix;
    if( !_matrixStack.empty() )
    {
        matrix = _matrixStack.back();
    }

    transform.computeLocalToWorldMatrix( matrix, this );

    pushMatrix( matrix );

    traverse( transform );

    popMatrix();
}

void
ComputeBoundsVisitor::apply( osg::Drawable& drawable )
{
    applyBoundingBox( drawable.getBoundingBox() );
}

void
ComputeBoundsVisitor::applyBoundingBox( const osg::box& bbox )
{
    if( _matrixStack.empty() )
    {
        _bb.expandBy( bbox );
    }
    else if( bbox.valid() )
    {
        const osg::dmat4& matrix = _matrixStack.back();
        _bb.expandBy( vec3( matrix * bbox.corner( 0 ) ) );
        _bb.expandBy( vec3( matrix * bbox.corner( 1 ) ) );
        _bb.expandBy( vec3( matrix * bbox.corner( 2 ) ) );
        _bb.expandBy( vec3( matrix * bbox.corner( 3 ) ) );
        _bb.expandBy( vec3( matrix * bbox.corner( 4 ) ) );
        _bb.expandBy( vec3( matrix * bbox.corner( 5 ) ) );
        _bb.expandBy( vec3( matrix * bbox.corner( 6 ) ) );
        _bb.expandBy( vec3( matrix * bbox.corner( 7 ) ) );
    }
}
