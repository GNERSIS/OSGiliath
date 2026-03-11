/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osganimationhardware example application
 */
#include <iostream>
#include <osg/geometry/Drawable.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgAnimation/core/AnimationManagerBase.hpp>
#include <osgAnimation/core/BasicAnimationManager.hpp>
#include <osgAnimation/morph/MorphGeometry.hpp>
#include <osgAnimation/morph/MorphTransformHardware.hpp>
#include <osgAnimation/skeletal/BoneMapVisitor.hpp>
#include <osgAnimation/skeletal/RigGeometry.hpp>
#include <osgAnimation/skeletal/RigTransformHardware.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgGA/manipulators/AnimationPathManipulator.hpp>
#include <osgGA/manipulators/DriveManipulator.hpp>
#include <osgGA/manipulators/FlightManipulator.hpp>
#include <osgGA/manipulators/KeySwitchMatrixManipulator.hpp>
#include <osgGA/manipulators/TerrainManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>
#include <sstream>

static unsigned int
getRandomValueinRange( unsigned int v )
{
    return static_cast<unsigned int>( ( rand() * 1.0 * v ) / ( RAND_MAX - 1 ) );
}

osg::ref_ptr<osg::Program> CommonProgram;

// show how to override the default RigTransformHardware for customized usage
struct MyRigTransformHardware : public osgAnimation::RigTransformHardware
{
        int _maxmatrix;

        MyRigTransformHardware() :
            _maxmatrix( 99 )
        {
        }

        virtual bool
        init( osgAnimation::RigGeometry& rig )
        {
            if( _perVertexInfluences.empty() )
            {
                prepareData( rig );
                return false;
            }
            if( !rig.getSkeleton() )
            {
                return false;
            }

            osgAnimation::BoneMapVisitor mapVisitor;
            rig.getSkeleton()->accept( mapVisitor );
            osgAnimation::BoneMap boneMap = mapVisitor.getBoneMap();

            if( !buildPalette( boneMap, rig ) )
            {
                return false;
            }

            osg::Geometry&  source = *rig.getSourceGeometry();
            osg::Vec3Array* positionSrc =
                dynamic_cast<osg::Vec3Array*>( source.getVertexArray() );

            if( !positionSrc )
            {
                OSG_WARN << "RigTransformHardware no vertex array in the geometry "
                         << rig.getName() << std::endl;
                return false;
            }

            // copy shallow from source geometry to rig
            rig.copyFrom( source );

            osg::ref_ptr<osg::Shader>   vertexshader;
            osg::ref_ptr<osg::StateSet> stateset = rig.getOrCreateStateSet();
            if( !CommonProgram.valid() )
            {
                CommonProgram = new osg::Program;
                CommonProgram->setName( "HardwareSkinning" );

                // set default source if _shader is not user set
                if( !vertexshader.valid() )
                {
                    vertexshader =
                        osgDB::readRefShaderFile( osg::Shader::VERTEX, "skinning.vert" );
                }

                if( !vertexshader.valid() )
                {
                    OSG_WARN << "RigTransformHardware can't load VertexShader"
                             << std::endl;
                    return false;
                }

                // replace max matrix by the value from uniform
                {
                    std::string str       = vertexshader->getShaderSource();
                    std::string toreplace = std::string( "MAX_MATRIX" );
                    std::size_t start     = str.find( toreplace );
                    if( std::string::npos != start )
                    {
                        std::stringstream ss;
                        ss << _maxmatrix;    // getMatrixPaletteUniform()->getNumElements();
                        str.replace( start, toreplace.size(), ss.str() );
                        vertexshader->setShaderSource( str );
                    }
                    else
                    {
                        OSG_WARN << "MAX_MATRIX not found in Shader! " << str
                                 << std::endl;
                    }
                    OSG_INFO << "Shader " << str << std::endl;
                }
                CommonProgram->addShader( vertexshader.get() );
            }

            unsigned int nbAttribs = getNumVertexAttrib();
            for( unsigned int i = 0; i < nbAttribs; i++ )
            {
                std::stringstream ss;
                ss << "boneWeight" << i;
                CommonProgram->addBindAttribLocation( ss.str(), _minAttribIndex + i );
                rig.setVertexAttribArray( _minAttribIndex + i, getVertexAttrib( i ) );
                OSG_INFO << "set vertex attrib " << ss.str() << std::endl;
            }

            stateset->removeUniform( "nbBonesPerVertex" );
            stateset->addUniform( new osg::Uniform( "nbBonesPerVertex",
                                                    _bonesPerVertex ) );

            stateset->removeUniform( "matrixPalette" );
            stateset->addUniform( _uniformMatrixPalette );

            stateset->setAttribute( CommonProgram.get() );

            _needInit = false;
            return true;
        }
};

struct SetupRigGeometry : public osg::DualModeVisitor
{
        bool _hardware;

        SetupRigGeometry( bool hardware = true ) :
            osg::DualModeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
            _hardware( hardware )
        {
        }

        void
        apply( osg::Geode& geode )
        {
            for( unsigned int i = 0; i < geode.getNumDrawables(); i++ )
            {
                apply( *geode.getDrawable( i ) );
            }
        }

