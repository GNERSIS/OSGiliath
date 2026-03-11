/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * K-d tree spatial index for ray intersection acceleration.
 * Built per-Geometry for fast line-of-sight and picking queries.
 */
#include <osg/geometry/KdTree.hpp>

#include <osg/core/io_utils.hpp>
#include <osg/core/Timer.hpp>
#include <osg/geometry/TemplatePrimitiveIndexFunctor.hpp>
#include <osg/geometry/TriangleIndexFunctor.hpp>
#include <osg/nodes/Geode.hpp>

using namespace osg;

// #define VERBOSE_OUTPUT

////////////////////////////////////////////////////////////////////////////////
//
// BuildKdTree Declarartion - class used for building an single KdTree

struct BuildKdTree
{
        BuildKdTree( KdTree& kdTree ) :
            _kdTree( kdTree )
        {
        }

        typedef std::vector<osg::vec3>    CenterList;
        typedef std::vector<unsigned int> Indices;
        typedef std::vector<unsigned int> AxisStack;

        bool
        build( KdTree::BuildOptions& options,
               osg::Geometry*        geometry );

        void
        computeDivisions( KdTree::BuildOptions& options );

        int
                   divide( KdTree::BuildOptions& options,
                           osg::box&             bb,
                           int                   nodeIndex,
                           unsigned int          level );

        KdTree&    _kdTree;

        osg::box   _bb;
        AxisStack  _axisStack;
        Indices    _primitiveIndices;
        CenterList _centers;

    protected:

        BuildKdTree&
        operator=( const BuildKdTree& )
        {
            return *this;
        }
};

struct PrimitiveIndicesCollector
{
        PrimitiveIndicesCollector() :
            _buildKdTree( 0 )
        {
        }

        inline void
        operator()( unsigned int p0 )
        {
            // OSG_NOTICE<<"    point ("<<p0<<")"<<std::endl;
            const osg::vec3& v0 = ( *( _buildKdTree->_kdTree.getVertices() ) )[p0];

            _buildKdTree->_kdTree.addPoint( p0 );

            osg::box bb;
            bb.expandBy( v0 );

            _buildKdTree->_primitiveIndices.push_back(
                static_cast<unsigned int>( _buildKdTree->_centers.size() )
            );
            _buildKdTree->_centers.push_back( bb.center() );
        }

        inline void
        operator()( unsigned int p0,
                    unsigned int p1 )
        {
            // OSG_NOTICE<<"    line ("<<p0<<", "<<p1<<")"<<std::endl;
            const osg::vec3& v0 = ( *( _buildKdTree->_kdTree.getVertices() ) )[p0];
            const osg::vec3& v1 = ( *( _buildKdTree->_kdTree.getVertices() ) )[p1];

            // discard degenerate points
            if( v0 == v1 )
            {
                // OSG_NOTICE<<"Disgarding degenerate triangle"<<std::endl;
                _buildKdTree->_kdTree._degenerateCount++;
                return;
            }

            _buildKdTree->_kdTree.addLine( p0, p1 );

            osg::box bb;
            bb.expandBy( v0 );
            bb.expandBy( v1 );

            _buildKdTree->_primitiveIndices.push_back(
                static_cast<unsigned int>( _buildKdTree->_centers.size() )
            );
            _buildKdTree->_centers.push_back( bb.center() );
        }

        inline void
        operator()( unsigned int p0,
                    unsigned int p1,
                    unsigned int p2 )
        {
            // OSG_NOTICE<<"    triangle ("<<p0<<", "<<p1<<", "<<p2<<")"<<std::endl;

            const osg::vec3& v0 = ( *( _buildKdTree->_kdTree.getVertices() ) )[p0];
            const osg::vec3& v1 = ( *( _buildKdTree->_kdTree.getVertices() ) )[p1];
            const osg::vec3& v2 = ( *( _buildKdTree->_kdTree.getVertices() ) )[p2];

            // discard degenerate points
            if( v0 == v1 || v1 == v2 || v2 == v0 )
            {
                // OSG_NOTICE<<"Disgarding degenerate triangle"<<std::endl;
                _buildKdTree->_kdTree._degenerateCount++;
                return;
            }

            _buildKdTree->_kdTree.addTriangle( p0, p1, p2 );

            osg::box bb;
            bb.expandBy( v0 );
            bb.expandBy( v1 );
            bb.expandBy( v2 );

            _buildKdTree->_primitiveIndices.push_back(
                static_cast<unsigned int>( _buildKdTree->_centers.size() )
            );
            _buildKdTree->_centers.push_back( bb.center() );
        }

