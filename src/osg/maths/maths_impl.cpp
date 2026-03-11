/* Free function implementations for osg::inverse, osg::determinant, osg::decompose
 * declared in osg/maths/transform.hpp
 *
 * This file uses ONLY the new osg/maths/ headers (t_mat4, t_vec3, t_quat)
 * and must NOT include the old Matrixf/Matrixd headers to avoid type conflicts.
 */

#include <cmath>
#include <osg/core/Export.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/quat.hpp>
#include <osg/maths/transform.hpp>

namespace osg
{

    // --- 4x4 matrix inversion using cofactor expansion ---
    template<typename T>
    static t_mat4<T>
    invert_4x4( const t_mat4<T>& m )
    {
        T A2323 = m[2][2] * m[3][3] - m[2][3] * m[3][2];
        T A1323 = m[2][1] * m[3][3] - m[2][3] * m[3][1];
        T A1223 = m[2][1] * m[3][2] - m[2][2] * m[3][1];
        T A0323 = m[2][0] * m[3][3] - m[2][3] * m[3][0];
        T A0223 = m[2][0] * m[3][2] - m[2][2] * m[3][0];
        T A0123 = m[2][0] * m[3][1] - m[2][1] * m[3][0];
        T A2313 = m[1][2] * m[3][3] - m[1][3] * m[3][2];
        T A1313 = m[1][1] * m[3][3] - m[1][3] * m[3][1];
        T A1213 = m[1][1] * m[3][2] - m[1][2] * m[3][1];
        T A2312 = m[1][2] * m[2][3] - m[1][3] * m[2][2];
        T A1312 = m[1][1] * m[2][3] - m[1][3] * m[2][1];
        T A1212 = m[1][1] * m[2][2] - m[1][2] * m[2][1];
        T A0313 = m[1][0] * m[3][3] - m[1][3] * m[3][0];
        T A0213 = m[1][0] * m[3][2] - m[1][2] * m[3][0];
        T A0312 = m[1][0] * m[2][3] - m[1][3] * m[2][0];
        T A0212 = m[1][0] * m[2][2] - m[1][2] * m[2][0];
        T A0113 = m[1][0] * m[3][1] - m[1][1] * m[3][0];
        T A0112 = m[1][0] * m[2][1] - m[1][1] * m[2][0];

        T det   = m[0][0] *
                  ( m[1][1] * A2323 - m[1][2] * A1323 + m[1][3] * A1223 ) -
                  m[0][1] *
                  ( m[1][0] * A2323 - m[1][2] * A0323 + m[1][3] * A0223 ) +
                  m[0][2] *
                  ( m[1][0] * A1323 - m[1][1] * A0323 + m[1][3] * A0123 ) -
                  m[0][3] *
                  ( m[1][0] * A1223 - m[1][1] * A0223 + m[1][2] * A0123 );

        if( std::abs( det ) < std::numeric_limits<T>::epsilon() )
        {
            return t_mat4<T>();    // return identity on singular matrix
        }

        T invDet = static_cast<T>( 1 ) / det;

        return t_mat4<T>(
            invDet * ( m[1][1] * A2323 - m[1][2] * A1323 + m[1][3] * A1223 ),
            invDet * -( m[0][1] * A2323 - m[0][2] * A1323 + m[0][3] * A1223 ),
            invDet * ( m[0][1] * A2313 - m[0][2] * A1313 + m[0][3] * A1213 ),
            invDet * -( m[0][1] * A2312 - m[0][2] * A1312 + m[0][3] * A1212 ),
            invDet * -( m[1][0] * A2323 - m[1][2] * A0323 + m[1][3] * A0223 ),
            invDet * ( m[0][0] * A2323 - m[0][2] * A0323 + m[0][3] * A0223 ),
            invDet * -( m[0][0] * A2313 - m[0][2] * A0313 + m[0][3] * A0213 ),
            invDet * ( m[0][0] * A2312 - m[0][2] * A0312 + m[0][3] * A0212 ),
            invDet * ( m[1][0] * A1323 - m[1][1] * A0323 + m[1][3] * A0123 ),
            invDet * -( m[0][0] * A1323 - m[0][1] * A0323 + m[0][3] * A0123 ),
            invDet * ( m[0][0] * A1313 - m[0][1] * A0313 + m[0][3] * A0113 ),
            invDet * -( m[0][0] * A1312 - m[0][1] * A0312 + m[0][3] * A0112 ),
            invDet * -( m[1][0] * A1223 - m[1][1] * A0223 + m[1][2] * A0123 ),
            invDet * ( m[0][0] * A1223 - m[0][1] * A0223 + m[0][2] * A0123 ),
            invDet * -( m[0][0] * A1213 - m[0][1] * A0213 + m[0][2] * A0113 ),
            invDet * ( m[0][0] * A1212 - m[0][1] * A0212 + m[0][2] * A0112 )
        );
    }

