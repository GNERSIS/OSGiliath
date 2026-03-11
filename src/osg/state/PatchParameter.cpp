/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Tessellation patch parameter attribute. Sets the number of
 * control points per patch and inner/outer tessellation levels.
 */
#include <osg/state/PatchParameter.hpp>

#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osg/state/State.hpp>

using namespace osg;

PatchParameter::PatchParameter( GLint vertices ) :
    _vertices( vertices ),
    _patchDefaultInnerLevel( 1.0F,
                             1.0F ),
    _patchDefaultOuterLevel( 1.0F,
                             1.0F,
                             1.0F,
                             1.0F )
{
}

PatchParameter::~PatchParameter()
{
}

void
PatchParameter::apply( State& state ) const
{
    GLExtensions* extensions = state.get<GLExtensions>();
    if( extensions->areTessellationShadersSupported )
    {

        extensions->glPatchParameteri( GL_PATCH_VERTICES, _vertices );
        extensions->glPatchParameterfv( GL_PATCH_DEFAULT_INNER_LEVEL,
                                        _patchDefaultInnerLevel.data() );
        extensions->glPatchParameterfv( GL_PATCH_DEFAULT_OUTER_LEVEL,
                                        _patchDefaultOuterLevel.data() );
    }
}