        inline void
        operator()( unsigned int p0,
                    unsigned int p1,
                    unsigned int p2,
                    unsigned int p3 )
        {
            // OSG_NOTICE<<"    quad ("<<p0<<", "<<p1<<", "<<p2<<",
            // "<<p3<<")"<<std::endl;

            const osg::vec3& v0 = ( *( _buildKdTree->_kdTree.getVertices() ) )[p0];
            const osg::vec3& v1 = ( *( _buildKdTree->_kdTree.getVertices() ) )[p1];
            const osg::vec3& v2 = ( *( _buildKdTree->_kdTree.getVertices() ) )[p2];
            const osg::vec3& v3 = ( *( _buildKdTree->_kdTree.getVertices() ) )[p3];

            // discard degenerate points
            if( v0 == v1 || v1 == v2 || v2 == v0 || v3 == v0 || v3 == v1 || v3 == v2 )
            {
                // OSG_NOTICE<<"Disgarding degenerate quad"<<std::endl;
                _buildKdTree->_kdTree._degenerateCount++;
                return;
            }

            _buildKdTree->_kdTree.addQuad( p0, p1, p2, p3 );

            osg::box bb;
            bb.expandBy( v0 );
            bb.expandBy( v1 );
            bb.expandBy( v2 );
            bb.expandBy( v3 );

            _buildKdTree->_primitiveIndices.push_back(
                static_cast<unsigned int>( _buildKdTree->_centers.size() )
            );
            _buildKdTree->_centers.push_back( bb.center() );
        }

        BuildKdTree* _buildKdTree;
};

////////////////////////////////////////////////////////////////////////////////
//
// BuildKdTree Implementation

bool
BuildKdTree::build( KdTree::BuildOptions& options,
                    osg::Geometry*        geometry )
{

#ifdef VERBOSE_OUTPUT
    OSG_NOTICE << "osg::KDTreeBuilder::createKDTree()" << std::endl;
    146
#endif

        osg::Vec3Array* vertices =
            dynamic_cast<osg::Vec3Array*>( geometry->getVertexArray() );
    if( !vertices )
    {
        return false;
    }

    if( vertices->size() <= options._targetNumTrianglesPerLeaf )
    {
        return false;
    }

    _bb = geometry->getBoundingBox();
    _kdTree.setVertices( vertices );

    unsigned int estimatedSize =
        ( unsigned int )( 2.0 *
                          float( vertices->size() ) /
                          float( options._targetNumTrianglesPerLeaf ) );

#ifdef VERBOSE_OUTPUT
    OSG_NOTICE << "kdTree->_kdNodes.reserve()=" << estimatedSize << std::endl
               << std::endl;
#endif

    _kdTree.getNodes().reserve( estimatedSize * 5 );

    computeDivisions( options );

    options._numVerticesProcessed += vertices->size();

    unsigned int estimatedNumTriangles =
        static_cast<unsigned int>( vertices->size() ) * 2;
    _primitiveIndices.reserve( estimatedNumTriangles );
    _centers.reserve( estimatedNumTriangles );

    osg::TemplatePrimitiveIndexFunctor<PrimitiveIndicesCollector> collectIndices;
    collectIndices._buildKdTree = this;
    geometry->accept( collectIndices );

    _primitiveIndices.reserve( vertices->size() );

    KdTree::KdNode node( -1, static_cast<int>( _primitiveIndices.size() ) );
    node.bb                                = _bb;

    int      nodeNum                       = _kdTree.addNode( node );

    osg::box bb                            = _bb;
    nodeNum                                = divide( options, bb, nodeNum, 0 );

    osg::KdTree::Indices& primitiveIndices = _kdTree.getPrimitiveIndices();

    KdTree::Indices       new_indices;
    new_indices.reserve( _primitiveIndices.size() );
    for( Indices::iterator itr = _primitiveIndices.begin();
         itr != _primitiveIndices.end();
         ++itr )
    {
        new_indices.push_back( primitiveIndices[*itr] );
    }
    primitiveIndices.swap( new_indices );

#ifdef VERBOSE_OUTPUT
    OSG_NOTICE << "Root nodeNum=" << nodeNum << std::endl;
#endif

    // OSG_NOTICE<<"_kdNodes.size()="<<k_kdNodes.size()<<"  estimated size =
    // "<<estimatedSize<<std::endl; OSG_NOTICE<<"_kdLeaves.size()="<<_kdLeaves.size()<<"
    // estimated size = "<<estimatedSize<<std::endl<<std::endl;

    return !_kdTree.getNodes().empty();
}

