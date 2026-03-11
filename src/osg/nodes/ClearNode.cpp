/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Sets the clear color and clear mask for its subgraph.
 * Typically used as the root node for background color.
 */
#include <osg/nodes/ClearNode.hpp>

#include <algorithm>

using namespace osg;

/**
 * ClearNode constructor.
 */
ClearNode::ClearNode() :
    _requiresClear( true ),
    _clearColor( 0.0F,
                 0.0F,
                 0.0F,
                 1.0F ),
    _clearMask( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT )
{
    setCullingActive( false );
    StateSet* stateset = new StateSet;
    stateset->setRenderBinDetails( -1, "RenderBin" );
    setStateSet( stateset );
}
