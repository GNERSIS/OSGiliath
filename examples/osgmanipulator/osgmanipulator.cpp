/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgmanipulator example application
 */
#include <iostream>
#include <osg/core/Inherit.hpp>
#include <osg/core/io_utils.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/CoordinateSystemNode.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/Material.hpp>
#include <osg/state/Program.hpp>
#include <osg/state/Shader.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgManipulator/AntiSquish>
#include <osgManipulator/RotateCylinderDragger>
#include <osgManipulator/Scale1DDragger>
#include <osgManipulator/Scale2DDragger>
#include <osgManipulator/TabBoxDragger>
#include <osgManipulator/TabBoxTrackballDragger>
#include <osgManipulator/TabPlaneDragger>
#include <osgManipulator/TabPlaneTrackballDragger>
#include <osgManipulator/TrackballDragger>
#include <osgManipulator/Translate1DDragger>
#include <osgManipulator/Translate2DDragger>
#include <osgManipulator/TranslateAxisDragger>
#include <osgManipulator/TranslatePlaneDragger>
#include <osgText/Text>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

class PlaneConstraint : public osgManipulator::Constraint
{
    public:

        PlaneConstraint()
        {
        }

        virtual bool
        constrain( osgManipulator::TranslateInLineCommand& command ) const
        {
            OSG_NOTICE << "PlaneConstraint TranslateInLineCommand "
                       << command.getTranslation() << std::endl;
            return true;
        }

        virtual bool
        constrain( osgManipulator::TranslateInPlaneCommand& command ) const
        {
            // command.setTranslation(osg::vec3(0.0f,0.0f,0.0f));
            OSG_NOTICE << "PlaneConstraint TranslateInPlaneCommand "
                       << command.getTranslation() << std::endl;
            return true;
        }

        virtual bool
        constrain( osgManipulator::Scale1DCommand& command ) const
        {
            // command.setScale(1.0f);
            OSG_NOTICE << "PlaneConstraint Scale1DCommand" << command.getScale()
                       << std::endl;
            return true;
        }

        virtual bool
        constrain( osgManipulator::Scale2DCommand& command ) const
        {
            // command.setScale(osg::dvec2(1.0,1.0));
            OSG_NOTICE << "PlaneConstraint Scale2DCommand " << command.getScale()
                       << std::endl;
            return true;
        }

        virtual bool
        constrain( osgManipulator::ScaleUniformCommand& command ) const
        {
            OSG_NOTICE << "PlaneConstraint ScaleUniformCommand" << command.getScale()
                       << std::endl;
            return true;
        }
};

osgManipulator::Dragger*
createDragger( const std::string& name )
{
    osgManipulator::Dragger* dragger = 0;
    if( "TabPlaneDragger" == name )
    {
        osgManipulator::TabPlaneDragger* d = new osgManipulator::TabPlaneDragger();
        d->setupDefaultGeometry( true );
        d->addConstraint( new PlaneConstraint() );
        dragger = d;
    }
    else if( "TabPlaneTrackballDragger" == name )
    {
        osgManipulator::TabPlaneTrackballDragger* d =
            new osgManipulator::TabPlaneTrackballDragger();
        d->setupDefaultGeometry();
        dragger = d;
    }
    else if( "TabBoxTrackballDragger" == name )
    {
        osgManipulator::TabBoxTrackballDragger* d =
            new osgManipulator::TabBoxTrackballDragger();
        d->setupDefaultGeometry();
        dragger = d;
    }
    else if( "TrackballDragger" == name )
    {
        osgManipulator::TrackballDragger* d = new osgManipulator::TrackballDragger();
        d->setupDefaultGeometry();
        // d->setAxisLineWidth(5.0f);
        // d->setPickCylinderHeight(0.1f);
        dragger = d;
    }
    else if( "Translate1DDragger" == name )
    {
        osgManipulator::Translate1DDragger* d = new osgManipulator::Translate1DDragger();
        d->setupDefaultGeometry();
        dragger = d;
    }
    else if( "Translate2DDragger" == name )
    {
        osgManipulator::Translate2DDragger* d = new osgManipulator::Translate2DDragger();
        d->setupDefaultGeometry();
        dragger = d;
    }
    else if( "TranslateAxisDragger" == name )
    {
        osgManipulator::TranslateAxisDragger* d =
            new osgManipulator::TranslateAxisDragger();
        d->setupDefaultGeometry();
        d->setAxisLineWidth( 5.0F );
        d->setPickCylinderRadius( 0.05F );
        d->setConeHeight( 0.2F );
        dragger = d;
    }
    else if( "TranslatePlaneDragger" == name )
    {
        osgManipulator::TranslatePlaneDragger* d =
            new osgManipulator::TranslatePlaneDragger();
        d->setupDefaultGeometry();
        dragger = d;
    }
    else if( "Scale1DDragger" == name )
    {
        osgManipulator::Scale1DDragger* d = new osgManipulator::Scale1DDragger();
        d->setupDefaultGeometry();
        dragger = d;
    }
    else if( "Scale2DDragger" == name )
    {
        osgManipulator::Scale2DDragger* d = new osgManipulator::Scale2DDragger();
        d->setupDefaultGeometry();
        dragger = d;
    }
    else if( "RotateCylinderDragger" == name )
    {
        osgManipulator::RotateCylinderDragger* d =
            new osgManipulator::RotateCylinderDragger();
        d->setupDefaultGeometry();
        dragger = d;
    }
    else if( "RotateSphereDragger" == name )
    {
        osgManipulator::RotateSphereDragger* d =
            new osgManipulator::RotateSphereDragger();
        d->setupDefaultGeometry();
        dragger = d;
    }
    else
    {
        osgManipulator::TabBoxDragger* d = new osgManipulator::TabBoxDragger();
        d->setupDefaultGeometry();
        dragger = d;
    }

    return dragger;
}

