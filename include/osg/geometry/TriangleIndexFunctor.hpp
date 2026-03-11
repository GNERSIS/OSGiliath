/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Index-preserving triangle functor. Like TriangleFunctor but
 * passes original vertex indices for attribute lookup.
 */
#pragma once

#include <osg/core/Notify.hpp>
#include <osg/geometry/PrimitiveSet.hpp>

namespace osg
{

    template<class T>
    class TriangleIndexFunctor : public PrimitiveIndexFunctor,
                                 public T
    {
        public:

            virtual void
            setVertexArray( unsigned int,
                            const vec2* )
            {
            }

            virtual void
            setVertexArray( unsigned int,
                            const vec3* )
            {
            }

            virtual void
            setVertexArray( unsigned int,
                            const vec4* )
            {
            }

            virtual void
            setVertexArray( unsigned int,
                            const dvec2* )
            {
            }

            virtual void
            setVertexArray( unsigned int,
                            const dvec3* )
            {
            }

            virtual void
            setVertexArray( unsigned int,
                            const dvec4* )
            {
            }

            virtual void
            drawArrays( GLenum  mode,
                        GLint   first,
                        GLsizei count )
            {
                switch( mode )
                {
                    case( GL_TRIANGLES ) :
                        {
                            unsigned int pos = static_cast<unsigned int>( first );
                            for( GLsizei i = 2; i < count; i += 3, pos += 3 )
                            {
                                this->operator()( pos, pos + 1, pos + 2 );
                            }
                            break;
                        }
                    case( GL_TRIANGLE_STRIP ) :
                        {
                            unsigned int pos = static_cast<unsigned int>( first );
                            for( GLsizei i = 2; i < count; ++i, ++pos )
                            {
                                if( i % 2 )
                                {
                                    this->operator()( pos, pos + 2, pos + 1 );
                                }
                                else
                                {
                                    this->operator()( pos, pos + 1, pos + 2 );
                                }
                            }
                            break;
                        }
                    case( GL_QUADS ) :
                        {
                            unsigned int pos = static_cast<unsigned int>( first );
                            for( GLsizei i = 3; i < count; i += 4, pos += 4 )
                            {
                                this->operator()( pos, pos + 1, pos + 2 );
                                this->operator()( pos, pos + 2, pos + 3 );
                            }
                            break;
                        }
                    case( GL_QUAD_STRIP ) :
                        {
                            unsigned int pos = static_cast<unsigned int>( first );
                            for( GLsizei i = 3; i < count; i += 2, pos += 2 )
                            {
                                this->operator()( pos, pos + 1, pos + 2 );
                                this->operator()( pos + 1, pos + 3, pos + 2 );
                            }
                            break;
                        }
                    case( GL_POLYGON ) :    // treat polygons as GL_TRIANGLE_FAN
                    case( GL_TRIANGLE_FAN ) :
                        {
                            unsigned int pos = static_cast<unsigned int>( first ) + 1;
                            for( GLsizei i = 2; i < count; ++i, ++pos )
                            {
                                this->operator()( static_cast<unsigned int>( first ),
                                                  pos,
                                                  pos + 1 );
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

                typedef GLubyte      Index;
                typedef const Index* IndexPointer;

                switch( mode )
                {
                    case( GL_TRIANGLES ) :
                        {
                            IndexPointer ilast = &indices[count];
                            for( IndexPointer iptr = indices; iptr < ilast; iptr += 3 )
                            {
                                this->operator()( *iptr, *( iptr + 1 ), *( iptr + 2 ) );
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
                                    this->operator()( *( iptr ),
                                                      *( iptr + 2 ),
                                                      *( iptr + 1 ) );
                                }
                                else
                                {
                                    this->operator()( *( iptr ),
                                                      *( iptr + 1 ),
                                                      *( iptr + 2 ) );
                                }
                            }
                            break;
                        }
                    case( GL_QUADS ) :
                        {
                            IndexPointer iptr = indices;
                            for( GLsizei i = 3; i < count; i += 4, iptr += 4 )
                            {
                                this->operator()( *( iptr ),
                                                  *( iptr + 1 ),
                                                  *( iptr + 2 ) );
                                this->operator()( *( iptr ),
                                                  *( iptr + 2 ),
                                                  *( iptr + 3 ) );
                            }
                            break;
                        }
                    case( GL_QUAD_STRIP ) :
                        {
                            IndexPointer iptr = indices;
                            for( GLsizei i = 3; i < count; i += 2, iptr += 2 )
                            {
                                this->operator()( *( iptr ),
                                                  *( iptr + 1 ),
                                                  *( iptr + 2 ) );
                                this->operator()( *( iptr + 1 ),
                                                  *( iptr + 3 ),
                                                  *( iptr + 2 ) );
                            }
                            break;
                        }
                    case( GL_POLYGON ) :    // treat polygons as GL_TRIANGLE_FAN
                    case( GL_TRIANGLE_FAN ) :
                        {
                            IndexPointer iptr  = indices;
                            Index        first = *iptr;
                            ++iptr;
                            for( GLsizei i = 2; i < count; ++i, ++iptr )
                            {
                                this->operator()( first, *( iptr ), *( iptr + 1 ) );
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

                typedef GLushort     Index;
                typedef const Index* IndexPointer;

                switch( mode )
                {
                    case( GL_TRIANGLES ) :
                        {
                            IndexPointer ilast = &indices[count];
                            for( IndexPointer iptr = indices; iptr < ilast; iptr += 3 )
                            {
                                this->operator()( *iptr, *( iptr + 1 ), *( iptr + 2 ) );
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
                                    this->operator()( *( iptr ),
                                                      *( iptr + 2 ),
                                                      *( iptr + 1 ) );
                                }
                                else
                                {
                                    this->operator()( *( iptr ),
                                                      *( iptr + 1 ),
                                                      *( iptr + 2 ) );
                                }
                            }
                            break;
                        }
                    case( GL_QUADS ) :
                        {
                            IndexPointer iptr = indices;
                            for( GLsizei i = 3; i < count; i += 4, iptr += 4 )
                            {
                                this->operator()( *( iptr ),
                                                  *( iptr + 1 ),
                                                  *( iptr + 2 ) );
                                this->operator()( *( iptr ),
                                                  *( iptr + 2 ),
                                                  *( iptr + 3 ) );
                            }
                            break;
                        }
                    case( GL_QUAD_STRIP ) :
                        {
                            IndexPointer iptr = indices;
                            for( GLsizei i = 3; i < count; i += 2, iptr += 2 )
                            {
                                this->operator()( *( iptr ),
                                                  *( iptr + 1 ),
                                                  *( iptr + 2 ) );
                                this->operator()( *( iptr + 1 ),
                                                  *( iptr + 3 ),
                                                  *( iptr + 2 ) );
                            }
                            break;
                        }
                    case( GL_POLYGON ) :    // treat polygons as GL_TRIANGLE_FAN
                    case( GL_TRIANGLE_FAN ) :
                        {
                            IndexPointer iptr  = indices;
                            Index        first = *iptr;
                            ++iptr;
                            for( GLsizei i = 2; i < count; ++i, ++iptr )
                            {
                                this->operator()( first, *( iptr ), *( iptr + 1 ) );
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

                typedef GLuint       Index;
                typedef const Index* IndexPointer;

                switch( mode )
                {
                    case( GL_TRIANGLES ) :
                        {
                            IndexPointer ilast = &indices[count];
                            for( IndexPointer iptr = indices; iptr < ilast; iptr += 3 )
                            {
                                this->operator()( *iptr, *( iptr + 1 ), *( iptr + 2 ) );
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
                                    this->operator()( *( iptr ),
                                                      *( iptr + 2 ),
                                                      *( iptr + 1 ) );
                                }
                                else
                                {
                                    this->operator()( *( iptr ),
                                                      *( iptr + 1 ),
                                                      *( iptr + 2 ) );
                                }
                            }
                            break;
                        }
                    case( GL_QUADS ) :
                        {
                            IndexPointer iptr = indices;
                            for( GLsizei i = 3; i < count; i += 4, iptr += 4 )
                            {
                                this->operator()( *( iptr ),
                                                  *( iptr + 1 ),
                                                  *( iptr + 2 ) );
                                this->operator()( *( iptr ),
                                                  *( iptr + 2 ),
                                                  *( iptr + 3 ) );
                            }
                            break;
                        }
                    case( GL_QUAD_STRIP ) :
                        {
                            IndexPointer iptr = indices;
                            for( GLsizei i = 3; i < count; i += 2, iptr += 2 )
                            {
                                this->operator()( *( iptr ),
                                                  *( iptr + 1 ),
                                                  *( iptr + 2 ) );
                                this->operator()( *( iptr + 1 ),
                                                  *( iptr + 3 ),
                                                  *( iptr + 2 ) );
                            }
                            break;
                        }
                    case( GL_POLYGON ) :    // treat polygons as GL_TRIANGLE_FAN
                    case( GL_TRIANGLE_FAN ) :
                        {
                            IndexPointer iptr  = indices;
                            Index        first = *iptr;
                            ++iptr;
                            for( GLsizei i = 2; i < count; ++i, ++iptr )
                            {
                                this->operator()( first, *( iptr ), *( iptr + 1 ) );
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
    };

}
