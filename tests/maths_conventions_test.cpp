/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */

// Pins the VSG-derived maths conventions documented in CLAUDE.md.
// Expected values are derived from the actual source of
// include/osg/maths/{mat4,transform,quat,compat,box,sphere}.hpp,
// not guessed. Each TEST documents the convention it locks down.
//
// Element access deliberately uses dmat4::operator()(row, col) and the
// type-level operator== rather than the raw .value[][] array or the
// .x/.y/.z anonymous-union members, so the test pins the conventions
// without tripping the bounds-/union-access tidy checks.
//
// NOLINTBEGIN(llvm-include-order): clang-tidy treats <gtest/gtest.h> as the
// main-module header and wants it first, but clang-format (the authoritative
// in-repo formatter) sorts strictly alphabetically. The two cannot agree
// once a standard header sorts before gtest; suppress the gtest-only clash.
#include <cstddef>
#include <gtest/gtest.h>
#include <osg/core/ref_ptr.hpp>
#include <osg/maths/box.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/maths/quat.hpp>
#include <osg/maths/sphere.hpp>
#include <osg/maths/transform.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <type_traits>
// NOLINTEND(llvm-include-order)

namespace
{

    // Matrix index constants. dmat4::operator()(row, col) reads
    // value[col][row] — row-first argument order over column-major storage.
    constexpr std::size_t col0 = 0;
    constexpr std::size_t col1 = 1;
    constexpr std::size_t col2 = 2;
    constexpr std::size_t col3 = 3;
    constexpr std::size_t row0 = 0;
    constexpr std::size_t row1 = 1;
    constexpr std::size_t row2 = 2;
    constexpr std::size_t row3 = 3;

    // Translation components used throughout.
    constexpr double      tx    = 7.0;
    constexpr double      ty    = 11.0;
    constexpr double      tz    = 13.0;

    constexpr double      zero  = 0.0;
    constexpr double      one   = 1.0;
    constexpr double      eps   = 1E-12;
    constexpr float       zeroF = 0.0F;

    // ── Column-major storage: translate() writes into column 3 ──
    // osg::translate(dvec3) is constexpr; the translation lands at
    // value[3][0..2], i.e. operator()(row, col3); value[3][3] == 1.
    TEST( ColumnMajorStorage,
          TranslateWritesColumnThree )
    {
        const osg::dmat4 m = osg::translate( osg::dvec3( tx, ty, tz ) );

        EXPECT_DOUBLE_EQ( m( row0, col3 ), tx );
        EXPECT_DOUBLE_EQ( m( row1, col3 ), ty );
        EXPECT_DOUBLE_EQ( m( row2, col3 ), tz );
        EXPECT_DOUBLE_EQ( m( row3, col3 ), one );

        // Columns 0..2 stay the identity basis.
        EXPECT_DOUBLE_EQ( m( row0, col0 ), one );
        EXPECT_DOUBLE_EQ( m( row1, col1 ), one );
        EXPECT_DOUBLE_EQ( m( row2, col2 ), one );
        // No translation has leaked into the bottom row of another column.
        EXPECT_DOUBLE_EQ( m( row3, col0 ), zero );
        EXPECT_DOUBLE_EQ( m( row3, col1 ), zero );
        EXPECT_DOUBLE_EQ( m( row3, col2 ), zero );
    }

    // ── Column-vector convention: M * v applies the translation ──
    TEST( ColumnVectorConvention,
          MatrixTimesVectorTranslates )
    {
        constexpr double px  = 1.0;
        constexpr double py  = 2.0;
        constexpr double pz  = 3.0;

        const osg::dmat4 m   = osg::translate( osg::dvec3( tx, ty, tz ) );
        const osg::dvec3 out = m * osg::dvec3( px, py, pz );

        EXPECT_TRUE( out == osg::dvec3( px + tx, py + ty, pz + tz ) );
    }

    // ── world = parent * local composition ──
    // Two translations compose additively; the column-vector convention
    // means world = parentWorld * localMatrix.
    TEST( ColumnVectorConvention,
          WorldEqualsParentTimesLocal )
    {
        constexpr double lx     = 1.0;
        constexpr double ly     = 0.0;
        constexpr double lz     = 0.0;

        const osg::dmat4 parent = osg::translate( osg::dvec3( tx, ty, tz ) );
        const osg::dmat4 local  = osg::translate( osg::dvec3( lx, ly, lz ) );
        const osg::dmat4 world  = parent * local;

        const osg::dvec3 out    = world * osg::dvec3( zero, zero, zero );
        EXPECT_TRUE( out == osg::dvec3( tx + lx, ty + ly, tz + lz ) );
    }

