/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Stack of ValueMaps for hierarchical metadata resolution.
 * Used internally for layered property lookups.
 */
#include <osg/core/ValueStack.hpp>

#include <stdlib.h>

using namespace osg;

ValueStack::ValueStack()
{
}

ValueStack::ValueStack( const ValueStack&  ps,
                        const osg::CopyOp& copyop ) :
    Inherit( ps,
             copyop )
{
}

ValueStack::~ValueStack()
{
}
