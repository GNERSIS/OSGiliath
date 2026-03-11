/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Combined triangle/line/point index functor. Calls separate
 * callbacks for each primitive type in a Drawable.
 */
#pragma once

#include <osg/geometry/Array.hpp>
#include <osg/geometry/PrimitiveSet.hpp>

namespace osg
{

    template<class T>
    class TriangleLinePointIndexFunctor : public osg::PrimitiveIndexFunctor,
                                          public T
    {
        public:

            virtual void
            setVertexArray( unsigned int,
                            const osg::vec2* )
            {
            }

            virtual void
            setVertexArray( unsigned int,
                            const osg::vec3* )
            {
            }

            virtual void
            setVertexArray( unsigned int,
                            const osg::vec4* )
            {
            }

            virtual void
            setVertexArray( unsigned int,
                            const osg::dvec2* )
            {
            }

            virtual void
            setVertexArray( unsigned int,
                            const osg::dvec3* )
            {
            }

            virtual void
            setVertexArray( unsigned int,
                            const osg::dvec4* )
            {
            }

            virtual void
            begin( GLenum mode )
            {
                _modeCache = mode;
                _indexCache.clear();
            }

            virtual void
            vertex( unsigned int vert )
            {
                _indexCache.push_back( vert );
            }

            virtual void
            end()
            {
                if( !_indexCache.empty() )
                {
                    drawElements( _modeCache,
                                  static_cast<GLsizei>( _indexCache.size() ),
                                  &_indexCache.front() );
                }
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
                    case( GL_LINES ) :
                        {
                            unsigned int pos = static_cast<unsigned int>( first );
                            for( GLsizei i = 0; i < count; i += 2, pos += 2 )
                            {
                                this->operator()( pos, pos + 1 );
                            }
                            break;
                        }
                    case( GL_LINE_STRIP ) :
                        {
                            unsigned int pos = static_cast<unsigned int>( first );
                            for( GLsizei i = 0; i < count - 1; i += 1, pos += 1 )
                            {
                                this->operator()( pos, pos + 1 );
                            }
                            break;
                        }
                    case( GL_LINE_LOOP ) :
                        {
                            unsigned int pos = static_cast<unsigned int>( first );
                            for( GLsizei i = 0; i < count - 1; i += 1, pos += 1 )
                            {
                                this->operator()( pos, pos + 1 );
                            }
                            this->operator()( pos, static_cast<unsigned int>( first ) );
                            break;
                        }
                    case( GL_POINTS ) :
                        {
                            unsigned int pos = static_cast<unsigned int>( first );
                            for( GLsizei i = 0; i < count; ++i )
                            {
                                this->operator()( pos + static_cast<unsigned int>( i ) );
                            }
                            break;
                        }
                    default :
                        break;
                }
            }

            template<typename I>
            void
            drawElements( GLenum   mode,
                          GLsizei  count,
                          const I* indices )
            {
                typedef I        Index;
                typedef const I* IndexPointer;

                if( indices == 0 || count == 0 )
                {
                    return;
                }

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
                    case( GL_LINES ) :
                        {
                            const I* iptr = indices;
                            for( GLsizei i = 0; i < count; i += 2, iptr += 2 )
                            {
                                this->operator()( *iptr, *( iptr + 1 ) );
                            }
                            break;
                        }
                    case( GL_LINE_STRIP ) :
                        {
                            const I* iptr = indices;
                            for( GLsizei i = 0; i < count - 1; i += 1, iptr += 1 )
                            {
                                this->operator()( *iptr, *( iptr + 1 ) );
                            }
                            break;
                        }
                    case( GL_LINE_LOOP ) :
                        {
                            const I* iptr  = indices;
                            I        first = *iptr;
                            for( GLsizei i = 0; i < count - 1; i += 1, iptr += 1 )
                            {
                                this->operator()( *iptr, *( iptr + 1 ) );
                            }
                            this->operator()( *iptr, first );
                            break;
                        }
                    case GL_POINTS :
                        {
                            IndexPointer ilast = &indices[count];
                            for( IndexPointer iptr = indices; iptr < ilast; iptr += 1 )
                            {
                                this->operator()( *iptr );
                            }
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
                drawElements<GLubyte>( mode, count, indices );
            }

            virtual void
            drawElements( GLenum          mode,
                          GLsizei         count,
                          const GLushort* indices )
            {
                drawElements<GLushort>( mode, count, indices );
            }

            virtual void
            drawElements( GLenum        mode,
                          GLsizei       count,
                          const GLuint* indices )
            {
                drawElements<GLuint>( mode, count, indices );
            }

            GLenum                    _modeCache;
            std::vector<GLuint>       _indexCache;
            std::vector<unsigned int> _remap;
    };

}
