/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgshaders example application
 */
/* file:        examples/osgglsl/osgshaders.cpp
 * author:        Mike Weiblen 2005-04-05
 *
 * A demo of the OpenGL Shading Language shaders using core OSG.
 *
 * See http://www.3dlabs.com/opengl2/ for more information regarding
 * the OpenGL Shading Language.
 */

#include "GL2Scene.hpp"

#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/events/GUIActionAdapter.hpp>
#include <osgGA/events/GUIEventAdapter.hpp>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgViewer/core/Viewer.hpp>

using namespace osg;

///////////////////////////////////////////////////////////////////////////

class KeyHandler : public osgGA::GUIEventHandler
{
    public:

        KeyHandler( GL2ScenePtr gl2Scene ) :
            _gl2Scene( gl2Scene )
        {
        }

        bool
        handle( const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter& )
        {
            if( ea.getEventType() != osgGA::GUIEventAdapter::KEYDOWN )
            {
                return false;
            }

            switch( ea.getKey() )
            {
                case 'x' :
                    _gl2Scene->reloadShaderSource();
                    return true;
                case 'y' :
                    _gl2Scene->toggleShaderEnable();
                    return true;
            }
            return false;
        }

    private:

        GL2ScenePtr _gl2Scene;
};

///////////////////////////////////////////////////////////////////////////

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

    osgViewer::Viewer viewer;

    // create the scene
    GL2ScenePtr       gl2Scene = new GL2Scene;

    viewer.setSceneData( gl2Scene->getRootNode().get() );

    viewer.addEventHandler( new KeyHandler( gl2Scene ) );
    return viewer.run();
}

/*EOF*/
