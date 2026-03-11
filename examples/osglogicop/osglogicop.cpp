/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osglogicop example application
 */
#include <iostream>
#include <osg/core/Inherit.hpp>
#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/LogicOp.hpp>
#include <osg/state/Program.hpp>
#include <osg/state/Shader.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/registry/Registry.hpp>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgViewer/core/Viewer.hpp>

///////////////////////////////////////////////////////////////////////////
// in-line GLSL source code for GL 4.6 core profile

static const char* logicopVertexShader = {
    "#version 460 core\n"
    "layout(location = 0) in vec4 osg_Vertex;\n"
    "layout(location = 2) in vec3 osg_Normal;\n"
    "layout(location = 3) in vec4 osg_Color;\n"
    "uniform mat4 osg_ModelViewMatrix;\n"
    "uniform mat4 osg_ModelViewProjectionMatrix;\n"
    "uniform mat3 osg_NormalMatrix;\n"
    "out vec3 vNormal;\n"
    "out vec3 vFragPos;\n"
    "out vec4 vColor;\n"
    "void main() {\n"
    "    vFragPos = vec3(osg_ModelViewMatrix * osg_Vertex);\n"
    "    vNormal = normalize(osg_NormalMatrix * osg_Normal);\n"
    "    vColor = osg_Color;\n"
    "    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
    "}\n"
};

static const char* logicopFragmentShader = {
    "#version 460 core\n"
    "in vec3 vNormal;\n"
    "in vec3 vFragPos;\n"
    "in vec4 vColor;\n"
    "out vec4 fragColor;\n"
    "void main() {\n"
    "    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.8));\n"
    "    float diff = max(dot(normalize(vNormal), lightDir), 0.0) * 0.7 + 0.3;\n"
    "    fragColor = vec4(vColor.rgb * diff, vColor.a);\n"
    "}\n"
};

const int                  _ops_nb              = 16;
const osg::LogicOp::Opcode _operations[_ops_nb] = {
    osg::LogicOp::Opcode::CLEAR,
    osg::LogicOp::Opcode::SET,
    osg::LogicOp::Opcode::COPY,
    osg::LogicOp::Opcode::COPY_INVERTED,
    osg::LogicOp::Opcode::NOOP,
    osg::LogicOp::Opcode::INVERT,
    osg::LogicOp::Opcode::AND,
    osg::LogicOp::Opcode::NAND,
    osg::LogicOp::Opcode::OR,
    osg::LogicOp::Opcode::NOR,
    osg::LogicOp::Opcode::XOR,
    osg::LogicOp::Opcode::EQUIV,
    osg::LogicOp::Opcode::AND_REVERSE,
    osg::LogicOp::Opcode::AND_INVERTED,
    osg::LogicOp::Opcode::OR_REVERSE,
    osg::LogicOp::Opcode::OR_INVERTED
};

const char* _ops_name[_ops_nb] = {
    "osg::LogicOp::Opcode::CLEAR",
    "osg::LogicOp::Opcode::SET",
    "osg::LogicOp::Opcode::COPY",
    "osg::LogicOp::Opcode::COPY_INVERTED",
    "osg::LogicOp::Opcode::NOOP",
    "osg::LogicOp::Opcode::INVERT",
    "osg::LogicOp::Opcode::AND",
    "osg::LogicOp::Opcode::NAND",
    "osg::LogicOp::Opcode::OR",
    "osg::LogicOp::Opcode::NOR",
    "osg::LogicOp::Opcode::XOR",
    "osg::LogicOp::Opcode::EQUIV",
    "osg::LogicOp::Opcode::AND_REVERSE",
    "osg::LogicOp::Opcode::AND_INVERTED",
    "osg::LogicOp::Opcode::OR_REVERSE",
    "osg::LogicOp::Opcode::OR_INVERTED"
};

