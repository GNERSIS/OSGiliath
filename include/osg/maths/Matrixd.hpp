/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * 64-bit double-precision matrix alias. Typedef to dmat4 with
 * legacy API wrappers for backward compatibility.
 */
#pragma once

#include <osg/core/Object.hpp>
#include <osg/maths/quat.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>

namespace osg
{

    class mat4;

    class OSG_EXPORT dmat4
    {
        public:

            using value_type       = double;
            using other_value_type = float;

            inline dmat4() noexcept
            {
                makeIdentity();
            }

            inline dmat4( const dmat4& mat ) noexcept
            {
                set( mat.data() );
            }

            dmat4( const mat4& mat ) noexcept;

            inline explicit dmat4( const float* const ptr ) noexcept
            {
                set( ptr );
            }

            inline explicit dmat4( const double* const ptr ) noexcept
            {
                set( ptr );
            }

            inline explicit dmat4( const quat& quat ) noexcept
            {
                makeRotate( quat );
            }

            dmat4( value_type a00,
                   value_type a01,
                   value_type a02,
                   value_type a03,
                   value_type a10,
                   value_type a11,
                   value_type a12,
                   value_type a13,
                   value_type a20,
                   value_type a21,
                   value_type a22,
                   value_type a23,
                   value_type a30,
                   value_type a31,
                   value_type a32,
                   value_type a33 ) noexcept;

            ~dmat4() noexcept
            {
            }

            int
            compare( const dmat4& m ) const noexcept;

            bool
            operator<( const dmat4& m ) const noexcept
            {
                return compare( m ) < 0;
            }

            bool
            operator==( const dmat4& m ) const noexcept
            {
                return compare( m ) == 0;
            }

            bool
            operator!=( const dmat4& m ) const noexcept
            {
                return compare( m ) != 0;
            }

            constexpr value_type&
            operator()( int row,
                        int col ) noexcept
            {
                return _mat[row][col];
            }

            constexpr value_type
            operator()( int row,
                        int col ) const noexcept
            {
                return _mat[row][col];
            }

            inline bool
            valid() const noexcept
            {
                return !isNaN();
            }

            inline bool
            isNaN() const noexcept
            {
                return osg::isNaN( _mat[0][0] ) ||
                       osg::isNaN( _mat[0][1] ) ||
                       osg::isNaN( _mat[0][2] ) ||
                       osg::isNaN( _mat[0][3] ) ||
                       osg::isNaN( _mat[1][0] ) ||
                       osg::isNaN( _mat[1][1] ) ||
                       osg::isNaN( _mat[1][2] ) ||
                       osg::isNaN( _mat[1][3] ) ||
                       osg::isNaN( _mat[2][0] ) ||
                       osg::isNaN( _mat[2][1] ) ||
                       osg::isNaN( _mat[2][2] ) ||
                       osg::isNaN( _mat[2][3] ) ||
                       osg::isNaN( _mat[3][0] ) ||
                       osg::isNaN( _mat[3][1] ) ||
                       osg::isNaN( _mat[3][2] ) ||
                       osg::isNaN( _mat[3][3] );
            }

            inline dmat4&
            operator=( const dmat4& rhs ) noexcept
            {
                if( &rhs == this )
                {
                    return *this;
                }
                set( rhs.data() );
                return *this;
            }

            dmat4&
            operator=( const mat4& other ) noexcept;

            inline void
            set( const dmat4& rhs ) noexcept
            {
                set( rhs.data() );
            }

            void
            set( const mat4& rhs ) noexcept;

            inline void
            set( const float* const ptr ) noexcept
            {
                value_type* local_ptr = ( value_type* )_mat;
                for( int i = 0; i < 16; ++i )
                {
                    local_ptr[i] = ( value_type )ptr[i];
                }
            }

            inline void
            set( const double* const ptr ) noexcept
            {
                value_type* local_ptr = ( value_type* )_mat;
                for( int i = 0; i < 16; ++i )
                {
                    local_ptr[i] = ( value_type )ptr[i];
                }
            }

            void
            set( value_type a00,
                 value_type a01,
                 value_type a02,
                 value_type a03,
                 value_type a10,
                 value_type a11,
                 value_type a12,
                 value_type a13,
                 value_type a20,
                 value_type a21,
                 value_type a22,
                 value_type a23,
                 value_type a30,
                 value_type a31,
                 value_type a32,
                 value_type a33 ) noexcept;

            value_type*
            ptr() noexcept
            {
                return ( value_type* )_mat;
            }

            const value_type*
            ptr() const noexcept
            {
                return ( const value_type* )_mat;
            }

