/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Vertex attribute instance divisor for instanced rendering.
 * Controls per-instance vs. per-vertex attribute advance rate.
 */
#include <osg/state/VertexAttribDivisor.hpp>

#include <osg/rendering/GLExtensions.hpp>
#include <osg/state/State.hpp>

using namespace osg;

VertexAttribDivisor::VertexAttribDivisor() :
    _index( 0 ),
    _divisor( 0 )
{
}

VertexAttribDivisor::VertexAttribDivisor( unsigned int index,
                                          unsigned int divisor ) :
    _index( index ),
    _divisor( divisor )
{
}

VertexAttribDivisor::~VertexAttribDivisor()
{
}

void
VertexAttribDivisor::apply( State& state ) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    if( extensions->glVertexAttribDivisor )
    {
        extensions->glVertexAttribDivisor( _index, _divisor );
    }
}
