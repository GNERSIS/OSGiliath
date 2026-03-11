/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Default update traversal visitor. Runs update callbacks on
 * nodes and statesets each frame before culling.
 */
#include <osgUtil/culling/UpdateVisitor.hpp>

using namespace osg;
using namespace osgUtil;

UpdateVisitor::UpdateVisitor() :
    osg::DualModeVisitor( osg::DualModeVisitor::UPDATE_VISITOR,
                          osg::DualModeVisitor::TRAVERSE_ALL_CHILDREN )
{
}

UpdateVisitor::~UpdateVisitor()
{
}

void
UpdateVisitor::reset()
{
}
