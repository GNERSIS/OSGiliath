/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Primitive restart index attribute. Sets the special index value
 * that restarts a triangle strip or line strip mid-stream.
 */
#include <osg/geometry/PrimitiveRestartIndex.hpp>

#include <osg/core/Notify.hpp>
#include <osg/rendering/GLExtensions.hpp>
#include <osg/state/State.hpp>

using namespace osg;

PrimitiveRestartIndex::PrimitiveRestartIndex()
{
    _restartIndex = 0;
}

PrimitiveRestartIndex::PrimitiveRestartIndex( unsigned int restartIndex )
{
    _restartIndex = restartIndex;
}

PrimitiveRestartIndex::PrimitiveRestartIndex(
    const PrimitiveRestartIndex& primitiveRestartIndex,
    const CopyOp&                copyop
) :
    Inherit( primitiveRestartIndex,
             copyop )
{
    _restartIndex = primitiveRestartIndex._restartIndex;
}

PrimitiveRestartIndex::~PrimitiveRestartIndex()
{
}

int
PrimitiveRestartIndex::compare( const StateAttribute& sa ) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Parameter macros below.
    COMPARE_StateAttribute_Types( PrimitiveRestartIndex, sa )

        COMPARE_StateAttribute_Parameter(
            _restartIndex
        ) return 0;    // passed all the above comparison macros, must be equal.
}

void
PrimitiveRestartIndex::apply( State& state ) const
{
    // get "per-context" extensions
    const GLExtensions* extensions = state.get<GLExtensions>();
    if( extensions->glPrimitiveRestartIndex )
    {
        extensions->glPrimitiveRestartIndex( _restartIndex );
        return;
    }

    OSG_WARN << "PrimitiveRestartIndex failed as the required graphics capabilities "
                "were not found\n"
                "   OpenGL 3.1 or GL_NV_primitive_restart extension is required."
             << std::endl;
}
