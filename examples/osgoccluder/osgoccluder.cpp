/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgoccluder example application
 */
#include <iostream>
#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Billboard.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/nodes/OccluderNode.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/io/WriteFile.hpp>
#include <osgDB/registry/Registry.hpp>
#include <osgGA/manipulators/DriveManipulator.hpp>
#include <osgGA/manipulators/FlightManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgViewer/core/Viewer.hpp>

class OccluderEventHandler : public osgGA::GUIEventHandler
{
    public:

        OccluderEventHandler( osgViewer::Viewer* viewer ) :
            _viewer( viewer )
        {
        }

        virtual bool
        handle( const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter& );

        void
        addPoint( const osg::vec3& pos );

        void
        endOccluder();

        osg::Group*
        rootNode()
        {
            return dynamic_cast<osg::Group*>( _viewer->getSceneData() );
        }

        osgViewer::Viewer*                      _viewer;
        osg::ref_ptr<osg::Group>                _occluders;
        osg::ref_ptr<osg::ConvexPlanarOccluder> _convexPlanarOccluder;
};

bool
OccluderEventHandler::handle( const osgGA::GUIEventAdapter& ea,
                              osgGA::GUIActionAdapter&      aa )
{
    switch( ea.getEventType() )
    {
        case( osgGA::GUIEventAdapter::KEYDOWN ) :
            {
                if( ea.getKey() == 'a' )

                {
                    osgViewer::View* view = dynamic_cast<osgViewer::View*>( &aa );
                    osgUtil::LineSegmentIntersector::Intersections intersections;
                    if( view && view->computeIntersections( ea, intersections ) )
                    {
                        const osgUtil::LineSegmentIntersector::Intersection& hit =
                            *( intersections.begin() );
                        if( hit.matrix.valid() )
                        {
                            addPoint( osg::vec3( hit.localIntersectionPoint *
                                                 ( *hit.matrix ) ) );
                        }
                        else
                        {
                            addPoint( osg::vec3( hit.localIntersectionPoint ) );
                        }
                    }

                    return true;
                }
                else if( ea.getKey() == 'e' )
                {
                    endOccluder();
                    return true;
                }
                else if( ea.getKey() == 'O' )
                {
                    if( _occluders.valid() )
                    {

                        // .osgt format removed; debug save disabled
                        // if (osgDB::writeNodeFile(*_occluders,"saved_occluders.osgt"))
                        //     std::cout<<"saved occluders to
                        //     'saved_occluders.osgt'"<<std::endl;
                    }
                    else
                    {
                        std::cout << "no occluders to save" << std::endl;
                    }
                    return true;
                }
                return false;
            }

        default :
            return false;
    }
}

void
OccluderEventHandler::addPoint( const osg::vec3& pos )
{
    std::cout << "add point " << pos << std::endl;

    if( !_convexPlanarOccluder.valid() )
    {
        _convexPlanarOccluder = new osg::ConvexPlanarOccluder;
    }

    osg::ConvexPlanarPolygon& occluder = _convexPlanarOccluder->getOccluder();
    occluder.add( pos );

    //
    //     osg::sphere bs = rootNode()->getBound();
    //
    //     osg::ShapeDrawable* sd = new osg::ShapeDrawable(new
    //     osg::Sphere(pos,bs.radius*0.001f)); osg::Geode* geode = new osg::Geode;
    //     geode->addDrawable(sd);
    //
    //     rootNode()->addChild(geode);
    //
}

void
OccluderEventHandler::endOccluder()
{
    if( _convexPlanarOccluder.valid() )
    {
        if( _convexPlanarOccluder->getOccluder().getVertexList().size() >= 3 )
        {
            osg::OccluderNode* occluderNode = new osg::OccluderNode;
            occluderNode->setOccluder( _convexPlanarOccluder.get() );

            if( !_occluders.valid() )
            {
                _occluders = new osg::Group;
                if( rootNode() )
                {
                    rootNode()->addChild( _occluders.get() );
                }
            }
            _occluders->addChild( occluderNode );

            std::cout << "created occluder" << std::endl;
        }
        else
        {
            std::cout << "Occluder requires at least 3 points to create occluder."
                      << std::endl;
        }
    }
    else
    {
        std::cout << "No occluder points to create occluder with." << std::endl;
    }

    // reset current occluder.
    _convexPlanarOccluder = NULL;
}

