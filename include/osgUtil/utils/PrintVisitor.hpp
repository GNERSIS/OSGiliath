/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Debug visitor that prints the scene graph structure.
 * Outputs node hierarchy with indentation to a stream.
 */
#pragma once

#include <osg/traversal/NodeVisitor.hpp>
#include <osgUtil/Export>
#include <ostream>

namespace osgUtil
{

    class OSGUTIL_EXPORT PrintVisitor : public osg::DualModeVisitor
    {
        public:

            PrintVisitor( std::ostream& out,
                          int           indent = 0,
                          int           step   = 2 );

            using osg::DualModeVisitor::apply;

            void
            apply( osg::Node& node );

            std::ostream&
            output()
            {
                for( unsigned int i = 0; i < _indent; ++i )
                {
                    _out << " ";
                }
                return _out;
            }

            void
            enter()
            {
                _indent += _step;
            }

            void
            leave()
            {
                _indent -= _step;
            }

        protected:

            PrintVisitor&
            operator=( const PrintVisitor& )
            {
                return *this;
            }

            std::ostream& _out;
            unsigned int  _indent;
            unsigned int  _step;
    };

}
