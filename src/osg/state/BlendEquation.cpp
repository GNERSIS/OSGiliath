/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Blending equation mode (ADD, SUBTRACT, MIN, MAX, etc.).
 * Controls how source and destination colors are combined.
 */
#include <osg/state/BlendEquation.hpp>

#include <osg/core/buffered_value.hpp>
#include <osg/core/Notify.hpp>
#include <osg/rendering/GLExtensions.hpp>
#include <osg/state/State.hpp>

using namespace osg;

BlendEquation::BlendEquation() :
    _equationRGB( FUNC_ADD ),
    _equationAlpha( FUNC_ADD )
{
}

BlendEquation::BlendEquation( Equation equation ) :
    _equationRGB( equation ),
    _equationAlpha( equation )
{
}

BlendEquation::BlendEquation( Equation equationRGB,
                              Equation equationAlpha ) :
    _equationRGB( equationRGB ),
    _equationAlpha( equationAlpha )
{
}

BlendEquation::~BlendEquation()
{
}

void
BlendEquation::apply( State& state ) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();

    if( !extensions->isBlendEquationSupported )
    {
        OSG_WARN << "Warning: BlendEquation::apply(..) failed, BlendEquation is not "
                    "support by OpenGL driver."
                 << std::endl;
        return;
    }

    if( ( _equationRGB == ALPHA_MIN || _equationRGB == ALPHA_MAX ) &&
        !extensions->isSGIXMinMaxSupported )
    {
        OSG_WARN << "Warning: BlendEquation::apply(..) failed, SGIX_blend_alpha_minmax "
                    "extension is not supported by OpenGL driver."
                 << std::endl;
        return;
    }

    if( _equationRGB == LOGIC_OP && !extensions->isLogicOpSupported )
    {
        OSG_WARN << "Warning: BlendEquation::apply(..) failed, EXT_blend_logic_op "
                    "extension is not supported by OpenGL driver."
                 << std::endl;
        return;
    }

    if( _equationRGB == _equationAlpha )
    {
        extensions->glBlendEquation( static_cast<GLenum>( _equationRGB ) );
    }
    else
    {
        if( extensions->isBlendEquationSeparateSupported )
        {
            extensions->glBlendEquationSeparate( static_cast<GLenum>( _equationRGB ),
                                                 static_cast<GLenum>( _equationAlpha ) );
        }
        else
        {
            OSG_WARN << "Warning: BlendEquation::apply(..) failed, "
                        "EXT_blend_equation_separate extension is not supported by "
                        "OpenGL driver."
                     << std::endl;
            return;
        }
    }
}