    template<typename T>
    static T
    det_4x4( const t_mat4<T>& m )
    {
        T A2323 = m[2][2] * m[3][3] - m[2][3] * m[3][2];
        T A1323 = m[2][1] * m[3][3] - m[2][3] * m[3][1];
        T A1223 = m[2][1] * m[3][2] - m[2][2] * m[3][1];
        T A0323 = m[2][0] * m[3][3] - m[2][3] * m[3][0];
        T A0223 = m[2][0] * m[3][2] - m[2][2] * m[3][0];
        T A0123 = m[2][0] * m[3][1] - m[2][1] * m[3][0];

        return m[0][0] *
               ( m[1][1] * A2323 - m[1][2] * A1323 + m[1][3] * A1223 ) -
               m[0][1] *
               ( m[1][0] * A2323 - m[1][2] * A0323 + m[1][3] * A0223 ) +
               m[0][2] *
               ( m[1][0] * A1323 - m[1][1] * A0323 + m[1][3] * A0123 ) -
               m[0][3] *
               ( m[1][0] * A1223 - m[1][1] * A0223 + m[1][2] * A0123 );
    }

    // --- Exported free functions ---

    mat4
    inverse( const mat4& m )
    {
        return invert_4x4( m );
    }

    dmat4
    inverse( const dmat4& m )
    {
        return invert_4x4( m );
    }

    float
    determinant( const mat4& m )
    {
        return det_4x4( m );
    }

    double
    determinant( const dmat4& m )
    {
        return det_4x4( m );
    }

    // Simple decompose: extract T, R, S from an affine matrix (no skew)
    template<typename T>
    static bool
    decompose_impl( const t_mat4<T>& m,
                    t_vec3<T>&       translation,
                    t_quat<T>&       rotation,
                    t_vec3<T>&       scale )
    {
        // Translation is the last column
        translation = t_vec3<T>( m[3][0], m[3][1], m[3][2] );

        // Extract scale from column lengths
        t_vec3<T> col0( m[0][0], m[0][1], m[0][2] );
        t_vec3<T> col1( m[1][0], m[1][1], m[1][2] );
        t_vec3<T> col2( m[2][0], m[2][1], m[2][2] );

        T         sx = length( col0 );
        T         sy = length( col1 );
        T         sz = length( col2 );

        // Check for negative determinant (reflection)
        T         det = det_4x4( m );
        if( det < static_cast<T>( 0 ) )
        {
            sx = -sx;
        }

        scale = t_vec3<T>( sx, sy, sz );

        // Build rotation matrix by normalizing columns
        if( sx != static_cast<T>( 0 ) )
        {
            T inv = static_cast<T>( 1 ) / sx;
            col0  = col0 * inv;
        }
        if( sy != static_cast<T>( 0 ) )
        {
            T inv = static_cast<T>( 1 ) / sy;
            col1  = col1 * inv;
        }
        if( sz != static_cast<T>( 0 ) )
        {
            T inv = static_cast<T>( 1 ) / sz;
            col2  = col2 * inv;
        }

        // Extract quaternion directly from rotation matrix (Shepperd's method)
        // Avoids calling getRotate() which would recurse back into decompose()
        T r00 = col0[0], r01 = col1[0], r02 = col2[0];
        T r10 = col0[1], r11 = col1[1], r12 = col2[1];
        T r20 = col0[2], r21 = col1[2], r22 = col2[2];

        T trace = r00 + r11 + r22;
        T qx, qy, qz, qw;

        if( trace > static_cast<T>( 0 ) )
        {
            T s = std::sqrt( trace + static_cast<T>( 1 ) ) * static_cast<T>( 2 );
            qw  = static_cast<T>( 0.25 ) * s;
            qx  = ( r21 - r12 ) / s;
            qy  = ( r02 - r20 ) / s;
            qz  = ( r10 - r01 ) / s;
        }
        else if( r00 > r11 && r00 > r22 )
        {
            T s =
                std::sqrt( static_cast<T>( 1 ) + r00 - r11 - r22 ) * static_cast<T>( 2 );
            qw = ( r21 - r12 ) / s;
            qx = static_cast<T>( 0.25 ) * s;
            qy = ( r01 + r10 ) / s;
            qz = ( r02 + r20 ) / s;
        }
        else if( r11 > r22 )
        {
            T s =
                std::sqrt( static_cast<T>( 1 ) + r11 - r00 - r22 ) * static_cast<T>( 2 );
            qw = ( r02 - r20 ) / s;
            qx = ( r01 + r10 ) / s;
            qy = static_cast<T>( 0.25 ) * s;
            qz = ( r12 + r21 ) / s;
        }
        else
        {
            T s =
                std::sqrt( static_cast<T>( 1 ) + r22 - r00 - r11 ) * static_cast<T>( 2 );
            qw = ( r10 - r01 ) / s;
            qx = ( r02 + r20 ) / s;
            qy = ( r12 + r21 ) / s;
            qz = static_cast<T>( 0.25 ) * s;
        }

        rotation = t_quat<T>( qx, qy, qz, qw );
        return true;
    }

    bool
    decompose( const mat4& m,
               vec3&       t,
               quat&       r,
               vec3&       s )
    {
        return decompose_impl( m, t, r, s );
    }

    bool
    decompose( const dmat4& m,
               dvec3&       t,
               dquat&       r,
               dvec3&       s )
    {
        return decompose_impl( m, t, r, s );
    }

    bool
    decompose( const ldmat4&,
               ldvec3&,
               ldquat&,
               ldvec3& )
    {
        return false;
    }

}    // namespace osg
