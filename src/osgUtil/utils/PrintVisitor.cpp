/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Debug visitor that prints the scene graph structure.
 * Outputs node hierarchy with indentation to a stream.
 */
#include <osgUtil/utils/PrintVisitor.hpp>

namespace osgUtil
{

    PrintVisitor::PrintVisitor( std::ostream& out,
                                int           indent,
                                int           step ) :
        osg::DualModeVisitor( osg::DualModeVisitor::TRAVERSE_ALL_CHILDREN ),
        _out( out ),
        _indent( static_cast<unsigned int>( indent ) ),
        _step( static_cast<unsigned int>( step ) )
    {
    }

    void
    PrintVisitor::apply( osg::Node& node )
    {
        output() << node.libraryName() << "::" << node.className() << std::endl;

        enter();
        traverse( node );
        leave();
    }

}
