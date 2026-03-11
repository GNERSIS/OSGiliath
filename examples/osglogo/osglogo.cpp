/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osglogo example application
 */
#include <iostream>
#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/nodes/ClearNode.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/nodes/PositionAttitudeTransform.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/BlendFunc.hpp>
#include <osg/state/Material.hpp>
#include <osg/state/Program.hpp>
#include <osg/state/Shader.hpp>
#include <osg/state/Uniform.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osg/Version>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgText/Text>
#include <osgUtil/culling/CullVisitor.hpp>
#include <osgUtil/mesh/Tessellator.hpp>
#include <osgViewer/core/Viewer.hpp>

///////////////////////////////////////////////////////////////////////////
// in-line GLSL source code for GL 4.6 core profile

static const char* logoVertexShader = {
    "#version 460 core\n"
    "layout(location = 0) in vec4 osg_Vertex;\n"
    "layout(location = 2) in vec3 osg_Normal;\n"
    "layout(location = 3) in vec4 osg_Color;\n"
    "layout(location = 8) in vec4 osg_MultiTexCoord0;\n"
    "uniform mat4 osg_ModelViewProjectionMatrix;\n"
    "uniform mat4 osg_ModelViewMatrix;\n"
    "uniform mat3 osg_NormalMatrix;\n"
    "out vec3 vNormal;\n"
    "out vec3 vFragPos;\n"
    "out vec4 vColor;\n"
    "out vec2 vTexCoord;\n"
    "void main() {\n"
    "    vFragPos = vec3(osg_ModelViewMatrix * osg_Vertex);\n"
    "    vNormal = normalize(osg_NormalMatrix * osg_Normal);\n"
    "    vColor = osg_Color;\n"
    "    vTexCoord = osg_MultiTexCoord0.xy;\n"
    "    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
    "}\n"
};

static const char* logoFragmentShader = {
    "#version 460 core\n"
    "in vec3 vNormal;\n"
    "in vec3 vFragPos;\n"
    "in vec4 vColor;\n"
    "in vec2 vTexCoord;\n"
    "uniform sampler2D baseTexture;\n"
    "uniform bool hasTexture;\n"
    "out vec4 fragColor;\n"
    "void main() {\n"
    "    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.8));\n"
    "    float diff = max(dot(normalize(vNormal), lightDir), 0.0) * 0.7 + 0.3;\n"
    "    vec4 baseColor = vColor;\n"
    "    if (hasTexture) baseColor = texture(baseTexture, vTexCoord);\n"
    "    fragColor = vec4(baseColor.rgb * diff, baseColor.a);\n"
    "}\n"
};

static bool s_useSDF = false;

class MyBillboardTransform : public osg::PositionAttitudeTransform
{
    public:

        MyBillboardTransform() :
            _axis( 0.0F,
                   0.0F,
                   1.0F ),
            _normal( 0.0F,
                     -1.0F,
                     0.0F )
        {
        }

        bool
        computeLocalToWorldMatrix( osg::dmat4&       matrix,
                                   osg::NodeVisitor* nv ) const
        {
            osg::quat             billboardRotation;
            osgUtil::CullVisitor* cullvisitor =
                dynamic_cast<osgUtil::CullVisitor*>( nv );
            if( cullvisitor )
            {
                osg::vec3 eyevector =
                    osg::vec3( cullvisitor->getEyeLocal() - _position );
                eyevector      = osg::normalize( eyevector );

                osg::vec3 side = _axis ^ _normal;
                side           = osg::normalize( side );

                float angle =
                    atan2( osg::dot( eyevector, _normal ), osg::dot( eyevector, side ) );
                billboardRotation =
                    osg::quat( ( osg::PI * 0.5 ) - angle, osg::vec3( _axis ) );
            }

            osg::preMultTranslate( matrix, _position );
            matrix = matrix * osg::dmat4( osg::rotate( _attitude ) );
            matrix = matrix * osg::dmat4( osg::rotate( billboardRotation ) );
            osg::preMultTranslate( matrix, -_pivotPoint );
            return true;
        }

        void
        setAxis( const osg::vec3& axis )
        {
            _axis = axis;
        }

        void
        setNormal( const osg::vec3& normal )
        {
            _normal = normal;
        }

    protected:

        virtual ~MyBillboardTransform()
        {
        }

        osg::vec3 _axis;
        osg::vec3 _normal;
};