void
BuildKdTree::computeDivisions( KdTree::BuildOptions& options )
{
    osg::vec3 dimensions( _bb.xMax() - _bb.xMin(),
                          _bb.yMax() - _bb.yMin(),
                          _bb.zMax() - _bb.zMin() );

#ifdef VERBOSE_OUTPUT
    OSG_NOTICE << "computeDivisions(" << options._maxNumLevels << ") " << dimensions
               << " { " << std::endl;
#endif

    _axisStack.reserve( options._maxNumLevels );

    for( unsigned int level = 0; level < options._maxNumLevels; ++level )
    {
        int axis = 0;
        if( dimensions[0] >= dimensions[1] )
        {
            if( dimensions[0] >= dimensions[2] )
            {
                axis = 0;
            }
            else
            {
                axis = 2;
            }
        }
        else if( dimensions[1] >= dimensions[2] )
        {
            axis = 1;
        }
        else
        {
            axis = 2;
        }

        _axisStack.push_back( static_cast<unsigned int>( axis ) );
        dimensions[static_cast<std::size_t>( axis )] /= 2.0F;

#ifdef VERBOSE_OUTPUT
        OSG_NOTICE << "  " << level << ", " << dimensions << ", " << axis << std::endl;
#endif
    }

#ifdef VERBOSE_OUTPUT
    OSG_NOTICE << "}" << std::endl;
#endif
}

