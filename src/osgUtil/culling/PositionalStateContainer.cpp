/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Collects positional state (lights) encountered during culling
 * for application during the draw traversal.
 */
#include <osgUtil/culling/PositionalStateContainer.hpp>

using namespace osg;
using namespace osgUtil;

// register a PositionalStateContainer prototype with the RenderBin prototype list.
// RegisterRenderBinProxy<PositionalStateContainer>
// s_registerPositionalStateContainerProxy;

PositionalStateContainer::PositionalStateContainer()
{
}

PositionalStateContainer::~PositionalStateContainer()
{
}

void
PositionalStateContainer::reset()
{
    _attrList.clear();
    _texAttrListMap.clear();
}

void
PositionalStateContainer::draw( osg::State&       state,
                                RenderLeaf*&      previous,
                                const osg::dmat4* postMultMatrix )
{

    if( previous )
    {
        StateGraph::moveToRootStateGraph( state, previous->_parent );
        state.apply();
        previous = NULL;
    }

    // apply the light list.
    for( AttrMatrixList::iterator litr = _attrList.begin(); litr != _attrList.end();
         ++litr )
    {
        if( postMultMatrix )
        {
            if( ( *litr ).second.valid() )
            {
                state.applyModelViewMatrix(
                    new osg::RefMatrix( ( *postMultMatrix ) * ( *( ( *litr ).second ) ) )
                );
            }
            else
            {
                state.applyModelViewMatrix( new osg::RefMatrix( *postMultMatrix ) );
            }
        }

        else
        {
            state.applyModelViewMatrix( ( *litr ).second.get() );
        }

        // apply the light source.
        litr->first->apply( state );

        // tell state about.
        state.haveAppliedAttribute( litr->first.get() );

        // set this state as a global default
        state.setGlobalDefaultAttribute( litr->first.get() );
    }

    for( TexUnitAttrMatrixListMap::iterator titr = _texAttrListMap.begin();
         titr != _texAttrListMap.end();
         ++titr )
    {
        state.setActiveTextureUnit( titr->first );

        AttrMatrixList attrList = titr->second;

        for( AttrMatrixList::iterator litr = attrList.begin(); litr != attrList.end();
             ++litr )
        {
            if( postMultMatrix )
            {
                if( ( *litr ).second.valid() )
                {
                    state.applyModelViewMatrix( new osg::RefMatrix(
                        ( *postMultMatrix ) * ( *( ( *litr ).second ) )
                    ) );
                }
                else
                {
                    state.applyModelViewMatrix( new osg::RefMatrix( *postMultMatrix ) );
                }
            }
            else
            {
                state.applyModelViewMatrix( ( *litr ).second.get() );
            }

            // apply the light source.
            litr->first->apply( state );

            // tell state about.
            state.haveAppliedTextureAttribute( titr->first, litr->first.get() );

            // set this state as a global default
            state.setGlobalDefaultTextureAttribute( titr->first, litr->first.get() );
        }
    }
}