            constexpr bool
            isIdentity() const noexcept
            {
                return _mat[0][0] ==
                       1.0 &&
                       _mat[0][1] ==
                       0.0 &&
                       _mat[0][2] ==
                       0.0 &&
                       _mat[0][3] ==
                       0.0 &&
                       _mat[1][0] ==
                       0.0 &&
                       _mat[1][1] ==
                       1.0 &&
                       _mat[1][2] ==
                       0.0 &&
                       _mat[1][3] ==
                       0.0 &&
                       _mat[2][0] ==
                       0.0 &&
                       _mat[2][1] ==
                       0.0 &&
                       _mat[2][2] ==
                       1.0 &&
                       _mat[2][3] ==
                       0.0 &&
                       _mat[3][0] ==
                       0.0 &&
                       _mat[3][1] ==
                       0.0 &&
                       _mat[3][2] ==
                       0.0 &&
                       _mat[3][3] == 1.0;
            }

            void
            makeIdentity() noexcept;

            void
            makeScale( const vec3& ) noexcept;
            void
                 makeScale( const dvec3& ) noexcept;
            void makeScale( value_type,
                            value_type,
                            value_type ) noexcept;

            void
            makeTranslate( const vec3& ) noexcept;
            void
                 makeTranslate( const dvec3& ) noexcept;
            void makeTranslate( value_type,
                                value_type,
                                value_type ) noexcept;

            void
            makeRotate( const vec3& from,
                        const vec3& to ) noexcept;
            void
            makeRotate( const dvec3& from,
                        const dvec3& to ) noexcept;
            void
            makeRotate( value_type  angle,
                        const vec3& axis ) noexcept;
            void
            makeRotate( value_type   angle,
                        const dvec3& axis ) noexcept;
            void
            makeRotate( value_type angle,
                        value_type x,
                        value_type y,
                        value_type z ) noexcept;
            void
            makeRotate( const quat& ) noexcept;
            void
            makeRotate( value_type  angle1,
                        const vec3& axis1,
                        value_type  angle2,
                        const vec3& axis2,
                        value_type  angle3,
                        const vec3& axis3 ) noexcept;
            void
            makeRotate( value_type   angle1,
                        const dvec3& axis1,
                        value_type   angle2,
                        const dvec3& axis2,
                        value_type   angle3,
                        const dvec3& axis3 ) noexcept;

            /** decompose the matrix into translation, rotation, scale and scale
             * orientation.*/
            void
            decompose( osg::vec3& translation,
                       osg::quat& rotation,
                       osg::vec3& scale,
                       osg::quat& so ) const noexcept;

            /** decompose the matrix into translation, rotation, scale and scale
             * orientation.*/
            void
            decompose( osg::dvec3& translation,
                       osg::quat&  rotation,
                       osg::dvec3& scale,
                       osg::quat&  so ) const noexcept;

            /** Set to an orthographic projection.
             * See glOrtho for further details.
             */
            void
            makeOrtho( double left,
                       double right,
                       double bottom,
                       double top,
                       double zNear,
                       double zFar ) noexcept;

            /** Get the orthographic settings of the orthographic projection matrix.
             * Note, if matrix is not an orthographic matrix then invalid values
             * will be returned.
             */
            bool
            getOrtho( double& left,
                      double& right,
                      double& bottom,
                      double& top,
                      double& zNear,
                      double& zFar ) const noexcept;

            /** float version of getOrtho(..) */
            bool
            getOrtho( float& left,
                      float& right,
                      float& bottom,
                      float& top,
                      float& zNear,
                      float& zFar ) const noexcept;

            /** Set to a 2D orthographic projection.
             * See glOrtho2D for further details.
             */
            inline void
            makeOrtho2D( double left,
                         double right,
                         double bottom,
                         double top ) noexcept
            {
                makeOrtho( left, right, bottom, top, -1.0, 1.0 );
            }

            /** Set to a perspective projection.
             * See glFrustum for further details.
             */
            void
            makeFrustum( double left,
                         double right,
                         double bottom,
                         double top,
                         double zNear,
                         double zFar ) noexcept;

            /** Get the frustum settings of a perspective projection matrix.
             * Note, if matrix is not a perspective matrix then invalid values
             * will be returned.
             */
            bool
            getFrustum( double& left,
                        double& right,
                        double& bottom,
                        double& top,
                        double& zNear,
                        double& zFar ) const noexcept;

            /** float version of getFrustum(..) */
            bool
            getFrustum( float& left,
                        float& right,
                        float& bottom,
                        float& top,
                        float& zNear,
                        float& zFar ) const noexcept;

            /** Set to a symmetrical perspective projection.
             * See gluPerspective for further details.
             * Aspect ratio is defined as width/height.
             */
            void
            makePerspective( double fovy,
                             double aspectRatio,
                             double zNear,
                             double zFar ) noexcept;

            /** Get the frustum settings of a symmetric perspective projection
             * matrix.
             * Return false if matrix is not a perspective matrix,
             * where parameter values are undefined.
             * Note, if matrix is not a symmetric perspective matrix then the
             * shear will be lost.
             * Asymmetric matrices occur when stereo, power walls, caves and
             * reality center display are used.
             * In these configuration one should use the AsFrustum method instead.
             */
            bool
            getPerspective( double& fovy,
                            double& aspectRatio,
                            double& zNear,
                            double& zFar ) const noexcept;

