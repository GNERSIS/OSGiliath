/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * SingleWindow — osgViewer library implementation.
 */
#include <osgViewer/config/SingleWindow.hpp>

#include <osg/core/io_utils.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/state/Stencil.hpp>
#include <osg/textures/Texture1D.hpp>
#include <osg/textures/TextureRectangle.hpp>
#include <osgViewer/core/Renderer.hpp>
#include <osgViewer/core/View.hpp>
#include <osgViewer/handlers/Keystone.hpp>
#include <osgViewer/platform/GraphicsWindow.hpp>

using namespace osgViewer;

void
SingleWindow::configure( osgViewer::View& view ) const
{
    osg::GraphicsContext::WindowingSystemInterface* wsi =
        osg::GraphicsContext::getWindowingSystemInterface();
    if( !wsi )
    {
        OSG_NOTICE << "SingleWindow::configure() : Error, no WindowSystemInterface "
                      "available, cannot create windows."
                   << std::endl;
        return;
    }

    osg::DisplaySettings*                      ds = getActiveDisplaySetting( view );

    osg::ref_ptr<osg::GraphicsContext::Traits> traits =
        new osg::GraphicsContext::Traits( ds );

    traits->readDISPLAY();
    if( traits->displayNum < 0 )
    {
        traits->displayNum = 0;
    }

    traits->screenNum        = static_cast<int>( _screenNum );
    traits->x                = _x;
    traits->y                = _y;
    traits->width            = _width;
    traits->height           = _height;
    traits->windowDecoration = _windowDecoration;
    traits->overrideRedirect = _overrideRedirect;
    traits->doubleBuffer     = true;
    traits->sharedContext    = 0;

    if( traits->width <= 0 || traits->height <= 0 )
    {
        osg::GraphicsContext::ScreenIdentifier si;
        si.readDISPLAY();

        // displayNum has not been set so reset it to 0.
        if( si.displayNum < 0 )
        {
            si.displayNum = 0;
        }

        si.screenNum = static_cast<int>( _screenNum );

        unsigned int width, height;
        wsi->getScreenResolution( si, width, height );
        if( traits->width <= 0 )
        {
            traits->width = static_cast<int>( width );
        }
        if( traits->height <= 0 )
        {
            traits->height = static_cast<int>( height );
        }
    }

    osg::ref_ptr<osg::GraphicsContext> gc =
        osg::GraphicsContext::createGraphicsContext( traits.get() );

    view.getCamera()->setGraphicsContext( gc.get() );

    osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>( gc.get() );
    if( gw )
    {
        OSG_INFO
            << "SingleWindow::configure - GraphicsWindow has been created successfully."
            << std::endl;
        gw->getEventQueue()
            ->getCurrentEventState()
            ->setWindowRectangle( traits->x, traits->y, traits->width, traits->height );
    }
    else
    {
        OSG_NOTICE << "SingleWindow::configure - GraphicsWindow has not been created "
                      "successfully."
                   << std::endl;
        return;
    }

    double fovy, aspectRatio, zNear, zFar;
    view.getCamera()->getProjectionMatrixAsPerspective( fovy, aspectRatio, zNear, zFar );

    double newAspectRatio    = double( traits->width ) / double( traits->height );
    double aspectRatioChange = newAspectRatio / aspectRatio;
    if( aspectRatioChange != 1.0 )
    {
        view.getCamera()->getProjectionMatrix() =
            osg::scale( 1.0 / aspectRatioChange, 1.0, 1.0 ) *
            view.getCamera()->getProjectionMatrix();
    }

    view.getCamera()->setViewport(
        new osg::Viewport( 0, 0, traits->width, traits->height )
    );

    GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;

    view.getCamera()->setDrawBuffer( buffer );
    view.getCamera()->setReadBuffer( buffer );

    if( ds->getKeystoneHint() )
    {
        if( ds->getKeystoneHint() && !ds->getKeystoneFileNames().empty() )
        {
            osgViewer::Keystone::loadKeystoneFiles( ds );
        }
        if( ds->getKeystones().empty() )
        {
            ds->getKeystones().push_back( new Keystone );
        }

        view.assignStereoOrKeystoneToCamera( view.getCamera(), ds );
    }
    else if( ds->getStereo() && ds->getUseSceneViewForStereoHint() )
    {
        view.assignStereoOrKeystoneToCamera( view.getCamera(), ds );
    }
}
