/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Point size state attribute and per-vertex point size enable.
 * Configures gl_PointSize for GL_POINTS rendering.
 */
#include <osg/state/Point.hpp>

#include <osg/core/Notify.hpp>
#include <osg/GL>

using namespace osg;

Point::Point()
{
    _size                = 1.0F;                   // TODO find proper default
    _fadeThresholdSize   = 1.0F;                   // TODO find proper default
    _distanceAttenuation = vec3( 1, 0.0, 0.0 );    // TODO find proper default

    _minSize             = 0.0;
    _maxSize = 100.0;    // depends on mulitsampling ... some default necessary
}

Point::Point( float size )
{
    _size                = size;
    _fadeThresholdSize   = 1.0F;                   // TODO find proper default
    _distanceAttenuation = vec3( 1, 0.0, 0.0 );    // TODO find proper default

    _minSize             = 0.0;
    _maxSize = 100.0;    // depends on mulitsampling ... some default necessary
}

Point::~Point()
{
}

void
Point::setSize( float size )
{
    _size = size;
}

void
Point::setFadeThresholdSize( float fadeThresholdSize )
{
    _fadeThresholdSize = fadeThresholdSize;
}

void
Point::setDistanceAttenuation( const vec3& distanceAttenuation )
{
    _distanceAttenuation = distanceAttenuation;
}

void
Point::setMinSize( float minSize )
{
    _minSize = minSize;
}

void
Point::setMaxSize( float maxSize )
{
    _maxSize = maxSize;
}

void
Point::apply( State& ) const
{
    // In Core Profile, glPointSize() sets the point size when
    // GL_PROGRAM_POINT_SIZE is disabled (the default).
    // Do NOT enable GL_PROGRAM_POINT_SIZE here - that would require the
    // vertex shader to write gl_PointSize and would cause glPointSize()
    // to be ignored. Shaders that want programmatic control should
    // enable GL_PROGRAM_POINT_SIZE and set gl_PointSize themselves.
    glPointSize( _size );
}
