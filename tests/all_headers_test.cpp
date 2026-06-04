/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */

// Standalone-in-sequence header compilation gate. The generated header
// all_osg_headers.hpp #includes every public osg/**/*.hpp (minus the
// documented exclusions in tests/CMakeLists.txt). The value of this TU
// is that it compiles at all under the full strict toolchain (-Werror);
// the runtime assertion is trivial.
//
// NOLINTBEGIN(misc-include-cleaner): all_osg_headers.hpp is intentionally
// included purely so its transitive includes are compiled; it exposes no
// symbol this TU names directly, which is exactly the point of the test.
#include <all_osg_headers.hpp>
// NOLINTEND(misc-include-cleaner)
#include <gtest/gtest.h>

namespace
{

    TEST( AllHeaders,
          CompileUnderStrictToolchain )
    {
        EXPECT_TRUE( true );
    }

}    // namespace
