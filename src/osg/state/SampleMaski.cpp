/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Sample mask for multisample rendering. Controls which samples
 * are written during MSAA via glSampleMaski.
 */
#include <osg/state/SampleMaski.hpp>

#include <osg/core/Notify.hpp>
#include <osg/rendering/GLExtensions.hpp>
#include <osg/state/State.hpp>

using namespace osg;

SampleMaski::SampleMaski()
{
    _sampleMask[0U] = ~0U;
    _sampleMask[1U] = ~0U;
}

SampleMaski::SampleMaski( const SampleMaski& sampleMaski,
                          const CopyOp&      copyop ) :
    Inherit( sampleMaski,
             copyop )
{
    _sampleMask[0U] = sampleMaski._sampleMask[0U];
    _sampleMask[1U] = sampleMaski._sampleMask[1U];
}

SampleMaski::~SampleMaski()
{
}

int
SampleMaski::compare( const StateAttribute& sa ) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Parameter macros below.
    COMPARE_StateAttribute_Types( SampleMaski, sa )

        COMPARE_StateAttribute_Parameter( _sampleMask[0U] )
            COMPARE_StateAttribute_Parameter(
                _sampleMask[1U]
            ) return 0;    // passed all the above comparison macros, must be equal.
}

void
SampleMaski::apply( State& state ) const
{
    // get "per-context" extensions
    const GLExtensions* extensions = state.get<GLExtensions>();

    if( ( extensions->isTextureMultisampleSupported ) ||
        ( extensions->isOpenGL32upported ) ||
        ( extensions->isSampleMaskiSupported ) )
    {
        extensions->glSampleMaski( 0U, _sampleMask[0U] );
        // For now we use only 32-bit Sample mask
        //         extensions->glSampleMaski(1u, _sampleMask[1u]);
        return;
    }

    OSG_WARN
        << "SampleMaski failed as the required graphics capabilities were not found. \n"
           "OpenGL 3.2 or  ARB_texture_multisample extension is required."
        << std::endl;
}
