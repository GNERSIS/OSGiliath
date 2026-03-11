/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Per-viewport depth range attribute. Sets independent near/far
 * depth ranges for indexed viewports in multi-viewport rendering.
 */
#include <osg/state/DepthRangeIndexed.hpp>

#include <osg/rendering/GLExtensions.hpp>
#include <osg/state/State.hpp>

using namespace osg;

DepthRangeIndexed::DepthRangeIndexed() :
    _index( 0 ),
    _zNear( 0.0 ),
    _zFar( 1.0 )
{
}

DepthRangeIndexed::~DepthRangeIndexed()
{
}

void
DepthRangeIndexed::setIndex( unsigned int index )
{
    if( _index == index )
    {
        return;
    }

    ReassignToParents needToReassingToParentsWhenMemberValueChanges( this );

    _index = index;
}

void
DepthRangeIndexed::apply( State& state ) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    if( extensions->glDepthRangeIndexed )
    {
        extensions->glDepthRangeIndexed( static_cast<GLuint>( _index ),
                                         static_cast<GLdouble>( _zNear ),
                                         static_cast<GLdouble>( _zFar ) );
    }
    else if( extensions->glDepthRangeIndexedf )
    {
        extensions->glDepthRangeIndexedf( static_cast<GLuint>( _index ),
                                          static_cast<GLfloat>( _zNear ),
                                          static_cast<GLfloat>( _zFar ) );
    }
    else
    {
        OSG_WARN << "Warning: DepthRangeIndexed::apply(..) failed, glDepthRangeIndexed "
                    "is not support by OpenGL driver."
                 << std::endl;
    }
}
