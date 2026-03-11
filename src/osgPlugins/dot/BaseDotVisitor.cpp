/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * BaseDotVisitor — osgPlugins library implementation.
 */
#include "BaseDotVisitor.hpp"

#include <cassert>
#include <fstream>
#include <osg/core/Notify.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/Node.hpp>

using namespace osg;

namespace osgDot
{

    BaseDotVisitor::BaseDotVisitor() :
        osg::DualModeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
    {
        // setNodeMaskOverride(0xffffff);

        _rankdir = "rankdir = LR;";
        // Set the locale used by the _nodes and _edges streams to the
        //   classic or "C" locale. This is needed because most of the
        //   Graphviz tools are not locale sensitive and get confused
        //   by id numbers containing commas or periods.
        _nodes.imbue( std::locale( "C" ) );
        _edges.imbue( std::locale( "C" ) );
    }

    BaseDotVisitor::~BaseDotVisitor()
    {
    }

    void
    BaseDotVisitor::setOptions( const osgDB::Options* options )
    {
        _options = const_cast<osgDB::Options*>( options );
        OSG_INFO << "BaseDotVisitor::setOptions(" << options << ")" << std::endl;
        if( _options.valid() && !( _options->getOptionString().empty() ) )
        {

            std::string optionString = _options->getOptionString();

            OSG_INFO << "  BaseDotVisitor::optionString (" << optionString << ")"
                     << std::endl;

            std::string::size_type pos = optionString.find( "rankdir" );
            if( pos != std::string::npos )
            {
                std::string::size_type semi_pos = optionString.find( ";", pos );
                if( semi_pos != std::string::npos )
                {
                    _rankdir = optionString.substr( pos, semi_pos - pos );
                    OSG_INFO << "  BaseDotVisitor::Set _rankdir to " << _rankdir
                             << std::endl;
                }
            }
        }
    }

    bool
    BaseDotVisitor::run( const osg::Node& root,
                         std::ostream*    fout )
    {

        if( fout && *fout )
        {
            root.accept( static_cast<osg::ConstNodeVisitor&>( *this ) );

            *fout << "digraph osg_scenegraph { " << _rankdir << std::endl;

            *fout << "bgcolor=transparent;" << std::endl;

            *fout << _nodes.str() << _edges.str();

            *fout << "}" << std::endl;

            _nodes.clear();
            _edges.clear();
            _objectMap.clear();

            return true;
        }

        return false;
    }

    void
    BaseDotVisitor::apply( Node& node )
    {
        int id;
        if( getOrCreateId( &node, id ) )
        {
            handle( node, id );
            handleNodeAndTraverse( node, id );
        }
    }

    void
    BaseDotVisitor::apply( Drawable& drawable )
    {
        int id;
        if( getOrCreateId( &drawable, id ) )
        {
            handle( drawable, id );
            handleNodeAndTraverse( drawable, id );
        }
    }

    void
    BaseDotVisitor::apply( Group& node )
    {
        int id;

        if( getOrCreateId( &node, id ) )
        {
            handle( node, id );
            handleNodeAndTraverse( node, id );

            unsigned int i;
            for( i = 0; i < node.getNumChildren(); i++ )
            {
                osg::Node* child = node.getChild( i );
                // handleNodeAndTraverse( *child );
                int        id2;
                getOrCreateId( child, id2 );
                handle( node, *child, id, id2 );
            }
        }
    }

    void
    BaseDotVisitor::handle( osg::Node&,
                            int )
    {
    }

    void
    BaseDotVisitor::handle( osg::Group&,
                            int )
    {
    }

    void
    BaseDotVisitor::handle( osg::Group&,
                            osg::Node&,
                            int,
                            int )
    {
    }

    void
    BaseDotVisitor::handleNodeAndTraverse( osg::Node& node,
                                           int        id )
    {
        osg::StateSet* ss = node.getStateSet();
        if( ss )
        {
            int id2;
            if( getOrCreateId( ss, id2 ) )
            {
                handle( *ss, id2 );
            }
            handle( node, *ss, id, id2 );
        }
        traverse( node );
    }

    void
    BaseDotVisitor::handle( osg::StateSet&,
                            int )
    {
    }

    void
    BaseDotVisitor::handle( osg::Node&,
                            osg::StateSet&,
                            int,
                            int )
    {
    }

    void
    BaseDotVisitor::handle( osg::Drawable&,
                            int )
    {
    }

    void
    BaseDotVisitor::handle( osg::Drawable&,
                            osg::StateSet&,
                            int,
                            int )
    {
    }

    bool
    BaseDotVisitor::getOrCreateId( osg::Object* object,
                                   int&         id )
    {
        ObjectMap::iterator it = _objectMap.find( object );
        if( it != _objectMap.end() )
        {
            id = it->second;
            return false;
        }

        id                 = _objectMap.size();
        _objectMap[object] = id;
        return true;
    }

}    // namespace osgDot
