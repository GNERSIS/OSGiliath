/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Rendering statistics collector. Counts primitives, vertices,
 * draw calls, and state changes during a frame.
 */
#pragma once

#include <map>
#include <osg/core/Inherit.hpp>
#include <osg/geometry/Drawable.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/PrimitiveSet.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/LOD.hpp>
#include <osg/nodes/Switch.hpp>
#include <osg/nodes/Transform.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osgUtil/Export>
#include <ostream>
#include <set>

namespace osgUtil
{

    /**
     * Statistics base class. Used to extract primitive information from
     * the renderBin(s).  Add a case of getStats(osgUtil::Statistics *stat)
     * for any new drawable (or drawable derived class) that you generate
     * (eg see Geometry.cpp).  There are 20 types of drawable counted - actually only
     * 14 cases can occur in reality.  these represent sets of GL_POINTS, GL_LINES
     * GL_LINESTRIPS, LOOPS, TRIANGLES, TRI-fans, tristrips, quads, quadstrips etc
     * The number of triangles rendered is inferred:
     * each triangle = 1 triangle (number of vertices/3)
     * each quad = 2 triangles (nverts/2)
     * each trifan or tristrip = (length-2) triangles and so on.
     */

    class OSGUTIL_EXPORT Statistics : public osg::PrimitiveFunctor
    {
        public:

            typedef std::pair<unsigned int, unsigned int> PrimitivePair;
            typedef std::map<GLenum, PrimitivePair>       PrimitiveValueMap;
            typedef std::map<GLenum, unsigned int>        PrimitiveCountMap;

            Statistics();

            enum StatsType
            {
                STAT_NONE,    // default
                STAT_FRAMERATE,
                STAT_GRAPHS,
                STAT_PRIMS,
                STAT_PRIMSPERVIEW,
                STAT_PRIMSPERBIN,
                STAT_DC,
                STAT_RESTART,    // hint to restart the stats
            };

            void
            reset();

            void
            setType( StatsType t )
            {
                stattype = t;
            }

            virtual void
            setVertexArray( unsigned int count,
                            const osg::vec3* )
            {
                _vertexCount += count;
            }

            virtual void
            setVertexArray( unsigned int count,
                            const osg::vec2* )
            {
                _vertexCount += count;
            }

            virtual void
            setVertexArray( unsigned int count,
                            const osg::vec4* )
            {
                _vertexCount += count;
            }

            virtual void
            setVertexArray( unsigned int count,
                            const osg::dvec3* )
            {
                _vertexCount += count;
            }

            virtual void
            setVertexArray( unsigned int count,
                            const osg::dvec2* )
            {
                _vertexCount += count;
            }

            virtual void
            setVertexArray( unsigned int count,
                            const osg::dvec4* )
            {
                _vertexCount += count;
            }

            virtual void
            drawArrays( GLenum mode,
                        GLint,
                        GLsizei count );
            virtual void
            drawElements( GLenum  mode,
                          GLsizei count,
                          const GLubyte* );
            virtual void
            drawElements( GLenum  mode,
                          GLsizei count,
                          const GLushort* );
            virtual void
            drawElements( GLenum  mode,
                          GLsizei count,
                          const GLuint* );

            virtual void
            begin( GLenum mode );

            inline void
            vertex()
            {
                PrimitivePair& prim = _primitiveCount[_currentPrimitiveFunctorMode];
                ++prim.second;
                _number_of_vertexes++;
            }

            virtual void
            vertex( float,
                    float,
                    float )
            {
                vertex();
            }

            virtual void
            vertex( const osg::vec3& )
            {
                vertex();
            }

            virtual void
            vertex( const osg::vec2& )
            {
                vertex();
            }

            virtual void
            vertex( const osg::vec4& )
            {
                vertex();
            }

            virtual void
            vertex( float,
                    float )
            {
                vertex();
            }

            virtual void
            vertex( float,
                    float,
                    float,
                    float )
            {
                vertex();
            }

            virtual void
            end();

            void
            addDrawable()
            {
                numDrawables++;
            }

            void
            addFastDrawable()
            {
                numFastDrawables++;
            }

            void
            addMatrix()
            {
                nummat++;
            }

            void
            addLight( int np )
            {
                nlights += np;
            }

            void
            addImpostor( int np )
            {
                nimpostor += np;
            }

            inline int
            getBins()
            {
                return nbins;
            }

            void
            setDepth( int d )
            {
                depth = d;
            }

            void
            addBins( int np )
            {
                nbins += np;
            }

            void
            setBinNo( int n )
            {
                _binNo = n;
            }

