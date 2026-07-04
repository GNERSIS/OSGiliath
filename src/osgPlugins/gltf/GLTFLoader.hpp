#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/nodes/Node.hpp>
#include <osg/state/BlendFunc.hpp>
#include <osg/state/CullFace.hpp>
#include <osg/state/Material.hpp>
#include <osg/state/Program.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osg/traversal/AnimationPath.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/registry/ReaderWriter.hpp>
#include <string>
#include <unordered_map>
#include <vector>

class GLTFLoader
{
    public:

        osg::ref_ptr<osg::Node>
        load( const std::string&                  filename,
              const osgDB::ReaderWriter::Options* options );

    private:

        // Raw buffer storage
        std::vector<std::vector<uint8_t>>                           _buffers;

        // Parsed JSON
        nlohmann::json                                              _json;

        // Loaded images (cached)
        std::vector<osg::ref_ptr<osg::Image>>                       _images;

        // Loaded textures (cached)
        std::vector<osg::ref_ptr<osg::Texture2D>>                   _textures;

        // Loaded materials (cached as StateSets)
        std::vector<osg::ref_ptr<osg::StateSet>>                    _materials;

        // Shared PBR material program
        osg::ref_ptr<osg::Program>                                  _pbrProgram;

        // Node index -> MatrixTransform mapping for animation targeting
        std::unordered_map<int, osg::ref_ptr<osg::MatrixTransform>> _nodeTransformMap;

        // Options for file loading
        const osgDB::ReaderWriter::Options*                         _options = nullptr;

        // Base directory for relative paths
        std::string                                                 _baseDir;

        // Loading phases
        bool
        parseJSON( const std::string& filename );
        bool
        parseGLB( const std::string& filename );
        void
        loadBuffers();
        void
        loadImages();
        void
        loadTextures();
        void
        loadMaterials();
        void
        loadAnimations();
        osg::Program*
        getPbrProgram();
        osg::ref_ptr<osg::Node>
        buildScene();

        // Helpers
        const uint8_t*
        getAccessorData( int     accessorIdx,
                         size_t& count,
                         int&    componentType,
                         int&    type,
                         size_t& stride ) const;
        osg::ref_ptr<osg::Geometry>
        buildPrimitive( const nlohmann::json& primitive ) const;
        osg::ref_ptr<osg::Node>
        buildNode( int nodeIdx );
};