            /** float version of getPerspective(..) */
            bool
            getPerspective( float& fovy,
                            float& aspectRatio,
                            float& zNear,
                            float& zFar ) const noexcept;

            /** Set the position and orientation to be a view matrix,
             * using the same convention as gluLookAt.
             */
            void
            makeLookAt( const dvec3& eye,
                        const dvec3& center,
                        const dvec3& up ) noexcept;

            /** Get to the position and orientation of a modelview matrix,
             * using the same convention as gluLookAt.
             */
            void
            getLookAt( vec3&      eye,
                       vec3&      center,
                       vec3&      up,
                       value_type lookDistance = 1.0F ) const noexcept;

            /** Get to the position and orientation of a modelview matrix,
             * using the same convention as gluLookAt.
             */
            void
            getLookAt( dvec3&     eye,
                       dvec3&     center,
                       dvec3&     up,
                       value_type lookDistance = 1.0F ) const noexcept;

            /** invert the matrix rhs, automatically select invert_4x3 or invert_4x4. */
            inline bool
            invert( const dmat4& rhs ) noexcept
            {
                bool is_4x3 = ( rhs._mat[0][3] ==
                                0.0 &&
                                rhs._mat[1][3] ==
                                0.0 &&
                                rhs._mat[2][3] ==
                                0.0 &&
                                rhs._mat[3][3] == 1.0 );
                return is_4x3 ? invert_4x3( rhs ) : invert_4x4( rhs );
            }

            /** 4x3 matrix invert, not right hand column is assumed to be 0,0,0,1. */
            bool
            invert_4x3( const dmat4& rhs ) noexcept;

            /** full 4x4 matrix invert. */
            bool
            invert_4x4( const dmat4& rhs ) noexcept;

            /** transpose a matrix */
            bool
            transpose( const dmat4& rhs ) noexcept;

            /** transpose orthogonal part of the matrix **/
            bool
            transpose3x3( const dmat4& rhs ) noexcept;

            /** ortho-normalize the 3x3 rotation & scale matrix */
            void
            orthoNormalize( const dmat4& rhs ) noexcept;

            // basic utility functions to create new matrices
            inline static dmat4
            identity( void ) noexcept;
            inline static dmat4
            scale( const vec3& sv ) noexcept;
            inline static dmat4
            scale( const dvec3& sv ) noexcept;
            inline static dmat4
            scale( value_type sx,
                   value_type sy,
                   value_type sz ) noexcept;
            inline static dmat4
            translate( const vec3& dv ) noexcept;
            inline static dmat4
            translate( const dvec3& dv ) noexcept;
            inline static dmat4
            translate( value_type x,
                       value_type y,
                       value_type z ) noexcept;
            inline static dmat4
            rotate( const vec3& from,
                    const vec3& to ) noexcept;
            inline static dmat4
            rotate( const dvec3& from,
                    const dvec3& to ) noexcept;
            inline static dmat4
            rotate( value_type angle,
                    value_type x,
                    value_type y,
                    value_type z ) noexcept;
            inline static dmat4
            rotate( value_type  angle,
                    const vec3& axis ) noexcept;
            inline static dmat4
            rotate( value_type   angle,
                    const dvec3& axis ) noexcept;
            inline static dmat4
            rotate( value_type  angle1,
                    const vec3& axis1,
                    value_type  angle2,
                    const vec3& axis2,
                    value_type  angle3,
                    const vec3& axis3 ) noexcept;
            inline static dmat4
            rotate( value_type   angle1,
                    const dvec3& axis1,
                    value_type   angle2,
                    const dvec3& axis2,
                    value_type   angle3,
                    const dvec3& axis3 ) noexcept;
            inline static dmat4
            rotate( const quat& quat ) noexcept;
            inline static dmat4
            inverse( const dmat4& matrix ) noexcept;
            inline static dmat4
            orthoNormal( const dmat4& matrix ) noexcept;
            /** Create an orthographic projection matrix.
             * See glOrtho for further details.
             */
            inline static dmat4
            ortho( double left,
                   double right,
                   double bottom,
                   double top,
                   double zNear,
                   double zFar ) noexcept;

            /** Create a 2D orthographic projection.
             * See glOrtho for further details.
             */
            inline static dmat4
            ortho2D( double left,
                     double right,
                     double bottom,
                     double top ) noexcept;

            /** Create a perspective projection.
             * See glFrustum for further details.
             */
            inline static dmat4
            frustum( double left,
                     double right,
                     double bottom,
                     double top,
                     double zNear,
                     double zFar ) noexcept;

            /** Create a symmetrical perspective projection.
             * See gluPerspective for further details.
             * Aspect ratio is defined as width/height.
             */
            inline static dmat4
            perspective( double fovy,
                         double aspectRatio,
                         double zNear,
                         double zFar ) noexcept;

            /** Create the position and orientation as per a camera,
             * using the same convention as gluLookAt.
             */
            inline static dmat4
            lookAt( const vec3& eye,
                    const vec3& center,
                    const vec3& up ) noexcept;

