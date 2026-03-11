/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * K-d tree spatial index for ray intersection acceleration.
 * Built per-Geometry for fast line-of-sight and picking queries.
 */
#pragma once

#include <map>
#include <osg/core/Inherit.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/Shape.hpp>

namespace osg
{

    /** Implementation of a kdtree for Geometry leaves, to enable fast intersection
     * tests.*/
    class OSG_EXPORT KdTree : public osg::Inherit<osg::Shape, KdTree>
    {
        public:

            KdTree();

            KdTree( const KdTree&      rhs,
                    const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osg,
                               KdTree )

            void
            accept( ShapeVisitor& sv ) override
            {
                sv.apply( *this );
            }

            void
            accept( ConstShapeVisitor& csv ) const override
            {
                csv.apply( *this );
            }

            struct OSG_EXPORT BuildOptions
            {
                    BuildOptions();

                    unsigned int _numVerticesProcessed;
                    unsigned int _targetNumTrianglesPerLeaf;
                    unsigned int _maxNumLevels;
            };

            /** Build the kdtree from the specified source geometry object.
             * retun true on success. */
            virtual bool
            build( BuildOptions&  buildOptions,
                   osg::Geometry* geometry );

            void
            setVertices( osg::Vec3Array* vertices )
            {
                _vertices = vertices;
            }

            const osg::Vec3Array*
            getVertices() const
            {
                return _vertices.get();
            }

            typedef std::vector<unsigned int> Indices;

            // index in the VertexIndices vector
            void
            setPrimitiveIndices( const Indices& indices )
            {
                _primitiveIndices = indices;
            }

            Indices&
            getPrimitiveIndices()
            {
                return _primitiveIndices;
            }

            const Indices&
            getPrimitiveIndices() const
            {
                return _primitiveIndices;
            }

            // vector containing the primitive vertex index data packed as
            // no_vertice_indices then vertex indices ie. for points it's (1, p0), for
            // lines (2, p0, p1) etc.
            void
            setVertexIndices( const Indices& indices )
            {
                _vertexIndices = indices;
            }

            Indices&
            getVertexIndices()
            {
                return _vertexIndices;
            }

            const Indices&
            getVertexIndices() const
            {
                return _vertexIndices;
            }

            inline unsigned int
            addPoint( unsigned int p0 )
            {
                unsigned int i = static_cast<unsigned int>( _vertexIndices.size() );
                _vertexIndices.push_back(
                    static_cast<unsigned int>( _primitiveIndices.size() ) +
                    _degenerateCount
                );
                _vertexIndices.push_back( 1 );
                _vertexIndices.push_back( p0 );
                _primitiveIndices.push_back( i );
                return i;
            }

            inline unsigned int
            addLine( unsigned int p0,
                     unsigned int p1 )
            {
                unsigned int i = static_cast<unsigned int>( _vertexIndices.size() );
                _vertexIndices.push_back(
                    static_cast<unsigned int>( _primitiveIndices.size() ) +
                    _degenerateCount
                );
                _vertexIndices.push_back( 2 );
                _vertexIndices.push_back( p0 );
                _vertexIndices.push_back( p1 );
                _primitiveIndices.push_back( i );
                return i;
            }

            inline unsigned int
            addTriangle( unsigned int p0,
                         unsigned int p1,
                         unsigned int p2 )
            {
                unsigned int i = static_cast<unsigned int>( _vertexIndices.size() );
                _vertexIndices.push_back(
                    static_cast<unsigned int>( _primitiveIndices.size() ) +
                    _degenerateCount
                );
                _vertexIndices.push_back( 3 );
                _vertexIndices.push_back( p0 );
                _vertexIndices.push_back( p1 );
                _vertexIndices.push_back( p2 );
                _primitiveIndices.push_back( i );
                return i;
            }

            inline unsigned int
            addQuad( unsigned int p0,
                     unsigned int p1,
                     unsigned int p2,
                     unsigned int p3 )
            {
                unsigned int i = static_cast<unsigned int>( _vertexIndices.size() );
                _vertexIndices.push_back(
                    static_cast<unsigned int>( _primitiveIndices.size() ) +
                    _degenerateCount
                );
                _vertexIndices.push_back( 4 );
                _vertexIndices.push_back( p0 );
                _vertexIndices.push_back( p1 );
                _vertexIndices.push_back( p2 );
                _vertexIndices.push_back( p3 );
                _primitiveIndices.push_back( i );
                return i;
            }

