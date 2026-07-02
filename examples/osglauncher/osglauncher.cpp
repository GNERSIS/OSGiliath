/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osglauncher example application
 */
// #include <cstdio>
// #include <cstdlib>
#include <iostream>
#include <list>
#include <osg/core/io_utils.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/ClearNode.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/nodes/PositionAttitudeTransform.hpp>
#include <osg/nodes/Projection.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/BlendFunc.hpp>
#include <osg/state/Depth.hpp>
#include <osg/state/Material.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/io/fstream.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgText/Text.hpp>
#include <osgUtil/culling/CullVisitor.hpp>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>
#include <sstream>
#include <string>

int
runApp( std::string xapp );

// class to handle events with a pick
class PickHandler : public osgGA::GUIEventHandler
{
    public:

        PickHandler( osgViewer::Viewer* viewer,
                     osgText::Text*     updateText ) :
            _viewer( viewer ),
            _updateText( updateText )
        {
        }

        ~PickHandler()
        {
        }

        bool
        handle( const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter&      us );

        std::string
        pick( const osgGA::GUIEventAdapter& event );

        void
        highlight( const std::string& name )
        {
            if( _updateText.get() )
            {
                _updateText->setText( name );
            }
        }

    protected:

        osgViewer::Viewer*          _viewer;
        osg::ref_ptr<osgText::Text> _updateText;
};

bool
PickHandler::handle( const osgGA::GUIEventAdapter& ea,
                     osgGA::GUIActionAdapter& )
{
    switch( ea.getEventType() )
    {
        case( osgGA::GUIEventAdapter::FRAME ) :
        case( osgGA::GUIEventAdapter::MOVE ) :
            {
                // osg::notify(osg::NOTICE)<<"MOVE "<<ea.getX()<<",
                // "<<ea.getY()<<std::endl;
                std::string picked_name = pick( ea );
                highlight( picked_name );
                return false;
            }
        case( osgGA::GUIEventAdapter::PUSH ) :
            {
                // osg::notify(osg::NOTICE)<<"PUSH "<<ea.getX()<<",
                // "<<ea.getY()<<std::endl;
                std::string picked_name = pick( ea );
                if( !picked_name.empty() )
                {
                    runApp( picked_name );
                    return true;
                }
                else
                {
                    return false;
                }
            }
        default :
            return false;
    }
}

std::string
PickHandler::pick( const osgGA::GUIEventAdapter& event )
{
    osgUtil::LineSegmentIntersector::Intersections intersections;
    if( _viewer->computeIntersections( event, intersections ) )
    {
        for( osgUtil::LineSegmentIntersector::Intersections::iterator hitr =
                 intersections.begin();
             hitr != intersections.end();
             ++hitr )
        {
            osg::Node* node = hitr->nodePath.empty() ? 0 : hitr->nodePath.back();
            if( node && !node->getName().empty() )
            {
                return node->getName();
            }
        }
    }

    return "";
}

osg::Node*
createHUD( osgText::Text* updateText )
{    // create the hud. derived from osgHud.cpp
    // adds a set of quads, each in a separate Geode - which can be picked individually
    // eg to be used as a menuing/help system!
    // Can pick texts too!
    osg::MatrixTransform* modelview_abs = new osg::MatrixTransform;
    modelview_abs->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    modelview_abs->setMatrix( osg::dmat4() );

    osg::Projection* projection = new osg::Projection;
    projection->setMatrix( osg::ortho2D( 0.0, ( double )1'280, 0.0, ( double )1'024 ) );
    projection->addChild( modelview_abs );

    std::string timesFont( "fonts/times.ttf" );

    // turn lighting off for the text and disable depth test to ensure its always ontop.
    osg::vec3   position( 50.0F, 510.0F, 0.0F );
    osg::vec3   delta( 0.0F, -60.0F, 0.0F );

    {    // this displays what has been selected
        osg::Geode*    geode    = new osg::Geode();
        osg::StateSet* stateset = geode->getOrCreateStateSet();
        stateset->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );
        geode->setName( "The text label" );
        geode->addDrawable( updateText );
        modelview_abs->addChild( geode );

        updateText->setCharacterSize( 20.0F );
        updateText->setFont( timesFont );
        updateText->setColor( osg::vec4( 1.0F, 1.0F, 0.0F, 1.0F ) );
        updateText->setText( "" );
        updateText->setPosition( position );

        position += delta;
    }

    return projection;

}    // end create HUDf

