/* OpenSceneGraph example, oldshadercomposition.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osgDB/ReadFile>
#include <osg/ShaderAttribute>
#include <osg/PositionAttitudeTransform>

osg::Node* createOldShaderCompositionScene(osg::ArgumentParser& arguments)
{
    osg::ref_ptr<osg::Node> node = osgDB::readRefNodeFiles(arguments);
    if (!node) node = osgDB::readRefNodeFile("cow.osgt");
    if (!node)
    {
        OSG_NOTICE<<"No data loaded"<<std::endl;
        return 0;
    }

    osg::Group* group = new osg::Group;
    double spacing = node->getBound().radius() * 2.0;

    osg::Vec3d position(0.0,0.0,0.0);

    osg::ShaderAttribute* sa1 = NULL;

    {
        osg::StateSet* stateset = group->getOrCreateStateSet();
        osg::ShaderAttribute* sa = new osg::ShaderAttribute;
        sa->setType(osg::StateAttribute::Type(10000));
        sa1 = sa;
        stateset->setAttribute(sa);

        {
            osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX);
            vertex_shader->addCodeInjection(-1,"#version 460 core\n");
            vertex_shader->addCodeInjection(-1,"in vec4 osg_Vertex;\n");
            vertex_shader->addCodeInjection(-1,"in vec4 osg_Color;\n");
            vertex_shader->addCodeInjection(-1,"in vec4 osg_MultiTexCoord0;\n");
            vertex_shader->addCodeInjection(-1,"uniform mat4 osg_ModelViewProjectionMatrix;\n");
            vertex_shader->addCodeInjection(-1,"out vec4 color;\n");
            vertex_shader->addCodeInjection(-1,"out vec4 texcoord;\n");
            vertex_shader->addCodeInjection(0,"color = osg_Color;\n");
            vertex_shader->addCodeInjection(0,"texcoord = osg_MultiTexCoord0;\n");
            vertex_shader->addCodeInjection(0,"gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n");
            sa->addShader(vertex_shader);
        }

        {
            osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT);
            fragment_shader->addCodeInjection(-1,"#version 460 core\n");
            fragment_shader->addCodeInjection(-1,"in vec4 color;\n");
            fragment_shader->addCodeInjection(-1,"in vec4 texcoord;\n");
            fragment_shader->addCodeInjection(-1,"uniform sampler2D baseTexture; \n");
            fragment_shader->addCodeInjection(-1,"out vec4 osg_FragColor;\n");
            fragment_shader->addCodeInjection(0,"osg_FragColor = color * texture( baseTexture, texcoord.xy );\n");

            sa->addShader(fragment_shader);
        }

        sa->addUniform(new osg::Uniform("baseTexture",0));

    }

    // inherit the ShaderComponents entirely from above
    {
        osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
        pat->setPosition(position);
        pat->addChild(node);

        position.x() += spacing;

        group->addChild(pat);

    }

    {
        osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
        pat->setPosition(position);
        pat->addChild(node);

        position.x() += spacing;

        osg::StateSet* stateset = pat->getOrCreateStateSet();
        stateset->setMode(GL_BLEND, osg::StateAttribute::ON);

        osg::ShaderAttribute* sa = new osg::ShaderAttribute;
        sa->setType(osg::StateAttribute::Type(10001));
        stateset->setAttribute(sa);

        {
            osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT);
            fragment_shader->addCodeInjection(0.9f,"osg_FragColor.a = osg_FragColor.a*0.5;\n");

            sa->addShader(fragment_shader);
        }

        group->addChild(pat);
    }

    // reuse the first ShaderAttribute's type and ShaderComponent, just use new uniform
    {
        osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
        pat->setPosition(position);
        pat->addChild(node);

        position.x() += spacing;

        osg::StateSet* stateset = pat->getOrCreateStateSet();
        osg::ShaderAttribute* sa = new osg::ShaderAttribute(*sa1);
        stateset->setAttribute(sa);

        // reuse the same ShaderComponent as the first branch
        sa->addUniform(new osg::Uniform("myColour",osg::Vec4(1.0f,1.0f,0.0f,1.0f)));

        group->addChild(pat);

    }


    // reuse the first ShaderAttribute's type and ShaderComponent, just use new uniform
    {
        osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
        pat->setPosition(position);
        pat->addChild(node);

        position.x() += spacing;

        osg::StateSet* stateset = pat->getOrCreateStateSet();
        osg::ShaderAttribute* sa = new osg::ShaderAttribute;
        sa->setType(osg::StateAttribute::Type(10000));
        stateset->setAttribute(sa);
        stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
        stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

        {
            osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX);
            vertex_shader->addCodeInjection(-1,"#version 460 core\n");
            vertex_shader->addCodeInjection(-1,"in vec4 osg_Vertex;\n");
            vertex_shader->addCodeInjection(-1,"uniform mat4 osg_ModelViewProjectionMatrix;\n");
            vertex_shader->addCodeInjection(0,"gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n");

            sa->addShader(vertex_shader);
        }

        {
            osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT);
            fragment_shader->addCodeInjection(-1,"#version 460 core\n");
            fragment_shader->addCodeInjection(-1,"uniform vec4 newColour;\n");
            fragment_shader->addCodeInjection(-1,"uniform float osg_FrameTime;\n");
            fragment_shader->addCodeInjection(-1,"out vec4 osg_FragColor;\n");
            fragment_shader->addCodeInjection(0,"osg_FragColor = vec4(newColour.r,newColour.g,newColour.b, 0.5+sin(osg_FrameTime*2.0)*0.5);\n");

            sa->addShader(fragment_shader);
            sa->addUniform(new osg::Uniform("newColour",osg::Vec4(1.0f,1.0f,1.0f,0.5f)));
        }

        group->addChild(pat);

    }

    return group;
}

