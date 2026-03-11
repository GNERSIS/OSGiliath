/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Minimal GLU compatibility functions. Provides gluErrorString
 * and gluProject/gluUnProject for legacy code paths.
 */
#pragma once

#include <osg/GL>

namespace osg
{

    /* Pixel storage modes, used by gluScaleImage */
    struct OSG_EXPORT PixelStorageModes
    {
            // sets defaults as per glGet docs in OpenGL red book
            PixelStorageModes();

            // use glGet's to retrieve all the current settings
            void
            retrieveStoreModes();

            // use glGet's to retrieve all the current 3D settings
            void
                  retrieveStoreModes3D();

            GLint pack_alignment;
            GLint pack_row_length;
            GLint pack_skip_rows;
            GLint pack_skip_pixels;
            GLint pack_lsb_first;
            GLint pack_swap_bytes;
            GLint pack_skip_images;
            GLint pack_image_height;

            GLint unpack_alignment;
            GLint unpack_row_length;
            GLint unpack_skip_rows;
            GLint unpack_skip_pixels;
            GLint unpack_lsb_first;
            GLint unpack_swap_bytes;
            GLint unpack_skip_images;
            GLint unpack_image_height;
    };

    extern OSG_EXPORT const GLubyte*
    gluErrorString( GLenum error );

    /** OSG specific gluScaleImage function that allows you to pass in the
     * PixelStoreModes, which enables the code to avoid glGet's that are associated with
     * the conventional gluScaleImage function. Avoiding glGet's allows this
     * gluScaleImage function to be called at any time, from any thread, there is no need
     * to have a graphics context current.*/
    extern OSG_EXPORT GLint
    gluScaleImage( PixelStorageModes* psm,
                   GLenum             format,
                   GLsizei            wIn,
                   GLsizei            hIn,
                   GLenum             typeIn,
                   const void*        dataIn,
                   GLsizei            wOut,
                   GLsizei            hOut,
                   GLenum             typeOut,
                   GLvoid*            dataOut );

    /** Traditional GLU gluScaleImage function that sets up the PixelStoreModes
     * automatically by doing glGets.; The use of glGet's means that you can only call
     * this function from a thread with a valid graphics context. The use of glGet's will
     * also result in lower performance due to the round trip to the OpenGL driver.*/
    extern OSG_EXPORT GLint
    gluScaleImage( GLenum      format,
                   GLsizei     wIn,
                   GLsizei     hIn,
                   GLenum      typeIn,
                   const void* dataIn,
                   GLsizei     wOut,
                   GLsizei     hOut,
                   GLenum      typeOut,
                   GLvoid*     dataOut );

    extern OSG_EXPORT GLint
    gluBuild1DMipmapLevels( GLenum      target,
                            GLint       internalFormat,
                            GLsizei     width,
                            GLenum      format,
                            GLenum      type,
                            GLint       level,
                            GLint       base,
                            GLint       max,
                            const void* data );
    extern OSG_EXPORT GLint
    gluBuild1DMipmaps( GLenum      target,
                       GLint       internalFormat,
                       GLsizei     width,
                       GLenum      format,
                       GLenum      type,
                       const void* data );
    extern OSG_EXPORT GLint
    gluBuild2DMipmapLevels( GLenum      target,
                            GLint       internalFormat,
                            GLsizei     width,
                            GLsizei     height,
                            GLenum      format,
                            GLenum      type,
                            GLint       level,
                            GLint       base,
                            GLint       max,
                            const void* data );
    extern OSG_EXPORT GLint
    gluBuild2DMipmaps( GLenum      target,
                       GLint       internalFormat,
                       GLsizei     width,
                       GLsizei     height,
                       GLenum      format,
                       GLenum      type,
                       const void* data );

    typedef void( GL_APIENTRY* GLTexImage3DProc )( GLenum        target,
                                                   GLint         level,
                                                   GLenum        internalFormat,
                                                   GLsizei       width,
                                                   GLsizei       height,
                                                   GLsizei       depth,
                                                   GLint         border,
                                                   GLenum        format,
                                                   GLenum        type,
                                                   const GLvoid* pixels );

    /** Small variation on normal gluBuild3DMipmapLevels as we pass in the function
     * pointer to glTexImage3D rather than rely on GLU style query for this function
     * pointer.*/
    extern OSG_EXPORT GLint
    gluBuild3DMipmapLevels( GLTexImage3DProc glTextImage3DProc,
                            GLenum           target,
                            GLint            internalFormat,
                            GLsizei          width,
                            GLsizei          height,
                            GLsizei          depth,
                            GLenum           format,
                            GLenum           type,
                            GLint            level,
                            GLint            base,
                            GLint            max,
                            const void*      data );