static osg::vec3 defaultPos( 0.0F,
                             0.0F,
                             0.0F );
static osg::vec3 centerScope( 0.0F,
                              0.0F,
                              0.0F );

class Xample
{
        std::string texture;
        std::string app;

    public:

        Xample( std::string image,
                std::string prog )
        {
            texture = image;
            app     = prog;
            osg::notify( osg::INFO ) << "New Xample!" << std::endl;
        };

        ~Xample() {};

        std::string
        getTexture()
        {
            return texture;
        }

        std::string
        getApp()
        {
            return app;
        }

    private:

        Xample()
        {
        }
};    // end class Xample

typedef std::list<Xample>::iterator OP;
static std::list<Xample>            Xamplelist;

void
printList()
{
    osg::notify( osg::INFO ) << "start printList()" << std::endl;
    for( OP i = Xamplelist.begin(); i != Xamplelist.end(); ++i )
    {
        Xample& x = *i;
        osg::notify( osg::INFO )
            << "current x.texture = " << x.getTexture() << std::endl;
        osg::notify( osg::INFO ) << "current x.app = " << x.getApp() << std::endl;
    }
    osg::notify( osg::INFO ) << "end printList()" << std::endl;
}    // end printList()

int
runApp( std::string xapp )
{
    osg::notify( osg::INFO ) << "start runApp()" << std::endl;
    for( OP i = Xamplelist.begin(); i != Xamplelist.end(); ++i )
    {
        Xample& x = *i;
        if( !xapp.compare( x.getApp() ) )
        {
            osg::notify( osg::INFO ) << "app found!" << std::endl;

            const char* cxapp = xapp.c_str();

            osg::notify( osg::INFO ) << "char* = " << cxapp << std::endl;

            return system( cxapp );
        }
    }
    osg::notify( osg::INFO ) << "app not found!" << std::endl;
    return 1;
}    // end printList()

void
readConfFile( const char* confFile )    // read confFile            1
{
    osg::notify( osg::INFO ) << "Start reading confFile" << std::endl;

    std::string fileName = osgDB::findDataFile( confFile );
    if( fileName.empty() )
    {
        osg::notify( osg::INFO ) << "Config file not found" << confFile << std::endl;
        return;
    }

    osgDB::ifstream in( fileName.c_str() );
    if( !in )
    {
        osg::notify( osg::INFO )
            << "File " << fileName << " can not be opened!" << std::endl;
        exit( 1 );
    }
    std::string imageBuffer;
    std::string appBuffer;

    while( !in.eof() )
    {
        std::getline( in, imageBuffer );
        std::getline( in, appBuffer );
        if( imageBuffer == "" || appBuffer == "" )
            ;
        else
        {
            osg::notify( osg::INFO ) << "imageBuffer: " << imageBuffer << std::endl;
            osg::notify( osg::INFO ) << "appBuffer: " << appBuffer << std::endl;
            // jeweils checken ob image vorhanden ist.

            Xample tmp( imageBuffer, appBuffer );    // create Xample objects    2

            Xamplelist.push_back( tmp );             // store objects in list    2
        }
    }

    in.close();

    osg::notify( osg::INFO ) << "End reading confFile" << std::endl;

    printList();
}    // end readConfFile

void
SetObjectTextureState( osg::Geode* geodeCurrent,
                       std::string texture )
{
    // retrieve or create a StateSet
    osg::StateSet*           stateTexture = geodeCurrent->getOrCreateStateSet();

    // load texture.jpg as an image
    osg::ref_ptr<osg::Image> imgTexture = osgDB::readRefImageFile( texture );

    // if the image is successfully loaded
    if( imgTexture )
    {
        // create a new two-dimensional texture object
        osg::Texture2D* texCube = new osg::Texture2D;

        // set the texture to the loaded image
        texCube->setImage( imgTexture );

        // set the texture to the state
        stateTexture->setTextureAttributeAndModes( 0, texCube, osg::StateAttribute::ON );

        // set the state of the current geode
        geodeCurrent->setStateSet( stateTexture );
    }
}    // end SetObjectTextureState

