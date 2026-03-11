/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract windowed graphics context. Extends GraphicsContext with
 * window management, event queue, and input handling.
 */
#include <osgViewer/platform/GraphicsWindow.hpp>

#include <osgViewer/core/View.hpp>
#include <osgViewer/core/ViewerBase.hpp>

using namespace osgViewer;

void
GraphicsWindow::getViews( Views& views )
{
    views.clear();
    osgViewer::View* prev = NULL;

    for( Cameras::iterator it = _cameras.begin(); it != _cameras.end(); it++ )
    {
        osgViewer::View* v = dynamic_cast<osgViewer::View*>( ( *it )->getView() );
        if( v )
        {
            // perform a simple test to reduce the number of duplicates
            if( v != prev )
            {
                // append view
                views.push_back( v );
            }
        }
    }

    // remove duplicates
    views.sort();
    views.unique();
}

void
GraphicsWindow::requestRedraw()
{
    Views views;
    getViews( views );

    if( views.empty() )
    {
        OSG_INFO << "GraphicsWindow::requestRedraw(): No views assigned yet."
                 << std::endl;
        return;
    }

    for( Views::iterator it = views.begin(); it != views.end(); it++ )
    {
        ( *it )->requestRedraw();
    }
}
