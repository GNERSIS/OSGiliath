/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Object-model micro-benchmarks: ref_ptr atomic churn, object creation,
 * scene build/teardown, parent-list global-lock contention, dirtyBound.
 * Targets findings of workspace/findings/perf-audit-core-scenegraph.md.
 */

#include <benchmark/benchmark.h>
#include <osg/core/Referenced.hpp>
#include <osg/core/ref_ptr.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/Group.hpp>
#include <vector>

namespace
{

    constexpr float       kQuadSize    = 1.0F;
    constexpr std::size_t kChainLength = 64;

    /// ref_ptr copy/destroy — one atomic increment + one atomic decrement.
    void
    BM_RefPtrCopy( benchmark::State& state )
    {
        osg::ref_ptr<osg::Referenced> obj = new osg::Referenced;
        for( auto _ : state )
        {
            osg::ref_ptr<osg::Referenced> copy( obj );
            benchmark::DoNotOptimize( copy.get() );
        }
    }
    BENCHMARK( BM_RefPtrCopy );

    /// Baseline: raw pointer copy (no refcount traffic).
    void
    BM_RawPtrCopy( benchmark::State& state )
    {
        osg::ref_ptr<osg::Referenced> obj = new osg::Referenced;
        for( auto _ : state )
        {
            osg::Referenced* copy = obj.get();
            benchmark::DoNotOptimize( copy );
        }
    }
    BENCHMARK( BM_RawPtrCopy );

    /// Object creation throughput: one Group per iteration (malloc + ctor +
    /// std::string name + observer slot + atomic release on destroy).
    void
    BM_GroupCreateDestroy( benchmark::State& state )
    {
        for( auto _ : state )
        {
            osg::ref_ptr<osg::Group> group = osg::Group::create();
            benchmark::DoNotOptimize( group.get() );
        }
    }
    BENCHMARK( BM_GroupCreateDestroy );

    osg::ref_ptr<osg::Group>
    buildTree( std::size_t depth,
               std::size_t width )
    {
        osg::ref_ptr<osg::Group> root = osg::Group::create();
        if( depth == 0 )
        {
            for( std::size_t i = 0; i < width; ++i )
            {
                osg::ref_ptr<osg::Geode> geode = osg::Geode::create();
                geode->addDrawable( osg::createTexturedQuadGeometry(
                    osg::vec3( 0.0F, 0.0F, 0.0F ),
                    osg::vec3( kQuadSize, 0.0F, 0.0F ),
                    osg::vec3( 0.0F, kQuadSize, 0.0F ) ) );
                root->addChild( geode.get() );
            }
            return root;
        }
        for( std::size_t i = 0; i < width; ++i )
        {
            root->addChild( buildTree( depth - 1, width ).get() );
        }
        return root;
    }

    /// Build + tear down a wide two-level scene: allocation storm on the way
    /// up, atomic-unref cascade + global parent-mutex traffic on the way down.
    void
    BM_SceneBuildTeardown( benchmark::State& state )
    {
        const auto width = static_cast<std::size_t>( state.range( 0 ) );
        for( auto _ : state )
        {
            osg::ref_ptr<osg::Group> root = buildTree( 1, width );
            benchmark::DoNotOptimize( root.get() );
        }
        state.SetItemsProcessed(
            static_cast<std::int64_t>( state.iterations() ) *
            static_cast<std::int64_t>( width * width ) );
    }
    BENCHMARK( BM_SceneBuildTeardown )->Arg( 8 )->Arg( 32 )->Arg( 64 );

    constexpr std::size_t kMaxBenchThreads = 8;

    struct ThreadSlots
    {
            std::vector<osg::ref_ptr<osg::Group>> groups;
            std::vector<osg::ref_ptr<osg::Group>> children;

            ThreadSlots()
            {
                groups.reserve( kMaxBenchThreads );
                children.reserve( kMaxBenchThreads );
                for( std::size_t i = 0; i < kMaxBenchThreads; ++i )
                {
                    groups.push_back( osg::Group::create() );
                    children.push_back( osg::Group::create() );
                }
            }
    };

    ThreadSlots&
    threadSlots()
    {
        static ThreadSlots slots;
        return slots;
    }

    /// addChild/removeChild on per-thread DISJOINT groups. Scaling loss with
    /// thread count exposes the process-global parent-list mutex
    /// (Referenced::getGlobalReferencedMutex serializes addParent/removeParent).
    void
    BM_AddRemoveChildThreaded( benchmark::State& state )
    {
        ThreadSlots& slots = threadSlots();
        const auto   idx   = static_cast<std::size_t>( state.thread_index() );
        osg::Group*  group = slots.groups[idx].get();
        osg::Group*  child = slots.children[idx].get();
        for( auto _ : state )
        {
            group->addChild( child );
            group->removeChild( child );
        }
    }
    BENCHMARK( BM_AddRemoveChildThreaded )->Threads( 1 )->Threads( 4 )->Threads( 8 );

    /// dirtyBound at the leaf of a deep chain + getBound at the root:
    /// upward dirty walk + downward recompute cascade.
    void
    BM_DirtyBoundChainRecompute( benchmark::State& state )
    {
        osg::ref_ptr<osg::Group> root = osg::Group::create();
        osg::Group*              tail = root.get();
        for( std::size_t i = 0; i < kChainLength; ++i )
        {
            osg::ref_ptr<osg::Group> next = osg::Group::create();
            tail->addChild( next.get() );
            tail = next.get();
        }
        osg::ref_ptr<osg::Geode> leaf = osg::Geode::create();
        leaf->addDrawable( osg::createTexturedQuadGeometry(
            osg::vec3( 0.0F, 0.0F, 0.0F ),
            osg::vec3( kQuadSize, 0.0F, 0.0F ),
            osg::vec3( 0.0F, kQuadSize, 0.0F ) ) );
        tail->addChild( leaf.get() );

        for( auto _ : state )
        {
            leaf->dirtyBound();
            osg::sphere bound = root->getBound();
            benchmark::DoNotOptimize( bound );
        }
    }
    BENCHMARK( BM_DirtyBoundChainRecompute );

    /// Wide-group bound recompute (Group::computeBound iterates children twice).
    void
    BM_WideGroupBoundRecompute( benchmark::State& state )
    {
        const auto               width = static_cast<std::size_t>( state.range( 0 ) );
        osg::ref_ptr<osg::Group> root  = osg::Group::create();
        for( std::size_t i = 0; i < width; ++i )
        {
            osg::ref_ptr<osg::Geode> geode = osg::Geode::create();
            const float              x     = static_cast<float>( i ) * kQuadSize;
            geode->addDrawable( osg::createTexturedQuadGeometry(
                osg::vec3( x, 0.0F, 0.0F ),
                osg::vec3( kQuadSize, 0.0F, 0.0F ),
                osg::vec3( 0.0F, kQuadSize, 0.0F ) ) );
            root->addChild( geode.get() );
        }
        for( auto _ : state )
        {
            root->dirtyBound();
            osg::sphere bound = root->getBound();
            benchmark::DoNotOptimize( bound );
        }
    }
    BENCHMARK( BM_WideGroupBoundRecompute )->Arg( 256 )->Arg( 4096 );

}    // namespace
