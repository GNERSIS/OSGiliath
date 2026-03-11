/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Polygon depth offset to prevent z-fighting. Applies factor
 * and units bias for decals, outlines, and co-planar geometry.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/state/StateAttribute.hpp>

namespace osg
{

    /** PolygonOffset - encapsulates the OpenGL glPolygonOffset state.*/
    class OSG_EXPORT PolygonOffset : public osg::Inherit<StateAttribute, PolygonOffset>
    {
        public:

            PolygonOffset();

            PolygonOffset( float factor,
                           float units );

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            PolygonOffset( const PolygonOffset& po,
                           const CopyOp&        copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( po,
                         copyop ),
                _factor( po._factor ),
                _units( po._units )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               PolygonOffset )

            Type
            getType() const override
            {
                return Type::POLYGONOFFSET;
            }

            /** return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs.*/
            int
            compare( const StateAttribute& sa ) const override
            {
                // check the types are equal and then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( PolygonOffset, sa )

                    // compare each parameter in turn against the rhs.
                    COMPARE_StateAttribute_Parameter( _factor )
                        COMPARE_StateAttribute_Parameter(
                            _units
                        ) return 0;    // passed all the above comparison macros, must be
                                       // equal.
            }

            bool
            getModeUsage( StateAttribute::ModeUsage& usage ) const override
            {
                usage.usesMode( GL_POLYGON_OFFSET_FILL );
                usage.usesMode( GL_POLYGON_OFFSET_LINE );
                usage.usesMode( GL_POLYGON_OFFSET_POINT );
                return true;
            }

            inline void
            setFactor( float factor )
            {
                _factor = factor;
            }

            inline float
            getFactor() const
            {
                return _factor;
            }

            inline void
            setUnits( float units )
            {
                _units = units;
            }

            inline float
            getUnits() const
            {
                return _units;
            }

            void
            apply( State& state ) const override;

            static void
            setFactorMultiplier( float multiplier );
            static float
            getFactorMultiplier();

            static void
            setUnitsMultiplier( float multiplier );
            static float
            getUnitsMultiplier();

            static bool
            areFactorAndUnitsMultipliersSet();

            /** Checks with the OpenGL driver to try and pick multiplier appropriate for
               the hardware. note, requires a valid graphics context to be current. */
            static void
            setFactorAndUnitsMultipliersUsingBestGuessForDriver();

        protected:

            virtual ~PolygonOffset();

            float _factor;
            float _units;
    };

}
