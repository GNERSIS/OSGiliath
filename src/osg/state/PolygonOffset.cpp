/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Polygon depth offset to prevent z-fighting. Applies factor
 * and units bias for decals, outlines, and co-planar geometry.
 */
#include <osg/state/PolygonOffset.hpp>

#include <osg/core/Notify.hpp>
#include <osg/GL>
#include <string.h>

using namespace osg;

static float s_FactorMultipler = 1.0F;
static float s_UnitsMultipler  = 1.0F;
static bool  s_MultiplerSet    = false;

void
PolygonOffset::setFactorMultiplier( float multiplier )
{
    s_MultiplerSet    = true;
    s_FactorMultipler = multiplier;
}

float
PolygonOffset::getFactorMultiplier()
{
    return s_FactorMultipler;
}

void
PolygonOffset::setUnitsMultiplier( float multiplier )
{
    s_MultiplerSet   = true;
    s_UnitsMultipler = multiplier;
}

float
PolygonOffset::getUnitsMultiplier()
{
    return s_UnitsMultipler;
}

bool
PolygonOffset::areFactorAndUnitsMultipliersSet()
{
    return s_MultiplerSet;
}

void
PolygonOffset::setFactorAndUnitsMultipliersUsingBestGuessForDriver()
{
    s_MultiplerSet = true;
    // OSG_NOTICE<<"PolygonOffset::setFactorAndUnitMultipliersUsingBestGuessForDriver()"<<std::endl;

#if 0
    const GLubyte* renderer = glGetString(GL_RENDERER);
    if (renderer)
    {
        if ((strstr((const char*)renderer,"Radeon")!=0) ||
            (strstr((const char*)renderer,"RADEON")!=0) ||
            (strstr((const char*)renderer,"ALL-IN-WONDER")!=0))
        {
            setFactorMultiplier(1.0F);
            setUnitsMultiplier(128.0F);
            OSG_INFO<<"PolygonOffset::setFactorAndUnitsMultipliersUsingBestGuessForDriver() apply ATI workaround."<<std::endl;
        }
    }
#endif
}

PolygonOffset::PolygonOffset() :
    _factor( 0.0F ),
    _units( 0.0F )
{
}

PolygonOffset::PolygonOffset( float factor,
                              float units ) :
    _factor( factor ),
    _units( units )
{
}

PolygonOffset::~PolygonOffset()
{
}

void
PolygonOffset::apply( State& ) const
{
    if( !s_MultiplerSet )
    {
        setFactorAndUnitsMultipliersUsingBestGuessForDriver();
    }

    glPolygonOffset( _factor * s_FactorMultipler, _units * s_UnitsMultipler );
}
