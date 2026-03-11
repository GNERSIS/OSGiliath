/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Mesh simplification visitor. Reduces polygon count using
 * edge collapse with configurable error metrics.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osgUtil/Export>

namespace osgUtil
{

    /** A simplifier for reducing the number of traingles in osg::Geometry.
     */
    class OSGUTIL_EXPORT Simplifier : public osg::DualModeVisitor
    {
        public:

            using DualModeVisitor::apply;

            Simplifier( double sampleRatio   = 1.0,
                        double maximumError  = FLT_MAX,
                        double maximumLength = 0.0 );

            OSG_REGISTER_TYPE( osgUtil,
                               Simplifier )

            void
            setSampleRatio( float sampleRatio )
            {
                _sampleRatio = sampleRatio;
            }

            float
            getSampleRatio() const
            {
                return static_cast<float>( _sampleRatio );
            }

            /** Set the maximum point error that all point removals must be less than to
             * permit removal of a point. Note, Only used when down sampling. i.e.
             * sampleRatio < 1.0*/
            void
            setMaximumError( float error )
            {
                _maximumError = error;
            }

            float
            getMaximumError() const
            {
                return static_cast<float>( _maximumError );
            }

            /** Set the maximum length target that all edges must be shorted than.
             * Note, Only used when up sampling i.e. sampleRatio > 1.0.*/
            void
            setMaximumLength( float length )
            {
                _maximumLength = length;
            }

            float
            getMaximumLength() const
            {
                return static_cast<float>( _maximumLength );
            }

            void
            setDoTriStrip( bool on )
            {
                _triStrip = on;
            }

            bool
            getDoTriStrip() const
            {
                return _triStrip;
            }

            void
            setSmoothing( bool on )
            {
                _smoothing = on;
            }

            bool
            getSmoothing() const
            {
                return _smoothing;
            }

            class ContinueSimplificationCallback : public osg::Referenced
            {
                public:

                    /** return true if mesh should be continued to be simplified, return
                     * false to stop simplification.*/
                    virtual bool
                    continueSimplification( const Simplifier& simplifier,
                                            float             nextError,
                                            unsigned int      numOriginalPrimitives,
                                            unsigned int numRemainingPrimitives ) const
                    {
                        return simplifier.continueSimplificationImplementation(
                            nextError,
                            numOriginalPrimitives,
                            numRemainingPrimitives
                        );
                    }

                    virtual bool
                    requiresDownSampling( const Simplifier& simplifier ) const
                    {
                        return simplifier.requiresDownSamplingImplementation();
                    }

                protected:

                    virtual ~ContinueSimplificationCallback()
                    {
                    }
            };

            void
            setContinueSimplificationCallback( ContinueSimplificationCallback* cb )
            {
                _continueSimplificationCallback = cb;
            }

            ContinueSimplificationCallback*
            getContinueSimplificationCallback()
            {
                return _continueSimplificationCallback.get();
            }

            const ContinueSimplificationCallback*
            getContinueSimplificationCallback() const
            {
                return _continueSimplificationCallback.get();
            }

            bool
            continueSimplification( float        nextError,
                                    unsigned int numOriginalPrimitives,
                                    unsigned int numRemainingPrimitives ) const
            {
                if( _continueSimplificationCallback.valid() )
                {
                    return _continueSimplificationCallback->continueSimplification(
                        *this,
                        nextError,
                        numOriginalPrimitives,
                        numRemainingPrimitives
                    );
                }
                else
                {
                    return continueSimplificationImplementation(
                        nextError,
                        numOriginalPrimitives,
                        numRemainingPrimitives
                    );
                }
            }

            virtual bool
            continueSimplificationImplementation(
                float        nextError,
                unsigned int numOriginalPrimitives,
                unsigned int numRemainingPrimitives
            ) const
            {
                if( getSampleRatio() < 1.0 )
                {
                    return ( ( float )numRemainingPrimitives >
                             ( ( float )numOriginalPrimitives ) *
                             getSampleRatio() ) &&
                           nextError <= getMaximumError();
                }
                else
                {
                    return ( ( float )numRemainingPrimitives <
                             ( ( float )numOriginalPrimitives ) *
                             getSampleRatio() ) &&
                           nextError > getMaximumLength();
                }
            }

            bool
            requiresDownSampling() const
            {
                if( _continueSimplificationCallback.valid() )
                {
                    return _continueSimplificationCallback->requiresDownSampling(
                        *this
                    );
                }
                else
                {
                    return requiresDownSamplingImplementation();
                }
            }

            virtual bool
            requiresDownSamplingImplementation() const
            {
                return getSampleRatio() < 1.0;
            }

            virtual void
            apply( osg::Geometry& geom )
            {
                simplify( geom );
            }

            /** simply the geometry.*/
            void
                                              simplify( osg::Geometry& geometry );

            typedef std::vector<unsigned int> IndexList;    /// a list of point indices

            /** simply the geometry, whilst protecting key points from being modified.*/
            void
            simplify( osg::Geometry&   geometry,
                      const IndexList& protectedPoints );

        protected:

            double                                       _sampleRatio;
            double                                       _maximumError;
            double                                       _maximumLength;
            bool                                         _triStrip;
            bool                                         _smoothing;

            osg::ref_ptr<ContinueSimplificationCallback> _continueSimplificationCallback;
    };

}
