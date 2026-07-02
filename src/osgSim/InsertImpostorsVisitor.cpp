/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Traversal visitor that inserts Impostor nodes into a scene.
 * Automatically wraps subtrees for impostor rendering.
 */
#if defined( _MSC_VER )
    #pragma warning( disable : 4'786 )
#endif

#include <osgSim/InsertImpostorsVisitor.hpp>

#include <algorithm>
#include <osgSim/Impostor.hpp>

using namespace osg;
using namespace osgSim;

InsertImpostorsVisitor::InsertImpostorsVisitor()
{
    setTraversalMode( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN );
    _impostorThresholdRatio    = 30.0F;
    _maximumNumNestedImpostors = 3;
    _numNestedImpostors        = 0;
}

void
InsertImpostorsVisitor::reset()
{
    _groupList.clear();
    _lodList.clear();
    _numNestedImpostors = 0;
}

void
InsertImpostorsVisitor::apply( Node& node )
{
    traverse( node );
}

void
InsertImpostorsVisitor::apply( Group& node )
{
    _groupList.push_back( &node );

    ++_numNestedImpostors;
    if( _numNestedImpostors < _maximumNumNestedImpostors )
    {
        traverse( node );
    }
    --_numNestedImpostors;
}

void
InsertImpostorsVisitor::apply( LOD& node )
{
    if( dynamic_cast<osgSim::Impostor*>( &node ) == 0 )
    {
        _lodList.push_back( &node );
    }

    ++_numNestedImpostors;
    if( _numNestedImpostors < _maximumNumNestedImpostors )
    {
        traverse( node );
    }
    --_numNestedImpostors;
}

/* insert the required impostors into the scene graph.*/
void
InsertImpostorsVisitor::insertImpostors()
{

    bool _insertImpostorsAboveGroups = true;
    bool _replaceLODsByImpostors     = true;

    // handle group's
    if( _insertImpostorsAboveGroups )
    {
        std::sort( _groupList.begin(), _groupList.end() );

        Group* previousGroup = NULL;
        for( GroupList::iterator itr = _groupList.begin(); itr != _groupList.end();
             ++itr )
        {
            Group* group = ( *itr );
            if( group != previousGroup )
            {
                const sphere& bs = group->getBound();
                if( bs.valid() )
                {

                    // take a copy of the original parent list
                    // before we change it around by adding the group
                    // to an impostor.
                    Node::ParentList parentList = group->getParents();

                    Impostor*        impostor   = new Impostor;

                    // standard LOD settings
                    impostor->addChild( group );
                    impostor->setRange( 0, 0.0F, 1E7F );

                    // impostor specific settings.
                    impostor->setImpostorThresholdToBound( _impostorThresholdRatio );

                    // now replace the group by the new impostor in all of the
                    // group's original parent list.
                    for( Node::ParentList::iterator pitr = parentList.begin();
                         pitr != parentList.end();
                         ++pitr )
                    {
                        ( *pitr )->replaceChild( group, impostor );
                    }
                }
            }
        }
    }

    // handle LOD's
    if( _replaceLODsByImpostors )
    {
        std::sort( _lodList.begin(), _lodList.end() );

        LOD* previousLOD = NULL;
        for( LODList::iterator itr = _lodList.begin(); itr != _lodList.end(); ++itr )
        {
            osg::LOD* lod = ( *itr );
            if( lod != previousLOD )
            {
                const osg::sphere& bs = lod->getBound();
                if( bs.valid() )
                {

                    // take a copy of the original parent list
                    // before we change it around by adding the lod
                    // to an impostor.
                    osg::Node::ParentList parentList = lod->getParents();

                    Impostor*             impostor   = new Impostor;

                    // standard LOD settings
                    for( unsigned int ci = 0; ci < lod->getNumChildren(); ++ci )
                    {
                        impostor->addChild( lod->getChild( ci ) );
                        impostor->setRange( ci,
                                            lod->getMinRange( ci ),
                                            lod->getMaxRange( ci ) );
                    }

                    impostor->setCenter( lod->getCenter() );
                    impostor->setCenterMode( lod->getCenterMode() );

                    // impostor specific settings.
                    impostor->setImpostorThresholdToBound( _impostorThresholdRatio );

                    // now replace the lod by the new impostor in all of the
                    // lod's original parent list.
                    for( Node::ParentList::iterator pitr = parentList.begin();
                         pitr != parentList.end();
                         ++pitr )
                    {
                        ( *pitr )->replaceChild( lod, impostor );
                    }
                }
            }
        }
    }
}
