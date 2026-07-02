/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgtext example application
 */
#include <osg/core/io_utils.hpp>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Camera.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/Sequence.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/PolygonMode.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/io/WriteFile.hpp>
#include <osgDB/registry/Registry.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgText/Font.hpp>
#include <osgText/Text.hpp>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

osg::Group*
createHUDText()
{

    osg::Group*                 rootNode = new osg::Group;

    osg::ref_ptr<osgText::Font> font     = osgText::readRefFontFile( "fonts/arial.ttf" );

    // osg::setNotifyLevel(osg::INFO);

    osg::Geode*                 geode = new osg::Geode;
    rootNode->addChild( geode );

    float     windowHeight = 1024.0F;
    float     windowWidth  = 1280.0F;
    float     margin       = 50.0F;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Examples of how to set up different text layout
    //

    osg::vec4 layoutColor( 1.0F, 1.0F, 0.0F, 1.0F );
    float     layoutCharacterSize = 20.0F;

    {
        osgText::Text* text = new osgText::Text;
        text->setFont( font );
        text->setColor( layoutColor );
        text->setCharacterSize( layoutCharacterSize );
        text->setPosition( osg::vec3( margin, windowHeight - margin, 0.0F ) );

        // the default layout is left to right, typically used in languages
        // originating from europe such as English, French, German, Spanish etc..
        text->setLayout( osgText::Text::LEFT_TO_RIGHT );

        text->setText( "text->setLayout(osgText::Text::LEFT_TO_RIGHT);" );
        geode->addDrawable( text );
    }

    {
        osgText::Text* text = new osgText::Text;
        text->setFont( font );
        text->setColor( layoutColor );
        text->setCharacterSize( layoutCharacterSize );
        text->setPosition(
            osg::vec3( windowWidth - margin, windowHeight - margin, 0.0F )
        );

        // right to left layouts would be used for hebrew or arabic fonts.
        text->setLayout( osgText::Text::RIGHT_TO_LEFT );
        text->setAlignment( osgText::Text::RIGHT_BASE_LINE );

        text->setText( "text->setLayout(osgText::Text::RIGHT_TO_LEFT);" );
        geode->addDrawable( text );
    }

    {
        osgText::Text* text = new osgText::Text;
        text->setFont( font );
        text->setColor( layoutColor );
        text->setPosition( osg::vec3( margin, windowHeight - margin, 0.0F ) );
        text->setCharacterSize( layoutCharacterSize );

        // vertical font layout would be used for asian fonts.
        text->setLayout( osgText::Text::VERTICAL );

        text->setText( "text->setLayout(osgText::Text::VERTICAL);" );
        geode->addDrawable( text );
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Examples of how to set up different font resolution
    //

    osg::vec4 fontSizeColor( 0.0F, 1.0F, 1.0F, 1.0F );
    float     fontSizeCharacterSize = 30;

    osg::vec3 cursor = osg::vec3( margin * 2, windowHeight - margin * 2, 0.0F );

    {
        osgText::Text* text = new osgText::Text;
        text->setFont( font );
        text->setColor( fontSizeColor );
        text->setCharacterSize( fontSizeCharacterSize );
        text->setPosition( cursor );

        // use text that uses 10 by 10 texels as a target resolution for fonts.
        text->setFontResolution( 10, 10 );    // blocky but small texture memory usage

        text->setText(
            "text->setFontResolution(10,10); // blocky but small texture memory usage"
        );
        geode->addDrawable( text );
    }

    cursor.y -= fontSizeCharacterSize;
    {
        osgText::Text* text = new osgText::Text;
        text->setFont( font );
        text->setColor( fontSizeColor );
        text->setCharacterSize( fontSizeCharacterSize );
        text->setPosition( cursor );

        // use text that uses 20 by 20 texels as a target resolution for fonts.
        text->setFontResolution(
            20,
            20
        );    // smoother but higher texture memory usage (but still quite low).

        text->setText( "text->setFontResolution(20,20); // smoother but higher texture "
                       "memory usage (but still quite low)." );
        geode->addDrawable( text );
    }

    cursor.y -= fontSizeCharacterSize;
    {
        osgText::Text* text = new osgText::Text;
        text->setFont( font );
        text->setColor( fontSizeColor );
        text->setCharacterSize( fontSizeCharacterSize );
        text->setPosition( cursor );

        // use text that uses 40 by 40 texels as a target resolution for fonts.
        text->setFontResolution(
            40,
            40
        );    // even smoother but again higher texture memory usage.

        text->setText( "text->setFontResolution(40,40); // even smoother but again "
                       "higher texture memory usage." );
        geode->addDrawable( text );
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Examples of how to set up different sized text
    //

    osg::vec4 characterSizeColor( 1.0F, 0.0F, 1.0F, 1.0F );

    cursor.y -= fontSizeCharacterSize * 2.0F;

    {
        osgText::Text* text = new osgText::Text;
        text->setFont( font );
        text->setColor( characterSizeColor );
        text->setFontResolution( 20, 20 );
        text->setPosition( cursor );

        // use text that is 20 units high.
        text->setCharacterSize( 20 );    // small

        text->setText( "text->setCharacterSize(20.0f); // small" );
        geode->addDrawable( text );
    }

    cursor.y -= 30.0F;
    {
        osgText::Text* text = new osgText::Text;
        text->setFont( font );
        text->setColor( characterSizeColor );
        text->setFontResolution( 30, 30 );
        text->setPosition( cursor );

        // use text that is 30 units high.
        text->setCharacterSize( 30.0F );    // medium

        text->setText( "text->setCharacterSize(30.0f); // medium" );
        geode->addDrawable( text );
    }

    cursor.y -= 50.0F;
    {
        osgText::Text* text = new osgText::Text;
        text->setFont( font );
        text->setColor( characterSizeColor );
        text->setFontResolution( 40, 40 );
        text->setPosition( cursor );

        // use text that is 60 units high.
        text->setCharacterSize( 60.0F );    // large

        text->setText( "text->setCharacterSize(60.0f); // large" );
        geode->addDrawable( text );
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Examples of how to set up different alignments
    //

    osg::vec4 alignmentSizeColor( 0.0F, 1.0F, 0.0F, 1.0F );
    float     alignmentCharacterSize = 25.0F;
    cursor.x                         = 640;
    cursor.y                         = margin * 4.0F;

    typedef std::pair<osgText::Text::AlignmentType, std::string> AlignmentPair;
    typedef std::vector<AlignmentPair>                           AlignmentList;
    AlignmentList                                                alignmentList;
    alignmentList.push_back(
        AlignmentPair( osgText::Text::LEFT_TOP,
                       "text->setAlignment(\nosgText::Text::LEFT_TOP);" )
    );
    alignmentList.push_back(
        AlignmentPair( osgText::Text::LEFT_CENTER,
                       "text->setAlignment(\nosgText::Text::LEFT_CENTER);" )
    );
    alignmentList.push_back(
        AlignmentPair( osgText::Text::LEFT_BOTTOM,
                       "text->setAlignment(\nosgText::Text::LEFT_BOTTOM);" )
    );
    alignmentList.push_back(
        AlignmentPair( osgText::Text::CENTER_TOP,
                       "text->setAlignment(\nosgText::Text::CENTER_TOP);" )
    );
    alignmentList.push_back(
        AlignmentPair( osgText::Text::CENTER_CENTER,
                       "text->setAlignment(\nosgText::Text::CENTER_CENTER);" )
    );
    alignmentList.push_back(
        AlignmentPair( osgText::Text::CENTER_BOTTOM,
                       "text->setAlignment(\nosgText::Text::CENTER_BOTTOM);" )
    );
    alignmentList.push_back(
        AlignmentPair( osgText::Text::RIGHT_TOP,
                       "text->setAlignment(\nosgText::Text::RIGHT_TOP);" )
    );
    alignmentList.push_back(
        AlignmentPair( osgText::Text::RIGHT_CENTER,
                       "text->setAlignment(\nosgText::Text::RIGHT_CENTER);" )
    );
    alignmentList.push_back(
        AlignmentPair( osgText::Text::RIGHT_BOTTOM,
                       "text->setAlignment(\nosgText::Text::RIGHT_BOTTOM);" )
    );
    alignmentList.push_back(
        AlignmentPair( osgText::Text::LEFT_BASE_LINE,
                       "text->setAlignment(\nosgText::Text::LEFT_BASE_LINE);" )
    );
    alignmentList.push_back(
        AlignmentPair( osgText::Text::CENTER_BASE_LINE,
                       "text->setAlignment(\nosgText::Text::CENTER_BASE_LINE);" )
    );
    alignmentList.push_back(
        AlignmentPair( osgText::Text::RIGHT_BASE_LINE,
                       "text->setAlignment(\nosgText::Text::RIGHT_BASE_LINE);" )
    );
    alignmentList.push_back(
        AlignmentPair( osgText::Text::LEFT_BOTTOM_BASE_LINE,
                       "text->setAlignment(\nosgText::Text::LEFT_BOTTOM_BASE_LINE);" )
    );
    alignmentList.push_back(
        AlignmentPair( osgText::Text::CENTER_BOTTOM_BASE_LINE,
                       "text->setAlignment(\nosgText::Text::CENTER_BOTTOM_BASE_LINE);" )
    );
    alignmentList.push_back(
        AlignmentPair( osgText::Text::RIGHT_BOTTOM_BASE_LINE,
                       "text->setAlignment(\nosgText::Text::RIGHT_BOTTOM_BASE_LINE);" )
    );

    osg::Sequence* sequence = new osg::Sequence;
    {
        for( AlignmentList::iterator itr = alignmentList.begin();
             itr != alignmentList.end();
             ++itr )
        {
            osg::Geode* alignmentGeode = new osg::Geode;
            sequence->addChild( alignmentGeode );
            sequence->setTime( sequence->getNumChildren(), 1.0F );

            osgText::Text* text = new osgText::Text;
            text->setFont( font );
            text->setColor( alignmentSizeColor );
            text->setCharacterSize( alignmentCharacterSize );
            text->setPosition( cursor );
            text->setDrawMode( osgText::Text::TEXT |
                               osgText::Text::ALIGNMENT |
                               osgText::Text::BOUNDINGBOX );

            text->setAlignment( itr->first );
            text->setText( itr->second );

            alignmentGeode->addDrawable( text );
        }
    }

    sequence->setMode( osg::Sequence::START );
    sequence->setInterval( osg::Sequence::LOOP, 0, -1 );
    sequence->setDuration( 1.0F, -1 );

    rootNode->addChild( sequence );

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Examples of how to set up different fonts...
    //

    cursor.x = margin * 2.0F;
    cursor.y = margin * 2.0F;

    osg::vec4 fontColor( 1.0F, 0.5F, 0.0F, 1.0F );
    float     fontCharacterSize = 20.0F;
    float     spacing           = 40.0F;

    {
        osg::ref_ptr<osgText::Text> text = new osgText::Text;
        text->setColor( fontColor );
        text->setPosition( cursor );
        text->setCharacterSize( fontCharacterSize );

        text->setFont( 0 );
        text->setText( "text->setFont(0); // inbuilt font." );
        geode->addDrawable( text );

        cursor.x = text->getBoundingBox().xMax() + spacing;
    }

    {
        osg::ref_ptr<osgText::Font> arial =
            osgText::readRefFontFile( "fonts/arial.ttf" );

        osg::ref_ptr<osgText::Text> text = new osgText::Text;
        text->setColor( fontColor );
        text->setPosition( cursor );
        text->setCharacterSize( fontCharacterSize );

        text->setFont( arial );
        text->setText( arial != 0 ? "text->setFont(\"fonts/arial.ttf\");"
                                  : "unable to load \"fonts/arial.ttf\"" );
        geode->addDrawable( text );

        cursor.x = text->getBoundingBox().xMax() + spacing;
    }

    {
        osg::ref_ptr<osgText::Font> times =
            osgText::readRefFontFile( "fonts/times.ttf" );

        osg::ref_ptr<osgText::Text> text = new osgText::Text;
        text->setColor( fontColor );
        text->setPosition( cursor );
        text->setCharacterSize( fontCharacterSize );

        geode->addDrawable( text );
        text->setFont( times );
        text->setText( times != 0 ? "text->setFont(\"fonts/times.ttf\");"
                                  : "unable to load \"fonts/times.ttf\"" );

        cursor.x = text->getBoundingBox().xMax() + spacing;
    }

    cursor.x = margin * 2.0F;
    cursor.y = margin;

    {
        osg::ref_ptr<osgText::Font> dirtydoz =
            osgText::readRefFontFile( "fonts/dirtydoz.ttf" );

        osg::ref_ptr<osgText::Text> text = new osgText::Text;
        text->setColor( fontColor );
        text->setPosition( cursor );
        text->setCharacterSize( fontCharacterSize );

        text->setFont( dirtydoz );
        text->setText( dirtydoz != 0 ? "text->setFont(\"fonts/dirtydoz.ttf\");"
                                     : "unable to load \"fonts/dirtydoz.ttf\"" );
        geode->addDrawable( text );

        cursor.x = text->getBoundingBox().xMax() + spacing;
    }

    {
        osg::ref_ptr<osgText::Font> fudd = osgText::readRefFontFile( "fonts/fudd.ttf" );

        osg::ref_ptr<osgText::Text> text = new osgText::Text;
        text->setColor( fontColor );
        text->setPosition( cursor );
        text->setCharacterSize( fontCharacterSize );

        text->setFont( fudd );
        text->setText( fudd != 0 ? "text->setFont(\"fonts/fudd.ttf\");"
                                 : "unable to load \"fonts/fudd.ttf\"" );
        geode->addDrawable( text );

        cursor.x = text->getBoundingBox().xMax() + spacing;
    }

    return rootNode;
}

// create text which sits in 3D space such as would be inserted into a normal model
osg::Group*
create3DText( const osg::vec3& center,
              float            radius )
{

    osg::Geode*    geode = new osg::Geode;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Examples of how to set up axis/orientation alignments
    //

    float          characterSize = radius * 0.2F;

    osg::vec3      pos( center.x - radius * .5F,
                        center.y - radius * .5F,
                        center.z - radius * .5F );

    osgText::Text* text1 = new osgText::Text;
    text1->setFont( "fonts/times.ttf" );
    text1->setCharacterSize( characterSize );
    text1->setPosition( pos );
    text1->setAxisAlignment( osgText::Text::XY_PLANE );
    text1->setText( "XY_PLANE" );
    geode->addDrawable( text1 );

    osgText::Text* text2 = new osgText::Text;
    text2->setFont( "fonts/times.ttf" );
    text2->setCharacterSize( characterSize );
    text2->setPosition( pos );
    text2->setAxisAlignment( osgText::Text::YZ_PLANE );
    text2->setText( "YZ_PLANE" );
    geode->addDrawable( text2 );

    osgText::Text* text3 = new osgText::Text;
    text3->setFont( "fonts/times.ttf" );
    text3->setCharacterSize( characterSize );
    text3->setPosition( pos );
    text3->setAxisAlignment( osgText::Text::XZ_PLANE );
    text3->setText( "XZ_PLANE" );
    geode->addDrawable( text3 );

    osg::vec4      characterSizeModeColor( 1.0F, 0.0F, 0.5F, 1.0F );

    osgText::Text* text4 = new osgText::Text;
    text4->setFont( "fonts/times.ttf" );
    text4->setCharacterSize( characterSize );
    text4->setPosition( center );
    text4->setAxisAlignment( osgText::Text::SCREEN );

    // reproduce outline bounding box compute problem with backdrop on.
    text4->setBackdropType( osgText::Text::OUTLINE );
    text4->setDrawMode( osgText::Text::TEXT | osgText::Text::BOUNDINGBOX );

    text4->setText( "SCREEN" );
    geode->addDrawable( text4 );

    osgText::Text* text5 = new osgText::Text;
    text5->setColor( characterSizeModeColor );
    text5->setFont( "fonts/times.ttf" );
    // text5->setCharacterSize(characterSize);
    text5->setCharacterSize( 32.0F );    // medium
    text5->setPosition( center - osg::vec3( 0.0, 0.0, 0.2 ) * radius );
    text5->setAxisAlignment( osgText::Text::SCREEN );
    text5->setCharacterSizeMode( osgText::Text::SCREEN_COORDS );
    text5->setDrawMode( osgText::Text::TEXT | osgText::Text::BOUNDINGBOX );
    text5->setText( "CharacterSizeMode SCREEN_COORDS(size 32.0)" );
    geode->addDrawable( text5 );

    osgText::Text* text6 = new osgText::Text;
    text6->setColor( characterSizeModeColor );
    text6->setFont( "fonts/times.ttf" );
    text6->setCharacterSize( characterSize );
    text6->setPosition( center - osg::vec3( 0.0, 0.0, 0.4 ) * radius );
    text6->setAxisAlignment( osgText::Text::SCREEN );
    text6->setCharacterSizeMode(
        osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT
    );
    text6->setText(
        "CharacterSizeMode OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT"
    );
    geode->addDrawable( text6 );

    osgText::Text* text7 = new osgText::Text;
    text7->setColor( characterSizeModeColor );
    text7->setFont( "fonts/times.ttf" );
    text7->setCharacterSize( characterSize );
    text7->setPosition( center - osg::vec3( 0.0, 0.0, 0.6 ) * radius );
    text7->setAxisAlignment( osgText::Text::SCREEN );
    text7->setCharacterSizeMode( osgText::Text::OBJECT_COORDS );
    text7->setText( "CharacterSizeMode OBJECT_COORDS (default)" );
    geode->addDrawable( text7 );

#if 0
    osg::ShapeDrawable* shape = new osg::ShapeDrawable(new osg::Sphere(center,characterSize*0.2F));
    // GL_LIGHTING removed for Core Profile compatibility
    geode->addDrawable(shape);
#endif

    osg::Group* rootNode = new osg::Group;
    rootNode->addChild( geode );

    return rootNode;
}

class UpdateTextOperation : public osg::Operation
{
    public:

        UpdateTextOperation( const osg::vec3& center,
                             float            diameter,
                             osg::Group*      group ) :
            osg::Referenced( true ),
            Operation( "UpdateTextOperation",
                       true ),
            _center( center ),
            _diameter( diameter ),
            _maxNumChildren( 200 ),
            _maxNumTextPerGeode( 10 ),
            _group( group )
        {
        }

        virtual void
        operator()( osg::Object* callingObject )
        {
            // decided which method to call according to whole has called me.
            osgViewer::Viewer* viewer =
                dynamic_cast<osgViewer::Viewer*>( callingObject );

            if( viewer )
            {
                update();
            }
            else
            {
                load();
            }
        }

        void
        update()
        {
            // osg::notify(osg::NOTICE)<<"*** Doing update"<<std::endl;

            std::lock_guard<std::mutex> lock( _mutex );

            if( _mergeSubgraph.valid() )
            {
                _group->addChild( _mergeSubgraph.get() );

                _mergeSubgraph = 0;

                if( _group->getNumChildren() > _maxNumChildren )
                {
                    osg::Geode* geode =
                        dynamic_cast<osg::Geode*>( _group->getChild( 0 ) );
                    if( geode )
                    {
                        _availableSubgraph.push_back( geode );
                        geode->removeDrawables( 0, geode->getNumDrawables() );
                    }
                    _group->removeChild( 0, 1 );
                }

                _waitOnMergeBlock.release();
            }
        }

        void
        load()
        {

            // osg::notify(osg::NOTICE)<<"Doing load"<<std::endl;

            osg::ref_ptr<osg::Geode> geode;
            {
                std::lock_guard<std::mutex> lock( _mutex );
                if( !_availableSubgraph.empty() )
                {
                    geode = _availableSubgraph.front();
                    _availableSubgraph.pop_front();
                }
            }

            if( !geode )
            {
                geode = new osg::Geode;
            }

            for( unsigned int i = 0; i < _maxNumTextPerGeode; ++i )
            {
                float        x = float( rand() ) / float( RAND_MAX ) - 0.5F;
                float        y = float( rand() ) / float( RAND_MAX ) - 0.5F;
                float        z = float( i ) / float( _maxNumTextPerGeode ) - 0.5F;
                osg::vec3    position( x, y, z );

                std::string  str;
                unsigned int _numCharacters = 5;
                for( unsigned int ni = 0; ni < _numCharacters; ++ni )
                {
                    str.push_back(
                        char( 32.0 + ( float( rand() ) / float( RAND_MAX ) ) * 128.0F )
                    );
                }

                osgText::Text* text = new osgText::Text;
                text->setDataVariance( osg::Object::DataVariance::DYNAMIC );
                text->setPosition( _center + position * _diameter );
                text->setFont( "times.ttf" );
                text->setText( str );
                text->setCharacterSize( 0.025F * _diameter );
                text->setAxisAlignment( osgText::Text::SCREEN );

                geode->addDrawable( text );
            }

            {
                std::lock_guard<std::mutex> lock( _mutex );
                _mergeSubgraph = geode;
            }

            // osg::notify(osg::NOTICE)<<"Waiting on merge"<<std::endl;

            _waitOnMergeBlock.block();
        }

        virtual void
        release()
        {
            _waitOnMergeBlock.release();
        }

        typedef std::list<osg::ref_ptr<osg::Geode>> AvailableList;

        osg::vec3                                   _center;
        float                                       _diameter;
        unsigned int                                _maxNumChildren;
        unsigned int                                _maxNumTextPerGeode;

        std::mutex                                  _mutex;
        osg::ref_ptr<osg::Group>                    _group;
        osg::ref_ptr<osg::Geode>                    _mergeSubgraph;
        AvailableList                               _availableSubgraph;
        osg::Block                                  _waitOnMergeBlock;

        unsigned int                                _counter;
};

struct TextCounterCallback : public osg::NodeCallback
{
        unsigned int _textCounter;

        TextCounterCallback() :
            _textCounter( 100'000 )
        {
        }

        virtual void
        operator()( osg::Node* node,
                    osg::NodeVisitor* )
        {
            osgText::Text* text = dynamic_cast<osgText::Text*>( node );
            if( text )
            {
                std::stringstream str;
                str << "Text Counter " << _textCounter;
                OSG_NOTICE << "Updating text : " << str.str() << std::endl;

                text->setText( str.str() );

                ++_textCounter;
            }
        }
};

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    // Headless check: build default scene for offscreen capture before creating viewer
    {
        std::string headlessOutput;
        if( arguments.read( "--headless", headlessOutput ) )
        {
            osg::vec3    center( 0.0F, 0.0F, 0.0F );
            float        radius = 100.0F;
            osg::Group*  group  = new osg::Group;
            osg::Camera* camera = new osg::Camera;
            camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
            camera->setProjectionMatrixAsOrtho2D( 0, 1'280, 0, 1'024 );
            camera->setViewMatrix( osg::dmat4() );
            camera->setClearMask( GL_DEPTH_BUFFER_BIT );
            camera->addChild( createHUDText() );
            group->addChild( camera );
            group->addChild( create3DText( center, radius ) );
            return osg::headlessCapture( group, headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    // construct the viewer.
    osgViewer::Viewer                                     viewer( arguments );

    typedef std::list<osg::ref_ptr<osg::OperationThread>> Threads;

    Threads                                               operationThreads;
    osg::ref_ptr<UpdateTextOperation>                     updateOperation;
    unsigned int                                          numThreads = 0;

    if( arguments.read( "--simple" ) )
    {
        osg::ref_ptr<osgText::Text> text = new osgText::Text;
        text->setFont( "fonts/times.ttf" );
        text->setAxisAlignment( osgText::Text::XZ_PLANE );
        text->setDrawMode( osgText::Text::TEXT |
                           osgText::Text::ALIGNMENT |
                           osgText::Text::BOUNDINGBOX |
                           osgText::Text::FILLEDBOUNDINGBOX );
        text->setText( "This is a simple test" );

        viewer.setSceneData( text.get() );
    }
    else if( arguments.read( "--plain" ) )
    {
        osg::ref_ptr<osgText::Text> text = new osgText::Text;
        text->setFont( "fonts/times.ttf" );
        text->setAxisAlignment( osgText::Text::XZ_PLANE );
        text->setText( "This is a plain test" );

        viewer.setSceneData( text.get() );
    }
    else if( arguments.read( "--counter" ) )
    {
        osg::ref_ptr<osgText::Text> text = new osgText::Text;
        text->setUpdateCallback( new TextCounterCallback() );
        text->setFont( "fonts/times.ttf" );
        text->setAxisAlignment( osgText::Text::XZ_PLANE );
        text->setText( "Text Counter :" );

        viewer.setSceneData( text.get() );
    }
    else if( arguments.read( "--alignment" ) )
    {
        osg::ref_ptr<osg::Group> group = new osg::Group;

        {
            osg::ref_ptr<osgText::Text> text = new osgText::Text;
            text->setFont( "fonts/times.ttf" );
            text->setAxisAlignment( osgText::Text::XZ_PLANE );
            text->setAlignment( osgText::Text::RIGHT_TOP );
            text->setText( "Alignment\nBefore:" );
            group->addChild( text );
        }

        {
            osg::ref_ptr<osgText::Text> text = new osgText::Text;
            text->setFont( "fonts/times.ttf" );
            text->setAxisAlignment( osgText::Text::XZ_PLANE );
            text->setText( "Alignment\nAfter:" );
            text->setAlignment( osgText::Text::LEFT_TOP );
            group->addChild( text );
        }

        viewer.setSceneData( group.get() );
    }
    else if( arguments.read( "--mt", numThreads ) || arguments.read( "--mt" ) )
    {
        // construct a multi-threaded text updating test.
        if( numThreads == 0 )
        {
            numThreads = 1;
        }

        // create a group to add everything into.
        osg::Group*             mainGroup = new osg::Group;

        osg::vec3               center( 0.5F, 0.5F, 0.5F );
        float                   diameter    = 1.0F;

        osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles( arguments );
        if( loadedModel.valid() )
        {
            mainGroup->addChild( loadedModel );

            center   = loadedModel->getBound().center;
            diameter = loadedModel->getBound().radius * 2.0F;
        }

        for( unsigned int i = 0; i < numThreads; ++i )
        {
            osg::Group* textGroup = new osg::Group;
            mainGroup->addChild( textGroup );

            // create the background thread
            osg::OperationThread* operationThread = new osg::OperationThread;

            operationThreads.push_back( operationThread );

            // create the operation that will run in the background and
            // sync once per frame with the main viewer loop.
            updateOperation = new UpdateTextOperation( center, diameter, textGroup );

            // add the operation to the operation thread and start it.
            operationThread->add( updateOperation.get() );
            operationThread->startThread();

            // add the operation to the viewer to sync once per frame.
            viewer.addUpdateOperation( updateOperation.get() );

            // add a unit cube for the text to appear within.
            osg::Geode* geode = new osg::Geode;
            geode->getOrCreateStateSet()->setAttribute(
                new osg::PolygonMode( osg::PolygonMode::Face::FRONT_AND_BACK,
                                      osg::PolygonMode::Mode::LINE )
            );
            geode->addDrawable( new osg::ShapeDrawable( new osg::Box( center,
                                                                      diameter ) ) );

            mainGroup->addChild( geode );
        }

        viewer.setSceneData( mainGroup );
    }
    else
    {
        // prepare scene.
        osg::vec3   center( 0.0F, 0.0F, 0.0F );
        float       radius = 100.0F;

        // make sure the root node is group so we can add extra nodes to it.
        osg::Group* group = new osg::Group;

        if( true )
        {
            // create the hud.
            osg::Camera* camera = new osg::Camera;
            camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
            camera->setProjectionMatrixAsOrtho2D( 0, 1'280, 0, 1'024 );
            camera->setViewMatrix( osg::dmat4() );
            camera->setClearMask( GL_DEPTH_BUFFER_BIT );
            camera->addChild( createHUDText() );

            group->addChild( camera );
        }

        if( true )
        {
            group->addChild( create3DText( center, radius ) );
        }

        // set the scene to render
        viewer.setSceneData( group );
    }

    std::string filename;
    if( arguments.read( "-o", filename ) )
    {
        osgDB::writeNodeFile( *viewer.getSceneData(), filename );
        return 0;
    }

    viewer.addEventHandler(
        new osgGA::StateSetManipulator( viewer.getCamera()->getOrCreateStateSet() )
    );
    viewer.addEventHandler( new osgViewer::StatsHandler() );
    viewer.run();

    if( !operationThreads.empty() )
    {
        for( Threads::iterator itr = operationThreads.begin();
             itr != operationThreads.end();
             ++itr )
        {
            ( *itr )->cancel();
        }
    }

    return 0;
}
