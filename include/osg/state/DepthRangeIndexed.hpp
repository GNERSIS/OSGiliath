/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Per-viewport depth range attribute. Sets independent near/far
 * depth ranges for indexed viewports in multi-viewport rendering.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/state/Depth.hpp>

namespace osg
{

    /** Encapsulates glDepthRangeIndexed function : the index version of glDepth
     */
    class OSG_EXPORT DepthRangeIndexed
        : public osg::Inherit<osg::StateAttribute, DepthRangeIndexed>
    {
        public:

            DepthRangeIndexed();

            DepthRangeIndexed( unsigned int index,
                               double       zNear = 0.0,
                               double       zFar  = 1.0 ) :
                _index( index ),
                _zNear( zNear ),
                _zFar( zFar )
            {
            }

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            DepthRangeIndexed( const DepthRangeIndexed& dp,
                               const CopyOp&            copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( dp,
                         copyop ),
                _index( dp._index ),
                _zNear( dp._zNear ),
                _zFar( dp._zFar )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               DepthRangeIndexed )

            Type
            getType() const override
            {
                return Type::DEPTHRANGEINDEXED;
            }

            /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            int
            compare( const StateAttribute& sa ) const override
            {
                // Check for equal types, then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( DepthRangeIndexed, sa )

                    COMPARE_StateAttribute_Parameter( _index );
                COMPARE_StateAttribute_Parameter( _zNear )
                    COMPARE_StateAttribute_Parameter( _zFar ) return 0;
            }

            /** Return the buffer index as the member identifier.*/
            unsigned int
            getMember() const override
            {
                return _index;
            }

            /** Set the index of the DepthRangeIndexed. */
            void
            setIndex( unsigned int index );

            /** Get the index of the DepthRangeIndexed. */
            unsigned int
            getIndex() const
            {
                return _index;
            }

            inline void
            setZNear( double zNear )
            {
                _zNear = zNear;
            }

            inline double
            getZNear() const
            {
                return _zNear;
            }

            inline void
            setZFar( double zFar )
            {
                _zFar = zFar;
            }

            inline double
            getZFar() const
            {
                return _zFar;
            }

            void
            apply( State& state ) const override;

        protected:

            virtual ~DepthRangeIndexed();

            unsigned int _index;
            double       _zNear;
            double       _zFar;
    };

}
