/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgcubemap example application
 */
#include <iostream>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/StateSet.hpp>
#include <osg/textures/TextureCubeMap.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/registry/Registry.hpp>
#include <osgGA/manipulators/DriveManipulator.hpp>
#include <osgGA/manipulators/FlightManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgUtil/utils/HalfWayMapGenerator.hpp>
#include <osgUtil/utils/HighlightMapGenerator.hpp>
#include <osgUtil/utils/ReflectionMapGenerator.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <string>
#include <vector>

int
main( int   argc,
      char* argv[] )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc, argv );

    // Headless capture: early check before viewer construction
    {
        std::string headlessOutput;
        if( arguments.read( "--headless", headlessOutput ) )
        {
            auto node = osgDB::readRefNodeFiles( arguments );
            if( !node )
            {
                node = osgDB::readRefNodeFile( "damaged_helmet.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    osgViewer::Viewer                 viewer;

    osg::ref_ptr<osg::TextureCubeMap> tcm = new osg::TextureCubeMap;

    tcm->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP );
    tcm->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP );
    tcm->setWrap( osg::Texture::WRAP_R, osg::Texture::CLAMP );

    if( arguments.read( "--no-mip-map" ) )
    {
        tcm->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR );
        tcm->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );
    }
    else
    {
        tcm->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
        tcm->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );
    }

    if( arguments.read( "--hardware-mip-map" ) || arguments.read( "--hmp" ) )
    {
        OSG_NOTICE << "tcm->setUseHardwareMipMapGeneration(true)" << std::endl;
        tcm->setUseHardwareMipMapGeneration( true );
    }

    std::string filename;
    if( arguments.read( "--posx", filename ) )
    {
        tcm->setImage( osg::TextureCubeMap::POSITIVE_X,
                       osgDB::readImageFile( filename ) );
    }
    if( arguments.read( "--negx", filename ) )
    {
        tcm->setImage( osg::TextureCubeMap::NEGATIVE_X,
                       osgDB::readImageFile( filename ) );
    }
    if( arguments.read( "--posy", filename ) )
    {
        tcm->setImage( osg::TextureCubeMap::POSITIVE_Y,
                       osgDB::readImageFile( filename ) );
    }
    if( arguments.read( "--negy", filename ) )
    {
        tcm->setImage( osg::TextureCubeMap::NEGATIVE_Y,
                       osgDB::readImageFile( filename ) );
    }
    if( arguments.read( "--posz", filename ) )
    {
        tcm->setImage( osg::TextureCubeMap::POSITIVE_Z,
                       osgDB::readImageFile( filename ) );
    }
    if( arguments.read( "--negz", filename ) )
    {
        tcm->setImage( osg::TextureCubeMap::NEGATIVE_Z,
                       osgDB::readImageFile( filename ) );
    }

    int numValidImages = 0;
    if( tcm->getImage( osg::TextureCubeMap::POSITIVE_X ) )
    {
        ++numValidImages;
    }
    if( tcm->getImage( osg::TextureCubeMap::NEGATIVE_X ) )
    {
        ++numValidImages;
    }
    if( tcm->getImage( osg::TextureCubeMap::POSITIVE_Y ) )
    {
        ++numValidImages;
    }
    if( tcm->getImage( osg::TextureCubeMap::NEGATIVE_Y ) )
    {
        ++numValidImages;
    }
    if( tcm->getImage( osg::TextureCubeMap::POSITIVE_Z ) )
    {
        ++numValidImages;
    }
    if( tcm->getImage( osg::TextureCubeMap::NEGATIVE_Z ) )
    {
        ++numValidImages;
    }

    if( numValidImages != 6 )
    {
        // generate the six highlight map images (light direction = [1, 1, -1])
        osgUtil::HighlightMapGenerator* mapgen = new osgUtil::HighlightMapGenerator(
            osg::vec3( 1, 1, -1 ),            // light direction
            osg::vec4( 1, 0.9F, 0.8F, 1 ),    // light color
            8
        );                                    // specular exponent

        mapgen->generateMap();

        // assign the six images to the texture object
        if( !tcm->getImage( osg::TextureCubeMap::POSITIVE_X ) )
        {
            tcm->setImage( osg::TextureCubeMap::POSITIVE_X,
                           mapgen->getImage( osg::TextureCubeMap::POSITIVE_X ) );
        }
        if( !tcm->getImage( osg::TextureCubeMap::NEGATIVE_X ) )
        {
            tcm->setImage( osg::TextureCubeMap::NEGATIVE_X,
                           mapgen->getImage( osg::TextureCubeMap::NEGATIVE_X ) );
        }
        if( !tcm->getImage( osg::TextureCubeMap::POSITIVE_Y ) )
        {
            tcm->setImage( osg::TextureCubeMap::POSITIVE_Y,
                           mapgen->getImage( osg::TextureCubeMap::POSITIVE_Y ) );
        }
        if( !tcm->getImage( osg::TextureCubeMap::NEGATIVE_Y ) )
        {
            tcm->setImage( osg::TextureCubeMap::NEGATIVE_Y,
                           mapgen->getImage( osg::TextureCubeMap::NEGATIVE_Y ) );
        }
        if( !tcm->getImage( osg::TextureCubeMap::POSITIVE_Z ) )
        {
            tcm->setImage( osg::TextureCubeMap::POSITIVE_Z,
                           mapgen->getImage( osg::TextureCubeMap::POSITIVE_Z ) );
        }
        if( !tcm->getImage( osg::TextureCubeMap::NEGATIVE_Z ) )
        {
            tcm->setImage( osg::TextureCubeMap::NEGATIVE_Z,
                           mapgen->getImage( osg::TextureCubeMap::NEGATIVE_Z ) );
        }
    }

    float LODBias;
    if( arguments.read( "--lod", LODBias ) )
    {
        tcm->setLODBias( LODBias );
    }

    osg::ref_ptr<osg::Program> program = new osg::Program;
    std::string                shaderFilename;
    while( arguments.read( "-s", shaderFilename ) )
    {
        osg::ref_ptr<osg::Shader> shader = osgDB::readRefShaderFile( shaderFilename );
        if( shader )
        {
            program->addShader( shader );
        }
    }

    // load the nodes from the commandline arguments.
    osg::ref_ptr<osg::Node> rootnode = osgDB::readRefNodeFiles( arguments );

    // if not loaded assume no arguments passed in, try use default mode instead.
    if( !rootnode )
    {
        rootnode = osgDB::readRefNodeFile( "damaged_helmet.glb" );
    }

    if( !rootnode )
    {
        osg::notify( osg::NOTICE )
            << "Please specify a model filename on the command line." << std::endl;
        return 1;
    }

    osg::StateSet* ss = rootnode->getOrCreateStateSet();

    // enable texturing, replacing any textures in the subgraphs
    ss->setTextureAttributeAndModes( 0,
                                     tcm,
                                     osg::StateAttribute::OVERRIDE |
                                         osg::StateAttribute::ON );

    // Use ADD blend mode: the highlight map adds specular highlights to the lit base
    // color
    ss->setDefine( "TEXTURE_ENV_FUNCTION0", "texenv_ADD" );

    if( program->getNumShaders() > 0 )
    {
        ss->setAttribute( program.get() );
        ss->addUniform( new osg::Uniform( "baseTexture", 0 ) );
    }

    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize( rootnode );

    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData( rootnode );

    // create the windows and run the threads.
    viewer.realize();
    return viewer.run();
}