osg::Geode*
createTexturedCube( float       fRadius,
                    osg::vec3   vPosition,
                    std::string texture,
                    std::string geodeName )
{
    // create a cube shape
    osg::Box*           bCube = new osg::Box( vPosition, fRadius );
    // osg::Box *bCube = new osg::Box(vPosition,fRadius);

    // create a container that makes the cube drawable
    osg::ShapeDrawable* sdCube = new osg::ShapeDrawable( bCube );

    // create a geode object to as a container for our drawable cube object
    osg::Geode*         geodeCube = new osg::Geode();
    geodeCube->setName( geodeName );

    // set the object texture state
    SetObjectTextureState( geodeCube, texture );

    // add our drawable cube to the geode container
    geodeCube->addDrawable( sdCube );

    return ( geodeCube );
}    // end CreateCube

osg::PositionAttitudeTransform*
getPATransformation( osg::Node* object,
                     osg::vec3  position,
                     osg::vec3  scale,
                     osg::vec3  pivot )
{
    osg::PositionAttitudeTransform* tmpTrans = new osg::PositionAttitudeTransform();
    tmpTrans->addChild( object );

    tmpTrans->setPosition( osg::dvec3( position ) );
    tmpTrans->setScale( osg::dvec3( scale ) );
    tmpTrans->setPivotPoint( osg::dvec3( pivot ) );

    return tmpTrans;
}

void
printBoundings( osg::Node*  current,
                std::string name )
{
    const osg::sphere& currentBound = current->getBound();
    osg::notify( osg::INFO ) << name << std::endl;
    osg::notify( osg::INFO ) << "center = " << currentBound.center << std::endl;
    osg::notify( osg::INFO ) << "radius = " << currentBound.radius << std::endl;

    // return currentBound.radius();
}

osg::Group*
setupGraph()    // create Geodes/Nodes from Xamplelist    3
{
    osg::Group* xGroup = new osg::Group();

    // positioning and sizes
    float       defaultRadius = 0.8F;

    int         itemsInLine   = 4;    // name says everything
    float       offset        = 0.05F;
    float       bs            = ( defaultRadius / 4 ) + offset;
    float       xstart        = ( 3 * bs ) * ( -1 );
    float       zstart        = xstart * ( -1 );
    float       xnext         = xstart;
    float       znext         = zstart;
    float       xjump         = ( 2 * bs );
    float       zjump         = xjump;
    osg::vec3   vScale( 0.5F, 0.5F, 0.5F );
    osg::vec3   vPivot( 0.0F, 0.0F, 0.0F );

    // run through Xampleliste
    int         z = 1;
    for( OP i = Xamplelist.begin(); i != Xamplelist.end(); ++i, ++z )
    {
        Xample&    x = *i;

        osg::Node* tmpCube =
            createTexturedCube( defaultRadius, defaultPos, x.getTexture(), x.getApp() );
        printBoundings( tmpCube, x.getApp() );
        osg::vec3                       vPosition( xnext, 0.0F, znext );
        osg::PositionAttitudeTransform* transX =
            getPATransformation( tmpCube, vPosition, vScale, vPivot );
        xGroup->addChild( transX );

        // line feed
        if( z < itemsInLine )
        {
            xnext += xjump;
        }
        else
        {
            xnext  = xstart;
            znext -= zjump;
            z      = 0;
        }
    }    // end run through list

    return xGroup;
}    // end setupGraph

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    if( argc <= 1 )
    {
        readConfFile( "osg.conf" );    // read ConfigFile        1
    }
    else
    {
        readConfFile( argv[1] );    // read ConfigFile        1
    }

    // construct the viewer.
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

    osgViewer::Viewer           viewer;

    osg::ref_ptr<osgText::Text> updateText = new osgText::Text;
    updateText->setDataVariance( osg::Object::DataVariance::DYNAMIC );

    // add the handler for doing the picking
    viewer.addEventHandler( new PickHandler( &viewer, updateText.get() ) );

    // add the stats handler
    viewer.addEventHandler( new osgViewer::StatsHandler );

    osg::Group* root = new osg::Group();

    root->addChild( setupGraph() );

    // add the HUD subgraph.
    root->addChild( createHUD( updateText.get() ) );

    // add model to viewer.
    viewer.setSceneData( root );

    osg::dmat4 lookAt;
    lookAt = osg::lookAt( osg::vec3( 0.0F, -4.0F, 0.0F ),
                          centerScope,
                          osg::vec3( 0.0F, 0.0F, 1.0F ) );

    viewer.getCamera()->setViewMatrix( lookAt );

    viewer.realize();

    while( !viewer.done() )
    {
        // fire off the cull and draw traversals of the scene.
        viewer.frame();
    }

    return 0;
}    // end main
