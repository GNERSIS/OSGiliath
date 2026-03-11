/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Rendering hint attribute for implementation-quality trade-offs.
 * Controls line smoothing, polygon smoothing, and fog quality hints.
 */
#include <osg/state/Hint.hpp>

#include <osg/state/GLDefines.hpp>
#include <osg/state/StateSet.hpp>

using namespace osg;

static bool
isValidCoreProfileHintTarget( GLenum target )
{
    switch( target )
    {
        case GL_LINE_SMOOTH_HINT :
        case GL_POLYGON_SMOOTH_HINT :
        case GL_TEXTURE_COMPRESSION_HINT :
        case GL_FRAGMENT_SHADER_DERIVATIVE_HINT :
            return true;
        default :
            return false;
    }
}

void
Hint::apply( State& /*state*/ ) const
{
    if( _target == GL_NONE || _mode == GL_NONE )
    {
        return;
    }

    // In Core Profile only certain hint targets are valid.
    // Deprecated targets (GL_FOG_HINT, GL_GENERATE_MIPMAP_HINT,
    // GL_PERSPECTIVE_CORRECTION_HINT, GL_POINT_SMOOTH_HINT) are skipped.
    if( !isValidCoreProfileHintTarget( _target ) )
    {
        return;
    }

    glHint( _target, _mode );
}

void
Hint::setTarget( GLenum target )
{
    if( _target == target )
    {
        return;
    }

    ReassignToParents needToReassingToParentsWhenMemberValueChanges( this );

    _target = target;
}