// The DraggerContainer node is used to fix the dragger's size on the screen
class DraggerContainer : public osg::Inherit<osg::Group, DraggerContainer>
{
    public:

        OSG_REGISTER_TYPE( osgManipulator,
                           DraggerContainer )

        DraggerContainer() :
            _draggerSize( 240.0F ),
            _active( true )
        {
        }

        DraggerContainer( const DraggerContainer& copy,
                          const osg::CopyOp&      copyop = osg::CopyOp::SHALLOW_COPY ) :
            Inherit( copy,
                     copyop ),
            _dragger( copy._dragger ),
            _draggerSize( copy._draggerSize ),
            _active( copy._active )
        {
        }

        void
        accept( osg::NodeVisitor& nv ) override
        {
            if( nv.validNodeMask( *this ) )
            {
                nv.pushOntoNodePath( this );
                nv.apply( *this );
                nv.popFromNodePath();
            }
        }

        void
        setDragger( osgManipulator::Dragger* dragger )
        {
            _dragger = dragger;
            if( !containsNode( dragger ) )
            {
                addChild( dragger );
            }
        }

        osgManipulator::Dragger*
        getDragger()
        {
            return _dragger.get();
        }

        const osgManipulator::Dragger*
        getDragger() const
        {
            return _dragger.get();
        }

        void
        setDraggerSize( float size )
        {
            _draggerSize = size;
        }

        float
        getDraggerSize() const
        {
            return _draggerSize;
        }

        void
        setActive( bool b )
        {
            _active = b;
        }

        bool
        getActive() const
        {
            return _active;
        }

        void
        traverse( osg::NodeVisitor& nv )
        {
            if( _dragger.valid() )
            {
                if( _active && nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR )
                {
                    osgUtil::CullVisitor* cv = static_cast<osgUtil::CullVisitor*>( &nv );

                    float                 pixelSize =
                        cv->pixelSize( _dragger->getBound().center, 0.48F );
                    if( pixelSize != _draggerSize )
                    {
                        float pixelScale =
                            pixelSize > 0.0F ? _draggerSize / pixelSize : 1.0F;
                        osg::dvec3 scaleFactor( pixelScale, pixelScale, pixelScale );

                        osg::dvec3 trans = osg::getTrans( _dragger->getMatrix() );
                        _dragger->setMatrix( osg::translate( trans ) *
                                             osg::scale( scaleFactor ) );
                    }
                }
            }
            osg::Group::traverse( nv );
        }

    protected:

        osg::ref_ptr<osgManipulator::Dragger> _dragger;
        float                                 _draggerSize;
        bool                                  _active;
};

