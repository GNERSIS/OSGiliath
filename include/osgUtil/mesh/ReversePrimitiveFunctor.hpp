/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Primitive functor that reverses winding order.
 * Used for generating back-face geometry for shadow volumes.
 */
#pragma once

#include <osg/core/Notify.hpp>
#include <osg/geometry/PrimitiveSet.hpp>
#include <osgUtil/Export.hpp>

namespace osgUtil
{

    class OSGUTIL_EXPORT ReversePrimitiveFunctor : public osg::PrimitiveIndexFunctor
    {
        public:

            virtual ~ReversePrimitiveFunctor()
            {
            }

            osg::PrimitiveSet*
            getReversedPrimitiveSet()
            {
                return _reversedPrimitiveSet.get();
            }

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
            drawArrays( GLenum  mode,
                        GLint   first,
                        GLsizei count );
            virtual void
            drawElements( GLenum         mode,
                          GLsizei        count,
                          const GLubyte* indices );
            virtual void
            drawElements( GLenum          mode,
                          GLsizei         count,
                          const GLushort* indices );
            virtual void
            drawElements( GLenum        mode,
                          GLsizei       count,
                          const GLuint* indices );

            /// Mimics the OpenGL \c glBegin() function.
            virtual void
            begin( GLenum mode );
            virtual void
            vertex( unsigned int /*pos*/ );
            virtual void
                                            end();

            osg::ref_ptr<osg::PrimitiveSet> _reversedPrimitiveSet;

        private:

            bool _running;
    };

}    // end osgUtil namespace
