/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Convenience functions for writing scene graphs, images, and shaders
 * to files using the plugin registry.
 */
#include <osgDB/io/WriteFile.hpp>

#include <osg/core/Notify.hpp>
#include <osg/core/Object.hpp>
#include <osg/images/Image.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/Node.hpp>
#include <osgDB/registry/Registry.hpp>

using namespace osg;
using namespace osgDB;

bool
osgDB::writeObjectFile( const Object&      object,
                        const std::string& filename,
                        const Options*     options )
{
    ReaderWriter::WriteResult wr =
        Registry::instance()->writeObject( object, filename, options );
    if( !wr.success() )
    {
        OSG_WARN << "Error writing file " << filename << ": " << wr.statusMessage()
                 << std::endl;
    }
    return wr.success();
}

bool
osgDB::writeImageFile( const Image&       image,
                       const std::string& filename,
                       const Options*     options )
{
    ReaderWriter::WriteResult wr =
        Registry::instance()->writeImage( image, filename, options );
    if( !wr.success() )
    {
        OSG_WARN << "Error writing file " << filename << ": " << wr.statusMessage()
                 << std::endl;
    }
    return wr.success();
}

bool
osgDB::writeHeightFieldFile( const HeightField& HeightField,
                             const std::string& filename,
                             const Options*     options )
{
    ReaderWriter::WriteResult wr =
        Registry::instance()->writeHeightField( HeightField, filename, options );
    if( !wr.success() )
    {
        OSG_WARN << "Error writing file " << filename << ": " << wr.statusMessage()
                 << std::endl;
    }
    return wr.success();
}

bool
osgDB::writeNodeFile( const Node&        node,
                      const std::string& filename,
                      const Options*     options )
{
    ReaderWriter::WriteResult wr =
        Registry::instance()->writeNode( node, filename, options );
    if( !wr.success() )
    {
        OSG_WARN << "Error writing file " << filename << ": " << wr.statusMessage()
                 << std::endl;
    }
    return wr.success();
}

bool
osgDB::writeShaderFile( const Shader&      shader,
                        const std::string& filename,
                        const Options*     options )
{
    ReaderWriter::WriteResult wr =
        Registry::instance()->writeShader( shader, filename, options );
    if( !wr.success() )
    {
        OSG_WARN << "Error writing file " << filename << ": " << wr.statusMessage()
                 << std::endl;
    }
    return wr.success();
}

bool
osgDB::writeScriptFile( const Script&      image,
                        const std::string& filename,
                        const Options*     options )
{
    ReaderWriter::WriteResult wr =
        Registry::instance()->writeScript( image, filename, options );
    if( !wr.success() )
    {
        OSG_WARN << "Error writing file " << filename << ": " << wr.statusMessage()
                 << std::endl;
    }
    return wr.success();
}