int
BuildKdTree::divide( KdTree::BuildOptions& options,
                     osg::box&             bb,
                     int                   nodeIndex,
                     unsigned int          level )
{
    KdTree::KdNode& node         = _kdTree.getNode( nodeIndex );

    bool            needToDivide = level <
                                   _axisStack.size() &&
                                   ( node.first <
                                     0 &&
                                     static_cast<unsigned int>( node.second ) >
                                     options._targetNumTrianglesPerLeaf );

    if( !needToDivide )
    {
        if( node.first < 0 )
        {
            int istart = -node.first - 1;
            int iend   = istart + node.second - 1;

            // leaf is done, now compute bound on it.
            node.bb.init();
            for( int i = istart; i <= iend; ++i )
            {
                unsigned int primitiveIndex =
                    _kdTree.getPrimitiveIndices()
                        [_primitiveIndices[static_cast<std::size_t>( i )]];
                primitiveIndex++;    // skip original Primitive index
                unsigned int numPoints = _kdTree.getVertexIndices()[primitiveIndex++];

                for( ; numPoints > 0; --numPoints )
                {
                    unsigned int     vi = _kdTree.getVertexIndices()[primitiveIndex++];
                    const osg::vec3& v  = ( *_kdTree.getVertices() )[vi];
                    node.bb.expandBy( v );
                }
            }

            if( node.bb.valid() )
            {
                float epsilon  = 1E-6F;
                node.bb.min.x -= epsilon;
                node.bb.min.y -= epsilon;
                node.bb.min.z -= epsilon;
                node.bb.max.x += epsilon;
                node.bb.max.y += epsilon;
                node.bb.max.z += epsilon;
            }

#ifdef VERBOSE_OUTPUT
            if( !node.bb.valid() )
            {
                OSG_NOTICE << "After reset " << node.first << "," << node.second
                           << std::endl;
                OSG_NOTICE << "  bb.min (" << node.bb.min << ")" << std::endl;
                OSG_NOTICE << "  bb.max (" << node.bb.max << ")" << std::endl;
            }
            else
            {
                OSG_NOTICE << "Set bb for nodeIndex = " << nodeIndex << std::endl;
            }
#endif
        }

        return nodeIndex;
    }

    int axis = static_cast<int>( _axisStack[level] );

#ifdef VERBOSE_OUTPUT
    OSG_NOTICE << "divide(" << nodeIndex << ", " << level << "), axis=" << axis
               << std::endl;
#endif

    if( node.first < 0 )
    {
        // leaf node as first <= 0, so look at dividing it.

        int   istart = -node.first - 1;
        int   iend   = istart + node.second - 1;

        // OSG_NOTICE<<"  divide leaf"<<std::endl;

        float original_min            = bb.min[static_cast<std::size_t>( axis )];
        float original_max            = bb.max[static_cast<std::size_t>( axis )];

        float mid                     = ( original_min + original_max ) * 0.5F;

        int   originalLeftChildIndex  = 0;
        int   originalRightChildIndex = 0;
        bool  insitueDivision         = false;

        {
            // osg::Vec3Array* vertices = kdTree._vertices.get();
            int left  = istart;
            int right = iend;

            while( left < right )
            {
                while( left <
                       right &&
                       ( _centers[_primitiveIndices[static_cast<std::size_t>( left )]]
                                 [static_cast<std::size_t>( axis )] <= mid ) )
                {
                    ++left;
                }

                while( left <
                       right &&
                       ( _centers[_primitiveIndices[static_cast<std::size_t>( right )]]
                                 [static_cast<std::size_t>( axis )] > mid ) )
                {
                    --right;
                }

                if( left < right )
                {
                    std::swap( _primitiveIndices[static_cast<std::size_t>( left )],
                               _primitiveIndices[static_cast<std::size_t>( right )] );
                    ++left;
                    --right;
                }
            }

            if( left == right )
            {
                if( _centers[_primitiveIndices[static_cast<std::size_t>( left )]]
                            [static_cast<std::size_t>( axis )] <= mid )
                {
                    ++left;
                }
                else
                {
                    --right;
                }
            }

            KdTree::KdNode leftLeaf( -istart - 1, ( right - istart ) + 1 );
            KdTree::KdNode rightLeaf( -left - 1, ( iend - left ) + 1 );

            if( leftLeaf.second <= 0 )
            {
                // OSG_NOTICE<<"LeftLeaf empty"<<std::endl;
                originalLeftChildIndex = 0;
                // originalRightChildIndex = addNode(rightLeaf);
                originalRightChildIndex = nodeIndex;
                insitueDivision         = true;
            }
            else if( rightLeaf.second <= 0 )
            {
                // OSG_NOTICE<<"RightLeaf empty"<<std::endl;
                //  originalLeftChildIndex = addNode(leftLeaf);
                originalLeftChildIndex  = nodeIndex;
                originalRightChildIndex = 0;
                insitueDivision         = true;
            }
            else
            {
                originalLeftChildIndex  = _kdTree.addNode( leftLeaf );
                originalRightChildIndex = _kdTree.addNode( rightLeaf );
            }
        }

        float restore = bb.max[static_cast<std::size_t>( axis )];
        bb.max[static_cast<std::size_t>( axis )] = mid;

        // OSG_NOTICE<<"  divide leftLeaf "<<kdTree.getNode(nodeNum).first<<std::endl;
        int leftChildIndex = originalLeftChildIndex != 0
                               ? divide( options, bb, originalLeftChildIndex, level + 1 )
                               : 0;

        bb.max[static_cast<std::size_t>( axis )] = restore;

        restore = bb.min[static_cast<std::size_t>( axis )];
        bb.min[static_cast<std::size_t>( axis )] = mid;

        // OSG_NOTICE<<"  divide rightLeaf "<<kdTree.getNode(nodeNum).second<<std::endl;
        int rightChildIndex =
            originalRightChildIndex != 0
                ? divide( options, bb, originalRightChildIndex, level + 1 )
                : 0;

        bb.min[static_cast<std::size_t>( axis )] = restore;

        if( !insitueDivision )
        {
            // take a second reference to node we are working on as the std::vector<>
            // resize could have invalidate the previous node ref.
            KdTree::KdNode& newNodeRef = _kdTree.getNode( nodeIndex );

            newNodeRef.first           = leftChildIndex;
            newNodeRef.second          = rightChildIndex;

            insitueDivision            = true;

            newNodeRef.bb.init();
            if( leftChildIndex != 0 )
            {
                newNodeRef.bb.expandBy( _kdTree.getNode( leftChildIndex ).bb );
            }
            if( rightChildIndex != 0 )
            {
                newNodeRef.bb.expandBy( _kdTree.getNode( rightChildIndex ).bb );
            }

            if( !newNodeRef.bb.valid() )
            {
                OSG_NOTICE << "leftChildIndex=" << leftChildIndex
                           << " && originalLeftChildIndex=" << originalLeftChildIndex
                           << std::endl;
                OSG_NOTICE << "rightChildIndex=" << rightChildIndex
                           << " && originalRightChildIndex=" << originalRightChildIndex
                           << std::endl;

                OSG_NOTICE << "Invalid BB leftChildIndex=" << leftChildIndex << ", "
                           << rightChildIndex << std::endl;
                OSG_NOTICE << "  bb.min (" << newNodeRef.bb.min << ")" << std::endl;
                OSG_NOTICE << "  bb.max (" << newNodeRef.bb.max << ")" << std::endl;

                if( leftChildIndex != 0 )
                {
                    OSG_NOTICE << "  getNode(leftChildIndex).bb min = "
                               << _kdTree.getNode( leftChildIndex ).bb.min << std::endl;
                    OSG_NOTICE << "                                 max = "
                               << _kdTree.getNode( leftChildIndex ).bb.max << std::endl;
                }
                if( rightChildIndex != 0 )
                {
                    OSG_NOTICE << "  getNode(rightChildIndex).bb min = "
                               << _kdTree.getNode( rightChildIndex ).bb.min << std::endl;
                    OSG_NOTICE << "                              max = "
                               << _kdTree.getNode( rightChildIndex ).bb.max << std::endl;
                }
            }
        }
    }
    else
    {
        OSG_NOTICE << "NOT expecting to get here" << std::endl;
    }

    return nodeIndex;
}

