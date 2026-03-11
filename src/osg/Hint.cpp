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

#include <osg/Hint>
#include <osg/GLDefines>
#include <osg/StateSet>

#ifndef GL_LINE_SMOOTH_HINT
    #define GL_LINE_SMOOTH_HINT 0x0C52
#endif
#ifndef GL_POLYGON_SMOOTH_HINT
    #define GL_POLYGON_SMOOTH_HINT 0x0C53
#endif
#ifndef GL_TEXTURE_COMPRESSION_HINT
    #define GL_TEXTURE_COMPRESSION_HINT 0x84EF
#endif

using namespace osg;

static bool isValidCoreProfileHintTarget(GLenum target)
{
    switch (target)
    {
        case GL_LINE_SMOOTH_HINT:
        case GL_POLYGON_SMOOTH_HINT:
        case GL_TEXTURE_COMPRESSION_HINT:
        case GL_FRAGMENT_SHADER_DERIVATIVE_HINT:
            return true;
        default:
            return false;
    }
}

void Hint::apply(State& /*state*/) const
{
    if (_target==GL_NONE || _mode==GL_NONE) return;

    // In Core Profile only certain hint targets are valid.
    // Deprecated targets (GL_FOG_HINT, GL_GENERATE_MIPMAP_HINT,
    // GL_PERSPECTIVE_CORRECTION_HINT, GL_POINT_SMOOTH_HINT) are skipped.
    if (!isValidCoreProfileHintTarget(_target)) return;

    glHint(_target, _mode);
}

void Hint::setTarget(GLenum target)
{
    if (_target==target) return;

    ReassignToParents needToReassingToParentsWhenMemberValueChanges(this);

    _target = target;
}

