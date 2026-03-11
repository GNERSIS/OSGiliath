/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Configuration for view frustum culling behavior: small feature
 * threshold, near/far computation mode, and culling mask.
 */
#include <osg/traversal/CullSettings.hpp>

#include <osg/core/ApplicationUsage.hpp>
#include <osg/core/ArgumentParser.hpp>
#include <osg/core/Notify.hpp>
#include <osg/core/os_utils.hpp>
#include <stdlib.h>
#include <string.h>

using namespace osg;

CullSettings::CullSettings( const CullSettings& cs )
{
    setCullSettings( cs );
}

void
CullSettings::setDefaults()
{
    _inheritanceMask                         = ALL_VARIABLES;
    _inheritanceMaskActionOnAttributeSetting = DISABLE_ASSOCIATED_INHERITANCE_MASK_BIT;
    _cullingMode                             = DEFAULT_CULLING;
    _LODScale                                = 1.0F;
    _smallFeatureCullingPixelSize            = 2.0F;

    _computeNearFar                          = COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES;
    _nearFarRatio                            = 0.0005;
    _impostorActive                          = true;
    _depthSortImpostorSprites                = false;
    _impostorPixelErrorThreshold             = 4.0F;
    _numFramesToKeepImpostorSprites          = 10;
    _cullMask                                = 0XFF'FF'FF'FF;
    _cullMaskLeft                            = 0XFF'FF'FF'FF;
    _cullMaskRight                           = 0XFF'FF'FF'FF;

    // override during testing
    //_computeNearFar = COMPUTE_NEAR_FAR_USING_PRIMITIVES;
    //_nearFarRatio = 0.00005f;
}

void
CullSettings::setCullSettings( const CullSettings& rhs )
{
    _inheritanceMask = rhs._inheritanceMask;
    _inheritanceMaskActionOnAttributeSetting =
        rhs._inheritanceMaskActionOnAttributeSetting;

    _computeNearFar                 = rhs._computeNearFar;
    _cullingMode                    = rhs._cullingMode;
    _LODScale                       = rhs._LODScale;
    _smallFeatureCullingPixelSize   = rhs._smallFeatureCullingPixelSize;

    _clampProjectionMatrixCallback  = rhs._clampProjectionMatrixCallback;
    _nearFarRatio                   = rhs._nearFarRatio;
    _impostorActive                 = rhs._impostorActive;
    _depthSortImpostorSprites       = rhs._depthSortImpostorSprites;
    _impostorPixelErrorThreshold    = rhs._impostorPixelErrorThreshold;
    _numFramesToKeepImpostorSprites = rhs._numFramesToKeepImpostorSprites;

    _cullMask                       = rhs._cullMask;
    _cullMaskLeft                   = rhs._cullMaskLeft;
    _cullMaskRight                  = rhs._cullMaskRight;
}

void
CullSettings::inheritCullSettings( const CullSettings& settings,
                                   unsigned int        inheritanceMask )
{
    if( inheritanceMask & COMPUTE_NEAR_FAR_MODE )
    {
        _computeNearFar = settings._computeNearFar;
    }
    if( inheritanceMask & NEAR_FAR_RATIO )
    {
        _nearFarRatio = settings._nearFarRatio;
    }
    if( inheritanceMask & IMPOSTOR_ACTIVE )
    {
        _impostorActive = settings._impostorActive;
    }
    if( inheritanceMask & DEPTH_SORT_IMPOSTOR_SPRITES )
    {
        _depthSortImpostorSprites = settings._depthSortImpostorSprites;
    }
    if( inheritanceMask & IMPOSTOR_PIXEL_ERROR_THRESHOLD )
    {
        _impostorPixelErrorThreshold = settings._impostorPixelErrorThreshold;
    }
    if( inheritanceMask & NUM_FRAMES_TO_KEEP_IMPOSTORS_SPRITES )
    {
        _numFramesToKeepImpostorSprites = settings._numFramesToKeepImpostorSprites;
    }
    if( inheritanceMask & CULL_MASK )
    {
        _cullMask = settings._cullMask;
    }
    if( inheritanceMask & CULL_MASK_LEFT )
    {
        _cullMaskLeft = settings._cullMaskLeft;
    }
    if( inheritanceMask & CULL_MASK_RIGHT )
    {
        _cullMaskRight = settings._cullMaskRight;
    }
    if( inheritanceMask & CULLING_MODE )
    {
        _cullingMode = settings._cullingMode;
    }
    if( inheritanceMask & LOD_SCALE )
    {
        _LODScale = settings._LODScale;
    }
    if( inheritanceMask & SMALL_FEATURE_CULLING_PIXEL_SIZE )
    {
        _smallFeatureCullingPixelSize = settings._smallFeatureCullingPixelSize;
    }
    if( inheritanceMask & CLAMP_PROJECTION_MATRIX_CALLBACK )
    {
        _clampProjectionMatrixCallback = settings._clampProjectionMatrixCallback;
    }
}

