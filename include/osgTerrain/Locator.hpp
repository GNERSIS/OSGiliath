/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Geographic coordinate locator. Transforms between local tile
 * coordinates and geographic (lat/lon) coordinates.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/nodes/CoordinateSystemNode.hpp>
#include <osgTerrain/Export.hpp>

namespace osgTerrain
{

    class OSGTERRAIN_EXPORT Locator : public osg::Inherit<osg::Object, Locator>
    {
        public:

            Locator();

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            Locator( const Locator&,
                     const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgTerrain,
                               Locator )

            /** CoordinateSystemType provides the classification of the type coordinate
             * system represented.*/
            enum CoordinateSystemType
            {
                /** GEOCENTRIC coordinate systems are ones mapped to the around the
                 * ellipsoid, i.e. whole earth.*/
                GEOCENTRIC,

                /** GEOGRAPHIC coordinate systems are ones mapped to latitude and
                 * longitude.*/
                GEOGRAPHIC,

                /** PROJECTED coordinate systems are ones projected to a local projected
                 * coordinate system i.e. UTMs.*/
                PROJECTED
            };

            /** Set the CoordinatesSyetemType.
             * Note, the user must keep the CoordinateSystemString consistent with the
             * type of the CoordinateSystem.*/
            void
            setCoordinateSystemType( CoordinateSystemType type )
            {
                _coordinateSystemType = type;
            }

            /** Get the CoordinatesSyetemType.*/
            CoordinateSystemType
            getCoordinateSystemType() const
            {
                return _coordinateSystemType;
            }

            /** Set the coordinate system format string. Typical values would be WKT,
             * PROJ4, USGS etc.*/
            void
            setFormat( const std::string& format )
            {
                _format = format;
            }

            /** Get the coordinate system format string.*/
            const std::string&
            getFormat() const
            {
                return _format;
            }

            /** Set the CoordinateSystem reference string, should be stored in a form
             * consistent with the Format.*/
            void
            setCoordinateSystem( const std::string& cs )
            {
                _cs = cs;
            }

            /** Get the CoordinateSystem reference string.*/
            const std::string&
            getCoordinateSystem() const
            {
                return _cs;
            }

            /** Set EllipsoidModel to describe the model used to map lat, long and height
             * into geocentric XYZ and back. */
            void
            setEllipsoidModel( osg::EllipsoidModel* ellipsode )
            {
                _ellipsoidModel = ellipsode;
            }

            /** Get the EllipsoidModel.*/
            osg::EllipsoidModel*
            getEllipsoidModel()
            {
                return _ellipsoidModel.get();
            }

            /** Get the const EllipsoidModel.*/
            const osg::EllipsoidModel*
            getEllipsoidModel() const
            {
                return _ellipsoidModel.get();
            }

            /** Set the transformation from local coordinates to model coordinates.*/
            void
            setTransform( const osg::dmat4& transform )
            {
                _transform = transform;
                _inverse   = osg::inverse( _transform );
            }

            /** Set the transformation from local coordinates to model coordinates.*/
            const osg::dmat4&
            getTransform() const
            {
                return _transform;
            }

            /** Set the extents of the local coords.*/
            void
            setTransformAsExtents( double minX,
                                   double minY,
                                   double maxX,
                                   double maxY );

            virtual bool
            orientationOpenGL() const;

            virtual bool
            convertLocalToModel( const osg::dvec3& local,
                                 osg::dvec3&       world ) const;

            virtual bool
            convertModelToLocal( const osg::dvec3& world,
                                 osg::dvec3&       local ) const;

            static bool
            convertLocalCoordBetween( const Locator&    source,
                                      const osg::dvec3& sourceNDC,
                                      const Locator&    destination,
                                      osg::dvec3&       destinationNDC )
            {
                osg::dvec3 model;
                if( !source.convertLocalToModel( sourceNDC, model ) )
                {
                    return false;
                }
                if( !destination.convertModelToLocal( model, destinationNDC ) )
                {
                    return false;
                }
                return true;
            }

            bool
            computeLocalBounds( Locator&    source,
                                osg::dvec3& bottomLeft,
                                osg::dvec3& topRight ) const;

            void
            setDefinedInFile( bool flag )
            {
                _definedInFile = flag;
            }

            bool
            getDefinedInFile() const
            {
                return _definedInFile;
            }

            void
            setTransformScaledByResolution( bool scaledByResolution )
            {
                _transformScaledByResolution = scaledByResolution;
            }

            bool
            getTransformScaledByResolution() const
            {
                return _transformScaledByResolution;
            }

        protected:

            virtual ~Locator();

            CoordinateSystemType              _coordinateSystemType;

            std::string                       _format;
            std::string                       _cs;
            osg::ref_ptr<osg::EllipsoidModel> _ellipsoidModel;

            osg::dmat4                        _transform;
            osg::dmat4                        _inverse;

            bool                              _definedInFile;
            bool                              _transformScaledByResolution;
    };

}
