/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Per-viewport scissor rectangle for multi-viewport rendering.
 * Sets independent clip regions for indexed viewports.
 */
#include <osg/state/ScissorIndexed.hpp>

#include <osg/rendering/GLExtensions.hpp>
#include <osg/state/State.hpp>

using namespace osg;

ScissorIndexed::ScissorIndexed() :
    _index( 0 ),
    _x( 0.0F ),
    _y( 0.0F ),
    _width( 800.0F ),
    _height( 600.0F )    // defaults same as osg::Viewport and osg::Scissor
{
}

ScissorIndexed::~ScissorIndexed()
{
}

void
ScissorIndexed::setIndex( unsigned int index )
{
    if( _index == index )
    {
        return;
    }

    ReassignToParents needToReassingToParentsWhenMemberValueChanges( this );

    _index = index;
}

void
ScissorIndexed::apply( State& state ) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    if( extensions->glScissorIndexed )
    {
        extensions->glScissorIndexed( static_cast<GLuint>( _index ),
                                      static_cast<GLint>( _x ),
                                      static_cast<GLint>( _y ),
                                      static_cast<GLsizei>( _width ),
                                      static_cast<GLsizei>( _height ) );
    }
    else
    {
        OSG_WARN << "Warning: ScissorIndexed::apply(..) failed, glScissorIndexed is not "
                    "support by OpenGL driver."
                 << std::endl;
    }
}
