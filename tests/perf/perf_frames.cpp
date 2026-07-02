/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Frame stress harness: renders synthetic worst-case scenes through the
 * full cull+draw pipeline on a headless EGL context and reports per-frame
 * wall-clock statistics. Scenarios target the loss areas identified in
 * workspace/findings/perf-audit-core-scenegraph.md:
 *
 *   grid        N drawables, one shared StateSet   → per-leaf floor
 *               (uniform sweep, leaf churn, draw submission)
 *   statesets   N drawables, N unique StateSets    → StateGraph/state-diff
 *   transforms  N drawables under N MatrixTransforms → cull matrix path
 *   transparent N blended drawables                → depth-sort bin path
 *
 * Usage:
 *   perf_frames --scene grid --count 10000 --frames 200 [--warmup 20]
 */

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <osg/core/ArgumentParser.hpp>
#include <osg/core/ref_ptr.hpp>
#include <osg/core/Stats.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/rendering/GraphicsContext.hpp>
#include <osg/state/Viewport.hpp>
#include <osg/state/Material.hpp>
#include <osg/state/StateSet.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <string>
#include <vector>

namespace
{

    constexpr int    kDefaultCount     = 10000;
    constexpr int    kDefaultFrames    = 200;
    constexpr int    kDefaultWarmup    = 20;
    constexpr int    kViewportWidth    = 1024;
    constexpr int    kViewportHeight   = 768;
    constexpr float  kQuadSize         = 0.8F;
    constexpr float  kCellSpacing      = 1.0F;
    constexpr float  kTransparentAlpha = 0.5F;
    constexpr double kEyeDistanceX     = 2.0;
    constexpr double kEyeDistanceY     = 1.0;
    constexpr double kEyeDistanceZ     = 2.0;

    osg::ref_ptr<osg::Geometry>
    makeQuad( const osg::vec3& corner )
    {
        return osg::createTexturedQuadGeometry(
            corner,
            osg::vec3( kQuadSize, 0.0F, 0.0F ),
            osg::vec3( 0.0F, kQuadSize, 0.0F ) );
    }

    osg::vec3
    gridPosition( int index,
                  int side )
    {
        const int x = index % side;
        const int y = ( index / side ) % side;
        const int z = index / ( side * side );
        return osg::vec3( static_cast<float>( x ) * kCellSpacing,
                          static_cast<float>( y ) * kCellSpacing,
                          static_cast<float>( z ) * kCellSpacing );
    }

    int
    cubeSide( int count )
    {
        return std::max( 1,
                         static_cast<int>( std::ceil(
                             std::cbrt( static_cast<double>( count ) ) ) ) );
    }

    osg::ref_ptr<osg::StateSet>
    makeUniqueStateSet( int index,
                        int count )
    {
        osg::ref_ptr<osg::StateSet>  set      = new osg::StateSet;
        osg::ref_ptr<osg::Material>  material = new osg::Material;
        const float                  shade =
            static_cast<float>( index ) / static_cast<float>( count );
        material->setDiffuse( osg::Material::FRONT_AND_BACK,
                              osg::vec4( shade, 1.0F - shade, 0.5F, 1.0F ) );
        set->setAttribute( material.get() );
        return set;
    }

    osg::ref_ptr<osg::Node>
    buildScene( const std::string& scene,
                int                count )
    {
        osg::ref_ptr<osg::Group> root = osg::Group::create();
        const int                side = cubeSide( count );

        for( int i = 0; i < count; ++i )
        {
            const osg::vec3          pos   = gridPosition( i, side );
            osg::ref_ptr<osg::Geode> geode = osg::Geode::create();

            if( scene == "transforms" )
            {
                geode->addDrawable( makeQuad( osg::vec3( 0.0F, 0.0F, 0.0F ) ) );
                osg::ref_ptr<osg::MatrixTransform> xf =
                    osg::MatrixTransform::create();
                xf->setMatrix( osg::translate( static_cast<double>( pos.x ),
                                               static_cast<double>( pos.y ),
                                               static_cast<double>( pos.z ) ) );
                xf->addChild( geode.get() );
                root->addChild( xf.get() );
            }
            else
            {
                geode->addDrawable( makeQuad( pos ) );
                root->addChild( geode.get() );
            }

            if( scene == "statesets" )
            {
                geode->setStateSet( makeUniqueStateSet( i, count ).get() );
            }
            else if( scene == "transparent" )
            {
                osg::ref_ptr<osg::StateSet> set = makeUniqueStateSet( i, count );
                osg::Material*              material = static_cast<osg::Material*>(
                    set->getAttribute( osg::StateAttribute::Type::MATERIAL ) );
                osg::vec4 diffuse =
                    material->getDiffuse( osg::Material::FRONT_AND_BACK );
                diffuse.a = kTransparentAlpha;
                material->setDiffuse( osg::Material::FRONT_AND_BACK, diffuse );
                set->setMode( GL_BLEND, osg::StateAttribute::ON );
                set->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
                geode->setStateSet( set.get() );
            }
        }
        return root;
    }

    struct FrameStats
    {
            double meanMs   = 0.0;
            double medianMs = 0.0;
            double p95Ms    = 0.0;
            double minMs    = 0.0;
            double maxMs    = 0.0;
    };

