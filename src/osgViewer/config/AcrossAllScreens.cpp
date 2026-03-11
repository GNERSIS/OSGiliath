/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osgViewer: SingleScreen.
 */
#include <osgViewer/config/AcrossAllScreens>

#include <osg/core/io_utils.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osgViewer/config/SingleScreen>
#include <osgViewer/core/Renderer.hpp>
#include <osgViewer/core/View.hpp>
#include <osgViewer/platform/GraphicsWindow.hpp>

using namespace osgViewer;

void
AcrossAllScreens::configure( osgViewer::View& view ) const
{
    osg::GraphicsContext::WindowingSystemInterface* wsi =
        osg::GraphicsContext::getWindowingSystemInterface();
    if( !wsi )
    {
        OSG_NOTICE << "AcrossAllScreens::configure() : Error, no WindowSystemInterface "
                      "available, cannot create windows."
                   << std::endl;
        return;
    }

    osg::DisplaySettings* ds = getActiveDisplaySetting( view );

    double                fovy, aspectRatio, zNear, zFar;
    view.getCamera()->getProjectionMatrixAsPerspective( fovy, aspectRatio, zNear, zFar );

    osg::GraphicsContext::ScreenIdentifier si;
    si.readDISPLAY();

    // displayNum has not been set so reset it to 0.
    if( si.displayNum < 0 )
    {
        si.displayNum = 0;
    }

    unsigned int numScreens = wsi->getNumScreens( si );
    if( numScreens == 1 )
    {
        osg::ref_ptr<SingleScreen> ss = new SingleScreen( 0 );
        ss->configure( view );
    }
    else
    {

        double translate_x = 0.0;

        for( unsigned int i = 0; i < numScreens; ++i )
        {
            si.screenNum = static_cast<int>( i );

            unsigned int width, height;
            wsi->getScreenResolution( si, width, height );
            translate_x += double( width ) / ( double( height ) * aspectRatio );
        }

        bool stereoSplitScreens = numScreens ==
                                  2 &&
                                  ds->getStereoMode() ==
                                  osg::DisplaySettings::HORIZONTAL_SPLIT &&
                                  ds->getStereo();

        for( unsigned int i = 0; i < numScreens; ++i )
        {
            si.screenNum = static_cast<int>( i );

            unsigned int width, height;
            wsi->getScreenResolution( si, width, height );

            osg::ref_ptr<osg::GraphicsContext::Traits> traits =
                new osg::GraphicsContext::Traits( ds );
            traits->hostName         = si.hostName;
            traits->displayNum       = si.displayNum;
            traits->screenNum        = si.screenNum;
            traits->screenNum        = static_cast<int>( i );
            traits->x                = 0;
            traits->y                = 0;
            traits->width            = static_cast<int>( width );
            traits->height           = static_cast<int>( height );
            traits->windowDecoration = false;
            traits->doubleBuffer     = true;
            traits->sharedContext    = 0;

            osg::ref_ptr<osg::GraphicsContext> gc =
                osg::GraphicsContext::createGraphicsContext( traits.get() );

            osg::ref_ptr<osg::Camera> camera = new osg::Camera;
            camera->setGraphicsContext( gc.get() );

            osgViewer::GraphicsWindow* gw =
                dynamic_cast<osgViewer::GraphicsWindow*>( gc.get() );
            if( gw )
            {
                OSG_INFO << "  GraphicsWindow has been created successfully." << gw
                         << std::endl;

                gw->getEventQueue()->getCurrentEventState()->setWindowRectangle(
                    traits->x,
                    traits->y,
                    traits->width,
                    traits->height
                );
            }
            else
            {
                OSG_NOTICE << "  GraphicsWindow has not been created successfully."
                           << std::endl;
            }

            camera->setViewport(
                new osg::Viewport( 0, 0, traits->width, traits->height )
            );

            GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
            camera->setDrawBuffer( buffer );
            camera->setReadBuffer( buffer );

            if( stereoSplitScreens )
            {
                unsigned int leftCameraNum =
                    ( ds->getSplitStereoHorizontalEyeMapping() ==
                      osg::DisplaySettings::LEFT_EYE_LEFT_VIEWPORT )
                        ? 0
                        : 1;

                osg::ref_ptr<osg::DisplaySettings> ds_local =
                    new osg::DisplaySettings( *ds );
                ds_local->setStereoMode( leftCameraNum == i
                                             ? osg::DisplaySettings::LEFT_EYE
                                             : osg::DisplaySettings::RIGHT_EYE );
                camera->setDisplaySettings( ds_local.get() );

                view.addSlave( camera.get(), osg::dmat4(), osg::dmat4() );
            }
            else
            {
                double newAspectRatio =
                    double( traits->width ) / double( traits->height );
                double aspectRatioChange = newAspectRatio / aspectRatio;

                view.addSlave(
                    camera.get(),
                    osg::translate( translate_x - aspectRatioChange, 0.0, 0.0 ) *
                        osg::scale( 1.0 / aspectRatioChange, 1.0, 1.0 ),
                    osg::dmat4()
                );
                translate_x -= aspectRatioChange * 2.0;
            }
        }
    }

    view.assignSceneDataToCameras();
}
