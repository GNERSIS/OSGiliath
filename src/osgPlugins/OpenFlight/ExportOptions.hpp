/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * ExportOptions, derived from Options.
 * Provides: setFlightFileVersionNumber, getFlightFileVersionNumber, setFlightUnits,
 * getFlightUnits, setValidateOnly, getValidateOnly.
 */
//
// Copyright(c) 2008 Skew dmat4 Software LLC.
//

#pragma once

#include "FltWriteResult.hpp"

#include <osg/core/Notify.hpp>
#include <osg/nodes/Node.hpp>
#include <osgDB/io/FileNameUtils.hpp>
#include <osgDB/registry/Options.hpp>
#include <string>
#include <utility>
#include <vector>

namespace flt
{

    /*!
       Options class for controlling export behavior.
       Features a parser for the Option string as well as getter
       methods for supported options.
     */
    class ExportOptions : public osgDB::Options
    {
        public:

            ExportOptions( const Options* opt );
            ExportOptions();

            static const int VERSION_15_7;
            static const int VERSION_15_8;
            static const int VERSION_16_1;

            enum FlightUnits
            {
                METERS,
                KILOMETERS,
                FEET,
                INCHES,
                NAUTICAL_MILES,
            };

            void
            setFlightFileVersionNumber( int num )
            {
                _version = num;
            }

            int
            getFlightFileVersionNumber() const
            {
                return _version;
            }

            void
            setFlightUnits( FlightUnits units )
            {
                _units = units;
            }

            FlightUnits
            getFlightUnits() const
            {
                return _units;
            }

            void
            setValidateOnly( bool validate )
            {
                _validate = validate;
            }

            bool
            getValidateOnly() const
            {
                return _validate;
            }

            void
            setTempDir( const std::string& dir )
            {
                _tempDir = dir;
            }

            std::string
            getTempDir() const
            {
                return _tempDir;
            }

            void
            setLightingDefault( bool lighting )
            {
                _lightingDefault = lighting;
            }

            bool
            getLightingDefault() const
            {
                return _lightingDefault;
            }

            void
            setStripTextureFilePath( bool strip )
            {
                _stripTextureFilePath = strip;
            }

            bool
            getStripTextureFilePath() const
            {
                return _stripTextureFilePath;
            }

            FltWriteResult&
            getWriteResult() const
            {
                return ( wr_ );
            }

            // Parse the OptionString and override values based on
            //   what was set in the OptionString.
            void
            parseOptionsString();

        protected:

            int                    _version;
            FlightUnits            _units;
            bool                   _validate;
            std::string            _tempDir;
            bool                   _lightingDefault;
            bool                   _stripTextureFilePath;

            mutable FltWriteResult wr_;

            static std::string     _versionOption;
            static std::string     _unitsOption;
            static std::string     _validateOption;
            static std::string     _tempDirOption;
            static std::string     _lightingOption;
            static std::string     _stripTextureFilePathOption;
    };

}
