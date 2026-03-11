#include <osgFX/Cartoon>
#include <osgFX/Registry>

#include <osg/PolygonOffset>
#include <osg/Texture1D>
#include <osg/PolygonMode>
#include <osg/CullFace>
#include <osg/Image>
#include <osg/LineWidth>
#include <osg/Material>
#include <osg/Program>
#include <osg/Shader>


#include <sstream>

using namespace osgFX;

namespace
{

    osg::Image* create_sharp_lighting_map(int levels = 4, int texture_size = 16)
    {
        osg::ref_ptr<osg::Image> image = new osg::Image;
        image->setImage(texture_size, 1, 1, 4, GL_RGBA, GL_UNSIGNED_BYTE, new unsigned char[4*texture_size], osg::Image::USE_NEW_DELETE);
        for (int i=0; i<texture_size; ++i) {
            float c = i/static_cast<float>(texture_size);
            c = (1+static_cast<int>(sqrtf(c) * (levels))) / static_cast<float>(levels+1);
            *(image->data(i, 0)+0) = static_cast<unsigned char>(c*255);
            *(image->data(i, 0)+1) = static_cast<unsigned char>(c*255);
            *(image->data(i, 0)+2) = static_cast<unsigned char>(c*255);
            *(image->data(i, 0)+3) = 255;
        }
        return image.release();
    }

}


namespace
{

    // register a prototype for this effect
    Registry::Proxy proxy(new Cartoon);

}

///////////////////////////////////////////////////////////////////////////
// A port of Marco Jez's "cartoon.cg" to the OpenGL Shading Language
// by Mike Weiblen 2003-10-03,
//
// This shader is simplified due to limitations in the OGLSL implementation
// in the current 3Dlabs driver.  As the OGLSL implementation improves,
// need to revisit and enhance this shader.
namespace
{
    class OGLSL_Technique : public Technique {
    public:
        OGLSL_Technique(osg::Material* wf_mat, osg::LineWidth *wf_lw, int lightnum)
            : Technique(), _wf_mat(wf_mat), _wf_lw(wf_lw), _lightnum(lightnum) {}

        void getRequiredExtensions(std::vector<std::string>& extensions) const
        {
            extensions.push_back( "GL_ARB_shader_objects" );
            extensions.push_back( "GL_ARB_vertex_shader" );
            extensions.push_back( "GL_ARB_fragment_shader" );
        }

    protected:

        void define_passes()
        {
            // implement pass #1 (solid surfaces)
            {
                std::ostringstream vert_source;
                vert_source <<
                "#version 460 core\n"
                "layout(location = 0) in vec4 osg_Vertex;\n"
                "layout(location = 2) in vec3 osg_Normal;\n"
                "uniform mat4 osg_ModelViewMatrix;\n"
                "uniform mat4 osg_ModelViewProjectionMatrix;\n"
                "uniform mat3 osg_NormalMatrix;\n"
                "struct osg_LightSourceParameters {\n"
                "    vec4 ambient;\n"
                "    vec4 diffuse;\n"
                "    vec4 specular;\n"
                "    vec4 position;\n"
                "    vec3 spotDirection;\n"
                "    float spotExponent;\n"
                "    float spotCutoff;\n"
                "    float spotCosCutoff;\n"
                "    float constantAttenuation;\n"
                "    float linearAttenuation;\n"
                "    float quadraticAttenuation;\n"
                "};\n"
                "uniform osg_LightSourceParameters osg_LightSource;\n"
                "out float CartoonTexCoord;\n"
                "void main( void )\n"
                "{\n"
                    "    vec4 LightPosition = osg_LightSource.position;\n"
                    "    vec3 LightDirection;\n"
                    "    if (LightPosition[3]!=0.0) { \n"
                    "        vec4 eye_space_position = osg_ModelViewMatrix * osg_Vertex;\n"
                    "        LightDirection = (LightPosition.xyz-eye_space_position.xyz);\n"
                    "    } else {\n"
                    "        LightDirection = LightPosition.xyz;\n"
                    "    }\n"
                    "    vec3 eye_space_normal = normalize(osg_NormalMatrix * osg_Normal);\n"
                    "    CartoonTexCoord = max(0.0, dot(normalize(LightDirection), eye_space_normal));\n"
                    "    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
                "}\n";

                const char * frag_source =
                "#version 460 core\n"
                "uniform sampler1D CartoonTexUnit;"
                "in float CartoonTexCoord;"
                "out vec4 fragColor;"
                "void main( void )"
                "{"
                    "fragColor = texture( CartoonTexUnit, CartoonTexCoord );"
                "}";

                osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;

                osg::ref_ptr<osg::PolygonOffset> polyoffset = new osg::PolygonOffset;
                polyoffset->setFactor(1.0f);
                polyoffset->setUnits(1.0f);
                ss->setAttributeAndModes(polyoffset.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

                osg::ref_ptr<osg::Program> program = new osg::Program;
                program->addShader( new osg::Shader( osg::Shader::VERTEX, vert_source.str() ) );
                program->addShader( new osg::Shader( osg::Shader::FRAGMENT, frag_source ) );

                ss->addUniform( new osg::Uniform("CartoonTexUnit", 0));
                ss->setAttributeAndModes( program.get(), osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);


                osg::ref_ptr<osg::Texture1D> texture = new osg::Texture1D;
                texture->setImage(create_sharp_lighting_map());
                texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
                texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
                ss->setTextureAttributeAndModes(0, texture.get(), osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);

                addPass(ss.get());
            }

            // implement pass #2 (outlines)
            {
                osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;
                osg::ref_ptr<osg::PolygonMode> polymode = new osg::PolygonMode;
                polymode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
                ss->setAttributeAndModes(polymode.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

                osg::ref_ptr<osg::CullFace> cf = new osg::CullFace;
                cf->setMode(osg::CullFace::FRONT);
                ss->setAttributeAndModes(cf.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

                ss->setAttributeAndModes(_wf_lw.get(), osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);

                _wf_mat->setColorMode(osg::Material::OFF);
                _wf_mat->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
                _wf_mat->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
                _wf_mat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));

                // set by outline colour so no need to set here.
                //_wf_mat->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));

                ss->setAttributeAndModes(_wf_mat.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

                addPass(ss.get());

            }
        }

    private:
        osg::ref_ptr<osg::Material> _wf_mat;
        osg::ref_ptr<osg::LineWidth> _wf_lw;
        int _lightnum;
    };

}

///////////////////////////////////////////////////////////////////////////

Cartoon::Cartoon()
:    Effect(),
    _wf_mat(new osg::Material),
    _wf_lw(new osg::LineWidth(2.0f)),
    _lightnum(0)
{
    setOutlineColor(osg::Vec4(0, 0, 0, 1));
}

Cartoon::Cartoon(const Cartoon& copy, const osg::CopyOp& copyop)
:    Effect(copy, copyop),
    _wf_mat(static_cast<osg::Material* >(copyop(copy._wf_mat.get()))),
    _wf_lw(static_cast<osg::LineWidth *>(copyop(copy._wf_lw.get()))),
    _lightnum(copy._lightnum)
{
}

bool Cartoon::define_techniques()
{
    addTechnique(new OGLSL_Technique(_wf_mat.get(), _wf_lw.get(), _lightnum));
    return true;
}
