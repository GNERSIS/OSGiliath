/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Stencil buffer test and operation configuration. Used for shadows,
 * reflections, portals, and multi-pass masking techniques.
 */
#include <osg/state/Stencil.hpp>

#include <osg/core/Notify.hpp>
#include <osg/rendering/GLExtensions.hpp>
#include <osg/state/State.hpp>

using namespace osg;

Stencil::Stencil()
{
    // set up same defaults as glStencilFunc.
    _func     = Function::ALWAYS;
    _funcRef  = 0;
    _funcMask = ~0U;

    // set up same defaults as glStencilOp.
    _sfail = Operation::KEEP;
    _zfail = Operation::KEEP;
    _zpass = Operation::KEEP;

    // set up same defaults as glStencilMask.
    _writeMask = ~0U;
}

Stencil::~Stencil()
{
}

static Stencil::Operation
validateOperation( const GLExtensions* extensions,
                   Stencil::Operation  op )
{
    // only wrap requires validation
    if( op != Stencil::Operation::INCR_WRAP && op != Stencil::Operation::DECR_WRAP )
    {
        return op;
    }

    // wrap support
    if( extensions->isStencilWrapSupported )
    {
        return op;
    }
    else
    {
        return op == Stencil::Operation::INCR_WRAP ? Stencil::Operation::INCR
                                                   : Stencil::Operation::DECR;
    }
}

void
Stencil::apply( State& state ) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    Operation           sf         = validateOperation( extensions, _sfail );
    Operation           zf         = validateOperation( extensions, _zfail );
    Operation           zp         = validateOperation( extensions, _zpass );

    glStencilFunc( ( GLenum )_func, _funcRef, _funcMask );
    glStencilOp( ( GLenum )sf, ( GLenum )zf, ( GLenum )zp );
    glStencilMask( _writeMask );
}
