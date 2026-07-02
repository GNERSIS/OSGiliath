/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Weather precipitation (rain, snow). Renders falling particles
 * over a bounded volume relative to the camera.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/box.hpp>
#include <osg/maths/vec4.hpp>
#include <osg/nodes/Group.hpp>
#include <osgParticle/Export.hpp>
#include <osgUtil/culling/CullVisitor.hpp>

namespace osgParticle
{

    class OSGPARTICLE_EXPORT PrecipitationEffect : public osg::Node
    {
        public:

            PrecipitationEffect();
            PrecipitationEffect( const PrecipitationEffect& copy,
                                 const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            virtual const char*
            libraryName() const
            {
                return "osgParticle";
            }

            virtual const char*
            className() const
            {
                return "PrecipitationEffect";
            }

            virtual bool
            isSameKindAs( const osg::Object* obj ) const
            {
                return dynamic_cast<const PrecipitationEffect*>( obj ) != 0;
            }

            virtual void
            accept( osg::NodeVisitor& nv )
            {
                if( nv.validNodeMask( *this ) )
                {
                    nv.pushOntoNodePath( this );
                    nv.apply( *this );
                    nv.popFromNodePath();
                }
            }

            virtual void
            resizeGLObjectBuffers( unsigned int maxSize );
            virtual void
            releaseGLObjects( osg::State* state = 0 ) const;

            virtual void
            traverse( osg::NodeVisitor& nv );

            /** Set all the parameters to create an rain effect of specified intensity.*/
            void
            rain( float intensity );

            /** Set all the parameters to create an snow effect of specified intensity.*/
            void
            snow( float intensity );

            void
            setMaximumParticleDensity( float density )
            {
                if( _maximumParticleDensity == density )
                {
                    return;
                }
                _maximumParticleDensity = density;
                _dirty                  = true;
            }

            float
            getMaximumParticleDensity() const
            {
                return _maximumParticleDensity;
            }

            void
            setWind( const osg::vec3& wind )
            {
                _wind = wind;
            }

            const osg::vec3&
            getWind() const
            {
                return _wind;
            }

            void
            setPosition( const osg::vec3& position )
            {
                _origin = position;
            }

            const osg::vec3&
            getPosition() const
            {
                return _origin;
            }

            void
            setCellSize( const osg::vec3& cellSize )
            {
                if( _cellSize == cellSize )
                {
                    return;
                }
                _cellSize = cellSize;
                _dirty    = true;
            }

            const osg::vec3&
            getCellSize() const
            {
                return _cellSize;
            }

            void
            setParticleSpeed( float particleSpeed )
            {
                if( _particleSpeed == particleSpeed )
                {
                    return;
                }
                _particleSpeed = particleSpeed;
                _dirty         = true;
            }

            float
            getParticleSpeed() const
            {
                return _particleSpeed;
            }

            void
            setParticleSize( float particleSize )
            {
                if( _particleSize == particleSize )
                {
                    return;
                }
                _particleSize = particleSize;
                _dirty        = true;
            }

            float
            getParticleSize() const
            {
                return _particleSize;
            }

            void
            setParticleColor( const osg::vec4& color )
            {
                if( _particleColor == color )
                {
                    return;
                }
                _particleColor = color;
                _dirty         = true;
            }

            const osg::vec4&
            getParticleColor() const
            {
                return _particleColor;
            }

            void
            setNearTransition( float nearTransition )
            {
                _nearTransition = nearTransition;
            }

            float
            getNearTransition() const
            {
                return _nearTransition;
            }

            void
            setFarTransition( float farTransition )
            {
                _farTransition = farTransition;
            }

            float
            getFarTransition() const
            {
                return _farTransition;
            }

            void
            setUseFarLineSegments( bool useFarLineSegments )
            {
                _useFarLineSegments = useFarLineSegments;
            }

            bool
            getUseFarLineSegments() const
            {
                return _useFarLineSegments;
            }

            enum FogMode
            {
                FOG_LINEAR = 0X26'01,    // GL_LINEAR
                FOG_EXP    = 0X08'00,
                FOG_EXP2   = 0X08'01
            };

            void
            setFogDensity( float density )
            {
                _fogDensity = density;
            }

            float
            getFogDensity() const
            {
                return _fogDensity;
            }

            void
            setFogColor( const osg::vec4& color )
            {
                _fogColor = color;
            }

            const osg::vec4&
            getFogColor() const
            {
                return _fogColor;
            }

            void
            setFogMode( FogMode mode )
            {
                _fogMode = mode;
            }

            FogMode
            getFogMode() const
            {
                return _fogMode;
            }

            void
            setFogStart( float start )
            {
                _fogStart = start;
            }

            float
            getFogStart() const
            {
                return _fogStart;
            }

            void
            setFogEnd( float end )
            {
                _fogEnd = end;
            }

            float
            getFogEnd() const
            {
                return _fogEnd;
            }

            osg::Geometry*
            getQuadGeometry()
            {
                return _quadGeometry.get();
            }

            osg::StateSet*
            getQuadStateSet()
            {
                return _quadStateSet.get();
            }

            osg::Geometry*
            getLineGeometry()
            {
                return _lineGeometry.get();
            }

            osg::StateSet*
            getLineStateSet()
            {
                return _lineStateSet.get();
            }

            osg::Geometry*
            getPointGeometry()
            {
                return _pointGeometry.get();
            }

            osg::StateSet*
            getPointStateSet()
            {
                return _pointStateSet.get();
            }

            /** Internal drawable used to render batches of cells.*/
            class OSGPARTICLE_EXPORT PrecipitationDrawable
                : public osg::Inherit<osg::Drawable, PrecipitationDrawable>
            {
                public:

                    PrecipitationDrawable();
                    PrecipitationDrawable( const PrecipitationDrawable& copy,
                                           const osg::CopyOp&           copyop =
                                               osg::CopyOp::SHALLOW_COPY );

                    OSG_REGISTER_TYPE( osgParticle,
                                       PrecipitationDrawable )

                    using osg::Drawable::accept;

                    virtual bool
                    supports( const osg::PrimitiveFunctor& ) const
                    {
                        return false;
                    }

                    virtual void
                    accept( osg::PrimitiveFunctor& ) const
                    {
                    }

                    virtual bool
                    supports( const osg::PrimitiveIndexFunctor& ) const
                    {
                        return false;
                    }

                    virtual void
                    accept( osg::PrimitiveIndexFunctor& ) const
                    {
                    }

                    void
                    setRequiresPreviousMatrix( bool flag )
                    {
                        _requiresPreviousMatrix = flag;
                    }

                    bool
                    getRequiresPreviousMatrix() const
                    {
                        return _requiresPreviousMatrix;
                    }

                    void
                    setGeometry( osg::Geometry* geom )
                    {
                        _geometry = geom;
                    }

                    osg::Geometry*
                    getGeometry()
                    {
                        return _geometry.get();
                    }

                    const osg::Geometry*
                    getGeometry() const
                    {
                        return _geometry.get();
                    }

                    void
                    setDrawType( GLenum type )
                    {
                        _drawType = type;
                    }

                    GLenum
                    getDrawType() const
                    {
                        return _drawType;
                    }

                    void
                    setNumberOfVertices( unsigned int numVertices )
                    {
                        _numberOfVertices = numVertices;
                    }

                    unsigned int
                    getNumberOfVertices() const
                    {
                        return _numberOfVertices;
                    }

                    virtual void
                    resizeGLObjectBuffers( unsigned int maxSize );
                    virtual void
                    releaseGLObjects( osg::State* state ) const;

                    virtual void
                    drawImplementation( osg::RenderInfo& renderInfo ) const;

                    struct Cell
                    {
                            Cell( int in_i,
                                  int in_j,
                                  int in_k ) :
                                i( in_i ),
                                j( in_j ),
                                k( in_k )
                            {
                            }

                            inline bool
                            operator<( const Cell& rhs ) const
                            {
                                if( i < rhs.i )
                                {
                                    return true;
                                }
                                if( i > rhs.i )
                                {
                                    return false;
                                }
                                if( j < rhs.j )
                                {
                                    return true;
                                }
                                if( j > rhs.j )
                                {
                                    return false;
                                }
                                if( k < rhs.k )
                                {
                                    return true;
                                }
                                if( k > rhs.k )
                                {
                                    return false;
                                }
                                return false;
                            }

                            int i;
                            int j;
                            int k;
                    };

                    struct DepthMatrixStartTime
                    {
                            inline bool
                            operator<( const DepthMatrixStartTime& rhs ) const
                            {
                                return depth < rhs.depth;
                            }

                            float      depth;
                            float      startTime;
                            osg::dmat4 modelview;
                    };

                    typedef std::map<Cell, DepthMatrixStartTime> CellMatrixMap;

                    struct LessFunctor
                    {
                            inline bool
                            operator()( const CellMatrixMap::value_type* lhs,
                                        const CellMatrixMap::value_type* rhs ) const
                            {
                                return ( *lhs ).second < ( *rhs ).second;
                            }
                    };

                    CellMatrixMap&
                    getCurrentCellMatrixMap()
                    {
                        return _currentCellMatrixMap;
                    }

                    CellMatrixMap&
                    getPreviousCellMatrixMap()
                    {
                        return _previousCellMatrixMap;
                    }

                    inline void
                    newFrame()
                    {
                        _previousCellMatrixMap.swap( _currentCellMatrixMap );
                        _currentCellMatrixMap.clear();
                    }

                protected:

                    virtual ~PrecipitationDrawable();

                    bool                        _requiresPreviousMatrix;

                    osg::ref_ptr<osg::Geometry> _geometry;

                    mutable CellMatrixMap       _currentCellMatrixMap;
                    mutable CellMatrixMap       _previousCellMatrixMap;

                    GLenum                      _drawType;
                    unsigned int                _numberOfVertices;
            };

        protected:

            virtual ~PrecipitationEffect()
            {
            }

            void
            compileGLObjects( osg::RenderInfo& renderInfo ) const;

            void
            update();

            void
            createGeometry( unsigned int   numParticles,
                            osg::Geometry* quad_geometry,
                            osg::Geometry* line_geometry,
                            osg::Geometry* point_geometry );

            void
            setUpGeometries( unsigned int numParticles );

            struct PrecipitationDrawableSet
            {
                    osg::ref_ptr<PrecipitationDrawable> _quadPrecipitationDrawable;
                    osg::ref_ptr<PrecipitationDrawable> _linePrecipitationDrawable;
                    osg::ref_ptr<PrecipitationDrawable> _pointPrecipitationDrawable;
            };

            void
            cull( PrecipitationDrawableSet& pds,
                  osgUtil::CullVisitor*     cv ) const;
            bool
                                       build( const osg::vec3           eyeLocal,
                                              int                       i,
                                              int                       j,
                                              int                       k,
                                              float                     startTime,
                                              PrecipitationDrawableSet& pds,
                                              osg::Polytope&            frustum,
                                              osgUtil::CullVisitor*     cv ) const;

            // parameters
            bool                       _dirty;
            osg::vec3                  _wind;
            float                      _particleSpeed;
            float                      _particleSize;
            osg::vec4                  _particleColor;
            float                      _maximumParticleDensity;
            osg::vec3                  _cellSize;
            float                      _nearTransition;
            float                      _farTransition;
            bool                       _useFarLineSegments;
            FogMode                    _fogMode;
            float                      _fogDensity;
            osg::vec4                  _fogColor;
            float                      _fogStart;
            float                      _fogEnd;

            osg::ref_ptr<osg::Uniform> _inversePeriodUniform;
            osg::ref_ptr<osg::Uniform> _particleSizeUniform;
            osg::ref_ptr<osg::Uniform> _particleColorUniform;

            typedef std::pair<osg::NodeVisitor*, osg::NodePath>        ViewIdentifier;
            typedef std::map<ViewIdentifier, PrecipitationDrawableSet> ViewDrawableMap;

            std::mutex                                                 _mutex;
            ViewDrawableMap                                            _viewDrawableMap;

            osg::ref_ptr<osg::Geometry>                                _quadGeometry;
            osg::ref_ptr<osg::StateSet>                                _quadStateSet;

            osg::ref_ptr<osg::Geometry>                                _lineGeometry;
            osg::ref_ptr<osg::StateSet>                                _lineStateSet;

            osg::ref_ptr<osg::Geometry>                                _pointGeometry;
            osg::ref_ptr<osg::StateSet>                                _pointStateSet;

            // cache variables.
            float                                                      _period;
            osg::vec3                                                  _origin;
            osg::vec3                                                  _du;
            osg::vec3                                                  _dv;
            osg::vec3                                                  _dw;
            osg::vec3                                                  _inverse_du;
            osg::vec3                                                  _inverse_dv;
            osg::vec3                                                  _inverse_dw;

            double _previousFrameTime;
    };

}