            /** Create the position and orientation as per a camera,
             * using the same convention as gluLookAt.
             */
            inline static dmat4
            lookAt( const dvec3& eye,
                    const dvec3& center,
                    const dvec3& up ) noexcept;

            inline vec3
            preMult( const vec3& v ) const noexcept;
            inline dvec3
            preMult( const dvec3& v ) const noexcept;
            inline vec3
            postMult( const vec3& v ) const noexcept;
            inline dvec3
            postMult( const dvec3& v ) const noexcept;
            inline vec3
            operator*( const vec3& v ) const noexcept;
            inline dvec3
            operator*( const dvec3& v ) const noexcept;
            inline vec4
            preMult( const vec4& v ) const noexcept;
            inline dvec4
            preMult( const dvec4& v ) const noexcept;
            inline vec4
            postMult( const vec4& v ) const noexcept;
            inline dvec4
            postMult( const dvec4& v ) const noexcept;
            inline vec4
            operator*( const vec4& v ) const noexcept;
            inline dvec4
            operator*( const dvec4& v ) const noexcept;

#ifdef OSG_USE_DEPRECATED_API
            inline void
            set( const quat& q )
            {
                makeRotate( q );
            }    /// deprecated, replace with makeRotate(q)

            inline void
            get( quat& q ) const
            {
                q = getRotate();
            }    /// deprecated, replace with getRotate()
#endif

            void
            setRotate( const quat& q ) noexcept;
            /** Get the matrix rotation as a quat. Note that this function
             * assumes a non-scaled matrix and will return incorrect results
             * for scaled matrixces. Consider decompose() instead.
             */
            quat
            getRotate() const noexcept;

            void
            setTrans( value_type tx,
                      value_type ty,
                      value_type tz ) noexcept;
            void
            setTrans( const vec3& v ) noexcept;
            void
            setTrans( const dvec3& v ) noexcept;

            inline dvec3
            getTrans() const noexcept
            {
                return dvec3( _mat[3][0], _mat[3][1], _mat[3][2] );
            }

            inline dvec3
            getScale() const noexcept
            {
                dvec3 x_vec( _mat[0][0], _mat[1][0], _mat[2][0] );
                dvec3 y_vec( _mat[0][1], _mat[1][1], _mat[2][1] );
                dvec3 z_vec( _mat[0][2], _mat[1][2], _mat[2][2] );
                return dvec3( osg::length( x_vec ),
                              osg::length( y_vec ),
                              osg::length( z_vec ) );
            }

            /** apply a 3x3 transform of v*M[0..2,0..2]. */
            inline static vec3
            transform3x3( const vec3&  v,
                          const dmat4& m ) noexcept;

            /** apply a 3x3 transform of v*M[0..2,0..2]. */
            inline static dvec3
            transform3x3( const dvec3& v,
                          const dmat4& m ) noexcept;

            /** apply a 3x3 transform of M[0..2,0..2]*v. */
            inline static vec3
            transform3x3( const dmat4& m,
                          const vec3&  v ) noexcept;

            /** apply a 3x3 transform of M[0..2,0..2]*v. */
            inline static dvec3
            transform3x3( const dmat4& m,
                          const dvec3& v ) noexcept;

            // basic dmat4 multiplication, our workhorse methods.
            void
            mult( const dmat4&,
                  const dmat4& ) noexcept;
            void
            preMult( const dmat4& ) noexcept;
            void
            postMult( const dmat4& ) noexcept;

            /** Optimized version of preMult(translate(v)); */
            inline void
            preMultTranslate( const dvec3& v ) noexcept;
            inline void
            preMultTranslate( const vec3& v ) noexcept;
            /** Optimized version of postMult(translate(v)); */
            inline void
            postMultTranslate( const dvec3& v ) noexcept;
            inline void
            postMultTranslate( const vec3& v ) noexcept;

            /** Optimized version of preMult(scale(v)); */
            inline void
            preMultScale( const dvec3& v ) noexcept;
            inline void
            preMultScale( const vec3& v ) noexcept;
            /** Optimized version of postMult(scale(v)); */
            inline void
            postMultScale( const dvec3& v ) noexcept;
            inline void
            postMultScale( const vec3& v ) noexcept;

            /** Optimized version of preMult(rotate(q)); */
            inline void
            preMultRotate( const quat& q ) noexcept;
            /** Optimized version of postMult(rotate(q)); */
            inline void
            postMultRotate( const quat& q ) noexcept;

            inline void
            operator*=( const dmat4& other ) noexcept
            {
                if( this == &other )
                {
                    dmat4 temp( other );
                    postMult( temp );
                }
                else
                {
                    postMult( other );
                }
            }

            inline dmat4
            operator*( const dmat4& m ) const noexcept
            {
                osg::dmat4 r;
                r.mult( *this, m );
                return r;
            }

        protected:

            value_type _mat[4][4];
    };