osg::Node*
addDraggerToScene( osg::Node*         scene,
                   const std::string& name,
                   bool               fixedSizeInScreen )
{
    osg::MatrixTransform* transform = new osg::MatrixTransform;
    transform->addChild( scene );

    osgManipulator::Dragger* dragger = createDragger( name );

    osg::Group*              root    = new osg::Group;
    root->addChild( transform );

    if( fixedSizeInScreen )
    {
        DraggerContainer* draggerContainer = new DraggerContainer;
        draggerContainer->setDragger( dragger );
        root->addChild( draggerContainer );
    }
    else
    {
        root->addChild( dragger );
    }

    float scale = scene->getBound().radius * 1.6F;
    dragger->setMatrix( osg::dmat4( osg::translate( scene->getBound().center ) ) *
                        osg::dmat4( osg::scale( scale, scale, scale ) ) );

    if( dynamic_cast<osgManipulator::TabPlaneDragger*>( dragger ) )
    {
        dragger->addTransformUpdating(
            transform,
            osgManipulator::DraggerTransformCallback::HANDLE_TRANSLATE_IN_LINE
        );
    }
    else
    {
        dragger->addTransformUpdating( transform );
    }

    // we want the dragger to handle it's own events automatically
    dragger->setHandleEvents( true );

    // if we don't set an activation key or mod mask then any mouse click on
    // the dragger will activate it, however if do define either of ActivationModKeyMask
    // or and ActivationKeyEvent then you'll have to press either than mod key or the
    // specified key to be able to activate the dragger when you mouse click on it.
    // Please note the follow allows activation if either the ctrl key or the 'a' key is
    // pressed and held down.
    dragger->setActivationModKeyMask( osgGA::GUIEventAdapter::MODKEY_CTRL );
    dragger->setActivationKeyEvent( 'a' );

    return root;
}

///////////////////////////////////////////////////////////////////////////
// in-line GLSL source code for GL 4.6 core profile

