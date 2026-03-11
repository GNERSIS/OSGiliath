/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * GameOfLifePass example application
 */
#ifndef GAMEOFLIFEPASS_H
    #define GAMEOFLIFEPASS_H 1

    #include <osg/core/ref_ptr.hpp>
    #include <osg/geometry/Geometry.hpp>
    #include <osg/maths/compat.hpp>
    #include <osg/nodes/Camera.hpp>
    #include <osg/nodes/Geode.hpp>
    #include <osg/nodes/Group.hpp>
    #include <osg/nodes/Switch.hpp>
    #include <osg/textures/Texture2D.hpp>
    #include <osg/textures/TextureRectangle.hpp>

class ProcessPass
{
    public:

        ProcessPass( osg::TextureRectangle* in_tex,
                     osg::TextureRectangle* out_tex,
                     int                    width,
                     int                    height );
        ~ProcessPass();

        osg::ref_ptr<osg::Group>
        getRoot()
        {
            return _RootGroup;
        }

        osg::ref_ptr<osg::TextureRectangle>
        getOutputTexture()
        {
            return _OutTexture;
        }

        void
        setShader( std::string filename );

    private:

        osg::ref_ptr<osg::Group>
        createTexturedQuad();
        void
                                            setupCamera();

        osg::ref_ptr<osg::Group>            _RootGroup;
        osg::ref_ptr<osg::Camera>           _Camera;
        osg::ref_ptr<osg::TextureRectangle> _InTexture;
        osg::ref_ptr<osg::TextureRectangle> _OutTexture;
        int                                 _TextureWidth;
        int                                 _TextureHeight;
        osg::ref_ptr<osg::Program>          _FragmentProgram;
        osg::ref_ptr<osg::StateSet>         _StateSet;
};

class GameOfLifePass
{
    public:

        GameOfLifePass( osg::Image* in_image );
        ~GameOfLifePass();

        osg::ref_ptr<osg::Group>
        getRoot()
        {
            return _RootGroup;
        }

        osg::ref_ptr<osg::TextureRectangle>
        getOutputTexture();
        void
        setShader( std::string filename );
        // Switch branches so we flip textures
        void
        flip();

    private:

        osg::ref_ptr<osg::Group>
        createTexturedQuad();
        void
        setupCamera();
        void
        createOutputTextures();
        void
                                            activateBranch();

        osg::ref_ptr<osg::Group>            _RootGroup;
        osg::ref_ptr<osg::Camera>           _Camera;
        osg::ref_ptr<osg::TextureRectangle> _InOutTextureLife[2];
        int                                 _TextureWidth;
        int                                 _TextureHeight;
        int                                 _ActiveBranch;
        osg::ref_ptr<osg::Program>          _FragmentProgram;
        osg::ref_ptr<osg::StateSet>         _StateSet;
        osg::ref_ptr<osg::Switch>           _BranchSwith[2];
        ProcessPass*                        _ProcessPass[2];
};

#endif    // GAMEOFLIFEPASS_H
