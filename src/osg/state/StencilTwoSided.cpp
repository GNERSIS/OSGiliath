/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Two-sided stencil operations. Configures independent stencil
 * test and ops for front and back faces in a single pass.
 */
#include <osg/state/StencilTwoSided.hpp>

#include <osg/rendering/GLExtensions.hpp>
#include <osg/state/State.hpp>

using namespace osg;

StencilTwoSided::StencilTwoSided()
{
    // set up same defaults as glStencilFunc.
    _func[FRONT] = _func[BACK] = Function::ALWAYS;
    _funcRef[FRONT] = _funcRef[BACK] = 0;
    _funcMask[FRONT] = _funcMask[BACK] = ~0U;

    // set up same defaults as glStencilOp.
    _sfail[FRONT] = _sfail[BACK] = Operation::KEEP;
    _zfail[FRONT] = _zfail[BACK] = Operation::KEEP;
    _zpass[FRONT] = _zpass[BACK] = Operation::KEEP;

    _writeMask[FRONT] = _writeMask[BACK] = ~0U;
}

StencilTwoSided::StencilTwoSided( const StencilTwoSided& stencil,
                                  const CopyOp&          copyop ) :
    Inherit( stencil,
             copyop )
{
    _func[FRONT]      = stencil._func[FRONT];
    _funcRef[FRONT]   = stencil._funcRef[FRONT];
    _funcMask[FRONT]  = stencil._funcMask[FRONT];
    _sfail[FRONT]     = stencil._sfail[FRONT];
    _zfail[FRONT]     = stencil._zfail[FRONT];
    _zpass[FRONT]     = stencil._zpass[FRONT];
    _writeMask[FRONT] = stencil._writeMask[FRONT];

    _func[BACK]       = stencil._func[BACK];
    _funcRef[BACK]    = stencil._funcRef[BACK];
    _funcMask[BACK]   = stencil._funcMask[BACK];
    _sfail[BACK]      = stencil._sfail[BACK];
    _zfail[BACK]      = stencil._zfail[BACK];
    _zpass[BACK]      = stencil._zpass[BACK];
    _writeMask[BACK]  = stencil._writeMask[BACK];
}

StencilTwoSided::~StencilTwoSided()
{
}

int
StencilTwoSided::compare( const StateAttribute& sa ) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Parameter macros below.
    COMPARE_StateAttribute_Types( StencilTwoSided, sa )

        // compare each parameter in turn against the rhs.
        COMPARE_StateAttribute_Parameter(
            _func[FRONT]
        ) COMPARE_StateAttribute_Parameter( _funcRef[FRONT] )
            COMPARE_StateAttribute_Parameter(
                _funcMask[FRONT]
            ) COMPARE_StateAttribute_Parameter( _sfail[FRONT] )
                COMPARE_StateAttribute_Parameter(
                    _zfail[FRONT]
                ) COMPARE_StateAttribute_Parameter( _zpass[FRONT] )
                    COMPARE_StateAttribute_Parameter(
                        _writeMask[FRONT]
                    ) COMPARE_StateAttribute_Parameter( _func[BACK] )
                        COMPARE_StateAttribute_Parameter(
                            _funcRef[BACK]
                        ) COMPARE_StateAttribute_Parameter( _funcMask[BACK] )
                            COMPARE_StateAttribute_Parameter( _sfail[BACK] )
                                COMPARE_StateAttribute_Parameter( _zfail[BACK] )
                                    COMPARE_StateAttribute_Parameter( _zpass[BACK] )
                                        COMPARE_StateAttribute_Parameter(
                                            _writeMask[BACK]
                                        ) return 0;    // passed all the above comparison
                                                       // macros, must be equal.
}

void
StencilTwoSided::apply( State& state ) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();

    // Use core OpenGL 2.0+ separate stencil functions (required in Core Profile)
    // front face
    extensions->glStencilOpSeparate( GL_FRONT,
                                     ( GLenum )_sfail[FRONT],
                                     ( GLenum )_zfail[FRONT],
                                     ( GLenum )_zpass[FRONT] );
    extensions->glStencilMaskSeparate( GL_FRONT, _writeMask[FRONT] );
    extensions->glStencilFuncSeparate( GL_FRONT,
                                       ( GLenum )_func[FRONT],
                                       _funcRef[FRONT],
                                       _funcMask[FRONT] );

    // back face
    extensions->glStencilOpSeparate( GL_BACK,
                                     ( GLenum )_sfail[BACK],
                                     ( GLenum )_zfail[BACK],
                                     ( GLenum )_zpass[BACK] );
    extensions->glStencilMaskSeparate( GL_BACK, _writeMask[BACK] );
    extensions->glStencilFuncSeparate( GL_BACK,
                                       ( GLenum )_func[BACK],
                                       _funcRef[BACK],
                                       _funcMask[BACK] );
}
