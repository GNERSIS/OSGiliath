/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Render-backend micro-benchmarks: StateGraph find_or_insert (std::map
 * child lookup per StateSet push) and RenderLeaf set/reset ref_ptr churn
 * (the ~8 atomic RMW per visible leaf per frame identified by the audit).
 */

#include <benchmark/benchmark.h>
#include <osg/core/ref_ptr.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/RefMatrix.hpp>
#include <osg/state/StateSet.hpp>
#include <osgUtil/culling/RenderLeaf.hpp>
#include <osgUtil/culling/StateGraph.hpp>
#include <vector>

namespace
{

    constexpr float kQuadSize = 1.0F;

    std::vector<osg::ref_ptr<osg::StateSet>>
    makeStateSets( std::size_t count )
    {
        std::vector<osg::ref_ptr<osg::StateSet>> sets;
        sets.reserve( count );
        for( std::size_t i = 0; i < count; ++i )
        {
            sets.emplace_back( new osg::StateSet );
        }
        return sets;
    }

    /// Steady-state per-leaf push pattern: the StateSets already exist in the
    /// graph, so every push is a pure std::map find on the child list — the
    /// per-drawable cull cost with K unique StateSets in play.
    void
    BM_StateGraphFindWarm( benchmark::State& state )
    {
        const auto count = static_cast<std::size_t>( state.range( 0 ) );
        const auto sets  = makeStateSets( count );
        osg::ref_ptr<osgUtil::StateGraph> root = new osgUtil::StateGraph;

        for( const auto& set : sets )
        {
            benchmark::DoNotOptimize( root->find_or_insert( set.get() ) );
        }

        for( auto _ : state )
        {
            for( const auto& set : sets )
            {
                benchmark::DoNotOptimize( root->find_or_insert( set.get() ) );
            }
        }
        state.SetItemsProcessed( static_cast<std::int64_t>( state.iterations() ) *
                                 static_cast<std::int64_t>( count ) );
    }

    BENCHMARK( BM_StateGraphFindWarm )->Arg( 4 )->Arg( 64 )->Arg( 1'024 );

    /// Cold insert + clean cycle: allocation of StateGraph nodes on first
    /// encounter, then the per-frame clean() reuse pass.
    void
    BM_StateGraphInsertClean( benchmark::State& state )
    {
        const auto count = static_cast<std::size_t>( state.range( 0 ) );
        const auto sets  = makeStateSets( count );
        osg::ref_ptr<osgUtil::StateGraph> root = new osgUtil::StateGraph;

        for( auto _ : state )
        {
            for( const auto& set : sets )
            {
                benchmark::DoNotOptimize( root->find_or_insert( set.get() ) );
            }
            root->clean();
        }
        state.SetItemsProcessed( static_cast<std::int64_t>( state.iterations() ) *
                                 static_cast<std::int64_t>( count ) );
    }

    BENCHMARK( BM_StateGraphInsertClean )->Arg( 64 )->Arg( 1'024 );

    /// RenderLeaf set()/reset() cycle — exactly the pool-reuse pattern
    /// CullVisitor runs per visible drawable per frame. With
    /// OSGUTIL_RENDERBACKEND_USE_REF_PTR defined this is 6 atomic RMW per
    /// cycle on drawable/projection/modelview.
    void
    BM_RenderLeafSetReset( benchmark::State& state )
    {
        osg::ref_ptr<osg::Geometry> geometry =
            osg::createTexturedQuadGeometry( osg::vec3( 0.0F, 0.0F, 0.0F ),
                                             osg::vec3( kQuadSize, 0.0F, 0.0F ),
                                             osg::vec3( 0.0F, kQuadSize, 0.0F ) );
        osg::ref_ptr<osg::RefMatrix>      projection = new osg::RefMatrix;
        osg::ref_ptr<osg::RefMatrix>      modelview  = new osg::RefMatrix;

        osg::ref_ptr<osgUtil::RenderLeaf> leaf =
            new osgUtil::RenderLeaf( geometry.get(), projection.get(), modelview.get() );

        for( auto _ : state )
        {
            leaf->reset();
            leaf->set( geometry.get(), projection.get(), modelview.get() );
            benchmark::DoNotOptimize( leaf.get() );
        }
    }

    BENCHMARK( BM_RenderLeafSetReset );

}    // namespace
