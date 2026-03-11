/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Per-viewport rectangle for multi-viewport rendering.
 * Sets independent viewport regions for indexed viewports.
 */
#include <osg/state/ViewportIndexed.hpp>

#include <osg/rendering/GLExtensions.hpp>
#include <osg/state/State.hpp>

using namespace osg;

ViewportIndexed::ViewportIndexed() :
    _index( 0 )
{
}

ViewportIndexed::~ViewportIndexed()
{
}

void
ViewportIndexed::setIndex( unsigned int index )
{
    if( _index == index )
    {
        return;
    }

    ReassignToParents needToReassingToParentsWhenMemberValueChanges( this );

    _index = index;
}

void
ViewportIndexed::apply( State& state ) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    if( extensions->glViewportIndexedf )
    {
        extensions->glViewportIndexedf( static_cast<GLuint>( _index ),
                                        static_cast<GLfloat>( _x ),
                                        static_cast<GLfloat>( _y ),
                                        static_cast<GLfloat>( _width ),
                                        static_cast<GLfloat>( _height ) );
    }
    else
    {
        OSG_WARN << "Warning: ViewportIndexed::apply(..) failed, glViewportIndexed is "
                    "not support by OpenGL driver."
                 << std::endl;
    }
}
