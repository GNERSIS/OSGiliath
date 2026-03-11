/* Compatibility helpers for migrating from old OSG matrix API to new VSG-style types.
 * These free functions replace old member functions on Matrixd/Matrixf and Quat. */

#pragma once

#include <osg/maths/mat4.hpp>
#include <osg/maths/quat.hpp>
#include <osg/maths/transform.hpp>
#include <osg/maths/vec3.hpp>

namespace osg
{

    // --- Translation extraction/insertion ---

    /// Extract translation from column-major matrix (column 3, rows 0-2)
    template<typename T>
    inline t_vec3<T>
    getTrans( const t_mat4<T>& m )
    {
        return t_vec3<T>( m[3][0], m[3][1], m[3][2] );
    }

    /// Set translation in column-major matrix (column 3, rows 0-2)
    template<typename T>
    inline void
    setTrans( t_mat4<T>& m,
              T          x,
              T          y,
              T          z )
    {
        m[3][0] = x;
        m[3][1] = y;
        m[3][2] = z;
    }

    template<typename T>
    inline void
    setTrans( t_mat4<T>&       m,
              const t_vec3<T>& v )
    {
        m[3][0] = v.x;
        m[3][1] = v.y;
        m[3][2] = v.z;
    }

    // --- Quaternion helpers ---

    /// Returns true if this quaternion represents the identity rotation (no rotation).
    /// Old API: quat.zeroRotation()
    template<typename T>
    inline bool
    zeroRotation( const t_quat<T>& q )
    {
        return q.x ==
               numbers<T>::zero() &&
               q.y ==
               numbers<T>::zero() &&
               q.z ==
               numbers<T>::zero() &&
               q.w == numbers<T>::one();
    }

    // --- Cross-type vec3*dmat4 support ---
    // Old OSG allowed float vec3 * double mat4 seamlessly.
    // New types require matching template parameter. Provide convenience overloads.

    inline dvec3
    operator*( const vec3&  lhs,
               const dmat4& rhs )
    {
        return dvec3( lhs.x, lhs.y, lhs.z ) * rhs;
    }

    inline dvec3
    operator*( const dmat4& lhs,
               const vec3&  rhs )
    {
        return lhs * dvec3( rhs.x, rhs.y, rhs.z );
    }

    // --- Matrix decomposition helpers ---

    /// Extract rotation from mat4 (quat). Uses decompose.
    /// This replaces old mat.getRotate()
    template<typename T>
    inline t_quat<T>
    getRotate( const t_mat4<T>& m )
    {
        t_vec3<T> translation, scale;
        t_quat<T> rotation;
        decompose( m, translation, rotation, scale );
        return rotation;
    }

    // --- Projection matrix extraction helpers ---
    // These replace old Matrixd member functions
    // getOrtho/getFrustum/getPerspective/getLookAt.

    /// Extract orthographic projection parameters from a projection matrix.
    /// Returns false if the matrix is not an orthographic projection.
    template<typename T>
    inline bool
    getOrtho( const t_mat4<T>& m,
              T&               left,
              T&               right,
              T&               bottom,
              T&               top,
              T&               zNear,
              T&               zFar )
    {
        if( m[0][3] != 0.0 || m[1][3] != 0.0 || m[2][3] != 0.0 || m[3][3] != 1.0 )
        {
            return false;
        }

        zNear  = ( m[3][2] + 1.0 ) / m[2][2];
        zFar   = ( m[3][2] - 1.0 ) / m[2][2];

        left   = -( 1.0 + m[3][0] ) / m[0][0];
        right  = ( 1.0 - m[3][0] ) / m[0][0];

        bottom = -( 1.0 + m[3][1] ) / m[1][1];
        top    = ( 1.0 - m[3][1] ) / m[1][1];

        return true;
    }

