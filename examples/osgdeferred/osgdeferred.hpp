#include <osg/maths/compat.hpp>
#include <osg/nodes/Camera.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgShadow/ShadowedScene>

struct Pipeline
{
        int                      textureSize;
        osg::ref_ptr<osg::Group> graph;
        osg::Texture*            pass1Shadows;
        osg::Texture*            pass2Colors;
        osg::Texture*            pass2Normals;
        osg::Texture*            pass2Positions;
        osg::Texture*            pass3Final;
};

osg::Texture2D*
createFloatTexture2D();

osg::Camera*
createHUDCamera( double left   = 0,
                 double right  = 1,
                 double bottom = 0,
                 double top    = 1 );

osg::ref_ptr<osg::LightSource>
createLight( const osg::vec3& pos );

Pipeline
createPipelineEffectCompositor( osg::ref_ptr<osg::Group>               scene,
                                osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene,
                                const osg::vec3                        lightPos );

Pipeline
createPipelinePlainOSG( osg::ref_ptr<osg::Group>               scene,
                        osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene,
                        const osg::vec3                        lightPos );

osg::Camera*
createRTTCamera( osg::Camera::BufferComponent buffer,
                 osg::Texture*                tex,
                 bool                         isAbsolute = false );

osg::ref_ptr<osg::Group>
createSceneRoom();

osg::Geode*
createScreenQuad( float     width,
                  float     height,
                  float     scale  = 1,
                  osg::vec3 corner = osg::vec3() );

osg::Texture2D*
createTexture( const std::string& fileName );

osg::ref_ptr<osg::Camera>
createTextureDisplayQuad( const osg::vec3&     pos,
                          osg::StateAttribute* tex,
                          float                scale,
                          float                width  = 0.3,
                          float                height = 0.2 );

void
setAnimationPath( osg::ref_ptr<osg::MatrixTransform> node,
                  const osg::vec3&                   center,
                  float                              time,
                  float                              radius );

osg::ref_ptr<osg::StateSet>
setShaderProgram( osg::ref_ptr<osg::Camera> pass,
                  const std::string&        vert,
                  const std::string&        frag );
