/* Adapted from VulkanSceneGraph — MIT License, Copyright(c) 2018 Robert Osfield */

#pragma once

#include <osg/core/Export.hpp>
#include <osg/maths/common.hpp>

namespace osg
{

    /// create a 4x4 matrix that represents the rotation by a quaternion
    template<typename T>
    constexpr t_mat4<T>
    rotate( const t_quat<T>& q )
    {
        T       qxx( q.x * q.x );
        T       qyy( q.y * q.y );
        T       qzz( q.z * q.z );
        T       qxy( q.x * q.y );
        T       qxz( q.x * q.z );
        T       qyz( q.y * q.z );
        T       qwx( q.w * q.x );
        T       qwy( q.w * q.y );
        T       qwz( q.w * q.z );

        const T zero( numbers<T>::zero() );
        const T one( numbers<T>::one() );
        const T two( numbers<T>::two() );

        return t_mat4<T>( one - two * ( qyy + qzz ),
                          two * ( qxy + qwz ),
                          two * ( qxz - qwy ),
                          zero,
                          two * ( qxy - qwz ),
                          one - two * ( qxx + qzz ),
                          two * ( qyz + qwx ),
                          zero,
                          two * ( qxz + qwy ),
                          two * ( qyz - qwx ),
                          one - two * ( qxx + qyy ),
                          zero,
                          zero,
                          zero,
                          zero,
                          one );
    }

    /// create a 4x4 matrix that represents the rotation by a radian angle around an x,
    /// y, z axis
    template<typename T>
    t_mat4<T>
    rotate( T angle_radians,
            T x,
            T y,
            T z )
    {
        const T zero( numbers<T>::zero() );
        const T one( numbers<T>::one() );
        const T c           = std::cos( angle_radians );
        const T s           = std::sin( angle_radians );
        const T one_minus_c = one - c;
        return t_mat4<T>( x * x * one_minus_c + c,
                          y * x * one_minus_c + z * s,
                          x * z * one_minus_c - y * s,
                          zero,
                          x * y * one_minus_c - z * s,
                          y * y * one_minus_c + c,
                          y * z * one_minus_c + x * s,
                          zero,
                          x * z * one_minus_c + y * s,
                          y * z * one_minus_c - x * s,
                          z * z * one_minus_c + c,
                          zero,
                          zero,
                          zero,
                          zero,
                          one );
    }

    /// create a 4x4 matrix that represents the rotation by a radian angle around a vec3
    /// axis
    template<typename T>
    t_mat4<T>
    rotate( T                angle_radians,
            const t_vec3<T>& v )
    {
        return rotate( angle_radians, v.value[0], v.value[1], v.value[2] );
    }

    /// create a 4x4 matrix that represents the translation by x, y, z
    template<typename T>
    constexpr t_mat4<T>
    translate( T x,
               T y,
               T z )
    {
        const T zero( numbers<T>::zero() );
        const T one( numbers<T>::one() );
        return t_mat4<T>( one,
                          zero,
                          zero,
                          zero,
                          zero,
                          one,
                          zero,
                          zero,
                          zero,
                          zero,
                          one,
                          zero,
                          x,
                          y,
                          z,
                          one );
    }

    /// create a 4x4 matrix that represents the translation by vec3
    template<typename T>
    constexpr t_mat4<T>
    translate( const t_vec3<T>& v )
    {
        return translate( v.value[0], v.value[1], v.value[2] );
    }

    /// create a 4x4 matrix that represents the scale by {s, s, s}
    template<typename T>
    constexpr t_mat4<T>
    scale( T s )
    {
        const T zero( numbers<T>::zero() );
        const T one( numbers<T>::one() );
        return t_mat4<T>( s,
                          zero,
                          zero,
                          zero,
                          zero,
                          s,
                          zero,
                          zero,
                          zero,
                          zero,
                          s,
                          zero,
                          zero,
                          zero,
                          zero,
                          one );
    }

    /// create a 4x4 matrix that represents the scale by sx, sy, sz
    template<typename T>
    constexpr t_mat4<T>
    scale( T sx,
           T sy,
           T sz )
    {
        const T zero( numbers<T>::zero() );
        const T one( numbers<T>::one() );
        return t_mat4<T>( sx,
                          zero,
                          zero,
                          zero,
                          zero,
                          sy,
                          zero,
                          zero,
                          zero,
                          zero,
                          sz,
                          zero,
                          zero,
                          zero,
                          zero,
                          one );
    }

    /// create a 4x4 matrix that represents the scale by vec3
    template<typename T>
    constexpr t_mat4<T>
    scale( const t_vec3<T>& v )
    {
        return scale( v.value[0], v.value[1], v.value[2] );
    }