static const char* manipVertexShader = {
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

static const char* manipFragmentShader = {
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

static osg::Program*
createManipulatorProgram()
{
    osg::Program* program = new osg::Program;
    program->setName( "manipulator" );
    program->addShader( new osg::Shader( osg::Shader::VERTEX, manipVertexShader ) );
    program->addShader( new osg::Shader( osg::Shader::FRAGMENT, manipFragmentShader ) );
    return program;
}

osg::Node*
createDemoScene( bool fixedSizeInScreen )
{

    osg::Group* root = new osg::Group;

    // Add GLSL shaders for GL 4.6 core profile
    root->getOrCreateStateSet()->setAttributeAndModes( createManipulatorProgram(),
                                                       osg::StateAttribute::ON );

    osg::ref_ptr<osg::Geode>             geode_1     = new osg::Geode;
    osg::ref_ptr<osg::MatrixTransform>   transform_1 = new osg::MatrixTransform;

    osg::ref_ptr<osg::Geode>             geode_2     = new osg::Geode;
    osg::ref_ptr<osg::MatrixTransform>   transform_2 = new osg::MatrixTransform;

    osg::ref_ptr<osg::Geode>             geode_3     = new osg::Geode;
    osg::ref_ptr<osg::MatrixTransform>   transform_3 = new osg::MatrixTransform;

    osg::ref_ptr<osg::Geode>             geode_4     = new osg::Geode;
    osg::ref_ptr<osg::MatrixTransform>   transform_4 = new osg::MatrixTransform;

    osg::ref_ptr<osg::Geode>             geode_5     = new osg::Geode;
    osg::ref_ptr<osg::MatrixTransform>   transform_5 = new osg::MatrixTransform;

    osg::ref_ptr<osg::Geode>             geode_6     = new osg::Geode;
    osg::ref_ptr<osg::MatrixTransform>   transform_6 = new osg::MatrixTransform;

    osg::ref_ptr<osg::Geode>             geode_7     = new osg::Geode;
    osg::ref_ptr<osg::MatrixTransform>   transform_7 = new osg::MatrixTransform;

    const float                          radius      = 0.8F;
    const float                          height      = 1.0F;
    osg::ref_ptr<osg::TessellationHints> hints       = new osg::TessellationHints;
    hints->setDetailRatio( 2.0F );
    osg::ref_ptr<osg::ShapeDrawable> shape;

    shape = new osg::ShapeDrawable(
        new osg::Box( osg::vec3( 0.0F, 0.0F, -2.0F ), 10, 10.0F, 0.1F ),
        hints.get()
    );
    shape->setColor( osg::vec4( 0.5F, 0.5F, 0.7F, 1.0F ) );
    geode_1->addDrawable( shape.get() );

    shape = new osg::ShapeDrawable(
        new osg::Cylinder( osg::vec3( 0.0F, 0.0F, 0.0F ), radius * 2, radius ),
        hints.get()
    );
    shape->setColor( osg::vec4( 0.8F, 0.8F, 0.8F, 1.0F ) );
    geode_2->addDrawable( shape.get() );

    shape = new osg::ShapeDrawable(
        new osg::Cylinder( osg::vec3( -3.0F, 0.0F, 0.0F ), radius, radius ),
        hints.get()
    );
    shape->setColor( osg::vec4( 0.6F, 0.8F, 0.8F, 1.0F ) );
    geode_3->addDrawable( shape.get() );

    shape = new osg::ShapeDrawable(
        new osg::Cone( osg::vec3( 3.0F, 0.0F, 0.0F ), 2 * radius, radius ),
        hints.get()
    );
    shape->setColor( osg::vec4( 0.4F, 0.9F, 0.3F, 1.0F ) );
    geode_4->addDrawable( shape.get() );

    shape = new osg::ShapeDrawable(
        new osg::Cone( osg::vec3( 0.0F, -3.0F, 0.0F ), radius, height ),
        hints.get()
    );
    shape->setColor( osg::vec4( 0.2F, 0.5F, 0.7F, 1.0F ) );
    geode_5->addDrawable( shape.get() );

    shape = new osg::ShapeDrawable(
        new osg::Cylinder( osg::vec3( 0.0F, 3.0F, 0.0F ), radius, height ),
        hints.get()
    );
    shape->setColor( osg::vec4( 1.0F, 0.3F, 0.3F, 1.0F ) );
    geode_6->addDrawable( shape.get() );

    shape = new osg::ShapeDrawable(
        new osg::Cone( osg::vec3( 0.0F, 0.0F, 3.0F ), 2.0F, 2.0F ),
        hints.get()
    );
    shape->setColor( osg::vec4( 0.8F, 0.8F, 0.4F, 1.0F ) );
    geode_7->addDrawable( shape.get() );

    // material
    osg::ref_ptr<osg::Material> matirial = new osg::Material;
    matirial->setColorMode( osg::Material::DIFFUSE );
    matirial->setAmbient( osg::Material::FRONT_AND_BACK, osg::vec4( 0, 0, 0, 1 ) );
    matirial->setSpecular( osg::Material::FRONT_AND_BACK, osg::vec4( 1, 1, 1, 1 ) );
    matirial->setShininess( osg::Material::FRONT_AND_BACK, 64.0F );
    root->getOrCreateStateSet()->setAttributeAndModes( matirial.get(),
                                                       osg::StateAttribute::ON );

    transform_1.get()->addChild(
        addDraggerToScene( geode_1.get(), "TabBoxDragger", fixedSizeInScreen )
    );
    transform_2.get()->addChild(
        addDraggerToScene( geode_2.get(), "TabPlaneDragger", fixedSizeInScreen )
    );
    transform_3.get()->addChild(
        addDraggerToScene( geode_3.get(), "TabBoxTrackballDragger", fixedSizeInScreen )
    );
    transform_4.get()->addChild(
        addDraggerToScene( geode_4.get(), "TrackballDragger", fixedSizeInScreen )
    );
    transform_5.get()->addChild(
        addDraggerToScene( geode_5.get(), "Translate1DDragger", fixedSizeInScreen )
    );
    transform_6.get()->addChild(
        addDraggerToScene( geode_6.get(), "Translate2DDragger", fixedSizeInScreen )
    );
    transform_7.get()->addChild(
        addDraggerToScene( geode_7.get(), "TranslateAxisDragger", fixedSizeInScreen )
    );

    root->addChild( transform_1.get() );
    root->addChild( transform_2.get() );
    root->addChild( transform_3.get() );
    root->addChild( transform_4.get() );
    root->addChild( transform_5.get() );
    root->addChild( transform_6.get() );
    root->addChild( transform_7.get() );

    return root;
}

//
int
main( int    argc,
      char** argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc, argv );

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(
        arguments.getApplicationName()
    );
    arguments.getApplicationUsage()->setCommandLineUsage(
        arguments.getApplicationName() + " [options] filename ..."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--image <filename>",
        "Load an image and render it on a quad"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--dem <filename>",
        "Load an image/DEM and render it on a HeightField"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "-h or --help",
        "Display command line parameters"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--help-env",
        "Display environmental variables available"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--help-keys",
        "Display keyboard & mouse bindings available"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--help-all",
        "Display all command line, env vars and keyboard & mouse bindings."
    );

    arguments.getApplicationUsage()->addCommandLineOption(
        "--dragger <draggername>",
        "Use the specified dragger for manipulation [TabPlaneDragger, "
        "TabPlaneTrackballDragger, TrackballDragger, Translate1DDragger, "
        "Translate2DDragger, TranslateAxisDragger, TabBoxDragger, "
        "TranslatePlaneDragger, Scale1DDragger, Scale2DDragger, RotateCylinderDragger, "
        "RotateSphereDragger]"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--fixedDraggerSize",
        "Fix the size of the dragger geometry in the screen space"
    );

    bool fixedSizeInScreen = false;
    while( arguments.read( "--fixedDraggerSize" ) )
    {
        fixedSizeInScreen = true;
    }

    // Headless capture: early check before viewer construction
    {
        std::string headlessOutput;
        if( arguments.read( "--headless", headlessOutput ) )
        {
            auto node = osgDB::readRefNodeFiles( arguments );
            if( !node )
            {
                node = osgDB::readRefNodeFile( "duck.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    osgViewer::Viewer viewer( arguments );

    // add the window size toggle handler
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage( *arguments.getApplicationUsage() );

    if( arguments.read( "--test-NodeMask" ) )
    {
        const osg::ref_ptr<osg::Group> group = new osg::Group();
        group->setNodeMask( 0 );

        const osg::ref_ptr<osgManipulator::AntiSquish> antiSquish =
            new osgManipulator::AntiSquish();

        group->addChild( antiSquish.get() );

        const osg::ref_ptr<osg::Node> node = new osg::Node();
        node->setInitialBound( osg::sphere( osg::vec3( 0.0, 0.0, 0.0 ), 1.0 ) );

        antiSquish->addChild( node.get() );

        group->getBound();

        return 1;
    }

    // if user request help write it out to cout.
    bool         helpAll = arguments.read( "--help-all" );
    unsigned int helpType =
        ( ( helpAll || arguments.read( "-h" ) || arguments.read( "--help" ) )
              ? osg::ApplicationUsage::COMMAND_LINE_OPTION
              : 0 ) |
        ( ( helpAll || arguments.read( "--help-env" ) )
              ? osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE
              : 0 ) |
        ( ( helpAll || arguments.read( "--help-keys" ) )
              ? osg::ApplicationUsage::KEYBOARD_MOUSE_BINDING
              : 0 );
    if( helpType )
    {
        arguments.getApplicationUsage()->write( std::cout, helpType );
        return 1;
    }

    // report any errors if they have occurred when parsing the program arguments.
    if( arguments.errors() )
    {
        arguments.writeErrorMessages( std::cout );
        return 1;
    }

    std::string dragger_name = "TabBoxDragger";
    arguments.read( "--dragger", dragger_name );

    osg::Timer_t            start_tick = osg::Timer::instance()->tick();

    // read the scene from the list of file specified command line args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles( arguments );

    // if no model has been successfully loaded report failure.
    bool                    tragger2Scene( true );
    if( !loadedModel )
    {
        // std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        // return 1;
        loadedModel   = createDemoScene( fixedSizeInScreen );
        tragger2Scene = false;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occurred when parsing the program arguments.
    if( arguments.errors() )
    {
        arguments.writeErrorMessages( std::cout );
    }

    osg::Timer_t end_tick = osg::Timer::instance()->tick();

    std::cout << "Time to load = "
              << osg::Timer::instance()->delta_s( start_tick, end_tick ) << std::endl;

    // optimize the scene graph, remove redundant nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize( loadedModel );

    // pass the loaded scene graph to the viewer.
    if( tragger2Scene )
    {
        osg::Node* scene =
            addDraggerToScene( loadedModel.get(), dragger_name, fixedSizeInScreen );
        // Add GLSL shaders for GL 4.6 core profile
        scene->getOrCreateStateSet()->setAttributeAndModes( createManipulatorProgram(),
                                                            osg::StateAttribute::ON );
        viewer.setSceneData( scene );
    }
    else
    {
        viewer.setSceneData( loadedModel );
    }

    return viewer.run();
}
