/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgfadetext example application
 */
#include <iostream>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/CoordinateSystemNode.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/nodes/PositionAttitudeTransform.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osg/traversal/ClusterCullingCallback.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/manipulators/TerrainManipulator.hpp>
#include <osgSim/OverlayNode.hpp>
#include <osgSim/SphereSegment.hpp>
#include <osgText/FadeText.hpp>
#include <osgViewer/core/Viewer.hpp>

osg::Node*
createEarth()
{
    osg::TessellationHints* hints = new osg::TessellationHints;
    hints->setDetailRatio( 5.0F );

    osg::ShapeDrawable* sd =
        new osg::ShapeDrawable( new osg::Sphere( osg::vec3( 0.0, 0.0, 0.0 ),
                                                 osg::WGS_84_RADIUS_POLAR ),
                                hints );

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable( sd );

    std::string filename = osgDB::findDataFile( "Images/land_shallow_topo_2048.jpg" );
    geode->getOrCreateStateSet()->setTextureAttributeAndModes(
        0,
        new osg::Texture2D( osgDB::readRefImageFile( filename ) )
    );

    osg::CoordinateSystemNode* csn = new osg::CoordinateSystemNode;
    csn->setEllipsoidModel( new osg::EllipsoidModel() );
    csn->addChild( geode );

    return csn;
}

osgText::Text*
createText( osg::EllipsoidModel* ellipsoid,
            double               latitude,
            double               longitude,
            double               height,
            const std::string&   str )
{
    double X, Y, Z;
    ellipsoid->convertLatLongHeightToXYZ( osg::radians( latitude ),
                                          osg::radians( longitude ),
                                          height,
                                          X,
                                          Y,
                                          Z );

    osgText::Text* text   = new osgText::FadeText;

    osg::vec3      normal = osg::vec3( ellipsoid->computeLocalUpVector( X, Y, Z ) );
    text->setCullCallback(
        new osg::ClusterCullingCallback( osg::vec3( X, Y, Z ), normal, 0.0 )
    );

    text->setText( str );
    text->setFont( "fonts/arial.ttf" );
    text->setPosition( osg::vec3( X, Y, Z ) );
    text->setCharacterSize( 300000.0F );
    text->setCharacterSizeMode(
        osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT
    );
    text->setAutoRotateToScreen( true );

    return text;
}

osg::Node*
createFadeText( osg::EllipsoidModel* ellipsoid )
{
    osg::Group* group = new osg::Group;

    group->getOrCreateStateSet()->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );

    osg::Geode* geode = new osg::Geode;
    group->addChild( geode );

    std::vector<std::string> textList;
    textList.push_back( "Town" );
    textList.push_back( "City" );
    textList.push_back( "Village" );
    textList.push_back( "River" );
    textList.push_back( "Mountain" );
    textList.push_back( "Road" );
    textList.push_back( "Lake" );

    unsigned int numLat         = 15;
    unsigned int numLong        = 20;
    double       latitude       = 0.0;
    double       longitude      = -100.0;
    double       deltaLatitude  = 1.0F;
    double       deltaLongitude = 1.0F;

    unsigned int t              = 0;
    for( unsigned int i = 0; i < numLat; ++i, latitude += deltaLatitude )
    {
        double lgnt = longitude;
        for( unsigned int j = 0; j < numLong; ++j, ++t, lgnt += deltaLongitude )
        {
            geode->addDrawable(
                createText( ellipsoid, latitude, lgnt, 0, textList[t % textList.size()] )
            );
        }
    }

    return group;
}

class TextSettings : public osg::DualModeVisitor
{
    public:

        TextSettings( osg::ArgumentParser& arguments ) :
            osg::DualModeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
            _backdropTypeSet( false ),
            _backdropType( osgText::Text::NONE ),
            _shaderTechniqueSet( false ),
            _shaderTechnique( osgText::GREYSCALE )
        {
            if( arguments.read( "--outline" ) )
            {
                _backdropTypeSet = true;
                _backdropType    = osgText::Text::OUTLINE;
            }
            if( arguments.read( "--sdf" ) )
            {
                _shaderTechniqueSet = true;
                _shaderTechnique    = osgText::SIGNED_DISTANCE_FIELD;
            }
            if( arguments.read( "--all" ) )
            {
                _shaderTechniqueSet = true;
                _shaderTechnique    = osgText::ALL_FEATURES;
            }
            if( arguments.read( "--greyscale" ) )
            {
                _shaderTechniqueSet = true;
                _shaderTechnique    = osgText::GREYSCALE;
            }
            if( arguments.read( "--no-shader" ) )
            {
                _shaderTechniqueSet = true;
                _shaderTechnique    = osgText::NO_TEXT_SHADER;
            }
        }

        void
        apply( osg::Drawable& drawable )
        {
            osgText::Text* text = dynamic_cast<osgText::Text*>( &drawable );
            if( text )
            {
                if( _backdropTypeSet )
                {
                    text->setBackdropType( _backdropType );
                    text->setBackdropOffset( 0.1F );
                    text->setBackdropColor( osg::vec4( 1.0F, 0.0F, 0.0F, 1.0F ) );
                }
                if( _shaderTechniqueSet )
                {
                    text->setShaderTechnique( _shaderTechnique );
                }
            }
        }

        bool                        _backdropTypeSet;
        osgText::Text::BackdropType _backdropType;
        bool                        _shaderTechniqueSet;
        osgText::ShaderTechnique    _shaderTechnique;
};

int
main( int    argc,
      char** argv )
{
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

    viewer.getCamera()->setComputeNearFarMode(
        osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES
    );
    viewer.getCamera()->setNearFarRatio( 0.00001F );

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> root = createEarth();

    if( !root )
    {
        return 0;
    }

    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData( root.get() );

    osg::CoordinateSystemNode* csn =
        dynamic_cast<osg::CoordinateSystemNode*>( root.get() );
    if( csn )
    {
        // add fade text around the globe
        csn->addChild( createFadeText( csn->getEllipsoidModel() ) );
    }

    if( arguments.argc() > 1 )
    {
        TextSettings textSettings( arguments );
        root->accept( textSettings );
    }

    viewer.setCameraManipulator( new osgGA::TerrainManipulator );
    return viewer.run();
}
