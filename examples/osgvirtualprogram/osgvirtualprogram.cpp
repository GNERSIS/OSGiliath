#include <iostream>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/Material.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgViewer/core/Viewer.hpp>

extern osg::Node*
CreateSimpleHierarchy( osg::Node* model );
extern osg::Node*
CreateAdvancedHierarchy( osg::Node* model );

////////////////////////////////////////////////////////////////////////////////
osg::Node*
CreateGlobe( void )
{
    // File not found - create textured sphere
    osg::Geode*                          geode = new osg::Geode;
    osg::ref_ptr<osg::TessellationHints> hints = new osg::TessellationHints;
    hints->setDetailRatio( 0.3 );

#if 1
    osg::ref_ptr<osg::ShapeDrawable> shape =
        new osg::ShapeDrawable( new osg::Sphere( osg::vec3( 0.0F, 0.0F, 0.0F ), 4.0 ),
                                hints.get() );
#else
    osg::ref_ptr<osg::ShapeDrawable> shape = new osg::ShapeDrawable(
        new osg::Box( osg::vec3( -1.0F, -1.0F, -1.0F ), 2.0, 2.0, 2.0 )
    );
#endif

    shape->setColor( osg::vec4( 0.8F, 0.8F, 0.8F, 1.0F ) );

    geode->addDrawable( shape.get() );

    osg::StateSet*  stateSet = new osg::StateSet;

    osg::Texture2D* texture  = new osg::Texture2D(
        osgDB::readRefImageFile( "Images/land_shallow_topo_2048.jpg" )
    );

    osg::Material* material = new osg::Material;

    material->setAmbient( osg::Material::FRONT_AND_BACK,
                          osg::vec4( 0.9, 0.9, 0.9, 1.0 ) );

    material->setDiffuse( osg::Material::FRONT_AND_BACK,
                          osg::vec4( 0.9, 0.9, 0.9, 1.0 ) );

#if 1
    material->setSpecular( osg::Material::FRONT_AND_BACK,
                           osg::vec4( 0.7, 0.3, 0.3, 1.0 ) );

    material->setShininess( osg::Material::FRONT_AND_BACK, 25 );

#endif

    stateSet->setAttributeAndModes( material );
    stateSet->setTextureAttributeAndModes( 0, texture, osg::StateAttribute::ON );

    geode->setStateSet( stateSet );
    return geode;
}

////////////////////////////////////////////////////////////////////////////////
int
main( int    argc,
      char** argv )
{
    // construct the viewer.
    osg::ArgumentParser arguments( &argc, argv );
    // Headless capture: early check before viewer construction
    {
        std::string headlessOutput;
        if( arguments.read( "--headless", headlessOutput ) )
        {
            auto node = osgDB::readRefNodeFiles( arguments );
            if( !node )
            {
                node = osgDB::readRefNodeFile( "duck.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    osgViewer::Viewer viewer( arguments );

    bool useSimpleExample = arguments.read( "-s" ) || arguments.read( "--simple" );

    osg::ref_ptr<osg::Node> model;

    if( arguments.argc() > 1 && !arguments.isOption( 1 ) )
    {
        std::string filename = arguments[1];
        model                = osgDB::readRefNodeFile( filename );
        if( !model )
        {
            osg::notify( osg::NOTICE )
                << "Error, cannot read " << filename
                << ". Loading default earth model instead." << std::endl;
        }
    }

    if( model == NULL )
    {
        model = CreateGlobe();
    }

    osg::ref_ptr<osg::Node> node = useSimpleExample
                                     ? CreateSimpleHierarchy( model.get() )
                                     : CreateAdvancedHierarchy( model.get() );

    viewer.setSceneData( node );
    viewer.realize();
    viewer.run();

    return 0;
}