    // ── operator()(row, col) is row-first over column-major storage ──
    // The translation sits at (row, col3), confirming the argument order
    // is (row, col), not (col, row).
    TEST( IndexOperator,
          RowFirstArgumentOrder )
    {
        const osg::dmat4 m = osg::translate( osg::dvec3( tx, ty, tz ) );

        EXPECT_DOUBLE_EQ( m( row0, col3 ), tx );
        EXPECT_DOUBLE_EQ( m( row1, col3 ), ty );
        EXPECT_DOUBLE_EQ( m( row2, col3 ), tz );

        // The transposed indices are the identity basis, not the translation.
        EXPECT_DOUBLE_EQ( m( row3, col0 ), zero );
        EXPECT_DOUBLE_EQ( m( row3, col1 ), zero );
        EXPECT_DOUBLE_EQ( m( row3, col2 ), zero );
    }

    // ── MatrixTransform composition: computeLocalToWorldMatrix does
    //    matrix = matrix * _matrix (RELATIVE_RF) ──
    TEST( MatrixTransformComposition,
          LocalToWorldPostMultipliesOwnMatrix )
    {
        constexpr double quarterTurn = 1.5707963267948966;    // pi/2 radians
        constexpr double axisX       = 0.0;
        constexpr double axisY       = 0.0;
        constexpr double axisZ       = 1.0;

        // A translation parent and a rotation child do NOT commute, so the
        // composition order is observable.
        const osg::dmat4 parentMat = osg::translate( osg::dvec3( tx, ty, tz ) );
        const osg::dmat4 childMat  = osg::rotate( quarterTurn, axisX, axisY, axisZ );

        const osg::ref_ptr<osg::MatrixTransform> child =
            osg::MatrixTransform::create( childMat );

        // computeLocalToWorldMatrix accumulates: matrix = matrix * _matrix,
        // so seeding with the parent's world matrix yields parent * child.
        osg::dmat4 accumulated = parentMat;
        child->computeLocalToWorldMatrix( accumulated, nullptr );

        EXPECT_TRUE( accumulated == ( parentMat * childMat ) );
        // And the order matters: child * parent differs.
        EXPECT_FALSE( accumulated == ( childMat * parentMat ) );
    }

    // ── compat.hpp preMult(m, other) == m = m * other ──
    TEST( CompatHelpers,
          PreMultIsRightMultiply )
    {
        constexpr double ax    = 5.0;
        constexpr double ay    = 6.0;
        constexpr double az    = 8.0;

        osg::dmat4       m     = osg::translate( osg::dvec3( tx, ty, tz ) );
        const osg::dmat4 oth   = osg::translate( osg::dvec3( ax, ay, az ) );
        const osg::dmat4 expct = m * oth;

        osg::preMult( m, oth );
        EXPECT_TRUE( m == expct );
    }

    // ── compat.hpp postMult(m, other) == m = other * m ──
    TEST( CompatHelpers,
          PostMultIsLeftMultiply )
    {
        constexpr double ax    = 5.0;
        constexpr double ay    = 6.0;
        constexpr double az    = 8.0;

        osg::dmat4       m     = osg::translate( osg::dvec3( tx, ty, tz ) );
        const osg::dmat4 oth   = osg::translate( osg::dvec3( ax, ay, az ) );
        const osg::dmat4 expct = oth * m;

        osg::postMult( m, oth );
        EXPECT_TRUE( m == expct );
    }

    // ── compat.hpp postMultTranslate(m, v) == m = translate(v) * m ──
    TEST( CompatHelpers,
          PostMultTranslateLeftMultipliesTranslation )
    {
        constexpr double vx = 1.0;
        constexpr double vy = 0.0;
        constexpr double vz = 0.0;

        osg::dmat4       m  = osg::translate( osg::dvec3( tx, ty, tz ) );
        const osg::dvec3 v( vx, vy, vz );
        const osg::dmat4 expct = osg::translate( v ) * m;

        osg::postMultTranslate( m, v );
        EXPECT_TRUE( m == expct );
    }

    // ── compat.hpp preMultTranslate(m, v) == m = m * translate(v) ──
    TEST( CompatHelpers,
          PreMultTranslateRightMultipliesTranslation )
    {
        constexpr double vx = 1.0;
        constexpr double vy = 0.0;
        constexpr double vz = 0.0;

        osg::dmat4       m  = osg::translate( osg::dvec3( tx, ty, tz ) );
        const osg::dvec3 v( vx, vy, vz );
        const osg::dmat4 expct = m * osg::translate( v );

        osg::preMultTranslate( m, v );
        EXPECT_TRUE( m == expct );
    }