////////////////////////////////////////////////////////////////////////////////
//
// KdTree::BuildOptions

KdTree::BuildOptions::BuildOptions() :
    _numVerticesProcessed( 0 ),
    _targetNumTrianglesPerLeaf( 4 ),
    _maxNumLevels( 32 )
{
}

////////////////////////////////////////////////////////////////////////////////
//
// KdTree

KdTree::KdTree() :
    _degenerateCount( 0 )
{
}

KdTree::KdTree( const KdTree&      rhs,
                const osg::CopyOp& copyop ) :
    Inherit( rhs,
             copyop ),
    _degenerateCount( rhs._degenerateCount ),
    _vertices( rhs._vertices ),
    _primitiveIndices( rhs._primitiveIndices ),
    _vertexIndices( rhs._vertexIndices ),
    _kdNodes( rhs._kdNodes )
{
}

bool
KdTree::build( BuildOptions&  options,
               osg::Geometry* geometry )
{
    BuildKdTree build( *this );
    return build.build( options, geometry );
}

////////////////////////////////////////////////////////////////////////////////
//
// KdTreeBuilder
KdTreeBuilder::KdTreeBuilder() :
    osg::DualModeVisitor( osg::DualModeVisitor::TRAVERSE_ALL_CHILDREN )
{
    _kdTreePrototype = new osg::KdTree;
}

KdTreeBuilder::KdTreeBuilder( const KdTreeBuilder& rhs ) :
    osg::Object( rhs ),
    osg::DualModeVisitor( osg::DualModeVisitor::TRAVERSE_ALL_CHILDREN ),
    _buildOptions( rhs._buildOptions ),
    _kdTreePrototype( rhs._kdTreePrototype )
{
}

void
KdTreeBuilder::apply( osg::Geometry& geometry )
{
    osg::KdTree* previous = dynamic_cast<osg::KdTree*>( geometry.getShape() );
    if( previous )
    {
        return;
    }

    osg::ref_ptr<osg::KdTree> kdTree = osg::clone( _kdTreePrototype.get() );

    if( kdTree->build( _buildOptions, &geometry ) )
    {
        geometry.setShape( kdTree.get() );
    }
}
