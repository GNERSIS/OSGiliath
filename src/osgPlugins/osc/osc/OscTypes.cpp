/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osgPlugins: BeginBundleImmediate.
 */
#include "OscTypes.hpp"

namespace osc
{

    BundleInitiator   BeginBundleImmediate( 1 );
    BundleTerminator  EndBundle;
    MessageTerminator EndMessage;
    NilType           Nil;
    InfinitumType     Infinitum;

}    // namespace osc