    /// transpose a 3x3 matrix
    template<typename T>
    constexpr t_mat3<T>
    transpose( const t_mat3<T>& m )
    {
        return t_mat3<T>( m[0][0],
                          m[1][0],
                          m[2][0],
                          m[0][1],
                          m[1][1],
                          m[2][1],
                          m[0][2],
                          m[1][2],
                          m[2][2] );
    }

    /// transpose a 4x4 matrix
    template<typename T>
    constexpr t_mat4<T>
    transpose( const t_mat4<T>& m )
    {
        return t_mat4<T>( m[0][0],
                          m[1][0],
                          m[2][0],
                          m[3][0],
                          m[0][1],
                          m[1][1],
                          m[2][1],
                          m[3][1],
                          m[0][2],
                          m[1][2],
                          m[2][2],
                          m[3][2],
                          m[0][3],
                          m[1][3],
                          m[2][3],
                          m[3][3] );
    }

    /// create a 4x4 perspective projection matrix using OpenGL NDC (depth -1 to 1,
    /// non-flipped Y) fovy is in degrees
    template<typename T>
    constexpr t_mat4<T>
    perspective( T fovy,
                 T aspectRatio,
                 T zNear,
                 T zFar )
    {
        T f =
            static_cast<T>( 1.0 ) /
            std::tan( fovy * numbers<T>::degrees_to_radians() * static_cast<T>( 0.5 ) );
        T r = static_cast<T>( 1.0 ) / ( zNear - zFar );
        return t_mat4<T>( f / aspectRatio,
                          0,
                          0,
                          0,
                          0,
                          f,
                          0,
                          0,
                          0,
                          0,
                          ( zFar + zNear ) * r,
                          static_cast<T>( -1.0 ),
                          0,
                          0,
                          static_cast<T>( 2.0 ) * zFar * zNear * r,
                          0 );
    }

    /// create a 4x4 perspective projection matrix from frustum bounds, OpenGL NDC
    template<typename T>
    constexpr t_mat4<T>
    perspective( T left,
                 T right,
                 T bottom,
                 T top,
                 T zNear,
                 T zFar )
    {
        const T zero( numbers<T>::zero() );
        const T one( numbers<T>::one() );
        const T two( numbers<T>::two() );
        return t_mat4<T>( two * zNear / ( right - left ),
                          zero,
                          zero,
                          zero,
                          zero,
                          two * zNear / ( top - bottom ),
                          zero,
                          zero,
                          ( right + left ) / ( right - left ),
                          ( top + bottom ) / ( top - bottom ),
                          -( zFar + zNear ) / ( zFar - zNear ),
                          -one,
                          zero,
                          zero,
                          -two * zFar * zNear / ( zFar - zNear ),
                          zero );
    }

    /// create a 4x4 orthographic projection matrix using OpenGL NDC (depth -1 to 1,
    /// non-flipped Y)
    template<typename T>
    constexpr t_mat4<T>
    orthographic( T left,
                  T right,
                  T bottom,
                  T top,
                  T zNear,
                  T zFar )
    {
        return t_mat4<T>( static_cast<T>( 2.0 ) / ( right - left ),
                          0,
                          0,
                          0,
                          0,
                          static_cast<T>( 2.0 ) / ( top - bottom ),
                          0,
                          0,
                          0,
                          0,
                          static_cast<T>( -2.0 ) / ( zFar - zNear ),
                          0,
                          -( right + left ) / ( right - left ),
                          -( top + bottom ) / ( top - bottom ),
                          -( zFar + zNear ) / ( zFar - zNear ),
                          1 );
    }

    /// create a 4x4 lookAt view matrix (standard OpenGL right-handed)
    template<typename T>
    constexpr t_mat4<T>
    lookAt( const t_vec3<T>& eye,
            const t_vec3<T>& center,
            const t_vec3<T>& up )
    {
        using vec_type = t_vec3<T>;

        const T  zero( numbers<T>::zero() );
        const T  one( numbers<T>::one() );

        vec_type forward   = normalize( center - eye );
        vec_type up_normal = normalize( up );
        vec_type side      = normalize( cross( forward, up_normal ) );
        vec_type u         = normalize( cross( side, forward ) );

        // Basis vectors as rows: row_i · v gives eye coordinate i
        return t_mat4<T>( side[0],
                          u[0],
                          -forward[0],
                          zero,
                          side[1],
                          u[1],
                          -forward[1],
                          zero,
                          side[2],
                          u[2],
                          -forward[2],
                          zero,
                          zero,
                          zero,
                          zero,
                          one ) *
               osg::translate( -eye.x, -eye.y, -eye.z );
    }

