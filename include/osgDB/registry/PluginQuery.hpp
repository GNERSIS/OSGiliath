/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Queries available file format plugins. Lists supported
 * extensions and protocol schemes from loaded plugins.
 */
#pragma once

#include <list>
#include <osg/core/ref_ptr.hpp>
#include <osg/core/Referenced.hpp>
#include <osgDB/Export>
#include <osgDB/registry/ReaderWriter.hpp>
#include <string>

namespace osgDB
{

    typedef std::list<std::string> FileNameList;

    FileNameList OSGDB_EXPORT
    listAllAvailablePlugins();

    class ReaderWriterInfo : public osg::Referenced
    {
        public:

            ReaderWriterInfo() :
                features( ReaderWriter::FEATURE_NONE )
            {
            }

            std::string                        plugin;
            std::string                        description;
            ReaderWriter::FormatDescriptionMap protocols;
            ReaderWriter::FormatDescriptionMap extensions;
            ReaderWriter::FormatDescriptionMap options;
            ReaderWriter::FormatDescriptionMap environment;
            ReaderWriter::Features             features;

        protected:

            virtual ~ReaderWriterInfo()
            {
            }
    };

    typedef std::list<osg::ref_ptr<ReaderWriterInfo>> ReaderWriterInfoList;

    bool OSGDB_EXPORT
    queryPlugin( const std::string&    fileName,
                 ReaderWriterInfoList& infoList );

    bool OSGDB_EXPORT
    outputPluginDetails( std::ostream&      out,
                         const std::string& fileName );

}