    class RefDMat4 : public Object,
                     public dmat4
    {
        public:

            RefDMat4() :
                Object( false ),
                dmat4()
            {
            }

            RefDMat4( const dmat4& other ) :
                Object( false ),
                dmat4( other )
            {
            }

            RefDMat4( const mat4& other ) :
                Object( false ),
                dmat4( other )
            {
            }

            RefDMat4( const RefDMat4& other ) :
                Object( other ),
                dmat4( other )
            {
            }

            explicit RefDMat4( const dmat4::value_type* const def ) :
                Object( false ),
                dmat4( def )
            {
            }

            RefDMat4( dmat4::value_type a00,
                      dmat4::value_type a01,
                      dmat4::value_type a02,
                      dmat4::value_type a03,
                      dmat4::value_type a10,
                      dmat4::value_type a11,
                      dmat4::value_type a12,
                      dmat4::value_type a13,
                      dmat4::value_type a20,
                      dmat4::value_type a21,
                      dmat4::value_type a22,
                      dmat4::value_type a23,
                      dmat4::value_type a30,
                      dmat4::value_type a31,
                      dmat4::value_type a32,
                      dmat4::value_type a33 ) :
                Object( false ),
                dmat4( a00,
                       a01,
                       a02,
                       a03,
                       a10,
                       a11,
                       a12,
                       a13,
                       a20,
                       a21,
                       a22,
                       a23,
                       a30,
                       a31,
                       a32,
                       a33 )
            {
            }

            virtual Object*
            cloneType() const
            {
                return new RefDMat4();
            }

            virtual Object*
            clone( const CopyOp& ) const
            {
                return new RefDMat4( *this );
            }

            virtual bool
            isSameKindAs( const Object* obj ) const
            {
                return dynamic_cast<const RefDMat4*>( obj ) != NULL;
            }

            virtual const char*
            libraryName() const
            {
                return "osg";
            }

            virtual const char*
            className() const
            {
                return "dmat4";
            }

        protected:

            virtual ~RefDMat4()
            {
            }
    };

    // static utility methods
    inline dmat4
    dmat4::identity( void ) noexcept
    {
        dmat4 m;
        m = osg::dmat4();
        return m;
    }

    inline dmat4
    dmat4::scale( value_type sx,
                  value_type sy,
                  value_type sz ) noexcept
    {
        dmat4 m;
        m.makeScale( sx, sy, sz );
        return m;
    }

    inline dmat4
    dmat4::scale( const vec3& v ) noexcept
    {
        return scale( v.x, v.y, v.z );
    }

    inline dmat4
    dmat4::scale( const dvec3& v ) noexcept
    {
        return scale( v.x, v.y, v.z );
    }

    inline dmat4
    dmat4::translate( value_type tx,
                      value_type ty,
                      value_type tz ) noexcept
    {
        dmat4 m;
        m.makeTranslate( tx, ty, tz );
        return m;
    }

    inline dmat4
    dmat4::translate( const vec3& v ) noexcept
    {
        return translate( v.x, v.y, v.z );
    }

    inline dmat4
    dmat4::translate( const dvec3& v ) noexcept
    {
        return translate( v.x, v.y, v.z );
    }

    inline dmat4
    dmat4::rotate( const quat& q ) noexcept
    {
        return dmat4( q );
    }

    inline dmat4
    dmat4::rotate( value_type angle,
                   value_type x,
                   value_type y,
                   value_type z ) noexcept
    {
        dmat4 m;
        m = osg::quat( angle, osg::vec3( x, y, z ) );
        return m;
    }

    inline dmat4
    dmat4::rotate( value_type  angle,
                   const vec3& axis ) noexcept
    {
        dmat4 m;
        m = osg::quat( angle, osg::vec3( axis ) );
        return m;
    }

    inline dmat4
    dmat4::rotate( value_type   angle,
                   const dvec3& axis ) noexcept
    {
        dmat4 m;
        m = osg::quat( angle, osg::vec3( axis ) );
        return m;
    }

    inline dmat4
    dmat4::rotate( value_type  angle1,
                   const vec3& axis1,
                   value_type  angle2,
                   const vec3& axis2,
                   value_type  angle3,
                   const vec3& axis3 ) noexcept
    {
        dmat4 m;
        m = osg::quat( angle1, osg::vec3( axis1, angle2, axis2, angle3, axis3 ) );
        return m;
    }

    inline dmat4
    dmat4::rotate( value_type   angle1,
                   const dvec3& axis1,
                   value_type   angle2,
                   const dvec3& axis2,
                   value_type   angle3,
                   const dvec3& axis3 ) noexcept
    {
        dmat4 m;
        m = osg::quat( angle1, osg::vec3( axis1, angle2, axis2, angle3, axis3 ) );
        return m;
    }

    inline dmat4
    dmat4::rotate( const vec3& from,
                   const vec3& to ) noexcept
    {
        dmat4 m;
        m = osg::quat( from, osg::vec3( to ) );
        return m;
    }

    inline dmat4
    dmat4::rotate( const dvec3& from,
                   const dvec3& to ) noexcept
    {
        dmat4 m;
        m = osg::quat( from, osg::vec3( to ) );
        return m;
    }

