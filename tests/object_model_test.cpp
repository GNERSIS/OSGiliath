/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */

// Locks down the OSGiliath object model: the Drawable IS-A Node /
// Geometry IS-A Drawable hierarchy, the Inherit<> CRTP factory and
// introspection (create/className/libraryName/cloneType), the
// non-dynamic_cast cast<T>() downcast, and ref_ptr / observer_ptr
// lifetime semantics.

#include <gtest/gtest.h>
#include <osg/core/Object.hpp>
#include <osg/core/observer_ptr.hpp>
#include <osg/core/ref_ptr.hpp>
#include <osg/geometry/Drawable.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/Node.hpp>
#include <type_traits>

namespace
{

    // ── Static class hierarchy ──
    static_assert( std::is_base_of_v<osg::Node,
                                     osg::Drawable>,
                   "Drawable must derive from Node" );
    static_assert( std::is_base_of_v<osg::Drawable,
                                     osg::Geometry>,
                   "Geometry must derive from Drawable" );
    static_assert( std::is_base_of_v<osg::Object,
                                     osg::Node>,
                   "Node must derive from Object" );

    constexpr int         oneRef     = 1;
    constexpr int         twoRefs    = 2;

    constexpr const char* kOsgLib    = "osg";
    constexpr const char* kGeomClass = "Geometry";

    TEST( Hierarchy,
          StaticBaseOf )
    {
        EXPECT_TRUE( ( std::is_base_of_v<osg::Node, osg::Drawable> ));
        EXPECT_TRUE( ( std::is_base_of_v<osg::Drawable, osg::Geometry> ));
    }

    // ── Inherit<> CRTP: create() returns ref_ptr, names + cloneType ──
    TEST( InheritCrtp,
          CreateReturnsRefPtr )
    {
        const osg::ref_ptr<osg::Geometry> geom = osg::Geometry::create();
        ASSERT_TRUE( geom.valid() );
        EXPECT_EQ( geom->referenceCount(), oneRef );

        // create() yields ref_ptr<Geometry>, not a naked pointer.
        static_assert( std::is_same_v<decltype( osg::Geometry::create() ),
                                      osg::ref_ptr<osg::Geometry>>,
                       "Geometry::create() must return ref_ptr<Geometry>" );
    }

    TEST( InheritCrtp,
          ClassAndLibraryName )
    {
        const osg::ref_ptr<osg::Geometry> geom = osg::Geometry::create();
        EXPECT_STREQ( geom->className(), kGeomClass );
        EXPECT_STREQ( geom->libraryName(), kOsgLib );
    }

    TEST( InheritCrtp,
          CloneTypeProducesSameKind )
    {
        const osg::ref_ptr<osg::Geometry> geom = osg::Geometry::create();

        // cloneType() returns a raw owning pointer; adopt it immediately.
        const osg::ref_ptr<osg::Object>   fresh( geom->cloneType() );
        ASSERT_TRUE( fresh.valid() );

        // cloneType() makes a default-constructed object of the same kind.
        EXPECT_TRUE( geom->isSameKindAs( fresh.get() ) );
        EXPECT_STREQ( fresh->className(), kGeomClass );
        EXPECT_EQ( &fresh->type_info(), &geom->type_info() );
    }

    // ── cast<T>(): non-dynamic_cast downcast via is_compatible() chain ──
    TEST( Cast,
          UpcastThenDowncastSucceeds )
    {
        const osg::ref_ptr<osg::Geometry> geom  = osg::Geometry::create();
        osg::Object* const                asObj = geom.get();

        // Object* down to Geometry* succeeds.
        auto* const                       asGeom = asObj->cast<osg::Geometry>();
        EXPECT_EQ( asGeom, geom.get() );

        // Object* down to an intermediate base also succeeds.
        auto* const asDraw = asObj->cast<osg::Drawable>();
        EXPECT_EQ( asDraw, static_cast<osg::Drawable*>( geom.get() ) );
    }

    TEST( Cast,
          WrongTypeReturnsNull )
    {
        const osg::ref_ptr<osg::Geometry> geom  = osg::Geometry::create();
        osg::Object* const                asObj = geom.get();

        // A Geometry is not a Group: cast must fail with nullptr.
        auto* const                       asGroup = asObj->cast<osg::Group>();
        EXPECT_EQ( asGroup, nullptr );
    }

    // The is_compatible()/cast<T>() chain recognizes only types that
    // participate in the Inherit<> CRTP plus the Object base case. Geometry
    // and Drawable are Inherit<> links and match; Object matches via the
    // base-class fallback. Node is a hand-written class that derives from
    // Object WITHOUT overriding is_compatible(), so it is invisible to the
    // chain even though Geometry genuinely IS-A Node by C++ inheritance
    // (see the Hierarchy.StaticBaseOf static_asserts above). Consequently
    // cast<osg::Node>() returns nullptr — a sharp edge worth pinning.
    TEST( Cast,
          IsCompatibleChainCoversInheritLinksOnly )
    {
        const osg::ref_ptr<osg::Geometry> geom = osg::Geometry::create();

        EXPECT_TRUE( geom->is_compatible( typeid( osg::Geometry ) ) );
        EXPECT_TRUE( geom->is_compatible( typeid( osg::Drawable ) ) );
        EXPECT_TRUE( geom->is_compatible( typeid( osg::Object ) ) );

        // Node is not an Inherit<> link, so it is absent from the chain.
        EXPECT_FALSE( geom->is_compatible( typeid( osg::Node ) ) );
        EXPECT_EQ( geom->cast<osg::Node>(), nullptr );

        // An unrelated type never matches.
        EXPECT_FALSE( geom->is_compatible( typeid( osg::Group ) ) );
    }

    // ── ref_ptr reference counting ──
    TEST( RefPtr,
          CopyIncrementsAndScopeDecrements )
    {
        const osg::ref_ptr<osg::Geometry> geom = osg::Geometry::create();
        EXPECT_EQ( geom->referenceCount(), oneRef );

        // A second owning handle bumps the count; both point at one object.
        osg::ref_ptr<osg::Geometry> second = geom;
        EXPECT_EQ( geom->referenceCount(), twoRefs );
        EXPECT_EQ( second.get(), geom.get() );

        // Releasing the second handle decrements the count.
        second = nullptr;
        EXPECT_EQ( geom->referenceCount(), oneRef );
    }

    // ── observer_ptr::lock(): valid while alive, empty after release ──
    TEST( ObserverPtr,
          LockValidWhileAliveEmptyAfterRelease )
    {
        osg::observer_ptr<osg::Geometry> observer;

        {
            const osg::ref_ptr<osg::Geometry> geom = osg::Geometry::create();
            observer                               = geom;

            osg::ref_ptr<osg::Geometry> locked;
            EXPECT_TRUE( observer.lock( locked ) );
            ASSERT_TRUE( locked.valid() );
            EXPECT_EQ( locked.get(), geom.get() );
            // The lock added a transient owner that is released here.
            EXPECT_EQ( geom->referenceCount(), twoRefs );
        }

        // The observed object is gone; lock() must yield an empty ref_ptr.
        osg::ref_ptr<osg::Geometry> afterRelease;
        EXPECT_FALSE( observer.lock( afterRelease ) );
        EXPECT_FALSE( afterRelease.valid() );
    }

}    // namespace
