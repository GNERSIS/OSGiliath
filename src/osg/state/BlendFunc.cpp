/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Alpha blending function state attribute. Configures source and
 * destination blend factors for transparency rendering.
 */
#include <osg/state/BlendFunc.hpp>

#include <osg/core/Notify.hpp>
#include <osg/rendering/GLExtensions.hpp>
#include <osg/state/State.hpp>

using namespace osg;

BlendFunc::BlendFunc() :
    _source_factor( SRC_ALPHA ),
    _destination_factor( ONE_MINUS_SRC_ALPHA ),
    _source_factor_alpha( SRC_ALPHA ),
    _destination_factor_alpha( ONE_MINUS_SRC_ALPHA )
{
}

BlendFunc::BlendFunc( GLenum source,
                      GLenum destination ) :
    _source_factor( source ),
    _destination_factor( destination ),
    _source_factor_alpha( source ),
    _destination_factor_alpha( destination )
{
}

BlendFunc::BlendFunc( GLenum source,
                      GLenum destination,
                      GLenum source_alpha,
                      GLenum destination_alpha ) :
    _source_factor( source ),
    _destination_factor( destination ),
    _source_factor_alpha( source_alpha ),
    _destination_factor_alpha( destination_alpha )
{
}

BlendFunc::~BlendFunc()
{
}

void
BlendFunc::apply( State& state ) const
{
    if( _source_factor !=
        _source_factor_alpha ||
        _destination_factor != _destination_factor_alpha )
    {
        const GLExtensions* extensions = state.get<GLExtensions>();
        if( !extensions->isBlendFuncSeparateSupported )
        {
            OSG_WARN << "Warning: BlendFunc::apply(..) failed, BlendFuncSeparate is not "
                        "support by OpenGL driver, falling back to BlendFunc."
                     << std::endl;
        }
        else
        {
            extensions->glBlendFuncSeparate( _source_factor,
                                             _destination_factor,
                                             _source_factor_alpha,
                                             _destination_factor_alpha );
            return;
        }
    }

    glBlendFunc( _source_factor, _destination_factor );
}
