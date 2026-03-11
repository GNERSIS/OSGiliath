/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osggraphicscost example application
 */
#include <osg/maths/compat.hpp>
#include <osg/rendering/GraphicsCostEstimator.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

class CalibrateCostEsimator : public osg::GraphicsOperation
{
    public:

        CalibrateCostEsimator( osg::GraphicsCostEstimator* gce ) :
            osg::Referenced( true ),
            osg::GraphicsOperation( "CalbirateCostEstimator",
                                    false ),
            _gce( gce )
        {
        }

        virtual void
        operator()( osg::GraphicsContext* context )
        {
            osg::RenderInfo renderInfo( context->getState(), 0 );
            _gce->calibrate( renderInfo );
        }

        osg::ref_ptr<osg::GraphicsCostEstimator> _gce;
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

    osgViewer::Viewer       viewer( arguments );

    osg::ref_ptr<osg::Node> node = osgDB::readRefNodeFiles( arguments );
    if( !node )
    {
        return 0;
    }

    osg::ref_ptr<osg::GraphicsCostEstimator> gce = new osg::GraphicsCostEstimator;

    viewer.setSceneData( node );

    viewer.realize();

    osg::CostPair compileCost = gce->estimateCompileCost( node.get() );
    osg::CostPair drawCost    = gce->estimateDrawCost( node.get() );

    OSG_NOTICE << "estimateCompileCost(" << node->getName()
               << "), CPU=" << compileCost.first << " GPU=" << compileCost.second
               << std::endl;
    OSG_NOTICE << "estimateDrawCost(" << node->getName() << "), CPU=" << drawCost.first
               << " GPU=" << drawCost.second << std::endl;
    return viewer.run();
}
