/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Traversal micro-benchmarks: per-node visitor dispatch cost over a
 * balanced synthetic tree (accept → validNodeMask → nodePath push →
 * apply → traverse), mutable and const visitor variants.
 */

#include <benchmark/benchmark.h>
#include <osg/core/ref_ptr.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/traversal/NodeVisitor.hpp>

namespace
{

    constexpr std::size_t kBranching = 8;
    constexpr float       kQuadSize  = 1.0F;

    class CountingVisitor : public osg::NodeVisitor
    {
        public:

            CountingVisitor() :
                osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
            {
            }

            void
            apply( osg::Node& node ) override
            {
                ++_count;
                traverse( node );
            }

            std::size_t _count = 0;
    };

    class ConstCountingVisitor : public osg::ConstNodeVisitor
    {
        public:

            ConstCountingVisitor() :
                osg::ConstNodeVisitor( osg::ConstNodeVisitor::TRAVERSE_ALL_CHILDREN )
            {
            }

            void
            apply( const osg::Node& node ) override
            {
                ++_count;
                traverse( node );
            }

            std::size_t _count = 0;
    };

    osg::ref_ptr<osg::Group>
    buildBalancedTree( std::size_t depth )
    {
        osg::ref_ptr<osg::Group> node = osg::Group::create();
        if( depth == 0 )
        {
            osg::ref_ptr<osg::Geode> geode = osg::Geode::create();
            geode->addDrawable( osg::createTexturedQuadGeometry(
                osg::vec3( 0.0F, 0.0F, 0.0F ),
                osg::vec3( kQuadSize, 0.0F, 0.0F ),
                osg::vec3( 0.0F, kQuadSize, 0.0F ) ) );
            node->addChild( geode.get() );
            return node;
        }
        for( std::size_t i = 0; i < kBranching; ++i )
        {
            node->addChild( buildBalancedTree( depth - 1 ).get() );
        }
        return node;
    }

    /// Full mutable-visitor sweep. items/s ≈ nodes visited per second.
    void
    BM_NodeVisitorTraverse( benchmark::State& state )
    {
        const auto               depth = static_cast<std::size_t>( state.range( 0 ) );
        osg::ref_ptr<osg::Group> root  = buildBalancedTree( depth );

        std::size_t              visited = 0;
        for( auto _ : state )
        {
            CountingVisitor visitor;
            root->accept( visitor );
            visited = visitor._count;
            benchmark::DoNotOptimize( visited );
        }
        state.SetItemsProcessed( static_cast<std::int64_t>( state.iterations() ) *
                                 static_cast<std::int64_t>( visited ) );
    }
    BENCHMARK( BM_NodeVisitorTraverse )->Arg( 3 )->Arg( 4 )->Arg( 5 );

    /// Const-visitor sweep for comparison with the mutable path.
    void
    BM_ConstNodeVisitorTraverse( benchmark::State& state )
    {
        const auto               depth = static_cast<std::size_t>( state.range( 0 ) );
        osg::ref_ptr<osg::Group> root  = buildBalancedTree( depth );

        std::size_t              visited = 0;
        for( auto _ : state )
        {
            ConstCountingVisitor visitor;
            static_cast<const osg::Group*>( root.get() )->accept( visitor );
            visited = visitor._count;
            benchmark::DoNotOptimize( visited );
        }
        state.SetItemsProcessed( static_cast<std::int64_t>( state.iterations() ) *
                                 static_cast<std::int64_t>( visited ) );
    }
    BENCHMARK( BM_ConstNodeVisitorTraverse )->Arg( 3 )->Arg( 4 )->Arg( 5 );

}    // namespace