    inline dmat4
    dmat4::inverse( const dmat4& matrix ) noexcept
    {
        dmat4 m;
        m.invert( matrix );
        return m;
    }

    inline dmat4
    dmat4::orthoNormal( const dmat4& matrix ) noexcept
    {
        dmat4 m;
        m.orthoNormalize( matrix );
        return m;
    }

    inline dmat4
    dmat4::ortho( double left,
                  double right,
                  double bottom,
                  double top,
                  double zNear,
                  double zFar ) noexcept
    {
        dmat4 m;
        m.makeOrtho( left, right, bottom, top, zNear, zFar );
        return m;
    }

    inline dmat4
    dmat4::ortho2D( double left,
                    double right,
                    double bottom,
                    double top ) noexcept
    {
        dmat4 m;
        m.makeOrtho2D( left, right, bottom, top );
        return m;
    }

    inline dmat4
    dmat4::frustum( double left,
                    double right,
                    double bottom,
                    double top,
                    double zNear,
                    double zFar ) noexcept
    {
        dmat4 m;
        m.makeFrustum( left, right, bottom, top, zNear, zFar );
        return m;
    }

    inline dmat4
    dmat4::perspective( double fovy,
                        double aspectRatio,
                        double zNear,
                        double zFar ) noexcept
    {
        dmat4 m;
        m.makePerspective( fovy, aspectRatio, zNear, zFar );
        return m;
    }

    inline dmat4
    dmat4::lookAt( const vec3& eye,
                   const vec3& center,
                   const vec3& up ) noexcept
    {
        dmat4 m;
        m.makeLookAt( eye, center, up );
        return m;
    }

    inline dmat4
    dmat4::lookAt( const dvec3& eye,
                   const dvec3& center,
                   const dvec3& up ) noexcept
    {
        dmat4 m;
        m.makeLookAt( eye, center, up );
        return m;
    }

    inline vec3
    dmat4::postMult( const vec3& v ) const noexcept
    {
        value_type d =
            1.0F /
            ( _mat[3][0] * v.x + _mat[3][1] * v.y + _mat[3][2] * v.z + _mat[3][3] );
        return vec3(
            ( _mat[0][0] * v.x + _mat[0][1] * v.y + _mat[0][2] * v.z + _mat[0][3] ) * d,
            ( _mat[1][0] * v.x + _mat[1][1] * v.y + _mat[1][2] * v.z + _mat[1][3] ) * d,
            ( _mat[2][0] * v.x + _mat[2][1] * v.y + _mat[2][2] * v.z + _mat[2][3] ) * d
        );
    }

    inline dvec3
    dmat4::postMult( const dvec3& v ) const noexcept
    {
        value_type d =
            1.0F /
            ( _mat[3][0] * v.x + _mat[3][1] * v.y + _mat[3][2] * v.z + _mat[3][3] );
        return dvec3(
            ( _mat[0][0] * v.x + _mat[0][1] * v.y + _mat[0][2] * v.z + _mat[0][3] ) * d,
            ( _mat[1][0] * v.x + _mat[1][1] * v.y + _mat[1][2] * v.z + _mat[1][3] ) * d,
            ( _mat[2][0] * v.x + _mat[2][1] * v.y + _mat[2][2] * v.z + _mat[2][3] ) * d
        );
    }

    inline vec3
    dmat4::preMult( const vec3& v ) const noexcept
    {
        value_type d =
            1.0F /
            ( _mat[0][3] * v.x + _mat[1][3] * v.y + _mat[2][3] * v.z + _mat[3][3] );
        return vec3(
            ( _mat[0][0] * v.x + _mat[1][0] * v.y + _mat[2][0] * v.z + _mat[3][0] ) * d,
            ( _mat[0][1] * v.x + _mat[1][1] * v.y + _mat[2][1] * v.z + _mat[3][1] ) * d,
            ( _mat[0][2] * v.x + _mat[1][2] * v.y + _mat[2][2] * v.z + _mat[3][2] ) * d
        );
    }

    inline dvec3
    dmat4::preMult( const dvec3& v ) const noexcept
    {
        value_type d =
            1.0F /
            ( _mat[0][3] * v.x + _mat[1][3] * v.y + _mat[2][3] * v.z + _mat[3][3] );
        return dvec3(
            ( _mat[0][0] * v.x + _mat[1][0] * v.y + _mat[2][0] * v.z + _mat[3][0] ) * d,
            ( _mat[0][1] * v.x + _mat[1][1] * v.y + _mat[2][1] * v.z + _mat[3][1] ) * d,
            ( _mat[0][2] * v.x + _mat[1][2] * v.y + _mat[2][2] * v.z + _mat[3][2] ) * d
        );
    }

