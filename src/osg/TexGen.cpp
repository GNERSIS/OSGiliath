/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/
#include <osg/TexGen>
#include <osg/Notify>
#include <osg/io_utils>

using namespace osg;

TexGen::TexGen()
{
    _mode = OBJECT_LINEAR;
    _plane_s.set(1.0f, 0.0f, 0.0f, 0.0f);
    _plane_t.set(0.0f, 1.0f, 0.0f, 0.0f);
    _plane_r.set(0.0f, 0.0f, 1.0f, 0.0f);
    _plane_q.set(0.0f, 0.0f, 0.0f, 1.0f);
}

TexGen::~TexGen()
{
}

void TexGen::setPlane(Coord which, const Plane& plane)
{
    switch( which )
    {
        case S : _plane_s = plane; break;
        case T : _plane_t = plane; break;
        case R : _plane_r = plane; break;
        case Q : _plane_q = plane; break;
        default : OSG_WARN<<"Error: invalid 'which' passed TexGen::setPlane("<<(unsigned int)which<<","<<plane<<")"<<std::endl; break;
    }
}

const Plane& TexGen::getPlane(Coord which) const
{
    switch( which )
    {
        case S : return _plane_s;
        case T : return _plane_t;
        case R : return _plane_r;
        case Q : return _plane_q;
        default : OSG_WARN<<"Error: invalid 'which' passed TexGen::getPlane(which)"<<std::endl; return _plane_r;
    }
}

Plane& TexGen::getPlane(Coord which)
{
    switch( which )
    {
        case S : return _plane_s;
        case T : return _plane_t;
        case R : return _plane_r;
        case Q : return _plane_q;
        default : OSG_WARN<<"Error: invalid 'which' passed TexGen::getPlane(which)"<<std::endl; return _plane_r;
    }
}

void TexGen::setPlanesFromMatrix(const Matrixd& matrix)
{
    _plane_s.set(matrix(0,0),matrix(1,0),matrix(2,0),matrix(3,0));
    _plane_t.set(matrix(0,1),matrix(1,1),matrix(2,1),matrix(3,1));
    _plane_r.set(matrix(0,2),matrix(1,2),matrix(2,2),matrix(3,2));
    _plane_q.set(matrix(0,3),matrix(1,3),matrix(2,3),matrix(3,3));
}

void TexGen::apply(State&) const
{
    // No-op in Core Profile. The shader pipeline generates texture
    // coordinates based on the TexGen mode via shader defines.
    // The mode is communicated through GL_TEXTURE_GEN_S/T/R/Q
    // modes tracked by applyModeOnTexUnit().
}
