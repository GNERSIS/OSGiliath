/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * FltWriteResult, derived from WriteResult.
 * Provides: WriteResult, setNumErrors, getNumErrors, setNumWarnings, getNumWarnings,
 * warn.
 */
//
// Copyright(c) 2008 Skew dmat4 Software LLC.
//

#pragma once

#include <osg/core/Notify.hpp>
#include <osg/nodes/Node.hpp>
#include <osgDB/io/FileNameUtils.hpp>
#include <osgDB/registry/ReaderWriter.hpp>
#include <string>
#include <utility>
#include <vector>

namespace flt
{

    /*!
       Custom WriteResult to support proxy/validation ("validate" Option).
       If the application is able to #include this header and obtain the Writeresult
       from osgDB, then the app can query this class for warning or error
       conditions due to scene graph incompatibility with FLT.
     */
    class FltWriteResult : public osgDB::ReaderWriter::WriteResult
    {
        public:

            FltWriteResult( WriteResult::WriteStatus status = WriteResult::FILE_SAVED ) :
                WriteResult( status )
            {
            }

            void
            setNumErrors( int n );
            int
            getNumErrors() const;

            void
            setNumWarnings( int n );
            int
                                                                getNumWarnings() const;

            typedef std::pair<osg::NotifySeverity, std::string> MessagePair;
            typedef std::vector<MessagePair>                    MessageVector;

            void
            warn( const std::string& ss )
            {
                messages_.push_back( std::make_pair( osg::WARN, ss ) );
            }

            void
            error( const std::string& ss )
            {
                messages_.push_back( std::make_pair( osg::FATAL, ss ) );
            }

        protected:

            MessageVector messages_;
    };

}
