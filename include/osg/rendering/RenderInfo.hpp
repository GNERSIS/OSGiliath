/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Per-draw state passed to Drawable::drawImplementation. Provides
 * access to the current State, View, and graphics context.
 */
#pragma once

#include <osg/nodes/View.hpp>
#include <osg/state/State.hpp>

namespace osgUtil
{

    // forward declare RenderBin so we can refer to it in RenderInfo
    class RenderBin;

}

namespace osg
{

    class RenderInfo
    {
        public:

            RenderInfo() :
                _view( 0 )
            {
            }

            RenderInfo( const RenderInfo& rhs ) :
                _state( rhs._state ),
                _view( rhs._view ),
                _cameraStack( rhs._cameraStack ),
                _renderBinStack( rhs._renderBinStack ),
                _userData( rhs._userData )
            {
            }

            RenderInfo( State* state,
                        View*  view ) :
                _state( state ),
                _view( view )
            {
            }

            RenderInfo&
            operator=( const RenderInfo& rhs )
            {
                _state          = rhs._state;
                _view           = rhs._view;
                _cameraStack    = rhs._cameraStack;
                _renderBinStack = rhs._renderBinStack;
                _userData       = rhs._userData;
                return *this;
            }

            unsigned int
            getContextID() const
            {
                return _state.valid() ? _state->getContextID() : 0;
            }

            void
            setState( State* state )
            {
                _state = state;
            }

            State*
            getState()
            {
                return _state.get();
            }

            const State*
            getState() const
            {
                return _state.get();
            }

            void
            setView( View* view )
            {
                _view = view;
            }

            View*
            getView()
            {
                return _view;
            }

            const View*
            getView() const
            {
                return _view;
            }

            void
            pushCamera( Camera* camera )
            {
                _cameraStack.push_back( camera );
            }

            void
            popCamera()
            {
                if( !_cameraStack.empty() )
                {
                    _cameraStack.pop_back();
                }
            }

            typedef std::vector<Camera*> CameraStack;

            CameraStack&
            getCameraStack()
            {
                return _cameraStack;
            }

            Camera*
            getCurrentCamera()
            {
                return _cameraStack.empty() ? 0 : _cameraStack.back();
            }

            void
            pushRenderBin( osgUtil::RenderBin* bin )
            {
                _renderBinStack.push_back( bin );
            }

            void
            popRenderBin()
            {
                _renderBinStack.pop_back();
            }

            typedef std::vector<osgUtil::RenderBin*> RenderBinStack;

            RenderBinStack&
            getRenderBinStack()
            {
                return _renderBinStack;
            }

            void
            setUserData( Referenced* userData )
            {
                _userData = userData;
            }

            Referenced*
            getUserData()
            {
                return _userData.get();
            }

            const Referenced*
            getUserData() const
            {
                return _userData.get();
            }

        protected:

            ref_ptr<State>      _state;
            View*               _view;
            CameraStack         _cameraStack;
            RenderBinStack      _renderBinStack;
            ref_ptr<Referenced> _userData;
    };

}