class TechniqueEventHandler
    : public osg::Inherit<osgGA::GUIEventHandler, TechniqueEventHandler>
{
    public:

        OSG_REGISTER_TYPE( osglogicopApp,
                           TechniqueEventHandler )

        TechniqueEventHandler( osg::LogicOp* logicOp )
        {
            _logicOp   = logicOp;
            _ops_index = _ops_nb - 1;
        }

        TechniqueEventHandler()
        {
            std::cerr << "Error, can't initialize it!";
        }

        TechniqueEventHandler( const TechniqueEventHandler& rhs,
                               const osg::CopyOp&           copyop ) :
            Inherit( rhs,
                     copyop )
        {
        }

        virtual bool
        handle( const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter& );

        virtual void
        getUsage( osg::ApplicationUsage& usage ) const;

    protected:

        ~TechniqueEventHandler()
        {
        }

        osg::LogicOp* _logicOp;
        int           _ops_index;
};

bool
TechniqueEventHandler::handle( const osgGA::GUIEventAdapter& ea,
                               osgGA::GUIActionAdapter& )
{
    switch( ea.getEventType() )
    {
        case( osgGA::GUIEventAdapter::KEYDOWN ) :
            {
                if( ea.getKey() ==
                    osgGA::GUIEventAdapter::KEY_Right ||
                    ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Right )
                {
                    _ops_index++;
                    if( _ops_index >= _ops_nb )
                    {
                        _ops_index = 0;
                    }
                    _logicOp->setOpcode( _operations[_ops_index] );
                    std::cout << "Operation name = " << _ops_name[_ops_index]
                              << std::endl;
                    return true;
                }
                else if( ea.getKey() ==
                         osgGA::GUIEventAdapter::KEY_Left ||
                         ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Left )
                {
                    _ops_index--;
                    if( _ops_index < 0 )
                    {
                        _ops_index = _ops_nb - 1;
                    }
                    _logicOp->setOpcode( _operations[_ops_index] );
                    std::cout << "Operation name = " << _ops_name[_ops_index]
                              << std::endl;
                    return true;
                }
                return false;
            }

        default :
            return false;
    }
}

void
TechniqueEventHandler::getUsage( osg::ApplicationUsage& usage ) const
{
    usage.addKeyboardMouseBinding( "- or Left Arrow", "Advance to next opcode" );
    usage.addKeyboardMouseBinding( "+ or Right Array", "Move to previous opcode" );
}

int
main( int    argc,
      char** argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser     arguments( &argc, argv );

    // load the nodes from the commandline arguments.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles( arguments );

    // if not loaded assume no arguments passed in, try use default mode instead.
    if( !loadedModel )
    {
        loadedModel = osgDB::readRefNodeFile( "fox.glb" );
    }

    if( !loadedModel )
    {
        osg::notify( osg::NOTICE )
            << "Please specify model filename on the command line." << std::endl;
        return 1;
    }

    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( loadedModel );

    // Add GLSL shaders for GL 4.6 core profile
    {
        osg::StateSet* rootSS  = root->getOrCreateStateSet();
        osg::Program*  program = new osg::Program;
        program->setName( "logicop" );
        program->addShader( new osg::Shader( osg::Shader::VERTEX,
                                             logicopVertexShader ) );
        program->addShader( new osg::Shader( osg::Shader::FRAGMENT,
                                             logicopFragmentShader ) );
        rootSS->setAttributeAndModes( program, osg::StateAttribute::ON );
    }

    osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;
    osg::ref_ptr<osg::LogicOp>  logicOp =
        new osg::LogicOp( osg::LogicOp::Opcode::OR_INVERTED );

    stateset->setAttributeAndModes( logicOp,
                                    osg::StateAttribute::OVERRIDE |
                                        osg::StateAttribute::ON );

    // tell to sort the mesh before displaying it
    stateset->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

    loadedModel->setStateSet( stateset );

    // Headless capture: early check before viewer construction
    {
        std::string headlessOutput;
        if( arguments.read( "--headless", headlessOutput ) )
        {
            auto node = osgDB::readRefNodeFiles( arguments );
            if( !node )
            {
                node = osgDB::readRefNodeFile( "fox.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    osgViewer::Viewer viewer;

    viewer.addEventHandler( new TechniqueEventHandler( logicOp.get() ) );

    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize( root );

    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData( root );
    return viewer.run();
}
