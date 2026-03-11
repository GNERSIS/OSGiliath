/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Automatic shader generation visitor. Creates GLSL shaders
 * matching the fixed-function state set on each Drawable.
 */
/**
 * \brief    Shader generator framework.
 * \author   Maciej Krol
 */

#include <osgUtil/utils/ShaderGen.hpp>

#include "../shaders/shadergen_frag.cpp"
#include "../shaders/shadergen_vert.cpp"

#include <osg/geometry/Geometry.hpp>    // for ShaderGenVisitor::update
#include <osg/nodes/Geode.hpp>
#include <osg/textures/TexGen.hpp>
#include <sstream>

using namespace osgUtil;

namespace osgUtil
{

    ShaderGenVisitor::ShaderGenVisitor() :
        osg::DualModeVisitor( osg::DualModeVisitor::TRAVERSE_ALL_CHILDREN )
    {
    }

    void
    ShaderGenVisitor::assignUberProgram( osg::StateSet* stateSet )
    {
        if( stateSet )
        {
            osg::ref_ptr<osg::Program> uberProgram = new osg::Program;
            uberProgram->addShader( new osg::Shader( osg::Shader::VERTEX,
                                                     shadergen_vert ) );
            uberProgram->addShader( new osg::Shader( osg::Shader::FRAGMENT,
                                                     shadergen_frag ) );

            stateSet->setAttribute( uberProgram.get() );
            stateSet->addUniform( new osg::Uniform( "diffuseMap", 0 ) );

            remapStateSet( stateSet );
        }
    }

    void
    ShaderGenVisitor::apply( osg::Node& node )
    {
        osg::StateSet* stateSet = node.getStateSet();
        if( stateSet )
        {
            remapStateSet( stateSet );
        }

        traverse( node );
    }

    void
    ShaderGenVisitor::remapStateSet( osg::StateSet* stateSet )
    {
        if( !stateSet )
        {
            return;
        }

        // remove any modes that won't be appropriate when using shaders, and remap them
        // to the appropriate Uniform/Define combination

        if( !stateSet->getTextureModeList().empty() )
        {
            osg::StateSet::ModeList& textureModes = stateSet->getTextureModeList()[0];

            if( textureModes.count( GL_TEXTURE_2D ) > 0 )
            {
                osg::StateAttribute::GLModeValue textureMode =
                    textureModes[GL_TEXTURE_2D];
                stateSet->removeTextureMode( 0, GL_TEXTURE_2D );
                stateSet->setDefine( "OSG_TEXTURE_2D", textureMode );
            }

            // Check for TexGen sphere-map mode
            if( textureModes.count( GL_TEXTURE_GEN_S ) > 0 )
            {
                osg::StateAttribute::GLModeValue texgenMode =
                    textureModes[GL_TEXTURE_GEN_S];
                stateSet->removeTextureMode( 0, GL_TEXTURE_GEN_S );
                stateSet->setDefine( "OSG_TEXGEN_SPHERE_MAP", texgenMode );
            }
        }
    }

}    // namespace osgUtil