osg::Node*
createOccluder( const osg::vec3& v1,
                const osg::vec3& v2,
                const osg::vec3& v3,
                const osg::vec3& v4,
                float            holeRatio = -1.0F )
{
    // create an occluder which will sit alongside the loaded model.
    osg::OccluderNode*         occluderNode = new osg::OccluderNode;

    // create the convex planar occluder
    osg::ConvexPlanarOccluder* cpo = new osg::ConvexPlanarOccluder;

    // attach it to the occluder node.
    occluderNode->setOccluder( cpo );
    occluderNode->setName( "occluder" );

    // set the occluder up for the front face of the bounding box.
    osg::ConvexPlanarPolygon& occluder = cpo->getOccluder();
    occluder.add( v1 );
    occluder.add( v2 );
    occluder.add( v3 );
    occluder.add( v4 );

    // create a hole at the center of the occluder if needed.
    if( holeRatio > 0.0F )
    {
        // create hole.
        float                    ratio           = holeRatio;
        float                    one_minus_ratio = 1 - ratio;
        osg::vec3                center          = ( v1 + v2 + v3 + v4 ) * 0.25F;
        osg::vec3                v1dash          = v1 * ratio + center * one_minus_ratio;
        osg::vec3                v2dash          = v2 * ratio + center * one_minus_ratio;
        osg::vec3                v3dash          = v3 * ratio + center * one_minus_ratio;
        osg::vec3                v4dash          = v4 * ratio + center * one_minus_ratio;

        osg::ConvexPlanarPolygon hole;
        hole.add( v1dash );
        hole.add( v2dash );
        hole.add( v3dash );
        hole.add( v4dash );

        cpo->addHole( hole );
    }

    // create a drawable for occluder.
    osg::Geometry*  geom   = new osg::Geometry;

    osg::Vec3Array* coords = new osg::Vec3Array( occluder.getVertexList().begin(),
                                                 occluder.getVertexList().end() );
    geom->setVertexArray( coords );

    osg::Vec4Array* colors = new osg::Vec4Array( 1 );
    ( *colors )[0].set( 1.0F, 1.0F, 1.0F, 0.5F );
    geom->setColorArray( colors, osg::Array::BIND_OVERALL );

    osg::DrawElementsUShort* indices =
        new osg::DrawElementsUShort( osg::PrimitiveSet::TRIANGLES );
    indices->push_back( 0 );
    indices->push_back( 1 );
    indices->push_back( 2 );
    indices->push_back( 0 );
    indices->push_back( 2 );
    indices->push_back( 3 );
    geom->addPrimitiveSet( indices );

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable( geom );

    osg::StateSet* stateset = new osg::StateSet;
    stateset->setMode( GL_BLEND, osg::StateAttribute::ON );
    stateset->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

    geom->setStateSet( stateset );

    // add the occluder geode as a child of the occluder,
    // as the occluder can't self occlude its subgraph the
    // geode will never be occluded by this occluder.
    occluderNode->addChild( geode );

    return occluderNode;
}

osg::Group*
createOccludersAroundModel( osg::Node* model )
{
    osg::Group* scene = new osg::Group;
    scene->setName( "rootgroup" );

    // add the loaded model into a the scene group.
    scene->addChild( model );
    model->setName( "model" );

    // get the bounding volume of the model.
    const osg::sphere bs = model->getBound();

    // create a bounding box around the sphere.
    osg::box          bb;
    bb.expandBy( bs );

    // front
    scene->addChild(
        createOccluder( bb.corner( 0 ), bb.corner( 1 ), bb.corner( 5 ), bb.corner( 4 ) )
    );

    // right side
    scene->addChild(
        createOccluder( bb.corner( 1 ), bb.corner( 3 ), bb.corner( 7 ), bb.corner( 5 ) )
    );

    // left side
    scene->addChild(
        createOccluder( bb.corner( 2 ), bb.corner( 0 ), bb.corner( 4 ), bb.corner( 6 ) )
    );

    // back side
    scene->addChild(
        createOccluder( bb.corner( 3 ),
                        bb.corner( 2 ),
                        bb.corner( 6 ),
                        bb.corner( 7 ),
                        0.5F )
    );    // create a hole half the size of the occluder.

    return scene;
}

int
main( int    argc,
      char** argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc, argv );

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(
        arguments.getApplicationName() +
        " is the example which demonstrates use of convex planer occluders."
    );
    arguments.getApplicationUsage()->setCommandLineUsage(
        arguments.getApplicationName() + " [options] filename ..."
    );
    arguments.getApplicationUsage()->addCommandLineOption( "-h or --help",
                                                           "Display this information" );
    arguments.getApplicationUsage()->addCommandLineOption( "-m",
                                                           "Manually create occluders" );

    bool manuallyCreateOccluders = false;
    while( arguments.read( "-m" ) )
    {
        manuallyCreateOccluders = true;
    }

    // if user requests help write it out to cout.
    if( arguments.read( "-h" ) || arguments.read( "--help" ) )
    {
        arguments.getApplicationUsage()->write( std::cout );
        return 1;
    }

    // load the nodes from the commandline arguments.
    osg::ref_ptr<osg::Node> loadedmodel = osgDB::readRefNodeFiles( arguments );

    // if not loaded assume no arguments passed in, try using default mode instead.
    if( !loadedmodel )
    {
        loadedmodel = osgDB::readRefNodeFile( "fox.glb" );
    }

    if( !loadedmodel )
    {
        osg::notify( osg::NOTICE )
            << "Please specify a model filename on the command line." << std::endl;
        return 1;
    }

    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize( loadedmodel );

    // add the occluders to the loaded model.
    osg::ref_ptr<osg::Group> rootnode;

    if( manuallyCreateOccluders )
    {
        rootnode = new osg::Group;
        rootnode->addChild( loadedmodel );
    }
    else
    {
        rootnode = createOccludersAroundModel( loadedmodel.get() );
    }

    // Headless capture: early check before viewer construction
    {
        std::string headlessOutput;
        if( arguments.read( "--headless", headlessOutput ) )
        {
            auto node = osgDB::readRefNodeFiles( arguments );
            if( !node )
            {
                node = osgDB::readRefNodeFile( "fox.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    // initialize the viewer.
    osgViewer::Viewer viewer;

    if( manuallyCreateOccluders )
    {
        viewer.addEventHandler( new OccluderEventHandler( &viewer ) );
    }

    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData( rootnode );
    return viewer.run();
}
