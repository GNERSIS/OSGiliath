/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Multisample anti-aliasing state attribute. Enables/configures
 * MSAA sample coverage and alpha-to-coverage modes.
 */
#include <osg/state/Multisample.hpp>

#include <osg/core/buffered_value.hpp>
#include <osg/core/Notify.hpp>
#include <osg/rendering/GLExtensions.hpp>
#include <osg/state/State.hpp>

using namespace osg;

Multisample::Multisample() :
    _mode( DONT_CARE )
{
    _coverage = 1;
    _invert   = false;
}

Multisample::~Multisample()
{
}

void
Multisample::apply( State& state ) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    if( !extensions->isMultisampleSupported )
    {
        OSG_WARN << "Warning: Multisample::apply(..) failed, Multisample is not support "
                    "by OpenGL driver."
                 << std::endl;
        return;
    }

    if( extensions->isMultisampleFilterHintSupported )
    {
        glHint( GL_MULTISAMPLE_FILTER_HINT_NV, _mode );
    }

    extensions->glSampleCoverage( _coverage, _invert );
}
