/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Traverses the scene to detect display requirements.
 * Checks for stereo, stencil, and multisampling needs.
 */
#if defined( _MSC_VER )
    #pragma warning( disable : 4'786 )
#endif

#include <osgUtil/utils/DisplayRequirementsVisitor.hpp>

#include <list>
#include <set>
#include <stdio.h>
#include <string.h>

using namespace osg;
using namespace osgUtil;

DisplayRequirementsVisitor::DisplayRequirementsVisitor()
{
    setTraversalMode( NodeVisitor::TRAVERSE_ALL_CHILDREN );
}

void
DisplayRequirementsVisitor::applyStateSet( StateSet& stateset )
{
    if( !_ds )
    {
        _ds = new osg::DisplaySettings;
    }

    unsigned int min = 0;    // assume stencil not needed by this stateset.

    if( stateset.getMode( GL_STENCIL_TEST ) & StateAttribute::ON )
    {
        min = 1;    // number stencil bits we need at least.
    }

    if( stateset.getAttribute( StateAttribute::Type::STENCIL ) )
    {
        min = 1;    // number stencil bits we need at least.
    }

    if( min > _ds->getMinimumNumStencilBits() )
    {
        // only update if new minimum exceeds previous minimum.
        _ds->setMinimumNumStencilBits( min );
    }
}

void
DisplayRequirementsVisitor::apply( Node& node )
{
    osg::StateSet* stateset = node.getStateSet();
    if( stateset )
    {
        applyStateSet( *stateset );
    }

    if( strcmp( node.className(), "Impostor" ) == 0 )
    {
        if( !_ds )
        {
            _ds = new osg::DisplaySettings;
        }

        unsigned int min = 1;    // number alpha bits we need at least.
        if( min > _ds->getMinimumNumAlphaBits() )
        {
            // only update if new minimum exceeds previous minimum.
            _ds->setMinimumNumAlphaBits( min );
        }
    }

    traverse( node );
}
