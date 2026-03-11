/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * 32-bit floating-point matrix alias. Typedef to mat4 with
 * legacy API wrappers for backward compatibility.
 */
#include <osg/maths/mat4.hpp>

// specialise Matrix_implementaiton to be mat4
#define Matrix_implementation mat4

osg::mat4::mat4( const osg::dmat4& mat ) noexcept
{
    set( mat.data() );
}

osg::mat4&
osg::mat4::operator=( const osg::dmat4& rhs ) noexcept
{
    set( rhs.data() );
    return *this;
}

void
osg::mat4::set( const osg::dmat4& rhs ) noexcept
{
    set( rhs.data() );
}

// now compile up dmat4 via Matrix_implementation
#include "Matrix_implementation.cpp"