    /// Extract frustum parameters from a perspective projection matrix.
    /// Returns false if the matrix is not a perspective projection.
    template<typename T>
    inline bool
    getFrustum( const t_mat4<T>& m,
                T&               left,
                T&               right,
                T&               bottom,
                T&               top,
                T&               zNear,
                T&               zFar )
    {
        if( m[0][3] != 0.0 || m[1][3] != 0.0 || m[2][3] != -1.0 || m[3][3] != 0.0 )
        {
            return false;
        }

        zNear  = m[3][2] / ( m[2][2] - 1.0 );
        zFar   = m[3][2] / ( 1.0 + m[2][2] );

        left   = zNear * ( m[2][0] - 1.0 ) / m[0][0];
        right  = zNear * ( 1.0 + m[2][0] ) / m[0][0];

        top    = zNear * ( 1.0 + m[2][1] ) / m[1][1];
        bottom = zNear * ( m[2][1] - 1.0 ) / m[1][1];

        return true;
    }

    /// Extract symmetric perspective parameters (fovy, aspectRatio, zNear, zFar).
    /// Returns false if the matrix is not a symmetric perspective projection.
    template<typename T>
    inline bool
    getPerspective( const t_mat4<T>& m,
                    T&               fovy,
                    T&               aspectRatio,
                    T&               zNear,
                    T&               zFar )
    {
        T right( 0 );
        T left( 0 );
        T top( 0 );
        T bottom( 0 );

        // If this fails then matrix is not a perspective matrix.
        if( !getFrustum( m, left, right, bottom, top, zNear, zFar ) )
        {
            return false;
        }

        T temp_near( 0 );
        T temp_far( 0 );
        if( left == -right && bottom == -top )
        {
            // Symmetric perspective
            temp_near   = zNear;
            temp_far    = zFar;
            fovy        = osg::degrees( ( T )( 2.0 * std::atan( top / temp_near ) ) );
            aspectRatio = right / top;
        }
        else
        {
            // Asymmetric -- compute using center of frustum
            temp_near   = zNear;
            temp_far    = zFar;
            fovy        = osg::degrees( ( T )( std::atan( top / temp_near ) -
                                               std::atan( bottom / temp_near ) ) );
            aspectRatio = ( right - left ) / ( top - bottom );
        }
        return true;
    }

    /// Extract lookAt parameters from a view matrix.
    template<typename T>
    inline void
    getLookAt( const t_mat4<T>& m,
               t_vec3<T>&       eye,
               t_vec3<T>&       center,
               t_vec3<T>&       up,
               T                lookDistance = static_cast<T>( 1.0 ) )
    {
        t_mat4<T> inv = osg::inverse( m );

        eye           = osg::getTrans( inv );

        up.set( inv[0][1], inv[1][1], inv[2][1] );

        center.set( eye.x - inv[0][2] * lookDistance,
                    eye.y - inv[1][2] * lookDistance,
                    eye.z - inv[2][2] * lookDistance );
    }

    // --- Matrix identity check ---

    template<typename T>
    inline bool
    isIdentity( const t_mat4<T>& m )
    {
        const T zero = numbers<T>::zero();
        const T one  = numbers<T>::one();
        return m[0][0] ==
               one &&
               m[0][1] ==
               zero &&
               m[0][2] ==
               zero &&
               m[0][3] ==
               zero &&
               m[1][0] ==
               zero &&
               m[1][1] ==
               one &&
               m[1][2] ==
               zero &&
               m[1][3] ==
               zero &&
               m[2][0] ==
               zero &&
               m[2][1] ==
               zero &&
               m[2][2] ==
               one &&
               m[2][3] ==
               zero &&
               m[3][0] ==
               zero &&
               m[3][1] ==
               zero &&
               m[3][2] ==
               zero &&
               m[3][3] == one;
    }

    // --- transform3x3: transform a vec3 by the upper 3x3 of a mat4 ---

    /// transform3x3(v, m): v * M3x3 (row-vector convention, v on left)
    template<typename T>
    inline t_vec3<T>
    transform3x3( const t_vec3<T>& v,
                  const t_mat4<T>& m )
    {
        return t_vec3<T>( v[0] * m[0][0] + v[1] * m[1][0] + v[2] * m[2][0],
                          v[0] * m[0][1] + v[1] * m[1][1] + v[2] * m[2][1],
                          v[0] * m[0][2] + v[1] * m[1][2] + v[2] * m[2][2] );
    }

