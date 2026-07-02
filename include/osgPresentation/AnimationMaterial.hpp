/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Keyframed material animation for presentation effects.
 * Animates color, transparency, and material properties over time.
 */
#pragma once

#include <float.h>
#include <iosfwd>
#include <map>
#include <osg/core/Callback.hpp>
#include <osg/core/Inherit.hpp>
#include <osg/state/Material.hpp>
#include <osgPresentation/Export.hpp>

namespace osgPresentation
{

    /** AnimationMaterial for specify the time varying transformation pathway to use when
     * update camera and model objects. Subclassed from
     * Transform::ComputeTransformCallback allows AnimationMaterial to be attached
     * directly to Transform nodes to move subgraphs around the scene.
     */
    class OSGPRESENTATION_EXPORT AnimationMaterial
        : public osg::Inherit<osg::Object, AnimationMaterial>
    {
        public:

            AnimationMaterial() :
                _loopMode( LOOP )
            {
            }

            AnimationMaterial( const AnimationMaterial& ap,
                               const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY ) :
                Inherit( ap,
                         copyop ),
                _timeControlPointMap( ap._timeControlPointMap ),
                _loopMode( ap._loopMode )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               AnimationMaterial )

            /** get the transformation matrix for a point in time.*/
            bool
            getMaterial( double         time,
                         osg::Material& material ) const;

            void
            insert( double         time,
                    osg::Material* material );

            double
            getFirstTime() const
            {
                if( !_timeControlPointMap.empty() )
                {
                    return _timeControlPointMap.begin()->first;
                }
                else
                {
                    return 0.0;
                }
            }

            double
            getLastTime() const
            {
                if( !_timeControlPointMap.empty() )
                {
                    return _timeControlPointMap.rbegin()->first;
                }
                else
                {
                    return 0.0;
                }
            }

            double
            getPeriod() const
            {
                return getLastTime() - getFirstTime();
            }

            enum LoopMode
            {
                SWING,
                LOOP,
                NO_LOOPING
            };

            void
            setLoopMode( LoopMode lm )
            {
                _loopMode = lm;
            }

            LoopMode
            getLoopMode() const
            {
                return _loopMode;
            }

            typedef std::map<double, osg::ref_ptr<osg::Material>> TimeControlPointMap;

            TimeControlPointMap&
            getTimeControlPointMap()
            {
                return _timeControlPointMap;
            }

            const TimeControlPointMap&
            getTimeControlPointMap() const
            {
                return _timeControlPointMap;
            }

            /** read the anumation path from a flat ascii file stream.*/
            void
            read( std::istream& in );

            /** write the anumation path to a flat ascii file stream.*/
            void
            write( std::ostream& out ) const;

            bool
            requiresBlending() const;

        protected:

            virtual ~AnimationMaterial()
            {
            }

            void
                                interpolate( osg::Material&       material,
                                             float                r,
                                             const osg::Material& lhs,
                                             const osg::Material& rhs ) const;

            TimeControlPointMap _timeControlPointMap;
            LoopMode            _loopMode;
    };

    class OSGPRESENTATION_EXPORT AnimationMaterialCallback
        : public osg::Inherit<osg::NodeCallback, AnimationMaterialCallback>
    {
        public:

            AnimationMaterialCallback() :
                _useInverseMatrix( false ),
                _timeOffset( 0.0 ),
                _timeMultiplier( 1.0 ),
                _firstTime( DBL_MAX ),
                _latestTime( 0.0 ),
                _pause( false ),
                _pauseTime( 0.0 )
            {
            }

            AnimationMaterialCallback( const AnimationMaterialCallback& apc,
                                       const osg::CopyOp&               copyop ) :
                Inherit( apc,
                         copyop ),
                _animationMaterial( apc._animationMaterial ),
                _useInverseMatrix( apc._useInverseMatrix ),
                _timeOffset( apc._timeOffset ),
                _timeMultiplier( apc._timeMultiplier ),
                _firstTime( apc._firstTime ),
                _latestTime( apc._latestTime ),
                _pause( apc._pause ),
                _pauseTime( apc._pauseTime )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               AnimationMaterialCallback )

            AnimationMaterialCallback( AnimationMaterial* ap,
                                       double             timeOffset     = 0.0F,
                                       double             timeMultiplier = 1.0F ) :
                _animationMaterial( ap ),
                _useInverseMatrix( false ),
                _timeOffset( timeOffset ),
                _timeMultiplier( timeMultiplier ),
                _firstTime( DBL_MAX ),
                _latestTime( 0.0 ),
                _pause( false ),
                _pauseTime( 0.0 )
            {
            }

            void
            setAnimationMaterial( AnimationMaterial* path )
            {
                _animationMaterial = path;
            }

            AnimationMaterial*
            getAnimationMaterial()
            {
                return _animationMaterial.get();
            }

            const AnimationMaterial*
            getAnimationMaterial() const
            {
                return _animationMaterial.get();
            }

            void
            setTimeOffset( double offset )
            {
                _timeOffset = offset;
            }

            double
            getTimeOffset() const
            {
                return _timeOffset;
            }

            void
            setTimeMultiplier( double multiplier )
            {
                _timeMultiplier = multiplier;
            }

            double
            getTimeMultiplier() const
            {
                return _timeMultiplier;
            }

            void
            reset();

            void
            setPause( bool pause );

            /** get the animation time that is used to specify the position along the
             * AnimationMaterial. Animation time is computed from the formula
             * ((_latestTime-_firstTime)-_timeOffset)*_timeMultiplier.*/
            double
            getAnimationTime() const;

            /** implements the callback*/
            virtual void
            operator()( osg::Node*        node,
                        osg::NodeVisitor* nv );

            void
            update( osg::Node& node );

        public:

            osg::ref_ptr<AnimationMaterial> _animationMaterial;
            bool                            _useInverseMatrix;
            double                          _timeOffset;
            double                          _timeMultiplier;
            double                          _firstTime;
            double                          _latestTime;
            bool                            _pause;
            double                          _pauseTime;

        protected:

            ~AnimationMaterialCallback()
            {
            }
    };

}