            typedef int value_type;

            struct KdNode
            {
                    KdNode() :
                        first( 0 ),
                        second( 0 )
                    {
                    }

                    KdNode( value_type f,
                            value_type s ) :
                        first( f ),
                        second( s )
                    {
                    }

                    osg::box   bb;

                    value_type first;
                    value_type second;
            };

            typedef std::vector<KdNode> KdNodeList;

            int
            addNode( const KdNode& node )
            {
                int num = static_cast<int>( _kdNodes.size() );
                _kdNodes.push_back( node );
                return num;
            }

            KdNode&
            getNode( int nodeNum )
            {
                return _kdNodes[static_cast<size_t>( nodeNum )];
            }

            const KdNode&
            getNode( int nodeNum ) const
            {
                return _kdNodes[static_cast<size_t>( nodeNum )];
            }

            KdNodeList&
            getNodes()
            {
                return _kdNodes;
            }

            const KdNodeList&
            getNodes() const
            {
                return _kdNodes;
            }

            template<class IntersectFunctor>
            void
            intersect( IntersectFunctor& functor,
                       const KdNode&     node ) const
            {
                if( node.first < 0 )
                {
                    // treat as a leaf
                    int istart = -node.first - 1;
                    int iend   = istart + node.second;

                    for( int i = istart; i < iend; ++i )
                    {
                        unsigned int primitiveIndex =
                            _primitiveIndices[static_cast<size_t>( i )];
                        int originalPIndex =
                            static_cast<int>( _vertexIndices[primitiveIndex++] );
                        unsigned int numVertices = _vertexIndices[primitiveIndex++];
                        switch( numVertices )
                        {
                            case( 1 ) :
                                functor.intersect( _vertices.get(),
                                                   originalPIndex,
                                                   _vertexIndices[primitiveIndex] );
                                break;
                            case( 2 ) :
                                functor.intersect( _vertices.get(),
                                                   originalPIndex,
                                                   _vertexIndices[primitiveIndex],
                                                   _vertexIndices[primitiveIndex + 1] );
                                break;
                            case( 3 ) :
                                functor.intersect( _vertices.get(),
                                                   originalPIndex,
                                                   _vertexIndices[primitiveIndex],
                                                   _vertexIndices[primitiveIndex + 1],
                                                   _vertexIndices[primitiveIndex + 2] );
                                break;
                            case( 4 ) :
                                functor.intersect( _vertices.get(),
                                                   originalPIndex,
                                                   _vertexIndices[primitiveIndex],
                                                   _vertexIndices[primitiveIndex + 1],
                                                   _vertexIndices[primitiveIndex + 2],
                                                   _vertexIndices[primitiveIndex + 3] );
                                break;
                            default :
                                OSG_NOTICE << "Warning: KdTree::intersect() encounted "
                                              "unsupported primitive size of "
                                           << numVertices << std::endl;
                                break;
                        }
                    }
                }
                else if( functor.enter( node.bb ) )
                {
                    if( node.first > 0 )
                    {
                        intersect( functor,
                                   _kdNodes[static_cast<std::size_t>( node.first )] );
                    }
                    if( node.second > 0 )
                    {
                        intersect( functor,
                                   _kdNodes[static_cast<std::size_t>( node.second )] );
                    }

                    functor.leave();
                }
            }

            unsigned int _degenerateCount;

        protected:

            osg::ref_ptr<osg::Vec3Array> _vertices;
            Indices                      _primitiveIndices;
            Indices                      _vertexIndices;
            KdNodeList                   _kdNodes;
    };

    class OSG_EXPORT KdTreeBuilder : public osg::DualModeVisitor
    {
        public:

            KdTreeBuilder();

            KdTreeBuilder( const KdTreeBuilder& rhs );

            OSG_REGISTER_TYPE( osg,
                               KdTreeBuilder )

            using Object::clone;

            virtual KdTreeBuilder*
            clone()
            {
                return new KdTreeBuilder( *this );
            }

            using ConstNodeVisitor::apply;
            using NodeVisitor::apply;

            void
                                      apply( Geometry& geometry ) override;

            KdTree::BuildOptions      _buildOptions;

            osg::ref_ptr<osg::KdTree> _kdTreePrototype;

        protected:

            virtual ~KdTreeBuilder()
            {
            }
    };

}