osg::Geometry*
createWing( const osg::vec3& left,
            const osg::vec3& nose,
            const osg::vec3& right,
            float            chordRatio,
            const osg::vec4& color )
{
    osg::Geometry* geom           = new osg::Geometry;

    osg::vec3      normal         = ( nose - right ) ^ ( left - nose );
    normal                        = osg::normalize( normal );

    osg::vec3       left_to_right = right - left;
    osg::vec3       mid           = ( right + left ) * 0.5F;
    osg::vec3       mid_to_nose   = ( nose - mid ) * chordRatio * 0.5F;

    osg::Vec3Array* vertices      = new osg::Vec3Array;
    vertices->push_back( left );
    // vertices->push_back(mid+mid_to_nose);

    unsigned int noSteps = 40;
    for( unsigned int i = 1; i < noSteps; ++i )
    {
        float ratio = ( float )i / ( float )noSteps;
        vertices->push_back( left +
                             left_to_right *
                             ratio +
                             mid_to_nose *
                             ( cosf( ( ratio - 0.5F ) * osg::PI * 2.0F ) + 1.0F ) );
    }

    vertices->push_back( right );
    vertices->push_back( nose );

    geom->setVertexArray( vertices );

    osg::Vec3Array* normals = new osg::Vec3Array;
    normals->push_back( normal );
    geom->setNormalArray( normals, osg::Array::BIND_OVERALL );

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back( color );
    geom->setColorArray( colors, osg::Array::BIND_OVERALL );

    geom->addPrimitiveSet(
        new osg::DrawArrays( GL_TRIANGLE_FAN, 0, vertices->getNumElements() )
    );

    osgUtil::Tessellator tessellator;
    tessellator.retessellatePolygons( *geom );

    geom->setUseVertexBufferObjects( true );

    return geom;
}

osg::Node*
createTextBelow( const osg::box&    bb,
                 const std::string& label,
                 const std::string& )
{
    osg::Geode*    geode = new osg::Geode();

    std::string    font( "fonts/arial.ttf" );

    osgText::Text* text = new osgText::Text;

    text->setFont( font );
    text->setFontResolution( 64, 64 );

    if( s_useSDF )
    {
        text->setShaderTechnique( osgText::ALL_FEATURES );
    }

    text->setAlignment( osgText::Text::CENTER_CENTER );
    text->setAxisAlignment( osgText::Text::XZ_PLANE );
    text->setPosition( bb.center() - osg::vec3( 0.0F, 0.0F, bb.zMax() - bb.zMin() ) );
    text->setColor( osg::vec4( 0.37F, 0.48F, 0.67F, 1.0F ) );
    text->setText( label );

    geode->addDrawable( text );

    return geode;
}

osg::Node*
createTextLeft( const osg::box&    bb,
                const std::string& label,
                const std::string& subscript )
{
    osg::Geode*    geode = new osg::Geode();

    // std::string font("fonts/times.ttf");
    std::string    font( "fonts/arial.ttf" );

    osgText::Text* text = new osgText::Text;

    text->setFont( font );
    text->setFontResolution( 110, 120 );

    if( s_useSDF )
    {
        text->setShaderTechnique( osgText::ALL_FEATURES );
    }

    text->setAlignment( osgText::Text::RIGHT_CENTER );
    text->setAxisAlignment( osgText::Text::XZ_PLANE );
    text->setCharacterSize( ( bb.zMax() - bb.zMin() ) * 1.0F );

    text->setPosition( bb.center() - osg::vec3( bb.xMax() - bb.xMin(),
                                                -( bb.yMax() - bb.yMin() ) * 0.5F,
                                                ( bb.zMax() - bb.zMin() ) * 0.1F ) );
    text->setColor( osg::vec4( 0.20F, 0.45F, 0.60F, 1.0F ) );    // OGL logo colour

    text->setBackdropType( osgText::Text::OUTLINE );
    text->setBackdropOffset( 0.03F );
    text->setBackdropColor( osg::vec4( 0.0F, 0.0F, 0.5F, 1.0F ) );

    text->setColorGradientMode( osgText::Text::OVERALL );
    osg::vec4 lightblue( 0.30F, 0.6F, 0.90F, 1.0F );
    osg::vec4 blue( 0.10F, 0.30F, 0.40F, 1.0F );
    text->setColorGradientCorners( lightblue, blue, blue, lightblue );

    text->setText( label );

    geode->addDrawable( text );

    if( !subscript.empty() )
    {
        osgText::Text* subscriptText = new osgText::Text;
        subscriptText->setFont( font );
        subscriptText->setText( subscript );
        subscriptText->setAlignment( osgText::Text::RIGHT_CENTER );
        subscriptText->setAxisAlignment( osgText::Text::XZ_PLANE );
        subscriptText->setPosition( bb.center() -
                                    osg::vec3( ( bb.xMax() - bb.xMin() ) * 4.3F,
                                               -( bb.yMax() - bb.yMin() ) * 0.5F,
                                               ( bb.zMax() - bb.zMin() ) * 0.6F ) );
        subscriptText->setColor( osg::vec4( 0.0F, 0.0F, 0.0F, 1.0F ) );    // black

        geode->addDrawable( subscriptText );
    }

    return geode;
}