    /// transform3x3(m, v): M3x3 * v (column-vector convention, v on right)
    template<typename T>
    inline t_vec3<T>
    transform3x3( const t_mat4<T>& m,
                  const t_vec3<T>& v )
    {
        return t_vec3<T>( m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2],
                          m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2],
                          m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2] );
    }

    /// Cross-type: transform3x3 with vec3(float) and mat4(double)
    inline vec3
    transform3x3( const vec3&  v,
                  const dmat4& m )
    {
        return vec3(
            static_cast<float>( v[0] * m[0][0] + v[1] * m[1][0] + v[2] * m[2][0] ),
            static_cast<float>( v[0] * m[0][1] + v[1] * m[1][1] + v[2] * m[2][1] ),
            static_cast<float>( v[0] * m[0][2] + v[1] * m[1][2] + v[2] * m[2][2] )
        );
    }

    inline vec3
    transform3x3( const dmat4& m,
                  const vec3&  v )
    {
        return vec3(
            static_cast<float>( m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2] ),
            static_cast<float>( m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2] ),
            static_cast<float>( m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2] )
        );
    }

    // --- Cross-type vec4*dmat4 and dmat4*vec4 support ---

    inline dvec4
    operator*( const vec4&  lhs,
               const dmat4& rhs )
    {
        return dvec4( lhs.x, lhs.y, lhs.z, lhs.w ) * rhs;
    }

    inline dvec4
    operator*( const dmat4& lhs,
               const vec4&  rhs )
    {
        return lhs * dvec4( rhs.x, rhs.y, rhs.z, rhs.w );
    }

    // --- Cross-type mat4 multiply ---

    inline dmat4
    operator*( const mat4&  lhs,
               const dmat4& rhs )
    {
        return dmat4( lhs ) * rhs;
    }

    inline dmat4
    operator*( const dmat4& lhs,
               const mat4&  rhs )
    {
        return lhs * dmat4( rhs );
    }

    // --- Cross-type vec3 arithmetic ---
    // Old OSG allowed seamless mixing of float/double vec3 types.

    inline dvec3
    operator-( const dvec3& lhs,
               const vec3&  rhs )
    {
        return lhs - dvec3( rhs.x, rhs.y, rhs.z );
    }

    inline dvec3
    operator-( const vec3&  lhs,
               const dvec3& rhs )
    {
        return dvec3( lhs.x, lhs.y, lhs.z ) - rhs;
    }

    inline dvec3
    operator+( const dvec3& lhs,
               const vec3&  rhs )
    {
        return lhs + dvec3( rhs.x, rhs.y, rhs.z );
    }

    inline dvec3
    operator+( const vec3&  lhs,
               const dvec3& rhs )
    {
        return dvec3( lhs.x, lhs.y, lhs.z ) + rhs;
    }

    // --- Cross-type dvec4/vec4 arithmetic ---
    inline dvec4
    operator-( const dvec4& lhs,
               const vec4&  rhs )
    {
        return lhs - dvec4( rhs.x, rhs.y, rhs.z, rhs.w );
    }

    // --- Cross-type quat * dvec3 support ---
    // Old OSG used double quat, now it's float. Allow quat * dvec3 for backward compat.

    inline dvec3
    operator*( const quat&  q,
               const dvec3& v )
    {
        return dquat( q.x, q.y, q.z, q.w ) * v;
    }

    // --- Cross-type vec3 compound assignment ---
    inline vec3&
    operator+=( vec3&        lhs,
                const dvec3& rhs )
    {
        lhs.x += static_cast<float>( rhs.x );
        lhs.y += static_cast<float>( rhs.y );
        lhs.z += static_cast<float>( rhs.z );
        return lhs;
    }

    inline vec3&
    operator-=( vec3&        lhs,
                const dvec3& rhs )
    {
        lhs.x -= static_cast<float>( rhs.x );
        lhs.y -= static_cast<float>( rhs.y );
        lhs.z -= static_cast<float>( rhs.z );
        return lhs;
    }

    // --- Matrix postMultTranslate / postMultRotate compat ---
    // Old OSG: mat.postMultTranslate(v) meant "apply translate AFTER mat" (row-vector:
    // v*mat*T) In VSG column-vector convention: result = T * mat, so postMult uses other
    // * m
    template<typename T>
    inline void
    postMultTranslate( t_mat4<T>&       m,
                       const t_vec3<T>& v )
    {
        m = osg::translate( v ) * m;
    }

    template<typename T>
    inline void
    postMultRotate( t_mat4<T>&       m,
                    const t_quat<T>& q )
    {
        m = osg::rotate( q ) * m;
    }

    template<typename T>
    inline void
    postMultScale( t_mat4<T>&       m,
                   const t_vec3<T>& v )
    {
        m = osg::scale( v ) * m;
    }

    // --- preMult / postMult for matrix * matrix ---
    // Old OSG: preMult(mat, other) meant mat = other *_osg mat
    // In VSG column-vector convention: other *_osg mat == mat *_vsg other
    template<typename T>
    inline void
    preMult( t_mat4<T>&       m,
             const t_mat4<T>& other )
    {
        m = m * other;
    }

    // Old OSG: postMult(mat, other) meant mat = mat *_osg other
    // In VSG column-vector convention: mat *_osg other == other *_vsg mat
    template<typename T>
    inline void
    postMult( t_mat4<T>&       m,
              const t_mat4<T>& other )
    {
        m = other * m;
    }

    // --- Color packing helpers ---
    // Replace old vec4 member functions asABGR() and asRGBA()

    inline unsigned int
    asABGR( const vec4& c )
    {
        return ( ( unsigned int )( c.w * 255.0F ) << 24 ) |
               ( ( unsigned int )( c.z * 255.0F ) << 16 ) |
               ( ( unsigned int )( c.y * 255.0F ) << 8 ) |
               ( ( unsigned int )( c.x * 255.0F ) );
    }

    inline unsigned int
    asRGBA( const vec4& c )
    {
        return ( ( unsigned int )( c.x * 255.0F ) << 24 ) |
               ( ( unsigned int )( c.y * 255.0F ) << 16 ) |
               ( ( unsigned int )( c.z * 255.0F ) << 8 ) |
               ( ( unsigned int )( c.w * 255.0F ) );
    }

    // --- preMultTranslate / preMultScale / preMultRotate ---
    // Old OSG: mat.preMultTranslate(v) meant "apply translate BEFORE mat" (row-vector:
    // v*T*mat) In VSG column-vector convention: result = mat * T, so preMult uses m *
    // other
    template<typename T>
    inline void
    preMultTranslate( t_mat4<T>&       m,
                      const t_vec3<T>& v )
    {
        m = m * osg::translate( v );
    }

    template<typename T>
    inline void
    preMultScale( t_mat4<T>&       m,
                  const t_vec3<T>& v )
    {
        m = m * osg::scale( v );
    }

    template<typename T>
    inline void
    preMultRotate( t_mat4<T>&       m,
                   const t_quat<T>& q )
    {
        m = m * osg::rotate( q );
    }

    // --- makeRotate: build rotation from multiple angle/axis pairs ---
    // Replaces old mat = osg::quat(a1, osg::vec3(axis1, a2, axis2, a3, axis3))
    template<typename T>
    inline t_mat4<T>
    rotate( T                angle1,
            const t_vec3<T>& axis1,
            T                angle2,
            const t_vec3<T>& axis2,
            T                angle3,
            const t_vec3<T>& axis3 )
    {
        // Build composite quaternion from three angle-axis rotations
        t_quat<T> q1( angle1, axis1 );
        t_quat<T> q2( angle2, axis2 );
        t_quat<T> q3( angle3, axis3 );
        return osg::rotate( q1 * q2 * q3 );
    }

    // --- rotate(from_vec, to_vec): rotation matrix that rotates one vector to another
    // ---
    template<typename T>
    inline t_mat4<T>
    rotate( const t_vec3<T>& from,
            const t_vec3<T>& to )
    {
        t_quat<T> q( from, to );
        return osg::rotate( q );
    }

    // --- setRotate: set a matrix to a pure rotation from a quaternion ---
    template<typename T>
    inline void
    setRotate( t_mat4<T>&       m,
               const t_quat<T>& q )
    {
        m = osg::rotate( q );
    }

    // --- makeRotate: set a matrix from multiple angle/axis pairs ---
    template<typename T>
    inline void
    makeRotate( t_mat4<T>&       m,
                T                angle1,
                const t_vec3<T>& axis1,
                T                angle2,
                const t_vec3<T>& axis2,
                T                angle3,
                const t_vec3<T>& axis3 )
    {
        m = osg::rotate( angle1, axis1, angle2, axis2, angle3, axis3 );
    }

    // --- ortho: alias for orthographic ---
    template<typename T>
    inline t_mat4<T>
    ortho( T left,
           T right,
           T bottom,
           T top,
           T zNear,
           T zFar )
    {
        return orthographic( left, right, bottom, top, zNear, zFar );
    }

    // --- frustum: alias for perspective(l,r,b,t,n,f) ---
    template<typename T>
    inline t_mat4<T>
    frustum( T left,
             T right,
             T bottom,
             T top,
             T zNear,
             T zFar )
    {
        return perspective( left, right, bottom, top, zNear, zFar );
    }

    // --- ortho2D: 2D orthographic (zNear=-1, zFar=1) ---
    template<typename T>
    inline t_mat4<T>
    ortho2D( T left,
             T right,
             T bottom,
             T top )
    {
        return orthographic( left,
                             right,
                             bottom,
                             top,
                             static_cast<T>( -1 ),
                             static_cast<T>( 1 ) );
    }

    // --- componentMultiply for vec2 ---
    template<typename T>
    inline t_vec2<T>
    componentMultiply( const t_vec2<T>& a,
                       const t_vec2<T>& b )
    {
        return t_vec2<T>( a.x * b.x, a.y * b.y );
    }

    // --- Cross-type dot products (dvec3 * vec3, vec3 * dvec3 used as dot product) ---
    // These are needed where old OSG code uses operator* between vec3 and dvec3
    // as a dot product. The new types require matching template parameters for
    // operator*.

    inline double
    dot( const dvec3& a,
         const vec3&  b )
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    inline double
    dot( const vec3&  a,
         const dvec3& b )
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    // --- Cross-type cross product (dvec3 ^ vec3) ---
    inline dvec3
    cross( const dvec3& a,
           const vec3&  b )
    {
        return cross( a, dvec3( b.x, b.y, b.z ) );
    }

    inline dvec3
    cross( const vec3&  a,
           const dvec3& b )
    {
        return cross( dvec3( a.x, a.y, a.z ), b );
    }

    inline dvec3
    operator^( const dvec3& a,
               const vec3&  b )
    {
        return cross( a, b );
    }

    inline dvec3
    operator^( const vec3&  a,
               const dvec3& b )
    {
        return cross( a, b );
    }

    // --- sphere from box constructor ---
    // Old OSG: BoundingSphere(BoundingBox) — compute sphere enclosing a box
    template<typename T>
    inline t_sphere<T>
    sphereFromBox( const t_box<T>& bb )
    {
        t_sphere<T> s;
        if( bb.valid() )
        {
            s.center = ( bb.min + bb.max ) * static_cast<T>( 0.5 );
            s.radius = length( bb.max - bb.min ) * static_cast<T>( 0.5 );
        }
        return s;
    }

    // --- decompose with 4 args (translation, rotation, scale, so) ---
    // Old OSG: mat.decompose(pos, rot, scale, so)
    inline bool
    decompose( const dmat4& m,
               dvec3&       translation,
               dquat&       rotation,
               dvec3&       scale,
               dquat&       so )
    {
        // Simple decompose ignoring shear orientation
        bool result = decompose( m, translation, rotation, scale );
        so          = dquat();    // identity
        return result;
    }

    inline bool
    decompose( const mat4& m,
               vec3&       translation,
               quat&       rotation,
               vec3&       scale,
               quat&       so )
    {
        bool result = decompose( m, translation, rotation, scale );
        so          = quat();    // identity
        return result;
    }

    // --- valid() free functions for vec types (replaces old member valid()) ---
    // Returns true if no component is NaN
    template<typename T>
    inline bool
    valid( const t_vec2<T>& v )
    {
        return !( std::isnan( v.x ) || std::isnan( v.y ) );
    }

    template<typename T>
    inline bool
    valid( const t_vec3<T>& v )
    {
        return !( std::isnan( v.x ) || std::isnan( v.y ) || std::isnan( v.z ) );
    }

    template<typename T>
    inline bool
    valid( const t_vec4<T>& v )
    {
        return !( std::isnan( v.x ) ||
                  std::isnan( v.y ) ||
                  std::isnan( v.z ) ||
                  std::isnan( v.w ) );
    }

    // --- Cross-type overloads for dmat4 with float vec3/quat ---
    // Old OSG used double matrices with float vectors seamlessly.

    inline void
    preMultTranslate( dmat4&      m,
                      const vec3& v )
    {
        m = m * osg::translate( dvec3( v ) );
    }

    inline void
    preMultScale( dmat4&      m,
                  const vec3& v )
    {
        m = m * osg::scale( dvec3( v ) );
    }

    inline void
    preMultRotate( dmat4&      m,
                   const quat& q )
    {
        m = m * dmat4( osg::rotate( q ) );
    }

    inline void
    postMultTranslate( dmat4&      m,
                       const vec3& v )
    {
        m = osg::translate( dvec3( v ) ) * m;
    }

    inline void
    postMultRotate( dmat4&      m,
                    const quat& q )
    {
        m = dmat4( osg::rotate( q ) ) * m;
    }

    inline void
    postMultScale( dmat4&      m,
                   const vec3& v )
    {
        m = osg::scale( dvec3( v ) ) * m;
    }

    inline void
    preMult( dmat4&      m,
             const mat4& other )
    {
        m = m * dmat4( other );
    }

    inline void
    postMult( dmat4&      m,
              const mat4& other )
    {
        m = dmat4( other ) * m;
    }

    inline void
    preMult( mat4&        m,
             const dmat4& other )
    {
        m = mat4( dmat4( m ) * other );
    }

    inline void
    postMult( mat4&        m,
              const dmat4& other )
    {
        m = mat4( other * dmat4( m ) );
    }

    inline void
    preMultTranslate( mat4&        m,
                      const dvec3& v )
    {
        m = mat4( dmat4( m ) * osg::translate( v ) );
    }

    inline void
    preMultScale( mat4&        m,
                  const dvec3& v )
    {
        m = mat4( dmat4( m ) * osg::scale( v ) );
    }

    inline void
    postMultTranslate( mat4&        m,
                       const dvec3& v )
    {
        m = mat4( osg::translate( v ) * dmat4( m ) );
    }

    inline void
    postMultScale( mat4&        m,
                   const dvec3& v )
    {
        m = mat4( osg::scale( v ) * dmat4( m ) );
    }

    inline void
    preMultRotate( mat4&        m,
                   const dquat& q )
    {
        m = mat4( dmat4( m ) * osg::rotate( q ) );
    }

    inline void
    postMuDltRotate( mat4&        m,
                     const dquat& q )
    {
        m = mat4( osg::rotate( q ) * dmat4( m ) );
    }

    // --- Implicit mat4 -> dmat4 conversion for setMatrix etc. ---
    // Many APIs take dmat4 but code passes mat4. Provide free conversion.
    inline dmat4
    dmat4_from( const mat4& m )
    {
        return dmat4( m );
    }

}    // namespace osg
