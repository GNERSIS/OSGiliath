/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Single drawable + state + depth entry in a RenderBin.
 * The atomic unit of rendering in the draw traversal.
 */
#include <osgUtil/culling/RenderLeaf.hpp>

#include <osg/core/Notify.hpp>
#include <osgUtil/culling/StateGraph.hpp>

using namespace osg;
using namespace osgUtil;

void
RenderLeaf::render( osg::RenderInfo& renderInfo,
                    RenderLeaf*      previous )
{
    osg::State& state = *renderInfo.getState();

    // don't draw this leaf if the abort rendering flag has been set.
    if( state.getAbortRendering() )
    {
        // cout << "early abort"<<endl;
        return;
    }

    if( previous )
    {

        // apply matrices if required.
        state.applyProjectionMatrix( _projection.get() );
        state.applyModelViewMatrix( _modelview.get() );

        // apply state if required.
        StateGraph* prev_rg        = previous->_parent;
        StateGraph* prev_rg_parent = prev_rg->_parent;
        StateGraph* rg             = _parent;
        if( prev_rg_parent != rg->_parent )
        {
            StateGraph::moveStateGraph( state, prev_rg_parent, rg->_parent );

            // send state changes and matrix changes to OpenGL.
            state.apply( rg->getStateSet() );
        }
        else if( rg != prev_rg )
        {

            // send state changes and matrix changes to OpenGL.
            state.apply( rg->getStateSet() );
        }

        // if we are using osg::Program which requires OSG's generated uniforms to track
        // modelview and projection matrices then apply them now.
        if( state.getUseModelViewAndProjectionUniforms() )
        {
            state.applyModelViewAndProjectionUniformsIfRequired();
        }

        // draw the drawable
        _drawable->draw( renderInfo );
    }
    else
    {
        // apply matrices if required.
        state.applyProjectionMatrix( _projection.get() );
        state.applyModelViewMatrix( _modelview.get() );

        // apply state if required.
        StateGraph::moveStateGraph( state, NULL, _parent->_parent );

        state.apply( _parent->getStateSet() );

        // if we are using osg::Program which requires OSG's generated uniforms to track
        // modelview and projection matrices then apply them now.
        if( state.getUseModelViewAndProjectionUniforms() )
        {
            state.applyModelViewAndProjectionUniformsIfRequired();
        }

        // draw the drawable
        _drawable->draw( renderInfo );
    }

    if( _dynamic )
    {
        state.decrementDynamicObjectCount();
    }

    // OSG_NOTICE<<"RenderLeaf "<<_drawable->getName()<<" "<<_depth<<std::endl;
}
