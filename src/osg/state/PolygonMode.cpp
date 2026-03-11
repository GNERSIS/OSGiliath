/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Polygon rasterization mode (fill, line, point).
 * Used for wireframe rendering and debug visualization.
 */
#include <osg/state/PolygonMode.hpp>

#include <osg/core/Notify.hpp>
#include <osg/GL>

using namespace osg;

PolygonMode::PolygonMode() :
    _modeFront( Mode::FILL ),
    _modeBack( Mode::FILL )
{
}

PolygonMode::PolygonMode( Face face,
                          Mode mode ) :
    _modeFront( Mode::FILL ),
    _modeBack( Mode::FILL )
{
    setMode( face, mode );
}

PolygonMode::~PolygonMode()
{
}

void
PolygonMode::setMode( Face face,
                      Mode mode )
{
    switch( face )
    {
        case( Face::FRONT ) :
            _modeFront = mode;
            break;
        case( Face::BACK ) :
            _modeBack = mode;
            break;
        case( Face::FRONT_AND_BACK ) :
            _modeFront = mode;
            _modeBack  = mode;
            break;
    }
}

PolygonMode::Mode
PolygonMode::getMode( Face face ) const
{
    switch( face )
    {
        case( Face::FRONT ) :
            return _modeFront;
        case( Face::BACK ) :
            return _modeBack;
        case( Face::FRONT_AND_BACK ) :
            return _modeFront;
    }
    OSG_WARN << "Warning : invalid Face passed to PolygonMode::getMode(Face face)"
             << std::endl;
    return _modeFront;
}

void
PolygonMode::apply( State& ) const
{
    if( _modeFront == _modeBack )
    {
        glPolygonMode( GL_FRONT_AND_BACK, ( GLenum )_modeFront );
    }
    else
    {
        OSG_NOTICE << "Warning: PolygonMode::apply(State&) - only GL_FRONT_AND_BACK is "
                      "supported."
                   << std::endl;
    }
}