        void
        apply( osg::Drawable& geom )
        {
            if( _hardware )
            {
                osgAnimation::RigGeometry* rig =
                    dynamic_cast<osgAnimation::RigGeometry*>( &geom );
                if( rig )
                {
                    rig->setRigTransformImplementation( new MyRigTransformHardware );
                    osgAnimation::MorphGeometry* morph =
                        dynamic_cast<osgAnimation::MorphGeometry*>(
                            rig->getSourceGeometry()
                        );
                    if( morph )
                    {
                        morph->setMorphTransformImplementation(
                            new osgAnimation::MorphTransformHardware
                        );
                    }
                }
            }

#if 0
        if (geom.getName() != std::string("box")) // we disable compute of bounding box for all geometry except our bounding box
            geom.setComputeBoundingBoxCallback(new osg::Drawable::ComputeBoundingBoxCallback);
//            geom.setInitialBound(new osg::Drawable::ComputeBoundingBoxCallback);
#endif
        }
};

osg::Group*
createCharacterInstance( osg::Group* character,
                         bool        hardware )
{
    osg::ref_ptr<osg::Group> c;
    if( hardware )
    {
        c = osg::clone( character,
                        osg::CopyOp::DEEP_COPY_ALL &
                            ~osg::CopyOp::DEEP_COPY_PRIMITIVES &
                            ~osg::CopyOp::DEEP_COPY_ARRAYS );
    }
    else
    {
        c = osg::clone( character, osg::CopyOp::DEEP_COPY_ALL );
    }

    osgAnimation::AnimationManagerBase* animationManager =
        dynamic_cast<osgAnimation::AnimationManagerBase*>( c->getUpdateCallback() );

    osgAnimation::BasicAnimationManager* anim =
        dynamic_cast<osgAnimation::BasicAnimationManager*>( animationManager );
    const osgAnimation::AnimationList& list = animationManager->getAnimationList();
    int                                v    = getRandomValueinRange( list.size() );
    if( list[v]->getName() == std::string( "MatIpo_ipo" ) )
    {
        anim->playAnimation( list[v].get() );
        v = ( v + 1 ) % list.size();
    }

    anim->playAnimation( list[v].get() );

    SetupRigGeometry switcher( hardware );
    c->accept( switcher );

    return c.release();
}

int
main( int   argc,
      char* argv[] )
{
    osg::ArgumentParser arguments( &argc, argv );
    std::cerr << "This example works better with a character animation file (e.g. "
                 "nathan model)"
              << std::endl;

    osg::ArgumentParser psr( &argc, argv );

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

    osgViewer::Viewer viewer( psr );

    bool              hardware = true;
    int               maxChar  = 10;
    while( psr.read( "--software" ) )
    {
        hardware = false;
    }
    while( psr.read( "--number", maxChar ) )
    {
    }

    osg::ref_ptr<osg::Node>  node = osgDB::readRefNodeFiles( psr );
    osg::ref_ptr<osg::Group> root = dynamic_cast<osg::Group*>( node.get() );
    if( !root )
    {
        std::cout << psr.getApplicationName() << ": No data loaded" << std::endl;
        return 1;
    }

    {
        osgAnimation::AnimationManagerBase* animationManager =
            dynamic_cast<osgAnimation::AnimationManagerBase*>(
                root->getUpdateCallback()
            );
        if( !animationManager )
        {
            osg::notify(
                osg::FATAL
            ) << "no AnimationManagerBase found, updateCallback need to animate elements"
              << std::endl;
            return 1;
        }
    }

    osg::ref_ptr<osg::Group> scene = new osg::Group;

    // add the state manipulator
    viewer.addEventHandler(
        new osgGA::StateSetManipulator( viewer.getCamera()->getOrCreateStateSet() )
    );

    // add the thread model handler
    viewer.addEventHandler( new osgViewer::ThreadingHandler );

    // add the window size toggle handler
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );

    // add the stats handler
    viewer.addEventHandler( new osgViewer::StatsHandler );

    // add the help handler
    viewer.addEventHandler( new osgViewer::HelpHandler( psr.getApplicationUsage() ) );

    // add the LOD Scale handler
    viewer.addEventHandler( new osgViewer::LODScaleHandler );

    // add the screen capture handler
    viewer.addEventHandler( new osgViewer::ScreenCaptureHandler );

    viewer.setSceneData( scene.get() );

    viewer.realize();

    double xChar = maxChar;
    double yChar = xChar * 9.0 / 16;
    for( double i = 0.0; i < xChar; i++ )
    {
        for( double j = 0.0; j < yChar; j++ )
        {
            osg::ref_ptr<osg::Group> c = createCharacterInstance( root.get(), hardware );
            osg::MatrixTransform*    tr = new osg::MatrixTransform;
            tr->setMatrix(
                osg::translate( 2.0 * ( i - xChar * .5 ), 0.0, 2.0 * ( j - yChar * .5 ) )
            );
            tr->addChild( c.get() );
            scene->addChild( tr );
        }
    }
    std::cout << "created " << xChar * yChar << " instance" << std::endl;
    return viewer.run();
}
