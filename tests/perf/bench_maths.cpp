/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Maths micro-benchmarks. State::updateModelViewAndProjectionMatrixUniforms
 * performs a double-precision 4x4 inverse plus a mat4 multiply per render
 * leaf per frame — these pin the unit costs, float vs double, affine
 * (inverse_4x3 fast path) vs general (perspective).
 *
 * Note: transform.hpp declares inverse_4x3/inverse_4x4 as extern OSG_EXPORT
 * but no definition exists in libosg — only the auto-selecting osg::inverse
 * links. Benchmarks use osg::inverse with affine vs non-affine inputs to
 * hit both internal paths.
 */

#include <benchmark/benchmark.h>
#include <osg/maths/mat4.hpp>
#include <osg/maths/transform.hpp>
#include <osg/maths/vec3.hpp>

namespace
{

    constexpr double kRotationRadians = 0.7;
    constexpr double kTranslateX      = 1.5;
    constexpr double kTranslateY      = -2.0;
    constexpr double kTranslateZ      = 3.25;
    constexpr double kScaleFactor     = 1.75;
    constexpr double kFovYDegrees     = 45.0;
    constexpr double kAspectRatio     = 4.0 / 3.0;
    constexpr double kNearPlane       = 0.1;
    constexpr double kFarPlane        = 1000.0;

    osg::dmat4
    makeAffineDMat4()
    {
        return osg::translate( kTranslateX, kTranslateY, kTranslateZ ) *
               osg::rotate( kRotationRadians, osg::dvec3( 0.0, 0.0, 1.0 ) ) *
               osg::scale( kScaleFactor, kScaleFactor, kScaleFactor );
    }

    osg::dmat4
    makeGeneralDMat4()
    {
        return osg::perspective( kFovYDegrees, kAspectRatio, kNearPlane, kFarPlane ) *
               makeAffineDMat4();
    }

    /// The per-leaf normal-matrix input: affine modelview → inverse_4x3 path.
    void
    BM_DMat4InverseAffine( benchmark::State& state )
    {
        const osg::dmat4 m = makeAffineDMat4();
        for( auto _ : state )
        {
            osg::dmat4 inv = osg::inverse( m );
            benchmark::DoNotOptimize( inv );
        }
    }

    BENCHMARK( BM_DMat4InverseAffine );

    /// General (non-affine) inverse — the inverse_4x4 path.
    void
    BM_DMat4InverseGeneral( benchmark::State& state )
    {
        const osg::dmat4 m = makeGeneralDMat4();
        for( auto _ : state )
        {
            osg::dmat4 inv = osg::inverse( m );
            benchmark::DoNotOptimize( inv );
        }
    }

    BENCHMARK( BM_DMat4InverseGeneral );

    /// Float comparison point for the affine inverse.
    void
    BM_Mat4InverseAffineFloat( benchmark::State& state )
    {
        const osg::mat4 m( makeAffineDMat4() );
        for( auto _ : state )
        {
            osg::mat4 inv = osg::inverse( m );
            benchmark::DoNotOptimize( inv );
        }
    }

    BENCHMARK( BM_Mat4InverseAffineFloat );

    /// The per-leaf MVP multiply (dmat4 * dmat4, 64 scalar multiplies).
    void
    BM_DMat4Multiply( benchmark::State& state )
    {
        const osg::dmat4 a = makeAffineDMat4();
        const osg::dmat4 b = osg::inverse( a );
        for( auto _ : state )
        {
            osg::dmat4 product = a * b;
            benchmark::DoNotOptimize( product );
        }
    }

    BENCHMARK( BM_DMat4Multiply );

    void
    BM_Mat4MultiplyFloat( benchmark::State& state )
    {
        const osg::mat4 a( makeAffineDMat4() );
        const osg::mat4 b = osg::inverse( a );
        for( auto _ : state )
        {
            osg::mat4 product = a * b;
            benchmark::DoNotOptimize( product );
        }
    }

    BENCHMARK( BM_Mat4MultiplyFloat );

    /// vec3 transform through a dmat4 (cull-time bound-center transforms).
    void
    BM_DMat4TransformVec3( benchmark::State& state )
    {
        const osg::dmat4 m = makeAffineDMat4();
        const osg::dvec3 v( 1.0, 2.0, 3.0 );
        for( auto _ : state )
        {
            osg::dvec3 transformed = m * v;
            benchmark::DoNotOptimize( transformed );
        }
    }

    BENCHMARK( BM_DMat4TransformVec3 );

}    // namespace
