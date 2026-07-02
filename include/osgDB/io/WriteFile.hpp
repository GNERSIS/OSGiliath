/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Convenience functions for writing scene graphs, images, and shaders
 * to files using the plugin registry.
 */
#pragma once

#include <osg/geometry/Shape.hpp>
#include <osg/images/Image.hpp>
#include <osg/nodes/Node.hpp>
#include <osgDB/Export.hpp>
#include <osgDB/registry/Registry.hpp>
#include <string>

namespace osgDB
{

    /** Write an osg::Object to file.
     * Return true on success,
     * return false on failure.
     * Use the Options object to control cache operations and file search paths in
     * osgDB::Registry. The osgDB::Registry is used to load the appropriate ReaderWriter
     * plugin for the filename extension, and this plugin then handles the request to
     * write the specified file.*/
    extern OSGDB_EXPORT bool
    writeObjectFile( const osg::Object& object,
                     const std::string& filename,
                     const Options*     options );

    /** Write an osg::Object to file.
     * Return true on success,
     * return false on failure.
     * The osgDB::Registry is used to load the appropriate ReaderWriter plugin
     * for the filename extension, and this plugin then handles the request
     * to write the specified file.*/
    inline bool
    writeObjectFile( const osg::Object& object,
                     const std::string& filename )
    {
        return writeObjectFile( object, filename, Registry::instance()->getOptions() );
    }

    /** Write an osg::Image to file.
     * Return true on success,
     * return false on failure.
     * Use the Options object to control cache operations and file search paths in
     * osgDB::Registry. The osgDB::Registry is used to load the appropriate ReaderWriter
     * plugin for the filename extension, and this plugin then handles the request to
     * write the specified file.*/
    extern OSGDB_EXPORT bool
    writeImageFile( const osg::Image&  image,
                    const std::string& filename,
                    const Options*     options );

    /** Write an osg::Image to file.
     * Return true on success,
     * return false on failure.
     * The osgDB::Registry is used to load the appropriate ReaderWriter plugin
     * for the filename extension, and this plugin then handles the request
     * to write the specified file.*/
    inline bool
    writeImageFile( const osg::Image&  image,
                    const std::string& filename )
    {
        return writeImageFile( image, filename, Registry::instance()->getOptions() );
    }

    /** Write an osg::HeightField to file.
     * Return true on success,
     * return false on failure.
     * Use the Options object to control cache operations and file search paths in
     * osgDB::Registry. The osgDB::Registry is used to load the appropriate ReaderWriter
     * plugin for the filename extension, and this plugin then handles the request to
     * write the specified file.*/
    extern OSGDB_EXPORT bool
    writeHeightFieldFile( const osg::HeightField& hf,
                          const std::string&      filename,
                          const Options*          options );

    /** Write an osg::HeightField to file.
     * Return true on success,
     * return false on failure.
     * The osgDB::Registry is used to load the appropriate ReaderWriter plugin
     * for the filename extension, and this plugin then handles the request
     * to write the specified file.*/
    inline bool
    writeHeightFieldFile( const osg::HeightField& hf,
                          const std::string&      filename )
    {
        return writeHeightFieldFile( hf, filename, Registry::instance()->getOptions() );
    }

    /** Write an osg::Node to file.
     * Return true on success,
     * return false on failure.
     * Use the Options object to control cache operations and file search paths in
     * osgDB::Registry. The osgDB::Registry is used to load the appropriate ReaderWriter
     * plugin for the filename extension, and this plugin then handles the request to
     * write the specified file.*/
    extern OSGDB_EXPORT bool
    writeNodeFile( const osg::Node&   node,
                   const std::string& filename,
                   const Options*     options );

    /** Write an osg::Node to file.
     * Return true on success,
     * return false on failure.
     * The osgDB::Registry is used to load the appropriate ReaderWriter plugin
     * for the filename extension, and this plugin then handles the request
     * to write the specified file.*/
    inline bool
    writeNodeFile( const osg::Node&   node,
                   const std::string& filename )
    {
        return writeNodeFile( node, filename, Registry::instance()->getOptions() );
    }

    /** Write an osg::Shader to file.
     * Return true on success,
     * return false on failure.
     * Use the Options object to control cache operations and file search paths in
     * osgDB::Registry. The osgDB::Registry is used to load the appropriate ReaderWriter
     * plugin for the filename extension, and this plugin then handles the request to
     * write the specified file.*/
    extern OSGDB_EXPORT bool
    writeShaderFile( const osg::Shader& shader,
                     const std::string& filename,
                     const Options*     options );

    /** Write an osg::Shader to file.
     * Return true on success,
     * return false on failure.
     * The osgDB::Registry is used to load the appropriate ReaderWriter plugin
     * for the filename extension, and this plugin then handles the request
     * to write the specified file.*/
    inline bool
    writeShaderFile( const osg::Shader& shader,
                     const std::string& filename )
    {
        return writeShaderFile( shader, filename, Registry::instance()->getOptions() );
    }

    /** Write an osg::Script to file.
     * Return true on success,
     * return false on failure.
     * Use the Options object to control cache operations and file search paths in
     * osgDB::Registry. The osgDB::Registry is used to load the appropriate ReaderWriter
     * plugin for the filename extension, and this plugin then handles the request to
     * write the specified file.*/
    extern OSGDB_EXPORT bool
    writeScriptFile( const osg::Script& image,
                     const std::string& filename,
                     const Options*     options );

    /** Write an osg::Script to file.
     * Return true on success,
     * return false on failure.
     * The osgDB::Registry is used to load the appropriate ReaderWriter plugin
     * for the filename extension, and this plugin then handles the request
     * to write the specified file.*/
    inline bool
    writeScriptFile( const osg::Script& image,
                     const std::string& filename )
    {
        return writeScriptFile( image, filename, Registry::instance()->getOptions() );
    }

}