    /** Small variation on normal gluBuild3DMipmapLevels as we pass in the function
     * pointer to glTexImage3D rather than rely on GLU style query for this function
     * pointer.*/
    extern OSG_EXPORT GLint
    gluBuild3DMipmaps( GLTexImage3DProc glTextImage3DProc,
                       GLenum           target,
                       GLint            internalFormat,
                       GLsizei          width,
                       GLsizei          height,
                       GLsizei          depth,
                       GLenum           format,
                       GLenum           type,
                       const void*      data );

/* ErrorCode */
#define GLU_INVALID_ENUM               100'900
#define GLU_INVALID_VALUE              100'901
#define GLU_OUT_OF_MEMORY              100'902
#define GLU_INCOMPATIBLE_GL_VERSION    100'903
#define GLU_INVALID_OPERATION          100'904

/* Boolean */
#define GLU_FALSE                      0
#define GLU_TRUE                       1

/* QuadricDrawStyle */
#define GLU_POINT                      100'010
#define GLU_LINE                       100'011
#define GLU_FILL                       100'012
#define GLU_SILHOUETTE                 100'013

/* QuadricCallback */
/*      GLU_ERROR */

/* QuadricNormal */
#define GLU_SMOOTH                     100'000
#define GLU_FLAT                       100'001
#define GLU_NONE                       100'002

/* QuadricOrientation */
#define GLU_OUTSIDE                    100'020
#define GLU_INSIDE                     100'021

/* TessCallback */
#define GLU_TESS_BEGIN                 100'100
#define GLU_BEGIN                      100'100
#define GLU_TESS_VERTEX                100'101
#define GLU_VERTEX                     100'101
#define GLU_TESS_END                   100'102
#define GLU_END                        100'102
#define GLU_TESS_ERROR                 100'103
#define GLU_TESS_EDGE_FLAG             100'104
#define GLU_EDGE_FLAG                  100'104
#define GLU_TESS_COMBINE               100'105
#define GLU_TESS_BEGIN_DATA            100'106
#define GLU_TESS_VERTEX_DATA           100'107
#define GLU_TESS_END_DATA              100'108
#define GLU_TESS_ERROR_DATA            100'109
#define GLU_TESS_EDGE_FLAG_DATA        100'110
#define GLU_TESS_COMBINE_DATA          100'111

/* TessContour */
#define GLU_CW                         100'120
#define GLU_CCW                        100'121
#define GLU_INTERIOR                   100'122
#define GLU_EXTERIOR                   100'123
#define GLU_UNKNOWN                    100'124

/* TessProperty */
#define GLU_TESS_WINDING_RULE          100'140
#define GLU_TESS_BOUNDARY_ONLY         100'141
#define GLU_TESS_TOLERANCE             100'142

/* TessError */
#define GLU_TESS_ERROR1                100'151
#define GLU_TESS_ERROR2                100'152
#define GLU_TESS_ERROR3                100'153
#define GLU_TESS_ERROR4                100'154
#define GLU_TESS_ERROR5                100'155
#define GLU_TESS_ERROR6                100'156
#define GLU_TESS_ERROR7                100'157
#define GLU_TESS_ERROR8                100'158
#define GLU_TESS_MISSING_BEGIN_POLYGON 100'151
#define GLU_TESS_MISSING_BEGIN_CONTOUR 100'152
#define GLU_TESS_MISSING_END_POLYGON   100'153
#define GLU_TESS_MISSING_END_CONTOUR   100'154
#define GLU_TESS_COORD_TOO_LARGE       100'155
#define GLU_TESS_NEED_COMBINE_CALLBACK 100'156

/* TessWinding */
#define GLU_TESS_WINDING_ODD           100'130
#define GLU_TESS_WINDING_NONZERO       100'131
#define GLU_TESS_WINDING_POSITIVE      100'132
#define GLU_TESS_WINDING_NEGATIVE      100'133
#define GLU_TESS_WINDING_ABS_GEQ_TWO   100'134

    struct GLUtesselator;
    typedef GLUtesselator GLUtesselatorObj;
    typedef GLUtesselator GLUtriangulatorObj;

#define GLU_TESS_MAX_COORD 1.0E150

    /* Internal convenience typedefs */
    typedef void( GL_APIENTRY* _GLUfuncptr )();
    typedef void( GL_APIENTRY* GLU_TESS_CALLBACK )();

    extern OSG_EXPORT GLUtesselator* GL_APIENTRY
    gluNewTess( void );
    extern OSG_EXPORT void GL_APIENTRY
    gluDeleteTess( GLUtesselator* tess );

    extern OSG_EXPORT void GL_APIENTRY
    gluTessBeginContour( GLUtesselator* tess );
    extern OSG_EXPORT void GL_APIENTRY
    gluTessCallback( GLUtesselator* tess,
                     GLenum         which,
                     _GLUfuncptr    CallBackFunc );
    extern OSG_EXPORT void GL_APIENTRY
    gluTessEndContour( GLUtesselator* tess );
    extern OSG_EXPORT void GL_APIENTRY
    gluTessNormal( GLUtesselator* tess,
                   GLdouble       valueX,
                   GLdouble       valueY,
                   GLdouble       valueZ );
    extern OSG_EXPORT void GL_APIENTRY
    gluTessProperty( GLUtesselator* tess,
                     GLenum         which,
                     GLdouble       data );
    extern OSG_EXPORT void GL_APIENTRY
    gluTessVertex( GLUtesselator* tess,
                   GLdouble*      location,
                   GLvoid*        data );
    extern OSG_EXPORT void GL_APIENTRY
    gluTessBeginPolygon( GLUtesselator* tess,
                         GLvoid*        data );
    extern OSG_EXPORT void GL_APIENTRY
    gluTessEndPolygon( GLUtesselator* tess );
    extern OSG_EXPORT void GL_APIENTRY
    gluGetTessProperty( GLUtesselator* tess,
                        GLenum         which,
                        GLdouble*      value );

}
