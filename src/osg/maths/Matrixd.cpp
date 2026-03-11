/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * 64-bit double-precision matrix alias. Typedef to dmat4 with
 * legacy API wrappers for backward compatibility.
 */
#include <osg/maths/mat4.hpp>

// specialise Matrix_implementaiton to be dmat4
#define Matrix_implementation dmat4

osg::dmat4::dmat4( const osg::mat4& mat ) noexcept
{
    set( mat.data() );
}

osg::dmat4&
osg::dmat4::operator=( const osg::mat4& rhs ) noexcept
{
    set( rhs.data() );
    return *this;
}

void
osg::dmat4::set( const osg::mat4& rhs ) noexcept
{
    set( rhs.data() );
}

// now compile up dmat4 via Matrix_implementation
#include "Matrix_implementation.cpp"