osg::Node*
createGlobe( const osg::box&    bb,
             float              ratio,
             const std::string& filename )
{
    osg::MatrixTransform* xform = new osg::MatrixTransform;
    xform->setUpdateCallback(
        new osg::AnimationPathCallback( osg::dvec3( bb.center() ),
                                        osg::dvec3( 0.0, 0.0, 1.0 ),
                                        osg::radians( 10.0F ) )
    );

    osg::ref_ptr<osg::Node> bluemarble =
        filename.empty() ? 0 : osgDB::readRefNodeFile( filename.c_str() );
    if( bluemarble )
    {
        const osg::sphere&    bs         = bluemarble->getBound();
        float                 s          = 1.2 * bb.radius() / bs.radius;
        osg::MatrixTransform* positioner = new osg::MatrixTransform;
        positioner->setMatrix( osg::dmat4( osg::translate( -bs.center ) *
                                           osg::scale( s, s, s ) *
                                           osg::translate( bb.center() ) ) );
        positioner->addChild( bluemarble );

        xform->addChild( positioner );
    }
    else
    {

        osg::Geode*              geode    = new osg::Geode();

        osg::StateSet*           stateset = geode->getOrCreateStateSet();

        osg::ref_ptr<osg::Image> image =
            osgDB::readRefImageFile( "Images/land_shallow_topo_2048.jpg" );
        if( image )
        {
            osg::Texture2D* texture = new osg::Texture2D;
            texture->setImage( image );
            texture->setMaxAnisotropy( 8 );
            stateset->setTextureAttributeAndModes( 0, texture, osg::StateAttribute::ON );
            stateset->addUniform( new osg::Uniform( "hasTexture", true ) );
        }

        osg::Material* material = new osg::Material;
        stateset->setAttribute( material );

        // the globe
        geode->addDrawable(
            new osg::ShapeDrawable( new osg::Sphere( bb.center(), bb.radius() * ratio ) )
        );

        xform->addChild( geode );
    }

    return xform;
}

osg::Node*
createBox( const osg::box& bb,
           float           chordRatio )
{
    osg::Geode* geode = new osg::Geode();

    osg::vec4   white( 1.0F, 1.0F, 1.0F, 1.0F );

    // front faces.
    geode->addDrawable(
        createWing( bb.corner( 4 ), bb.corner( 6 ), bb.corner( 7 ), chordRatio, white )
    );
    geode->addDrawable(
        createWing( bb.corner( 7 ), bb.corner( 5 ), bb.corner( 4 ), chordRatio, white )
    );

    geode->addDrawable(
        createWing( bb.corner( 4 ), bb.corner( 5 ), bb.corner( 1 ), chordRatio, white )
    );
    geode->addDrawable(
        createWing( bb.corner( 1 ), bb.corner( 0 ), bb.corner( 4 ), chordRatio, white )
    );

    geode->addDrawable(
        createWing( bb.corner( 1 ), bb.corner( 5 ), bb.corner( 7 ), chordRatio, white )
    );
    geode->addDrawable(
        createWing( bb.corner( 7 ), bb.corner( 3 ), bb.corner( 1 ), chordRatio, white )
    );

    // back faces
    geode->addDrawable(
        createWing( bb.corner( 2 ), bb.corner( 0 ), bb.corner( 1 ), chordRatio, white )
    );
    geode->addDrawable(
        createWing( bb.corner( 1 ), bb.corner( 3 ), bb.corner( 2 ), chordRatio, white )
    );

    geode->addDrawable(
        createWing( bb.corner( 2 ), bb.corner( 3 ), bb.corner( 7 ), chordRatio, white )
    );
    geode->addDrawable(
        createWing( bb.corner( 7 ), bb.corner( 6 ), bb.corner( 2 ), chordRatio, white )
    );

    geode->addDrawable(
        createWing( bb.corner( 2 ), bb.corner( 6 ), bb.corner( 4 ), chordRatio, white )
    );
    geode->addDrawable(
        createWing( bb.corner( 4 ), bb.corner( 0 ), bb.corner( 2 ), chordRatio, white )
    );

    return geode;
}

