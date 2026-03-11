#pragma once

#include <osgDB/cache/DatabasePager.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/registry/Registry.hpp>
#include <osgGA/manipulators/KeySwitchMatrixManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgViewer/api/win32/GraphicsWindowWin32>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>
#include <string>

class cOSG
{
    public:

        cOSG( HWND hWnd );
        ~cOSG();

        void
        InitOSG( std::string filename );
        void
        InitManipulators( void );
        void
        InitSceneGraph( void );
        void
        InitCameraConfig( void );
        void
        SetupWindow( void );
        void
        SetupCamera( void );
        void
        PreFrameUpdate( void );
        void
        PostFrameUpdate( void );

        void
        Done( bool value )
        {
            mDone = value;
        }

        bool
        Done( void )
        {
            return mDone;
        }

        // static void Render(void* ptr);

        osgViewer::Viewer*
        getViewer()
        {
            return mViewer;
        }

    private:

        bool                                            mDone;
        std::string                                     m_ModelName;
        HWND                                            m_hWnd;
        osgViewer::Viewer*                              mViewer;
        osg::ref_ptr<osg::Group>                        mRoot;
        osg::ref_ptr<osg::Node>                         mModel;
        osg::ref_ptr<osgGA::TrackballManipulator>       trackball;
        osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator;
};

class CRenderingThread : public osg::Thread
{
    public:

        CRenderingThread( cOSG* ptr );
        virtual ~CRenderingThread();

        virtual void
        run();

    protected:

        cOSG* _ptr;
        bool  _done;
};
