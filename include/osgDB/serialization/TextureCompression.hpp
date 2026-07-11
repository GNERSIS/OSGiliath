/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#pragma once

#include <osg/core/ref_ptr.hpp>
#include <osgDB/Export.hpp>

namespace osg
{
    class Image;
    class Node;
}

namespace osgDB::serialization
{

    OSGDB_EXPORT osg::ref_ptr<osg::Image>
    compressImageDXT( const osg::Image& src );

    OSGDB_EXPORT void
    compressSceneTextures( osg::Node& root );

}