    FrameStats
    computeStats( std::vector<double>& samples )
    {
        FrameStats stats;
        std::sort( samples.begin(), samples.end() );
        const std::size_t n = samples.size();
        double            sum = 0.0;
        for( double s : samples )
        {
            sum += s;
        }
        stats.meanMs   = sum / static_cast<double>( n );
        stats.medianMs = samples[n / 2];
        stats.p95Ms    = samples[( n * 95 ) / 100];
        stats.minMs    = samples.front();
        stats.maxMs    = samples.back();
        return stats;
    }

}    // namespace

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    std::string         scene  = "grid";
    int                 count  = kDefaultCount;
    int                 frames = kDefaultFrames;
    int                 warmup = kDefaultWarmup;
    arguments.read( "--scene", scene );
    arguments.read( "--count", count );
    arguments.read( "--frames", frames );
    arguments.read( "--warmup", warmup );

    if( scene != "grid" && scene != "statesets" && scene != "transforms" &&
        scene != "transparent" )
    {
        std::fprintf( stderr,
                      "perf_frames: unknown --scene '%s' "
                      "(grid|statesets|transforms|transparent)\n",
                      scene.c_str() );
        return 1;
    }

    osg::ref_ptr<osg::Node>                    sceneRoot = buildScene( scene, count );

    osg::ref_ptr<osg::GraphicsContext::Traits> traits =
        new osg::GraphicsContext::Traits;
    traits->x            = 0;
    traits->y            = 0;
    traits->width        = kViewportWidth;
    traits->height       = kViewportHeight;
    traits->doubleBuffer = true;
    traits->headless     = true;
    traits->readDISPLAY();
    traits->setUndefinedScreenDetailsToDefaultScreen();

    osg::ref_ptr<osg::GraphicsContext> gc =
        osg::GraphicsContext::createGraphicsContext( traits.get() );
    if( !gc.valid() || !gc->valid() )
    {
        std::fprintf( stderr, "perf_frames: failed to create headless context\n" );
        return 1;
    }

    osgViewer::Viewer viewer;
    viewer.setThreadingModel( osgViewer::Viewer::SingleThreaded );
    viewer.getCamera()->setGraphicsContext( gc.get() );
    viewer.getCamera()->setViewport(
        new osg::Viewport( 0, 0, kViewportWidth, kViewportHeight ) );
    viewer.setSceneData( sceneRoot.get() );

    // Per-stage timing (cull / draw / GPU). Privilege-free alternative to perf:
    // the Renderer records begin/end/time-taken for each stage into the camera
    // Stats when the "rendering" (and "gpu") collect flags are set.
    osg::ref_ptr<osg::Stats> cameraStats =
        new osg::Stats( "camera", static_cast<unsigned int>( frames + warmup + 4 ) );
    cameraStats->collectStats( "rendering", true );
    cameraStats->collectStats( "gpu", true );
    viewer.getCamera()->setStats( cameraStats.get() );

    const osg::sphere& bound  = sceneRoot->getBound();
    const double       radius = bound.radius > 0.0 ? bound.radius : 1.0;
    const osg::dvec3   center( bound.center );
    const osg::dvec3   eye( center.x + radius * kEyeDistanceX,
                            center.y + radius * kEyeDistanceY,
                            center.z + radius * kEyeDistanceZ );
    viewer.getCamera()->setViewMatrixAsLookAt( eye,
                                               center,
                                               osg::dvec3( 0.0, 1.0, 0.0 ) );

    viewer.realize();

    for( int i = 0; i < warmup; ++i )
    {
        viewer.frame();
    }

    std::vector<double> samples;
    std::vector<double> cullSamples;
    std::vector<double> drawSamples;
    std::vector<double> gpuSamples;
    samples.reserve( static_cast<std::size_t>( frames ) );
    for( int i = 0; i < frames; ++i )
    {
        const auto t0 = std::chrono::steady_clock::now();
        viewer.frame();
        const auto t1 = std::chrono::steady_clock::now();
        samples.push_back(
            std::chrono::duration<double, std::milli>( t1 - t0 ).count() );

        const unsigned int frameNumber =
            viewer.getFrameStamp()->getFrameNumber();
        double cull = 0.0;
        double draw = 0.0;
        double gpu  = 0.0;
        if( cameraStats->getAttribute( frameNumber,
                                       "Cull traversal time taken",
                                       cull ) )
        {
            cullSamples.push_back( cull * 1000.0 );
        }
        if( cameraStats->getAttribute( frameNumber,
                                       "Draw traversal time taken",
                                       draw ) )
        {
            drawSamples.push_back( draw * 1000.0 );
        }
        if( cameraStats->getAttribute( frameNumber, "GPU draw time taken", gpu ) )
        {
            gpuSamples.push_back( gpu * 1000.0 );
        }
    }

    const FrameStats stats = computeStats( samples );
    std::printf( "RESULT scene=%s count=%d frames=%d mean_ms=%.3f median_ms=%.3f "
                 "p95_ms=%.3f min_ms=%.3f max_ms=%.3f fps=%.1f\n",
                 scene.c_str(),
                 count,
                 frames,
                 stats.meanMs,
                 stats.medianMs,
                 stats.p95Ms,
                 stats.minMs,
                 stats.maxMs,
                 stats.meanMs > 0.0 ? 1000.0 / stats.meanMs : 0.0 );

    auto meanOf = []( const std::vector<double>& v ) -> double
    {
        if( v.empty() )
        {
            return 0.0;
        }
        double sum = 0.0;
        for( double s : v )
        {
            sum += s;
        }
        return sum / static_cast<double>( v.size() );
    };
    std::printf( "STAGES scene=%s cull_ms=%.3f draw_ms=%.3f gpu_ms=%.3f "
                 "cpu_frame_ms=%.3f\n",
                 scene.c_str(),
                 meanOf( cullSamples ),
                 meanOf( drawSamples ),
                 meanOf( gpuSamples ),
                 meanOf( cullSamples ) + meanOf( drawSamples ) );
    return 0;
}
