/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Template functor applied to geometry primitives. Generates
 * per-triangle/line/point callbacks from PrimitiveSets.
 */
#pragma once

#include <osg/core/Notify.hpp>
#include <osg/geometry/PrimitiveSet.hpp>

namespace osg
{

    /** Provides access to the primitives that compose an \c osg::Drawable.
     *  <p>Notice that \c TemplatePrimitiveFunctor is a class template, and that it
     * inherits from its template parameter \c T. This template parameter must implement
     *  <tt>operator()(const osg::vec3 v1, const osg::vec3 v2, const osg::vec3
     *  v3, bool treatVertexDataAsTemporary)</tt>,
     *  <tt>operator()(const osg::vec3 v1, const osg::vec3 v2, bool
     *  treatVertexDataAsTemporary)</tt>, <tt>operator()(const osg::vec3 v1,
     *  const osg::vec3 v2, const osg::vec3 v3, bool treatVertexDataAsTemporary)</tt>,
     *  and <tt>operator()(const osg::vec3 v1, const osg::vec3 v2, const osg::vec3 v3,
     *  const osg::vec3 v4, bool treatVertexDataAsTemporary)</tt> which will be called
     *  for the matching primitive when the functor is applied to a \c Drawable.
     *  Parameters \c v1, \c v2, \c v3, and \c v4 are the vertices of the primitive.
     *  The last parameter, \c treatVertexDataAsTemporary, indicates whether these
     *  vertices are coming from a "real" vertex array, or from a temporary vertex array,
     *  created by the \c TemplatePrimitiveFunctor from some other geometry
     * representation.
     *  @see \c PrimitiveFunctor for general usage hints.
     */
    template<class T>
    class TemplatePrimitiveFunctor : public PrimitiveFunctor,
                                     public T
    {
        public:

            TemplatePrimitiveFunctor()
            {
                _vertexArraySize = 0;
                _vertexArrayPtr  = 0;
            }

            virtual ~TemplatePrimitiveFunctor()
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
                                                  *( vptr + 2 ),
                                                  false );
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
                                                      *( vptr + 1 ),
                                                      false );
                                }
                                else
                                {
                                    this->operator()( *( vptr ),
                                                      *( vptr + 1 ),
                                                      *( vptr + 2 ),
                                                      false );
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
                                                  *( vptr + 2 ),
                                                  *( vptr + 3 ),
                                                  false );
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
                                                  *( vptr + 3 ),
                                                  *( vptr + 2 ),
                                                  false );
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
                                                  *( vptr + 1 ),
                                                  false );
                            }
                            break;
                        }
                    case( GL_POINTS ) :
                        {
                            const vec3* vlast = &_vertexArrayPtr[first + count];
                            for( const vec3* vptr = &_vertexArrayPtr[first];
                                 vptr < vlast;
                                 vptr += 1 )
                            {
                                this->operator()( *( vptr ), false );
                            }
                            break;
                        }
                    case( GL_LINES ) :
                        {
                            const vec3* vlast = &_vertexArrayPtr[first + count - 1];
                            for( const vec3* vptr = &_vertexArrayPtr[first];
                                 vptr < vlast;
                                 vptr += 2 )
                            {
                                this->operator()( *( vptr ), *( vptr + 1 ), false );
                            }
                            break;
                        }
                    case( GL_LINE_STRIP ) :
                        {
                            const vec3* vlast = &_vertexArrayPtr[first + count - 1];
                            for( const vec3* vptr = &_vertexArrayPtr[first];
                                 vptr < vlast;
                                 vptr += 1 )
                            {
                                this->operator()( *( vptr ), *( vptr + 1 ), false );
                            }
                            break;
                        }
                    case( GL_LINE_STRIP_ADJACENCY ) :
                        {
                            const vec3* vlast = &_vertexArrayPtr[first + count - 2];
                            for( const vec3* vptr = &_vertexArrayPtr[first + 1];
                                 vptr < vlast;
                                 vptr += 1 )
                            {
                                this->operator()( *( vptr ), *( vptr + 1 ), false );
                            }
                            break;
                        }
                    case( GL_LINE_LOOP ) :
                        {
                            const vec3* vlast = &_vertexArrayPtr[first + count - 1];
                            for( const vec3* vptr = &_vertexArrayPtr[first];
                                 vptr < vlast;
                                 vptr += 1 )
                            {
                                this->operator()( *( vptr ), *( vptr + 1 ), false );
                            }
                            this->operator()( *( vlast ),
                                              _vertexArrayPtr[first],
                                              false );
                            break;
                        }
                    default :
                        break;
                }
            }

            template<class IndexType>
            void
            drawElementsTemplate( GLenum           mode,
                                  GLsizei          count,
                                  const IndexType* indices )
            {
                if( indices == 0 || count == 0 )
                {
                    return;
                }

                typedef const IndexType* IndexPointer;

                switch( mode )
                {
                    case( GL_TRIANGLES ) :
                        {
                            IndexPointer ilast = &indices[count];
                            for( IndexPointer iptr = indices; iptr < ilast; iptr += 3 )
                            {
                                this->operator()( _vertexArrayPtr[*iptr],
                                                  _vertexArrayPtr[*( iptr + 1 )],
                                                  _vertexArrayPtr[*( iptr + 2 )],
                                                  false );
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
                                                      _vertexArrayPtr[*( iptr + 1 )],
                                                      false );
                                }
                                else
                                {
                                    this->operator()( _vertexArrayPtr[*( iptr )],
                                                      _vertexArrayPtr[*( iptr + 1 )],
                                                      _vertexArrayPtr[*( iptr + 2 )],
                                                      false );
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
                                                  _vertexArrayPtr[*( iptr + 2 )],
                                                  _vertexArrayPtr[*( iptr + 3 )],
                                                  false );
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
                                                  _vertexArrayPtr[*( iptr + 3 )],
                                                  _vertexArrayPtr[*( iptr + 2 )],
                                                  false );
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
                                                  _vertexArrayPtr[*( iptr + 1 )],
                                                  false );
                            }
                            break;
                        }
                    case( GL_POINTS ) :
                        {
                            IndexPointer ilast = &indices[count];
                            for( IndexPointer iptr = indices; iptr < ilast; iptr += 1 )
                            {
                                this->operator()( _vertexArrayPtr[*iptr], false );
                            }
                            break;
                        }
                    case( GL_LINES ) :
                        {
                            IndexPointer ilast = &indices[count - 1];
                            for( IndexPointer iptr = indices; iptr < ilast; iptr += 2 )
                            {
                                this->operator()( _vertexArrayPtr[*iptr],
                                                  _vertexArrayPtr[*( iptr + 1 )],
                                                  false );
                            }
                            break;
                        }
                    case( GL_LINE_STRIP ) :
                        {
                            IndexPointer ilast = &indices[count - 1];
                            for( IndexPointer iptr = indices; iptr < ilast; iptr += 1 )
                            {
                                this->operator()( _vertexArrayPtr[*iptr],
                                                  _vertexArrayPtr[*( iptr + 1 )],
                                                  false );
                            }
                            break;
                        }
                    case( GL_LINE_STRIP_ADJACENCY ) :
                        {
                            IndexPointer ilast = &indices[count - 2];
                            for( IndexPointer iptr  = &indices[1]; iptr < ilast;
                                 iptr              += 1 )
                            {
                                this->operator()( _vertexArrayPtr[*iptr],
                                                  _vertexArrayPtr[*( iptr + 1 )],
                                                  false );
                            }
                            break;
                        }
                    case( GL_LINE_LOOP ) :
                        {
                            IndexPointer ilast = &indices[count - 1];
                            for( IndexPointer iptr = indices; iptr < ilast; iptr += 1 )
                            {
                                this->operator()( _vertexArrayPtr[*iptr],
                                                  _vertexArrayPtr[*( iptr + 1 )],
                                                  false );
                            }
                            this->operator()( _vertexArrayPtr[*( ilast )],
                                              _vertexArrayPtr[indices[0]],
                                              false );
                            break;
                        }
                    default :
                        break;
                }
            }

            virtual void
            drawElements( GLenum         mode,
                          GLsizei        count,
                          const GLubyte* indices )
            {
                drawElementsTemplate( mode, count, indices );
            }

            virtual void
            drawElements( GLenum          mode,
                          GLsizei         count,
                          const GLushort* indices )
            {
                drawElementsTemplate( mode, count, indices );
            }

            virtual void
            drawElements( GLenum        mode,
                          GLsizei       count,
                          const GLuint* indices )
            {
                drawElementsTemplate( mode, count, indices );
            }

        protected:

            unsigned int _vertexArraySize;
            const vec3*  _vertexArrayPtr;
    };

}
