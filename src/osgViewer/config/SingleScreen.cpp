/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osgViewer: SingleWindow.
 */
#include <osgViewer/config/SingleScreen>

#include <osg/core/io_utils.hpp>
#include <osgViewer/config/SingleWindow>
#include <osgViewer/core/Renderer.hpp>
#include <osgViewer/core/View.hpp>
#include <osgViewer/platform/GraphicsWindow.hpp>

using namespace osgViewer;

void
SingleScreen::configure( osgViewer::View& view ) const
{
    osg::ref_ptr<osgViewer::SingleWindow> singleWindow =
        new SingleWindow( 0, 0, -1, -1, _screenNum );
    singleWindow->setWindowDecoration( false );
    singleWindow->configure( view );
}