    // ── perspective(fovy_degrees, aspect, zNear, zFar): OpenGL NDC ──
    // For fovy=90, aspect=1: f = cot(45°) = 1.
    //   operator()(row1, col1) = f = 1            (non-flipped, positive Y)
    //   operator()(row0, col0) = f/aspect = 1
    //   operator()(row2, col2) = -(zFar+zNear)/(zFar-zNear)
    //   operator()(row3, col2) = -1   (value[2][3]; the W = -z divide term)
    //   operator()(row2, col3) = -2*zFar*zNear/(zFar-zNear)  (value[3][2])
    //   operator()(row3, col3) = 0    (value[3][3]; OpenGL NDC, not Vulkan)
    TEST( Perspective,
          OpenGLNdcLayout )
    {
        constexpr double fovy   = 90.0;
        constexpr double aspect = 1.0;
        constexpr double zNear  = 1.0;
        constexpr double zFar   = 2.0;
        constexpr double two    = 2.0;
        constexpr double minus1 = -1.0;

        const osg::dmat4 m      = osg::perspective( fovy, aspect, zNear, zFar );

        // cot(45°) == 1, written non-flipped (positive) at (row1, col1).
        EXPECT_NEAR( m( row1, col1 ), one, eps );
        EXPECT_GT( m( row1, col1 ), zero );
        EXPECT_NEAR( m( row0, col0 ), one, eps );

        // Depth maps to -1..1 (OpenGL NDC), W = -z (column-vector divide).
        const double expectedZZ = -( zFar + zNear ) / ( zFar - zNear );
        const double expectedWZ = -( two * zFar * zNear ) / ( zFar - zNear );

        EXPECT_NEAR( m( row2, col2 ), expectedZZ, eps );
        EXPECT_NEAR( m( row3, col2 ), minus1, eps );
        EXPECT_NEAR( m( row2, col3 ), expectedWZ, eps );
        EXPECT_NEAR( m( row3, col3 ), zero, eps );
    }

    // ── quat aliases: quat is float, dquat is double ──
    static_assert( std::is_same_v<osg::quat,
                                  osg::t_quat<float>>,
                   "osg::quat must alias t_quat<float>" );
    static_assert( std::is_same_v<osg::dquat,
                                  osg::t_quat<double>>,
                   "osg::dquat must alias t_quat<double>" );
    static_assert( std::is_same_v<osg::quat::value_type,
                                  float>,
                   "osg::quat value_type must be float" );
    static_assert( std::is_same_v<osg::dquat::value_type,
                                  double>,
                   "osg::dquat value_type must be double" );

    TEST( QuatAliases,
          FloatAndDouble )
    {
        EXPECT_TRUE( ( std::is_same_v<osg::quat, osg::t_quat<float>> ));
        EXPECT_TRUE( ( std::is_same_v<osg::dquat, osg::t_quat<double>> ));
    }

    // ── box add/expand grows min/max as expected ──
    TEST( Box,
          AddAndExpandGrowBounds )
    {
        constexpr float a = -1.0F;
        constexpr float b = 2.0F;
        constexpr float c = 5.0F;

        osg::box        bb;
        EXPECT_FALSE( bb.valid() );

        bb.add( a, a, a );
        EXPECT_TRUE( bb.valid() );
        EXPECT_FLOAT_EQ( bb.xMin(), a );
        EXPECT_FLOAT_EQ( bb.xMax(), a );

        bb.add( b, b, b );
        EXPECT_FLOAT_EQ( bb.xMin(), a );
        EXPECT_FLOAT_EQ( bb.xMax(), b );

        bb.expandBy( osg::vec3( c, zeroF, zeroF ) );
        EXPECT_FLOAT_EQ( bb.xMax(), c );
        EXPECT_FLOAT_EQ( bb.yMax(), b );
    }

    // ── compat.hpp sphereFromBox encloses a box ──
    TEST( Box,
          SphereFromBoxEnclosesBox )
    {
        constexpr float   lo   = -1.0F;
        constexpr float   hi   = 1.0F;
        constexpr float   half = 0.5F;

        const osg::box    bb( lo, lo, lo, hi, hi, hi );
        const osg::sphere s = osg::sphereFromBox( bb );
        EXPECT_TRUE( s.valid() );

        // Symmetric box centred on the origin; radius = |max-min|/2.
        const float       expectedRadius = osg::length( bb.max - bb.min ) * half;
        const osg::sphere expected( osg::vec3( zeroF, zeroF, zeroF ), expectedRadius );
        EXPECT_TRUE( s == expected );
    }

}    // namespace