static ApplicationUsageProxy ApplicationUsageProxyCullSettings_e0(
    ApplicationUsage::ENVIRONMENTAL_VARIABLE,
    "OSG_COMPUTE_NEAR_FAR_MODE <mode>",
    "DO_NOT_COMPUTE_NEAR_FAR | COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES | "
    "COMPUTE_NEAR_FAR_USING_PRIMITIVES"
);
static ApplicationUsageProxy
    ApplicationUsageProxyCullSettings_e1( ApplicationUsage::ENVIRONMENTAL_VARIABLE,
                                          "OSG_NEAR_FAR_RATIO <float>",
                                          "Set the ratio between near and far planes - "
                                          "must greater than 0.0 but less than 1.0." );

void
CullSettings::readEnvironmentalVariables()
{
    OSG_INFO << "CullSettings::readEnvironmentalVariables()" << std::endl;

    std::string value;
    if( getEnvVar( "OSG_COMPUTE_NEAR_FAR_MODE", value ) )
    {
        if( value == "DO_NOT_COMPUTE_NEAR_FAR" )
        {
            _computeNearFar = DO_NOT_COMPUTE_NEAR_FAR;
        }
        else if( value == "COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES" )
        {
            _computeNearFar = COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES;
        }
        else if( value == "COMPUTE_NEAR_FAR_USING_PRIMITIVES" )
        {
            _computeNearFar = COMPUTE_NEAR_FAR_USING_PRIMITIVES;
        }

        OSG_INFO << "Set compute near far mode to " << _computeNearFar << std::endl;
    }

    if( getEnvVar( "OSG_NEAR_FAR_RATIO", _nearFarRatio ) )
    {
        OSG_INFO << "Set near/far ratio to " << _nearFarRatio << std::endl;
    }
}

void
CullSettings::readCommandLine( ArgumentParser& arguments )
{
    OSG_INFO << "CullSettings::readCommandLine(ArgumentParser& arguments)" << std::endl;

    // report the usage options.
    if( arguments.getApplicationUsage() )
    {
        arguments.getApplicationUsage()->addCommandLineOption(
            "--COMPUTE_NEAR_FAR_MODE <mode>",
            "DO_NOT_COMPUTE_NEAR_FAR | COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES | "
            "COMPUTE_NEAR_FAR_USING_PRIMITIVES"
        );
        arguments.getApplicationUsage()->addCommandLineOption(
            "--NEAR_FAR_RATIO <float>",
            "Set the ratio between near and far planes - must greater than 0.0 but less "
            "than 1.0."
        );
    }

    while( arguments.read( "--NO_CULLING" ) )
    {
        setCullingMode( NO_CULLING );
    }
    while( arguments.read( "--VIEW_FRUSTUM" ) )
    {
        setCullingMode( VIEW_FRUSTUM_CULLING );
    }
    while( arguments.read( "--VIEW_FRUSTUM_SIDES" ) || arguments.read( "--vfs" ) )
    {
        setCullingMode( VIEW_FRUSTUM_SIDES_CULLING );
    }

    std::string str;
    while( arguments.read( "--COMPUTE_NEAR_FAR_MODE", str ) )
    {
        if( str == "DO_NOT_COMPUTE_NEAR_FAR" )
        {
            _computeNearFar = DO_NOT_COMPUTE_NEAR_FAR;
        }
        else if( str == "COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES" )
        {
            _computeNearFar = COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES;
        }
        else if( str == "COMPUTE_NEAR_FAR_USING_PRIMITIVES" )
        {
            _computeNearFar = COMPUTE_NEAR_FAR_USING_PRIMITIVES;
        }

        OSG_INFO << "Set compute near far mode to " << _computeNearFar << std::endl;
    }

    double value;
    while( arguments.read( "--NEAR_FAR_RATIO", value ) )
    {
        _nearFarRatio = value;

        OSG_INFO << "Set near/far ratio to " << _nearFarRatio << std::endl;
    }
}

void
CullSettings::write( std::ostream& out )
{
    out << "CullSettings: " << this << " {" << std::endl;

    out << "    _inheritanceMask = " << _inheritanceMask << std::endl;
    out << "    _inheritanceMaskActionOnAttributeSetting = "
        << _inheritanceMaskActionOnAttributeSetting << std::endl;
    out << "    _computeNearFar = " << _computeNearFar << std::endl;
    out << "    _cullingMode = " << _cullingMode << std::endl;
    out << "    _LODScale = " << _LODScale << std::endl;
    out << "    _smallFeatureCullingPixelSize = " << _smallFeatureCullingPixelSize
        << std::endl;
    out << "    _clampProjectionMatrixCallback = "
        << _clampProjectionMatrixCallback.get() << std::endl;
    out << "    _nearFarRatio = " << _nearFarRatio << std::endl;
    out << "    _impostorActive = " << _impostorActive << std::endl;
    out << "    _depthSortImpostorSprites = " << _depthSortImpostorSprites << std::endl;
    out << "    _impostorPixelErrorThreshold = " << _impostorPixelErrorThreshold
        << std::endl;
    out << "    _numFramesToKeepImpostorSprites = " << _numFramesToKeepImpostorSprites
        << std::endl;
    out << "    _cullMask = " << _cullMask << std::endl;
    out << "    _cullMaskLeft = " << _cullMaskLeft << std::endl;
    out << "    _cullMaskRight = " << _cullMaskRight << std::endl;

    out << "{" << std::endl;
}
