/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Convenience template that applies a functor to each triangle
 * in a Drawable. Decomposes strips and fans into individual triangles.
 */
#pragma once

#include <osg/core/Notify.hpp>
#include <osg/geometry/PrimitiveSet.hpp>

namespace osg
{

    /** Provides access to the triangles that compose an \c osg::Drawable. If the \c
     *  Drawable is not composed of triangles, the \c TriangleFunctor will convert
     *  the primitives to triangles whenever possible.
     *  <p>Notice that \c TriangleFunctor is a class template, and that it inherits
     *  from its template parameter \c T. This template parameter must implement
     *  <tt>T::operator() (const osg::vec3 v1, const osg::vec3 v2, const osg::vec3
     *  v3, bool treatVertexDataAsTemporary)</tt>, which will be called for every
     *  triangle when the functor is applied to a \c Drawable. Parameters \c v1, \c
     *  v2, and \c v3 are the triangle vertices. The fourth parameter, \c
     *  treatVertexDataAsTemporary, indicates whether these vertices are coming from
     *  a "real" vertex array, or from a temporary vertex array, created by the \c
     *  TriangleFunctor from some other geometry representation.
     *  @see \c PrimitiveFunctor for general usage hints.
     */
    template<class T>
    class TriangleFunctor : public PrimitiveFunctor,
                            public T
    {
        public:

            TriangleFunctor()
            {
                _vertexArraySize = 0;
                _vertexArrayPtr  = 0;
            }

            virtual ~TriangleFunctor()
            {
            }

            virtual void
            setVertexArray( unsigned int,
                            const vec2* )
            {
                notify( WARN ) << "Triangle Functor does not support vec2* vertex arrays"
                               << std::endl;
            }

            virtual void
            setVertexArray( unsigned int count,
                            const vec3*  vertices )
            {
                _vertexArraySize = count;
                _vertexArrayPtr  = vertices;
            }

            virtual void
            setVertexArray( unsigned int,
                            const vec4* )
            {
                notify( WARN ) << "Triangle Functor does not support vec4* vertex arrays"
                               << std::endl;
            }

            virtual void
            setVertexArray( unsigned int,
                            const dvec2* )
            {
                notify( WARN )
                    << "Triangle Functor does not support dvec2* vertex arrays"
                    << std::endl;
            }

            virtual void
            setVertexArray( unsigned int,
                            const dvec3* )
            {
                notify( WARN )
                    << "Triangle Functor does not support dvec3* vertex arrays"
                    << std::endl;
            }

            virtual void
            setVertexArray( unsigned int,
                            const dvec4* )
            {
                notify( WARN )
                    << "Triangle Functor does not support dvec4* vertex arrays"
                    << std::endl;
            }

            virtual void
            drawArrays( GLenum  mode,
                        GLint   first,
                        GLsizei count )
            {
                if( _vertexArrayPtr == 0 || count == 0 )
                {
                    return;
                }

                switch( mode )
                {
                    case( GL_TRIANGLES ) :
                        {
                            const vec3* vlast = &_vertexArrayPtr[first + count];
                            for( const vec3* vptr = &_vertexArrayPtr[first];
                                 vptr < vlast;
                                 vptr += 3 )
                            {
                                this->operator()( *( vptr ),
                                                  *( vptr + 1 ),
                                                  *( vptr + 2 ) );
                            }
                            break;
                        }
                    case( GL_TRIANGLE_STRIP ) :
                        {
                            const vec3* vptr = &_vertexArrayPtr[first];
                            for( GLsizei i = 2; i < count; ++i, ++vptr )
                            {
                                if( i % 2 )
                                {
                                    this->operator()( *( vptr ),
                                                      *( vptr + 2 ),
                                                      *( vptr + 1 ) );
                                }
                                else
                                {
                                    this->operator()( *( vptr ),
                                                      *( vptr + 1 ),
                                                      *( vptr + 2 ) );
                                }
                            }
                            break;
                        }
                    case( GL_QUADS ) :
                        {
                            const vec3* vptr = &_vertexArrayPtr[first];
                            for( GLsizei i = 3; i < count; i += 4, vptr += 4 )
                            {
                                this->operator()( *( vptr ),
                                                  *( vptr + 1 ),
                                                  *( vptr + 2 ) );
                                this->operator()( *( vptr ),
                                                  *( vptr + 2 ),
                                                  *( vptr + 3 ) );
                            }
                            break;
                        }
                    case( GL_QUAD_STRIP ) :
                        {
                            const vec3* vptr = &_vertexArrayPtr[first];
                            for( GLsizei i = 3; i < count; i += 2, vptr += 2 )
                            {
                                this->operator()( *( vptr ),
                                                  *( vptr + 1 ),
                                                  *( vptr + 2 ) );
                                this->operator()( *( vptr + 1 ),
                                                  *( vptr + 3 ),
                                                  *( vptr + 2 ) );
                            }
                            break;
                        }
                    case( GL_POLYGON ) :    // treat polygons as GL_TRIANGLE_FAN
                    case( GL_TRIANGLE_FAN ) :
                        {
                            const vec3* vfirst = &_vertexArrayPtr[first];
                            const vec3* vptr   = vfirst + 1;
                            for( GLsizei i = 2; i < count; ++i, ++vptr )
                            {
                                this->operator()( *( vfirst ),
                                                  *( vptr ),
                                                  *( vptr + 1 ) );
                            }
                            break;
                        }
                    case( GL_POINTS ) :
                    case( GL_LINES ) :
                    case( GL_LINE_STRIP ) :
                    case( GL_LINE_LOOP ) :
                    default :
                        // can't be converted into to triangles.
                        break;
                }
            }

            virtual void
            drawElements( GLenum         mode,
                          GLsizei        count,
                          const GLubyte* indices )
            {
                if( indices == 0 || count == 0 )
                {
                    return;
                }

                typedef const GLubyte* IndexPointer;

                switch( mode )
                {
                    case( GL_TRIANGLES ) :
                        {
                            IndexPointer ilast = &indices[count];
                            for( IndexPointer iptr = indices; iptr < ilast; iptr += 3 )
                            {
                                this->operator()( _vertexArrayPtr[*iptr],
                                                  _vertexArrayPtr[*( iptr + 1 )],
                                                  _vertexArrayPtr[*( iptr + 2 )] );
                            }
                            break;
                        }
                    case( GL_TRIANGLE_STRIP ) :
                        {
                            IndexPointer iptr = indices;
                            for( GLsizei i = 2; i < count; ++i, ++iptr )
                            {
                                if( i % 2 )
                                {
                                    this->operator()( _vertexArrayPtr[*( iptr )],
                                                      _vertexArrayPtr[*( iptr + 2 )],
                                                      _vertexArrayPtr[*( iptr + 1 )] );
                                }
                                else
                                {
                                    this->operator()( _vertexArrayPtr[*( iptr )],
                                                      _vertexArrayPtr[*( iptr + 1 )],
                                                      _vertexArrayPtr[*( iptr + 2 )] );
                                }
                            }
                            break;
                        }
                    case( GL_QUADS ) :
                        {
                            IndexPointer iptr = indices;
                            for( GLsizei i = 3; i < count; i += 4, iptr += 4 )
                            {
                                this->operator()( _vertexArrayPtr[*( iptr )],
                                                  _vertexArrayPtr[*( iptr + 1 )],
                                                  _vertexArrayPtr[*( iptr + 2 )] );
                                this->operator()( _vertexArrayPtr[*( iptr )],
                                                  _vertexArrayPtr[*( iptr + 2 )],
                                                  _vertexArrayPtr[*( iptr + 3 )] );
                            }
                            break;
                        }
                    case( GL_QUAD_STRIP ) :
                        {
                            IndexPointer iptr = indices;
                            for( GLsizei i = 3; i < count; i += 2, iptr += 2 )
                            {
                                this->operator()( _vertexArrayPtr[*( iptr )],
                                                  _vertexArrayPtr[*( iptr + 1 )],
                                                  _vertexArrayPtr[*( iptr + 2 )] );
                                this->operator()( _vertexArrayPtr[*( iptr + 1 )],
                                                  _vertexArrayPtr[*( iptr + 3 )],
                                                  _vertexArrayPtr[*( iptr + 2 )] );
                            }
                            break;
                        }
                    case( GL_POLYGON ) :    // treat polygons as GL_TRIANGLE_FAN
                    case( GL_TRIANGLE_FAN ) :
                        {
                            IndexPointer iptr   = indices;
                            const vec3&  vfirst = _vertexArrayPtr[*iptr];
                            ++iptr;
                            for( GLsizei i = 2; i < count; ++i, ++iptr )
                            {
                                this->operator()( vfirst,
                                                  _vertexArrayPtr[*( iptr )],
                                                  _vertexArrayPtr[*( iptr + 1 )] );
                            }
                            break;
                        }
                    case( GL_POINTS ) :
                    case( GL_LINES ) :
                    case( GL_LINE_STRIP ) :
                    case( GL_LINE_LOOP ) :
                    default :
                        // can't be converted into to triangles.
                        break;
                }
            }

            virtual void
            drawElements( GLenum          mode,
                          GLsizei         count,
                          const GLushort* indices )
            {
                if( indices == 0 || count == 0 )
                {
                    return;
                }

                typedef const GLushort* IndexPointer;

                switch( mode )
                {
                    case( GL_TRIANGLES ) :
                        {
                            IndexPointer ilast = &indices[count];
                            for( IndexPointer iptr = indices; iptr < ilast; iptr += 3 )
                            {
                                this->operator()( _vertexArrayPtr[*iptr],
                                                  _vertexArrayPtr[*( iptr + 1 )],
                                                  _vertexArrayPtr[*( iptr + 2 )] );
                            }
                            break;
                        }
                    case( GL_TRIANGLE_STRIP ) :
                        {
                            IndexPointer iptr = indices;
                            for( GLsizei i = 2; i < count; ++i, ++iptr )
                            {
                                if( i % 2 )
                                {
                                    this->operator()( _vertexArrayPtr[*( iptr )],
                                                      _vertexArrayPtr[*( iptr + 2 )],
                                                      _vertexArrayPtr[*( iptr + 1 )] );
                                }
                                else
                                {
                                    this->operator()( _vertexArrayPtr[*( iptr )],
                                                      _vertexArrayPtr[*( iptr + 1 )],
                                                      _vertexArrayPtr[*( iptr + 2 )] );
                                }
                            }
                            break;
                        }
                    case( GL_QUADS ) :
                        {
                            IndexPointer iptr = indices;
                            for( GLsizei i = 3; i < count; i += 4, iptr += 4 )
                            {
                                this->operator()( _vertexArrayPtr[*( iptr )],
                                                  _vertexArrayPtr[*( iptr + 1 )],
                                                  _vertexArrayPtr[*( iptr + 2 )] );
                                this->operator()( _vertexArrayPtr[*( iptr )],
                                                  _vertexArrayPtr[*( iptr + 2 )],
                                                  _vertexArrayPtr[*( iptr + 3 )] );
                            }
                            break;
                        }
                    case( GL_QUAD_STRIP ) :
                        {
                            IndexPointer iptr = indices;
                            for( GLsizei i = 3; i < count; i += 2, iptr += 2 )
                            {
                                this->operator()( _vertexArrayPtr[*( iptr )],
                                                  _vertexArrayPtr[*( iptr + 1 )],
                                                  _vertexArrayPtr[*( iptr + 2 )] );
                                this->operator()( _vertexArrayPtr[*( iptr + 1 )],
                                                  _vertexArrayPtr[*( iptr + 3 )],
                                                  _vertexArrayPtr[*( iptr + 2 )] );
                            }
                            break;
                        }
                    case( GL_POLYGON ) :    // treat polygons as GL_TRIANGLE_FAN
                    case( GL_TRIANGLE_FAN ) :
                        {
                            IndexPointer iptr   = indices;
                            const vec3&  vfirst = _vertexArrayPtr[*iptr];
                            ++iptr;
                            for( GLsizei i = 2; i < count; ++i, ++iptr )
                            {
                                this->operator()( vfirst,
                                                  _vertexArrayPtr[*( iptr )],
                                                  _vertexArrayPtr[*( iptr + 1 )] );
                            }
                            break;
                        }
                    case( GL_POINTS ) :
                    case( GL_LINES ) :
                    case( GL_LINE_STRIP ) :
                    case( GL_LINE_LOOP ) :
                    default :
                        // can't be converted into to triangles.
                        break;
                }
            }

            virtual void
            drawElements( GLenum        mode,
                          GLsizei       count,
                          const GLuint* indices )
            {
                if( indices == 0 || count == 0 )
                {
                    return;
                }

                typedef const GLuint* IndexPointer;

                switch( mode )
                {
                    case( GL_TRIANGLES ) :
                        {
                            IndexPointer ilast = &indices[count];
                            for( IndexPointer iptr = indices; iptr < ilast; iptr += 3 )
                            {
                                this->operator()( _vertexArrayPtr[*iptr],
                                                  _vertexArrayPtr[*( iptr + 1 )],
                                                  _vertexArrayPtr[*( iptr + 2 )] );
                            }
                            break;
                        }
                    case( GL_TRIANGLE_STRIP ) :
                        {
                            IndexPointer iptr = indices;
                            for( GLsizei i = 2; i < count; ++i, ++iptr )
                            {
                                if( i % 2 )
                                {
                                    this->operator()( _vertexArrayPtr[*( iptr )],
                                                      _vertexArrayPtr[*( iptr + 2 )],
                                                      _vertexArrayPtr[*( iptr + 1 )] );
                                }
                                else
                                {
                                    this->operator()( _vertexArrayPtr[*( iptr )],
                                                      _vertexArrayPtr[*( iptr + 1 )],
                                                      _vertexArrayPtr[*( iptr + 2 )] );
                                }
                            }
                            break;
                        }
                    case( GL_QUADS ) :
                        {
                            IndexPointer iptr = indices;
                            for( GLsizei i = 3; i < count; i += 4, iptr += 4 )
                            {
                                this->operator()( _vertexArrayPtr[*( iptr )],
                                                  _vertexArrayPtr[*( iptr + 1 )],
                                                  _vertexArrayPtr[*( iptr + 2 )] );
                                this->operator()( _vertexArrayPtr[*( iptr )],
                                                  _vertexArrayPtr[*( iptr + 2 )],
                                                  _vertexArrayPtr[*( iptr + 3 )] );
                            }
                            break;
                        }
                    case( GL_QUAD_STRIP ) :
                        {
                            IndexPointer iptr = indices;
                            for( GLsizei i = 3; i < count; i += 2, iptr += 2 )
                            {
                                this->operator()( _vertexArrayPtr[*( iptr )],
                                                  _vertexArrayPtr[*( iptr + 1 )],
                                                  _vertexArrayPtr[*( iptr + 2 )] );
                                this->operator()( _vertexArrayPtr[*( iptr + 1 )],
                                                  _vertexArrayPtr[*( iptr + 3 )],
                                                  _vertexArrayPtr[*( iptr + 2 )] );
                            }
                            break;
                        }
                    case( GL_POLYGON ) :    // treat polygons as GL_TRIANGLE_FAN
                    case( GL_TRIANGLE_FAN ) :
                        {
                            IndexPointer iptr   = indices;
                            const vec3&  vfirst = _vertexArrayPtr[*iptr];
                            ++iptr;
                            for( GLsizei i = 2; i < count; ++i, ++iptr )
                            {
                                this->operator()( vfirst,
                                                  _vertexArrayPtr[*( iptr )],
                                                  _vertexArrayPtr[*( iptr + 1 )] );
                            }
                            break;
                        }
                    case( GL_POINTS ) :
                    case( GL_LINES ) :
                    case( GL_LINE_STRIP ) :
                    case( GL_LINE_LOOP ) :
                    default :
                        // can't be converted into to triangles.
                        break;
                }
            }

        protected:

            unsigned int _vertexArraySize;
            const vec3*  _vertexArrayPtr;
    };

}
