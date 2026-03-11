#include <osg/state/DispatchCompute.hpp>

using namespace osg;

DispatchCompute::DispatchCompute( const DispatchCompute& o,
                                  const osg::CopyOp&     copyop ) :
    Inherit( o,
             copyop ),
    _numGroupsX( o._numGroupsX ),
    _numGroupsY( o._numGroupsY ),
    _numGroupsZ( o._numGroupsZ )
{
}

void
DispatchCompute::drawImplementation( RenderInfo& renderInfo ) const
{
    renderInfo.getState()->get<GLExtensions>()->glDispatchCompute(
        static_cast<GLuint>( _numGroupsX ),
        static_cast<GLuint>( _numGroupsY ),
        static_cast<GLuint>( _numGroupsZ )
    );
}