osg::Node*
createBoxNo5( const osg::box& bb,
              float           chordRatio )
{
    osg::Geode* geode = new osg::Geode();

    osg::vec4   white( 1.0F, 1.0F, 1.0F, 1.0F );

    // front faces.
    geode->addDrawable(
        createWing( bb.corner( 4 ), bb.corner( 6 ), bb.corner( 7 ), chordRatio, white )
    );

    geode->addDrawable(
        createWing( bb.corner( 1 ), bb.corner( 0 ), bb.corner( 4 ), chordRatio, white )
    );

    geode->addDrawable(
        createWing( bb.corner( 7 ), bb.corner( 3 ), bb.corner( 1 ), chordRatio, white )
    );

    // back faces
    geode->addDrawable(
        createWing( bb.corner( 2 ), bb.corner( 0 ), bb.corner( 1 ), chordRatio, white )
    );
    geode->addDrawable(
        createWing( bb.corner( 1 ), bb.corner( 3 ), bb.corner( 2 ), chordRatio, white )
    );

    geode->addDrawable(
        createWing( bb.corner( 2 ), bb.corner( 3 ), bb.corner( 7 ), chordRatio, white )
    );
    geode->addDrawable(
        createWing( bb.corner( 7 ), bb.corner( 6 ), bb.corner( 2 ), chordRatio, white )
    );

    geode->addDrawable(
        createWing( bb.corner( 2 ), bb.corner( 6 ), bb.corner( 4 ), chordRatio, white )
    );
    geode->addDrawable(
        createWing( bb.corner( 4 ), bb.corner( 0 ), bb.corner( 2 ), chordRatio, white )
    );

    return geode;
}

osg::Node*
createBoxNo5No2( const osg::box& bb,
                 float           chordRatio )
{
    osg::Geode* geode = new osg::Geode();

    // osg::vec4 red(1.0f,0.0f,0.0f,1.0f);
    // osg::vec4 green(0.0f,1.0f,0.0f,1.0f);
    // osg::vec4 blue(0.0f,0.0f,1.0f,1.0f);

    osg::vec4   red( 1.0F, 0.12F, 0.06F, 1.0F );
    osg::vec4   green( 0.21F, 0.48F, 0.03F, 1.0F );
    osg::vec4   blue( 0.20F, 0.45F, 0.60F, 1.0F );

    // front faces.
    geode->addDrawable(
        createWing( bb.corner( 4 ), bb.corner( 6 ), bb.corner( 7 ), chordRatio, red )
    );

    geode->addDrawable(
        createWing( bb.corner( 1 ), bb.corner( 0 ), bb.corner( 4 ), chordRatio, green )
    );

    geode->addDrawable(
        createWing( bb.corner( 7 ), bb.corner( 3 ), bb.corner( 1 ), chordRatio, blue )
    );

    return geode;
}

osg::Node*
createBackdrop( const osg::vec3& corner,
                const osg::vec3& top,
                const osg::vec3& right )
{
    osg::Geometry* geom   = new osg::Geometry;

    osg::vec3      normal = ( corner - top ) ^ ( right - corner );
    normal                = osg::normalize( normal );

    // Vertex order for GL_TRIANGLE_STRIP: top, corner, top-right, right
    osg::Vec3Array* vertices = new osg::Vec3Array;
    vertices->push_back( top );
    vertices->push_back( corner );
    vertices->push_back( right + ( top - corner ) );
    vertices->push_back( right );

    geom->setVertexArray( vertices );

    osg::Vec3Array* normals = new osg::Vec3Array;
    normals->push_back( normal );
    geom->setNormalArray( normals, osg::Array::BIND_OVERALL );

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back( osg::vec4( 1.0F, 1.0F, 1.0F, 1.0F ) );
    geom->setColorArray( colors, osg::Array::BIND_OVERALL );

    geom->addPrimitiveSet(
        new osg::DrawArrays( GL_TRIANGLE_STRIP, 0, vertices->getNumElements() )
    );
    geom->setUseVertexBufferObjects( true );

    osg::Geode* geode = new osg::Geode();
    geode->addDrawable( geom );

    return geode;
}

