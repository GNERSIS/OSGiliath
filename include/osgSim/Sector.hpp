/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Directional visibility sector for light points. Defines
 * azimuth and elevation angle ranges for light visibility.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/maths/Math.hpp>
#include <osg/maths/quat.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>
#include <osgSim/Export.hpp>

namespace osgSim
{

    class Sector : public osg::Object
    {
        public:

            Sector()
            {
            }

            Sector( const Sector&      copy,
                    const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY ) :
                osg::Object( copy,
                             copyop )
            {
            }

            virtual const char*
            libraryName() const
            {
                return "osgSim";
            }

            virtual const char*
            className() const
            {
                return "Sector";
            }

            virtual bool
            isSameKindAs( const osg::Object* obj ) const
            {
                return dynamic_cast<const Sector*>( obj ) != 0;
            }

            virtual float
            operator()( const osg::vec3& /*eyeLocal*/ ) const = 0;

        protected:

            virtual ~Sector()
            {
            }
    };

    class OSGSIM_EXPORT AzimRange
    {
        public:

            AzimRange() :
                _cosAzim( 1.0F ),
                _sinAzim( 0.0F ),
                _cosAngle( -1.0F ),
                _cosFadeAngle( -1.0F )
            {
            }

            void
            setAzimuthRange( float minAzimuth,
                             float maxAzimuth,
                             float fadeAngle = 0.0F );
            void
            getAzimuthRange( float& minAzimuth,
                             float& maxAzimuth,
                             float& fadeAngle ) const;

            inline float
            azimSector( const osg::vec3& eyeLocal ) const
            {
                float dotproduct = eyeLocal.x * _sinAzim + eyeLocal.y * _cosAzim;
                float length =
                    sqrtf( osg::square( eyeLocal.x ) + osg::square( eyeLocal.y ) );
                if( dotproduct < _cosFadeAngle * length )
                {
                    return 0.0F;    // out of sector.
                }
                if( dotproduct >= _cosAngle * length )
                {
                    return 1.0F;    // fully in sector.
                }
                return ( dotproduct - _cosFadeAngle * length ) /
                       ( ( _cosAngle - _cosFadeAngle ) * length );
            }

        protected:

            float _cosAzim;
            float _sinAzim;
            float _cosAngle;
            float _cosFadeAngle;
    };

    class OSGSIM_EXPORT ElevationRange
    {
        public:

            ElevationRange() :
                _cosMinElevation( -1.0F ),
                _cosMinFadeElevation( -1.0F ),
                _cosMaxElevation( 1.0 ),
                _cosMaxFadeElevation( 1.0 )
            {
            }

            void
            setElevationRange( float minElevation,
                               float maxElevation,
                               float fadeAngle = 0.0F );

            float
            getMinElevation() const;

            float
            getMaxElevation() const;

            float
            getFadeAngle() const;

            inline float
            elevationSector( const osg::vec3& eyeLocal ) const
            {
                float dotproduct = eyeLocal.z;    // against z axis - eyeLocal*(0,0,1).
                float length     = osg::length( eyeLocal );
                if( dotproduct > _cosMaxFadeElevation * length )
                {
                    return 0.0F;    // out of sector
                }
                if( dotproduct < _cosMinFadeElevation * length )
                {
                    return 0.0F;    // out of sector
                }
                if( dotproduct > _cosMaxElevation * length )
                {
                    // in uppoer fade band.
                    return ( dotproduct - _cosMaxFadeElevation * length ) /
                           ( ( _cosMaxElevation - _cosMaxFadeElevation ) * length );
                }
                if( dotproduct < _cosMinElevation * length )
                {
                    // in lower fade band.
                    return ( dotproduct - _cosMinFadeElevation * length ) /
                           ( ( _cosMinElevation - _cosMinFadeElevation ) * length );
                }
                return 1.0F;    // fully in sector
            }

        protected:

            float _cosMinElevation;
            float _cosMinFadeElevation;
            float _cosMaxElevation;
            float _cosMaxFadeElevation;
    };

    class OSGSIM_EXPORT AzimSector : public osg::Inherit<Sector, AzimSector>,
                                     public AzimRange
    {
        public:

            AzimSector() :
                AzimRange()
            {
            }

            AzimSector( const AzimSector&  copy,
                        const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY ) :
                Inherit( copy,
                         copyop ),
                AzimRange( copy )
            {
            }

            AzimSector( float minAzimuth,
                        float maxAzimuth,
                        float fadeAngle = 0.0F );

            OSG_REGISTER_TYPE( osgSim,
                               AzimSector )

            virtual float
            operator()( const osg::vec3& eyeLocal ) const;

        protected:

            virtual ~AzimSector()
            {
            }
    };

    class OSGSIM_EXPORT ElevationSector : public osg::Inherit<Sector, ElevationSector>,
                                          public ElevationRange
    {
        public:

            ElevationSector() :
                ElevationRange()
            {
            }

            ElevationSector( const ElevationSector& copy,
                             const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY ) :
                Inherit( copy,
                         copyop ),
                ElevationRange( copy )
            {
            }

            ElevationSector( float minElevation,
                             float maxElevation,
                             float fadeAngle = 0.0F );

            OSG_REGISTER_TYPE( osgSim,
                               ElevationSector )

            virtual float
            operator()( const osg::vec3& eyeLocal ) const;

        protected:

            virtual ~ElevationSector()
            {
            }
    };