    template<typename T>
    constexpr t_mat4<T>
    computeBillboardMatrix( const t_vec3<T>& centerEye,
                            T                autoscaleDistance )
    {
        const T zero( numbers<T>::zero() );
        const T one( numbers<T>::one() );

        auto    distance = -centerEye.z;
        auto    scale =
            ( distance < autoscaleDistance ) ? distance / autoscaleDistance : one;
        t_mat4<T> mS( scale,
                      zero,
                      zero,
                      zero,
                      zero,
                      scale,
                      zero,
                      zero,
                      zero,
                      zero,
                      scale,
                      zero,
                      zero,
                      zero,
                      zero,
                      one );

        t_mat4<T> mT( one,
                      zero,
                      zero,
                      zero,
                      zero,
                      one,
                      zero,
                      zero,
                      zero,
                      zero,
                      one,
                      zero,
                      centerEye.x,
                      centerEye.y,
                      centerEye.z,
                      one );

        return mT * mS;
    }

    /// Hint on axis, using Collada conventions, all Right Hand
    enum class CoordinateConvention
    {
        NO_PREFERENCE,
        X_UP,    // x up, y left/west, z out/south
        Y_UP,    // x right/east, y up, z out/south
        Z_UP,    // x right/east, y forward/north, z up
    };

    /// compute the transformation matrix required to transform from one coordinate frame
    /// convention to another. return true if required and matrix modified, return false
    /// if no transformation is required.
    extern OSG_EXPORT bool
    transform( CoordinateConvention source,
               CoordinateConvention destination,
               dmat4&               matrix );

    /// invert the top left 3x3 portion of a float 4x4 matrix.
    extern OSG_EXPORT mat3
    inverse_3x3( const mat4& m );

    /// invert the top left 3x3 portion of a double 4x4 matrix.
    extern OSG_EXPORT dmat3
    inverse_3x3( const dmat4& m );

    /// fast float matrix inversion that assumes the matrix is composed of only scales,
    /// rotations and translations forming a 4x3 matrix.
    extern OSG_EXPORT mat4
    inverse_4x3( const mat4& m );

    /// fast double matrix inversion that assumes the matrix is composed of only scales,
    /// rotations and translations forming a 4x3 matrix.
    extern OSG_EXPORT dmat4
    inverse_4x3( const dmat4& m );

    /// general purpose 4x4 float matrix inversion.
    extern OSG_EXPORT mat4
    inverse_4x4( const mat4& m );

    /// general purpose 4x4 double matrix inversion.
    extern OSG_EXPORT dmat4
    inverse_4x4( const dmat4& m );

    /// matrix float inversion with automatic selection of inverse_4x3 when appropriate,
    /// otherwise uses inverse_4x4
    extern OSG_EXPORT mat4
    inverse( const mat4& m );

    /// double matrix inversion with automatic selection of inverse_4x3 when appropriate,
    /// otherwise uses inverse_4x4
    extern OSG_EXPORT dmat4
    inverse( const dmat4& m );

    /// compute determinant of float matrix
    extern OSG_EXPORT float
    determinant( const mat4& m );

    /// compute determinant of double matrix
    extern OSG_EXPORT double
    determinant( const dmat4& m );

    /// decompose float matrix into translation, rotation and scale components.
    /// maps to TRS form: osg::translate(translation) * osg::rotate(rotation) *
    /// osg::scale(scale); assumes matrix has no skew or perspective components
    extern OSG_EXPORT bool
    decompose( const mat4& m,
               vec3&       translation,
               quat&       rotation,
               vec3&       scale );

    /// decompose double matrix into translation, rotation and scale components.
    /// maps to TRS form: osg::translate(translation) * osg::rotate(rotation) *
    /// osg::scale(scale); assumes matrix has no skew or perspective components
    extern OSG_EXPORT bool
    decompose( const dmat4& m,
               dvec3&       translation,
               dquat&       rotation,
               dvec3&       scale );

    /// decompose long double matrix into translation, rotation and scale components.
    /// maps to TRS form: osg::translate(translation) * osg::rotate(rotation) *
    /// osg::scale(scale); assumes matrix has no skew or perspective components
    extern OSG_EXPORT bool
    decompose( const ldmat4& m,
               ldvec3&       translation,
               ldquat&       rotation,
               ldvec3&       scale );

    /// compute the bounding sphere that encloses a frustum defined by specified float
    /// ModelViewMatrixProjection
    extern OSG_EXPORT sphere
    computeFrustumBound( const mat4& m );

    /// compute the bounding sphere that encloses a frustum defined by specified double
    /// ModelViewMatrixProjection
    extern OSG_EXPORT dsphere
    computeFrustumBound( const dmat4& m );

}    // namespace osg