            void
            addStateGraphs( int n )
            {
                numStateGraphs += n;
            }

            void
            addOrderedLeaves( int n )
            {
                numOrderedLeaves += n;
            }

            void
            add( const Statistics& stats );

        public:

            PrimitiveCountMap&
            getPrimitiveCountMap()
            {
                return _primitives_count;
            }

            const PrimitiveCountMap&
            getPrimitiveCountMap() const
            {
                return _primitives_count;
            }

            PrimitiveValueMap&
            getPrimitiveValueMap()
            {
                return _primitiveCount;
            }

            const PrimitiveValueMap&
            getPrimitiveValueMap() const
            {
                return _primitiveCount;
            }

            /// deprecated
            PrimitiveCountMap::iterator
            GetPrimitivesBegin()
            {
                return _primitives_count.begin();
            }

            /// deprecated
            PrimitiveCountMap::iterator
            GetPrimitivesEnd()
            {
                return _primitives_count.end();
            }

            int       numDrawables, nummat, nbins, numStateGraphs;
            int       numFastDrawables;
            int       nlights;
            int       depth;               // depth into bins - eg 1.1,1.2,1.3 etc
            int       _binNo;
            StatsType stattype;
            int       nimpostor;           // number of impostors rendered
            int       numOrderedLeaves;    // leaves from RenderBin fine grain ordering

            unsigned int      _vertexCount;
            PrimitiveValueMap _primitiveCount;
            GLenum            _currentPrimitiveFunctorMode;

        private:

            PrimitiveCountMap   _primitives_count;

            unsigned int        _total_primitives_count;
            unsigned int        _number_of_vertexes;

            inline unsigned int _calculate_primitives_number_by_mode( GLenum,
                                                                      GLsizei );
    };

    inline unsigned int
    Statistics::_calculate_primitives_number_by_mode( GLenum  mode,
                                                      GLsizei count )
    {
        switch( mode )
        {
            case GL_POINTS :
            case GL_LINE_LOOP :
            case GL_POLYGON :
                return static_cast<unsigned int>( count );
            case GL_LINES :
                return static_cast<unsigned int>( count / 2 );
            case GL_LINE_STRIP :
                return static_cast<unsigned int>( count - 1 );
            case GL_TRIANGLES :
                return static_cast<unsigned int>( count / 3 );
            case GL_TRIANGLE_STRIP :
            case GL_TRIANGLE_FAN :
                return static_cast<unsigned int>( count - 2 );
            case GL_QUADS :
                return static_cast<unsigned int>( count / 4 );
            case GL_QUAD_STRIP :
                return static_cast<unsigned int>( count / 2 - 1 );
            default :
                return 0;
        }
    }

    /** StatsVisitor for collecting statistics about scene graph.*/
    class OSGUTIL_EXPORT StatsVisitor : public osg::DualModeVisitor
    {
        public:

            typedef std::set<osg::Node*>     NodeSet;
            typedef std::set<osg::Drawable*> DrawableSet;
            typedef std::set<osg::StateSet*> StateSetSet;

            StatsVisitor();

            OSG_REGISTER_TYPE( osgUtil,
                               StatsVisitor )

            virtual void
            reset();

            using osg::ConstNodeVisitor::apply;
            using osg::NodeVisitor::apply;

            virtual void
            apply( osg::Node& node );
            virtual void
            apply( osg::Group& node );
            virtual void
            apply( osg::Transform& node );
            virtual void
            apply( osg::LOD& node );
            virtual void
            apply( osg::Switch& node );
            virtual void
            apply( osg::Geode& node );
            virtual void
            apply( osg::Drawable& drawable );
            virtual void
            apply( osg::StateSet& ss );

            virtual void
            totalUpStats();

            virtual void
                                print( std::ostream& out );

            unsigned int        _numInstancedGroup;
            unsigned int        _numInstancedSwitch;
            unsigned int        _numInstancedLOD;
            unsigned int        _numInstancedTransform;
            unsigned int        _numInstancedGeode;
            unsigned int        _numInstancedDrawable;
            unsigned int        _numInstancedGeometry;
            unsigned int        _numInstancedFastGeometry;
            unsigned int        _numInstancedStateSet;

            NodeSet             _groupSet;
            NodeSet             _transformSet;
            NodeSet             _lodSet;
            NodeSet             _switchSet;
            NodeSet             _geodeSet;
            DrawableSet         _drawableSet;
            DrawableSet         _geometrySet;
            DrawableSet         _fastGeometrySet;
            StateSetSet         _statesetSet;

            osgUtil::Statistics _uniqueStats;
            osgUtil::Statistics _instancedStats;
    };

}
