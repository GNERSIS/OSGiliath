/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Texture coordinate generation parameters. Retained for scene
 * graph compatibility — Core Profile generates tex coords in shaders.
 */
#include <osg/textures/TexGen.hpp>

#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>

using namespace osg;

TexGen::TexGen()
{
    _mode = OBJECT_LINEAR;
    _plane_s.set( 1.0F, 0.0F, 0.0F, 0.0F );
    _plane_t.set( 0.0F, 1.0F, 0.0F, 0.0F );
    _plane_r.set( 0.0F, 0.0F, 1.0F, 0.0F );
    _plane_q.set( 0.0F, 0.0F, 0.0F, 1.0F );
}

TexGen::~TexGen()
{
}

void
TexGen::setPlane( Coord        which,
                  const Plane& plane )
{
    switch( which )
    {
        case S :
            _plane_s = plane;
            break;
        case T :
            _plane_t = plane;
            break;
        case R :
            _plane_r = plane;
            break;
        case Q :
            _plane_q = plane;
            break;
        default :
            OSG_WARN << "Error: invalid 'which' passed TexGen::setPlane("
                     << ( unsigned int )which << "," << plane << ")" << std::endl;
            break;
    }
}

const Plane&
TexGen::getPlane( Coord which ) const
{
    switch( which )
    {
        case S :
            return _plane_s;
        case T :
            return _plane_t;
        case R :
            return _plane_r;
        case Q :
            return _plane_q;
        default :
            OSG_WARN << "Error: invalid 'which' passed TexGen::getPlane(which)"
                     << std::endl;
            return _plane_r;
    }
}

Plane&
TexGen::getPlane( Coord which )
{
    switch( which )
    {
        case S :
            return _plane_s;
        case T :
            return _plane_t;
        case R :
            return _plane_r;
        case Q :
            return _plane_q;
        default :
            OSG_WARN << "Error: invalid 'which' passed TexGen::getPlane(which)"
                     << std::endl;
            return _plane_r;
    }
}

void
TexGen::setPlanesFromMatrix( const dmat4& matrix )
{
    _plane_s.set( matrix( 0, 0 ), matrix( 1, 0 ), matrix( 2, 0 ), matrix( 3, 0 ) );
    _plane_t.set( matrix( 0, 1 ), matrix( 1, 1 ), matrix( 2, 1 ), matrix( 3, 1 ) );
    _plane_r.set( matrix( 0, 2 ), matrix( 1, 2 ), matrix( 2, 2 ), matrix( 3, 2 ) );
    _plane_q.set( matrix( 0, 3 ), matrix( 1, 3 ), matrix( 2, 3 ), matrix( 3, 3 ) );
}

void
TexGen::apply( State& ) const
{
    // No-op in Core Profile. The shader pipeline generates texture
    // coordinates based on the TexGen mode via shader defines.
    // The mode is communicated through GL_TEXTURE_GEN_S/T/R/Q
    // modes tracked by applyModeOnTexUnit().
}