    inline vec4
    dmat4::postMult( const vec4& v ) const noexcept
    {
        return vec4(
            _mat[0][0] * v.x + _mat[0][1] * v.y + _mat[0][2] * v.z + _mat[0][3] * v.w,
            _mat[1][0] * v.x + _mat[1][1] * v.y + _mat[1][2] * v.z + _mat[1][3] * v.w,
            _mat[2][0] * v.x + _mat[2][1] * v.y + _mat[2][2] * v.z + _mat[2][3] * v.w,
            _mat[3][0] * v.x + _mat[3][1] * v.y + _mat[3][2] * v.z + _mat[3][3] * v.w
        );
    }

    inline dvec4
    dmat4::postMult( const dvec4& v ) const noexcept
    {
        return dvec4(
            _mat[0][0] * v.x + _mat[0][1] * v.y + _mat[0][2] * v.z + _mat[0][3] * v.w,
            _mat[1][0] * v.x + _mat[1][1] * v.y + _mat[1][2] * v.z + _mat[1][3] * v.w,
            _mat[2][0] * v.x + _mat[2][1] * v.y + _mat[2][2] * v.z + _mat[2][3] * v.w,
            _mat[3][0] * v.x + _mat[3][1] * v.y + _mat[3][2] * v.z + _mat[3][3] * v.w
        );
    }

    inline vec4
    dmat4::preMult( const vec4& v ) const noexcept
    {
        return vec4(
            _mat[0][0] * v.x + _mat[1][0] * v.y + _mat[2][0] * v.z + _mat[3][0] * v.w,
            _mat[0][1] * v.x + _mat[1][1] * v.y + _mat[2][1] * v.z + _mat[3][1] * v.w,
            _mat[0][2] * v.x + _mat[1][2] * v.y + _mat[2][2] * v.z + _mat[3][2] * v.w,
            _mat[0][3] * v.x + _mat[1][3] * v.y + _mat[2][3] * v.z + _mat[3][3] * v.w
        );
    }

    inline dvec4
    dmat4::preMult( const dvec4& v ) const noexcept
    {
        return dvec4(
            _mat[0][0] * v.x + _mat[1][0] * v.y + _mat[2][0] * v.z + _mat[3][0] * v.w,
            _mat[0][1] * v.x + _mat[1][1] * v.y + _mat[2][1] * v.z + _mat[3][1] * v.w,
            _mat[0][2] * v.x + _mat[1][2] * v.y + _mat[2][2] * v.z + _mat[3][2] * v.w,
            _mat[0][3] * v.x + _mat[1][3] * v.y + _mat[2][3] * v.z + _mat[3][3] * v.w
        );
    }

    inline vec3
    dmat4::transform3x3( const vec3&  v,
                         const dmat4& m ) noexcept
    {
        return vec3( m._mat[0][0] * v.x + m._mat[1][0] * v.y + m._mat[2][0] * v.z,
                     m._mat[0][1] * v.x + m._mat[1][1] * v.y + m._mat[2][1] * v.z,
                     m._mat[0][2] * v.x + m._mat[1][2] * v.y + m._mat[2][2] * v.z );
    }

    inline dvec3
    dmat4::transform3x3( const dvec3& v,
                         const dmat4& m ) noexcept
    {
        return dvec3( m._mat[0][0] * v.x + m._mat[1][0] * v.y + m._mat[2][0] * v.z,
                      m._mat[0][1] * v.x + m._mat[1][1] * v.y + m._mat[2][1] * v.z,
                      m._mat[0][2] * v.x + m._mat[1][2] * v.y + m._mat[2][2] * v.z );
    }

    inline vec3
    dmat4::transform3x3( const dmat4& m,
                         const vec3&  v ) noexcept
    {
        return vec3( m._mat[0][0] * v.x + m._mat[0][1] * v.y + m._mat[0][2] * v.z,
                     m._mat[1][0] * v.x + m._mat[1][1] * v.y + m._mat[1][2] * v.z,
                     m._mat[2][0] * v.x + m._mat[2][1] * v.y + m._mat[2][2] * v.z );
    }

    inline dvec3
    dmat4::transform3x3( const dmat4& m,
                         const dvec3& v ) noexcept
    {
        return dvec3( m._mat[0][0] * v.x + m._mat[0][1] * v.y + m._mat[0][2] * v.z,
                      m._mat[1][0] * v.x + m._mat[1][1] * v.y + m._mat[1][2] * v.z,
                      m._mat[2][0] * v.x + m._mat[2][1] * v.y + m._mat[2][2] * v.z );
    }

    inline void
    dmat4::preMultTranslate( const dvec3& v ) noexcept
    {
        for( unsigned i = 0; i < 3; ++i )
        {
            double tmp = v[i];
            if( tmp == 0 )
            {
                continue;
            }
            _mat[3][0] += tmp * _mat[i][0];
            _mat[3][1] += tmp * _mat[i][1];
            _mat[3][2] += tmp * _mat[i][2];
            _mat[3][3] += tmp * _mat[i][3];
        }
    }