osg::Node*
createLogo( const std::string& filename,
            const std::string& label,
            const std::string& subscript )
{
    osg::box    bb( osg::vec3( 0.0F, 0.0F, 0.0F ), osg::vec3( 100.0F, 100.0F, 100.0F ) );
    float       chordRatio  = 0.5F;
    float       sphereRatio = 0.6F;

    // create a group to hold the whole model.
    osg::Group* logo_group = new osg::Group;

    osg::quat   r1, r2;
    r1 = osg::quat( -osg::radians( 45.0F ), osg::vec3( 0.0F, 0.0F, 1.0F ) );
    r2 = osg::quat( osg::radians( 45.0F ), osg::vec3( 1.0F, 0.0F, 0.0F ) );

    MyBillboardTransform* xform = new MyBillboardTransform;
    xform->setPivotPoint( osg::dvec3( bb.center() ) );
    xform->setPosition( osg::dvec3( bb.center() ) );
    xform->setAttitude( r2 * r1 );

    // // create a transform to orientate the box and globe.
    // osg::MatrixTransform* xform = new osg::MatrixTransform;
    // xform->setDataVariance(osg::Object::DataVariance::STATIC);
    // xform->setMatrix(osg::translate(-bb.center())*
    //                  osg::rotate(-osg::radians(45.0f),0.0f,0.0f,1.0f)*
    //                  osg::rotate(osg::radians(45.0f),1.0f,0.0f,0.0f)*
    //                  osg::translate(bb.center()));

    // add the box and globe to it.
    // xform->addChild(createBox(bb,chordRatio));
    // xform->addChild(createBoxNo5(bb,chordRatio));
    xform->addChild( createBoxNo5No2( bb, chordRatio ) );
    // add the transform to the group.
    logo_group->addChild( xform );

    logo_group->addChild( createGlobe( bb, sphereRatio, filename ) );

    // add the text to the group.
    // group->addChild(createTextBelow(bb));
    logo_group->addChild( createTextLeft( bb, label, subscript ) );

    // create the backdrop to render the shadow to.
    osg::vec3 corner( -900.0F, 150.0F, -100.0F );
    osg::vec3 top( 0.0F, 0.0F, 300.0F );
    top += corner;
    osg::vec3 right( 1100.0F, 0.0F, 0.0F );
    right += corner;

    // osg::Group* backdrop = new osg::Group;
    // backdrop->addChild(createBackdrop(corner,top,right));

    osg::ClearNode* backdrop = new osg::ClearNode;
    backdrop->setClearColor( osg::vec4( 1.0F, 1.0F, 1.0F, 0.0F ) );

    // osg::vec3 lightPosition(-500.0f,-2500.0f,500.0f);
    // osg::Node* scene = createShadowedScene(logo_group,backdrop,lightPosition,0.0f,0);

    osg::Group*    scene    = new osg::Group;

    osg::StateSet* stateset = scene->getOrCreateStateSet();

    // Add GLSL shaders for GL 4.6 core profile
    osg::Program*  program = new osg::Program;
    program->setName( "logo" );
    program->addShader( new osg::Shader( osg::Shader::VERTEX, logoVertexShader ) );
    program->addShader( new osg::Shader( osg::Shader::FRAGMENT, logoFragmentShader ) );
    stateset->setAttributeAndModes( program, osg::StateAttribute::ON );
    stateset->addUniform( new osg::Uniform( "baseTexture", 0 ) );
    stateset->addUniform( new osg::Uniform( "hasTexture", false ) );

    scene->addChild( logo_group );
    scene->addChild( backdrop );

    return scene;
}

int
main( int    argc,
      char** argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc, argv );

    osg::DisplaySettings::instance()->setMinimumNumAlphaBits( 8 );

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

    osgViewer::Viewer viewer;

    // add the state manipulator
    viewer.addEventHandler(
        new osgGA::StateSetManipulator( viewer.getCamera()->getOrCreateStateSet() )
    );

    // if user request help write it out to cout.
    if( arguments.read( "-h" ) || arguments.read( "--help" ) )
    {
        arguments.getApplicationUsage()->write( std::cout );
        return 1;
    }

    while( arguments.read( "--sdf" ) )
    {
        s_useSDF = true;
    }

    std::string label       = "OpenSceneGraph";
    std::string subscript   = "";

    bool        showVersion = false;
    while( arguments.read( "--version" ) )
    {
        showVersion = true;
    }
    if( showVersion )
    {
        label += " ";
        label += osgGetVersion();
    }

    while( arguments.read( "--label", label ) )
    {
    }
    while( arguments.read( "--subscript", subscript ) )
    {
    }

    osg::ref_ptr<osg::Node> node;

    if( arguments.argc() > 1 )
    {
        node = createLogo( arguments[1], label, subscript );
    }
    else
    {
        node = createLogo( "", label, subscript );
    }

    // add model to viewer.
    viewer.setSceneData( node.get() );
    return viewer.run();
}
