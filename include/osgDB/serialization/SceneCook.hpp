/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#pragma once

#include <cstdint>
#include <osg/core/Object.hpp>
#include <osgDB/Export.hpp>
#include <string>

namespace osgDB::serialization
{

    inline constexpr std::uint32_t sceneCookFormatVersion = 1U;

    OSGDB_EXPORT bool
    writeSceneCook( const osg::Object& root,
                    const std::string& path );

    OSGDB_EXPORT osg::ref_ptr<osg::Object>
    readSceneCook( const std::string& path );

}
