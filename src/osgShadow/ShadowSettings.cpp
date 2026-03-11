/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Shadow rendering configuration. Controls map resolution,
 * number of cascades, bias, and technique parameters.
 */
#include <osgShadow/ShadowSettings>

#include <float.h>

using namespace osgShadow;

ShadowSettings::ShadowSettings() :
    _receivesShadowTraversalMask( 0XFF'FF'FF'FF ),
    _castsShadowTraversalMask( 0XFF'FF'FF'FF ),
    _computeNearFearModeOverride( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR ),
    _lightNum( -1 ),
    _baseShadowTextureUnit( 1 ),
    _useShadowMapTextureOverride( true ),
    _textureSize( 2'048,
                  2'048 ),
    _minimumShadowMapNearFarRatio( 0.05 ),
    _maximumShadowMapDistance( DBL_MAX ),
    _shadowMapProjectionHint( PERSPECTIVE_SHADOW_MAP ),
    _perspectiveShadowMapCutOffAngle( 2.0 ),
    _numShadowMapsPerLight( 1 ),
    _multipleShadowMapHint( PARALLEL_SPLIT ),
    _shaderHint( NO_SHADERS ),
    // _shaderHint(PROVIDE_FRAGMENT_SHADER),
    _debugDraw( false )
{
    //_computeNearFearModeOverride =
    // osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES; _computeNearFearModeOverride
    //= osg::CullSettings::COMPUTE_NEAR_USING_PRIMITIVES);
}

ShadowSettings::ShadowSettings( const ShadowSettings& ss,
                                const osg::CopyOp&    copyop ) :
    Inherit( ss,
             copyop ),
    _receivesShadowTraversalMask( ss._receivesShadowTraversalMask ),
    _castsShadowTraversalMask( ss._castsShadowTraversalMask ),
    _computeNearFearModeOverride( ss._computeNearFearModeOverride ),
    _lightNum( ss._lightNum ),
    _baseShadowTextureUnit( ss._baseShadowTextureUnit ),
    _useShadowMapTextureOverride( ss._useShadowMapTextureOverride ),
    _textureSize( ss._textureSize ),
    _minimumShadowMapNearFarRatio( ss._minimumShadowMapNearFarRatio ),
    _maximumShadowMapDistance( ss._maximumShadowMapDistance ),
    _shadowMapProjectionHint( ss._shadowMapProjectionHint ),
    _perspectiveShadowMapCutOffAngle( ss._perspectiveShadowMapCutOffAngle ),
    _numShadowMapsPerLight( ss._numShadowMapsPerLight ),
    _multipleShadowMapHint( ss._multipleShadowMapHint ),
    _shaderHint( ss._shaderHint ),
    _debugDraw( ss._debugDraw )
{
}

ShadowSettings::~ShadowSettings()
{
}