    class OSGSIM_EXPORT AzimElevationSector
        : public osg::Inherit<Sector, AzimElevationSector>,
          public AzimRange,
          public ElevationRange
    {
        public:

            AzimElevationSector() :
                AzimRange(),
                ElevationRange()
            {
            }

            AzimElevationSector( const AzimElevationSector& copy,
                                 const osg::CopyOp&         copyop =
                                     osg::CopyOp::SHALLOW_COPY ) :
                Inherit( copy,
                         copyop ),
                AzimRange( copy ),
                ElevationRange( copy )
            {
            }

            AzimElevationSector( float minAzimuth,
                                 float maxAzimuth,
                                 float minElevation,
                                 float maxElevation,
                                 float fadeAngle = 0.0F );

            OSG_REGISTER_TYPE( osgSim,
                               AzimElevationSector )

            virtual float
            operator()( const osg::vec3& eyeLocal ) const;

        protected:

            virtual ~AzimElevationSector()
            {
            }
    };

    class OSGSIM_EXPORT ConeSector : public osg::Inherit<Sector, ConeSector>
    {
        public:

            ConeSector() :
                _axis( 0.0F,
                       0.0F,
                       1.0F ),
                _cosAngle( -1.0F ),
                _cosAngleFade( -1.0F )
            {
            }

            ConeSector( const ConeSector&  copy,
                        const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY ) :
                Inherit( copy,
                         copyop ),
                _axis( copy._axis ),
                _cosAngle( copy._cosAngle ),
                _cosAngleFade( copy._cosAngleFade )
            {
            }

            ConeSector( const osg::vec3& axis,
                        float            angle,
                        float            fadeangle = 0.0F );

            OSG_REGISTER_TYPE( osgSim,
                               ConeSector )

            void
            setAxis( const osg::vec3& axis );

            const osg::vec3&
            getAxis() const;

            void
            setAngle( float angle,
                      float fadeangle = 0.0F );

            float
            getAngle() const;

            float
            getFadeAngle() const;

            virtual float
            operator()( const osg::vec3& eyeLocal ) const;

        protected:

            virtual ~ConeSector()
            {
            }

            osg::vec3 _axis;
            float     _cosAngle;
            float     _cosAngleFade;
    };

    /* The DirectionalSector class was created to better handle OpenFlight directional
      lightpoints.  The Elevation and Azimuth Sectors above impose invalid limits on
      the elevation range which cause lightpoints whose direction vectors are not
      on the XY plane to be displayed incorrectly.  Corbin Holtz 4/04 */

    class OSGSIM_EXPORT DirectionalSector
        : public osg::Inherit<Sector, DirectionalSector>
    {
        public:

            DirectionalSector() :
                _direction( 0.0F,
                            0.0F,
                            1.0F ),
                _rollAngle( 0.0F ),
                _cosHorizAngle( -1.0F ),
                _cosVertAngle( -1.0F ),
                _cosHorizFadeAngle( -1.0F ),
                _cosVertFadeAngle( -1.0F )
            {
                computeMatrix();
            }

            DirectionalSector( const DirectionalSector& copy,
                               const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY ) :
                Inherit( copy,
                         copyop ),
                _direction( copy._direction ),
                _rollAngle( copy._rollAngle ),
                _local_to_LP( copy._local_to_LP ),
                _cosHorizAngle( copy._cosHorizAngle ),
                _cosVertAngle( copy._cosVertAngle ),
                _cosHorizFadeAngle( copy._cosHorizFadeAngle ),
                _cosVertFadeAngle( copy._cosVertFadeAngle )
            {
            }

            DirectionalSector( const osg::vec3& direction,
                               float            horizLobeAngle,
                               float            vertLobeAngle,
                               float            lobeRollAngle,
                               float            fadeAngle = 0.0F );

            OSG_REGISTER_TYPE( osgSim,
                               DirectionalSector )

            void
            setDirection( const osg::vec3& direction );

            const osg::vec3&
            getDirection() const;

            void
            setHorizLobeAngle( float angle );

            float
            getHorizLobeAngle() const;

            void
            setLobeRollAngle( float angle );

            float
            getLobeRollAngle() const;

            void
            setVertLobeAngle( float angle );

            float
            getVertLobeAngle() const;

            void
            setFadeAngle( float angle );

            float
            getFadeAngle() const;

            virtual float
            operator()( const osg::vec3& eyeLocal ) const;

            void
            computeMatrix();

        protected:

            virtual ~DirectionalSector()
            {
            }

            osg::vec3  _direction;
            float      _rollAngle;
            osg::dmat4 _local_to_LP;
            float      _cosHorizAngle;
            float      _cosVertAngle;
            float      _cosHorizFadeAngle;
            float      _cosVertFadeAngle;
    };

}
