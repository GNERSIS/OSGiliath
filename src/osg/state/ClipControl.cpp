/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Clip volume control attribute. Configures clip origin
 * (lower-left or upper-left) and depth range (negative-one-to-one
 * or zero-to-one).
 */
#include <osg/state/ClipControl.hpp>

#include <osg/rendering/GLExtensions.hpp>
#include <osg/state/State.hpp>

using namespace osg;

ClipControl::ClipControl( Origin    origin,
                          DepthMode depthMode ) :
    _origin( origin ),
    _depthMode( depthMode )
{
}

ClipControl::ClipControl( const ClipControl& clipControl,
                          const CopyOp&      copyop ) :
    Inherit( clipControl,
             copyop ),
    _origin( clipControl._origin ),
    _depthMode( clipControl._depthMode )
{
}

ClipControl::~ClipControl()
{
}

void
ClipControl::apply( State& state ) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();

    if( !extensions->isClipControlSupported )
    {
        return;
    }

    extensions->glClipControl( ( GLenum )_origin, ( GLenum )_depthMode );
}
