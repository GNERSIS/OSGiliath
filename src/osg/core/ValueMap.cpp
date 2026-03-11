/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * String-keyed map of ValueObjects. Provides dict-like access
 * to user metadata on scene graph objects.
 */
#include <osg/core/ValueStack.hpp>
#include <stdlib.h>

using namespace osg;

ValueMap::ValueMap()
{
}

ValueMap::ValueMap( const ValueMap&    vm,
                    const osg::CopyOp& copyop ) :
    Inherit( vm,
             copyop )
{
    for( KeyValueMap::const_iterator itr = vm._keyValueMap.begin();
         itr != vm._keyValueMap.end();
         ++itr )
    {
        _keyValueMap[itr->first] = osg::clone( itr->second.get(), copyop );
    }
}

ValueMap::~ValueMap()
{
}