    inline void
    dmat4::preMultTranslate( const vec3& v ) noexcept
    {
        for( unsigned i = 0; i < 3; ++i )
        {
            float tmp = v[i];
            if( tmp == 0 )
            {
                continue;
            }
            _mat[3][0] += tmp * _mat[i][0];
            _mat[3][1] += tmp * _mat[i][1];
            _mat[3][2] += tmp * _mat[i][2];
            _mat[3][3] += tmp * _mat[i][3];
        }
    }

    inline void
    dmat4::postMultTranslate( const dvec3& v ) noexcept
    {
        for( unsigned i = 0; i < 3; ++i )
        {
            double tmp = v[i];
            if( tmp == 0 )
            {
                continue;
            }
            _mat[0][i] += tmp * _mat[0][3];
            _mat[1][i] += tmp * _mat[1][3];
            _mat[2][i] += tmp * _mat[2][3];
            _mat[3][i] += tmp * _mat[3][3];
        }
    }

    inline void
    dmat4::postMultTranslate( const vec3& v ) noexcept
    {
        for( unsigned i = 0; i < 3; ++i )
        {
            float tmp = v[i];
            if( tmp == 0 )
            {
                continue;
            }
            _mat[0][i] += tmp * _mat[0][3];
            _mat[1][i] += tmp * _mat[1][3];
            _mat[2][i] += tmp * _mat[2][3];
            _mat[3][i] += tmp * _mat[3][3];
        }
    }

    inline void
    dmat4::preMultScale( const dvec3& v ) noexcept
    {
        _mat[0][0] *= v[0];
        _mat[0][1] *= v[0];
        _mat[0][2] *= v[0];
        _mat[0][3] *= v[0];
        _mat[1][0] *= v[1];
        _mat[1][1] *= v[1];
        _mat[1][2] *= v[1];
        _mat[1][3] *= v[1];
        _mat[2][0] *= v[2];
        _mat[2][1] *= v[2];
        _mat[2][2] *= v[2];
        _mat[2][3] *= v[2];
    }

    inline void
    dmat4::preMultScale( const vec3& v ) noexcept
    {
        _mat[0][0] *= v[0];
        _mat[0][1] *= v[0];
        _mat[0][2] *= v[0];
        _mat[0][3] *= v[0];
        _mat[1][0] *= v[1];
        _mat[1][1] *= v[1];
        _mat[1][2] *= v[1];
        _mat[1][3] *= v[1];
        _mat[2][0] *= v[2];
        _mat[2][1] *= v[2];
        _mat[2][2] *= v[2];
        _mat[2][3] *= v[2];
    }

    inline void
    dmat4::postMultScale( const dvec3& v ) noexcept
    {
        _mat[0][0] *= v[0];
        _mat[1][0] *= v[0];
        _mat[2][0] *= v[0];
        _mat[3][0] *= v[0];
        _mat[0][1] *= v[1];
        _mat[1][1] *= v[1];
        _mat[2][1] *= v[1];
        _mat[3][1] *= v[1];
        _mat[0][2] *= v[2];
        _mat[1][2] *= v[2];
        _mat[2][2] *= v[2];
        _mat[3][2] *= v[2];
    }

    inline void
    dmat4::postMultScale( const vec3& v ) noexcept
    {
        _mat[0][0] *= v[0];
        _mat[1][0] *= v[0];
        _mat[2][0] *= v[0];
        _mat[3][0] *= v[0];
        _mat[0][1] *= v[1];
        _mat[1][1] *= v[1];
        _mat[2][1] *= v[1];
        _mat[3][1] *= v[1];
        _mat[0][2] *= v[2];
        _mat[1][2] *= v[2];
        _mat[2][2] *= v[2];
        _mat[3][2] *= v[2];
    }

    inline void
    dmat4::preMultRotate( const quat& q ) noexcept
    {
        if( q.zeroRotation() )
        {
            return;
        }
        dmat4 r;
        r.setRotate( q );
        preMult( r );
    }

    inline void
    dmat4::postMultRotate( const quat& q ) noexcept
    {
        if( q.zeroRotation() )
        {
            return;
        }
        dmat4 r;
        r.setRotate( q );
        postMult( r );
    }

    inline vec3
    operator*( const vec3&  v,
               const dmat4& m ) noexcept
    {
        return osg::preMult( m, v );
    }

    inline dvec3
    operator*( const dvec3& v,
               const dmat4& m ) noexcept
    {
        return osg::preMult( m, v );
    }

    inline vec4
    operator*( const vec4&  v,
               const dmat4& m ) noexcept
    {
        return osg::preMult( m, v );
    }

    inline dvec4
    operator*( const dvec4& v,
               const dmat4& m ) noexcept
    {
        return osg::preMult( m, v );
    }

    inline vec3
    dmat4::operator*( const vec3& v ) const noexcept
    {
        return postMult( v );
    }

    inline dvec3
    dmat4::operator*( const dvec3& v ) const noexcept
    {
        return postMult( v );
    }

    inline vec4
    dmat4::operator*( const vec4& v ) const noexcept
    {
        return postMult( v );
    }

    inline dvec4
    dmat4::operator*( const dvec4& v ) const noexcept
    {
        return postMult( v );
    }

}    // namespace osg
