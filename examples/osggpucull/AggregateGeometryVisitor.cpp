/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * AggregateGeometryVisitor example application
 */
#include "AggregateGeometryVisitor.hpp"

AggregateGeometryVisitor::AggregateGeometryVisitor(
    ConvertTrianglesOperator* ctOperator
) :
    osg::DualModeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
{
    _ctOperator.setConverter( ctOperator );
    init();
}

void
AggregateGeometryVisitor::init()
{
    _aggregatedGeometry = new osg::Geometry;
    _ctOperator.initGeometry( _aggregatedGeometry.get() );
    _matrixStack.clear();
}

AggregateGeometryVisitor::AddObjectResult
AggregateGeometryVisitor::addObject( osg::Node*   object,
                                     unsigned int typeID,
                                     unsigned int lodNumber )
{
    unsigned int currentVertexFirst =
        _aggregatedGeometry->getVertexArray()->getNumElements();
    _currentTypeID    = typeID;
    _currentLodNumber = lodNumber;
    object->accept( *this );
    unsigned int currentVertexCount =
        _aggregatedGeometry->getVertexArray()->getNumElements() - currentVertexFirst;
    _aggregatedGeometry->addPrimitiveSet(
        new osg::DrawArrays( osg::PrimitiveSet::TRIANGLES,
                             currentVertexFirst,
                             currentVertexCount )
    );
    _matrixStack.clear();
    return AddObjectResult( currentVertexFirst,
                            currentVertexCount,
                            _aggregatedGeometry->getNumPrimitiveSets() - 1 );
}

void
AggregateGeometryVisitor::apply( osg::Node& node )
{
    bool pushed = _ctOperator.pushNode( &node );
    traverse( node );
    if( pushed )
    {
        _ctOperator.popNode();
    }
}

void
AggregateGeometryVisitor::apply( osg::Transform& transform )
{
    bool       pushed = _ctOperator.pushNode( &transform );
    osg::dmat4 matrix;
    if( !_matrixStack.empty() )
    {
        matrix = _matrixStack.back();
    }
    transform.computeLocalToWorldMatrix( matrix, this );
    _matrixStack.push_back( matrix );

    traverse( transform );

    _matrixStack.pop_back();
    if( pushed )
    {
        _ctOperator.popNode();
    }
}

void
AggregateGeometryVisitor::apply( osg::Geode& geode )
{
    bool       pushed = _ctOperator.pushNode( &geode );

    osg::dmat4 matrix;
    if( !_matrixStack.empty() )
    {
        matrix = _matrixStack.back();
    }
    for( unsigned int i = 0; i < geode.getNumDrawables(); ++i )
    {
        osg::Geometry* geom = geode.getDrawable( i )->asGeometry();
        if( geom != NULL )
        {
            _ctOperator.setGeometryData( matrix,
                                         geom,
                                         _aggregatedGeometry.get(),
                                         ( float )_currentTypeID,
                                         ( float )_currentLodNumber );
            geom->accept( _ctOperator );
        }
    }

    traverse( geode );
    if( pushed )
    {
        _ctOperator.popNode();
    }
}
