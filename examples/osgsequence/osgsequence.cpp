/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgsequence example application
 */
#include <iostream>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/nodes/Projection.hpp>
#include <osg/nodes/Sequence.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgText/Text.hpp>
#include <osgViewer/core/Viewer.hpp>

// create text drawable at 'pos'
osg::Geode*
createText( const std::string& str,
            const osg::vec3&   pos )
{
    // text drawable
    osgText::Text* text = new osgText::Text;
    text->setFont( std::string( "fonts/arial.ttf" ) );
    text->setPosition( pos );
    text->setText( str );

    // geode
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable( text );

    return geode;
}

osg::Node*
createTextGroup( const char** text )
{
    osg::Group*     group = new osg::Group;

    osg::vec3       pos( 120.0F, 800.0F, 0.0F );
    const osg::vec3 delta( 0.0F, -60.0F, 0.0F );

    // header
    const char**    t = text;
    group->addChild( createText( *t++, pos ) );
    pos += delta;

    // remainder of text under sequence
    osg::Sequence* seq = new osg::Sequence;
    group->addChild( seq );
    while( *t )
    {
        seq->addChild( createText( *t++, pos ) );
        seq->setTime( seq->getNumChildren() - 1, 2.0F );
        pos += delta;
    }

    // loop through all children
    seq->setInterval( osg::Sequence::LOOP, 0, -1 );

    // real-time playback, repeat indefinitely
    seq->setDuration( 1.0F, -1 );

    // must be started explicitly
    seq->setMode( osg::Sequence::START );

    return group;
}

osg::Node*
createHUD( osg::Node* node )
{
    // absolute transform
    osg::MatrixTransform* modelview_abs = new osg::MatrixTransform;
    modelview_abs->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    modelview_abs->setMatrix( osg::dmat4() );
    modelview_abs->addChild( node );

    // 2D projection node
    osg::Projection* projection = new osg::Projection;
    projection->setMatrix( osg::ortho2D( 0.0, ( double )1'280, 0.0, ( double )1'024 ) );
    projection->addChild( modelview_abs );

    // turn off lighting and depth test
    osg::StateSet* state = modelview_abs->getOrCreateStateSet();
    state->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );

    return projection;
}

osg::Node*
createScaledNode( osg::Node* node,
                  float      targetScale )
{
    // create scale matrix
    osg::MatrixTransform* transform = new osg::MatrixTransform;

    const osg::sphere&    bsphere   = node->getBound();
    float                 scale     = targetScale / bsphere.radius;
    transform->setMatrix( osg::dmat4( osg::scale( scale, scale, scale ) ) );
    transform->setDataVariance( osg::Object::DataVariance::STATIC );
    transform->addChild( node );

    return transform;
}

osg::Sequence*
createSequence( osg::ArgumentParser& arguments )
{
    // assumes any remaining parameters are models
    osg::Sequence*                   seq = new osg::Sequence;

    typedef std::vector<std::string> Filenames;
    Filenames                        filenames;

    if( arguments.argc() > 1 )
    {
        for( int i = 1; i < arguments.argc(); ++i )
        {
            filenames.push_back( arguments[i] );
        }
    }
    else
    {
        filenames.push_back( "duck.glb" );
        filenames.push_back( "milk_truck.glb" );
        filenames.push_back( "damaged_helmet.glb" );
        filenames.push_back( "fox.glb" );
    }

    for( Filenames::iterator itr = filenames.begin(); itr != filenames.end(); ++itr )
    {
        // load model
        osg::ref_ptr<osg::Node> node = osgDB::readRefNodeFile( *itr );

        if( node )
        {
            seq->addChild( createScaledNode( node.get(), 100.0F ) );
            seq->setTime( seq->getNumChildren() - 1, 1.0F );
        }
    }

    // loop through all children
    seq->setInterval( osg::Sequence::LOOP, 0, -1 );

    // real-time playback, repeat indefinitely
    seq->setDuration( 1.0F, -1 );

    seq->setMode( osg::Sequence::START );

    return seq;
}

// event handler to control sequence
class SequenceEventHandler : public osgGA::GUIEventHandler
{
    public:

        SequenceEventHandler( osg::Sequence* seq )
        {
            _seq = seq;
        }

        // handle keydown events
        virtual bool
        handle( const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter& )
        {
            if( ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN )
            {
                switch( ea.getKey() )
                {
                    case 's' :
                        {
                            osg::Sequence::SequenceMode mode = _seq->getMode();
                            if( mode == osg::Sequence::STOP )
                            {
                                mode = osg::Sequence::START;
                                std::cerr << "Start" << std::endl;
                            }
                            else if( mode == osg::Sequence::PAUSE )
                            {
                                mode = osg::Sequence::RESUME;
                                std::cerr << "Resume" << std::endl;
                            }
                            else
                            {
                                mode = osg::Sequence::PAUSE;
                                std::cerr << "Pause" << std::endl;
                            }
                            _seq->setMode( mode );
                        }
                        break;
                    case 'l' :
                        {
                            osg::Sequence::LoopMode mode;
                            int                     begin, end;
                            _seq->getInterval( mode, begin, end );
                            if( mode == osg::Sequence::LOOP )
                            {
                                mode = osg::Sequence::SWING;
                                std::cerr << "Swing" << std::endl;
                            }
                            else
                            {
                                mode = osg::Sequence::LOOP;
                                std::cerr << "Loop" << std::endl;
                            }
                            _seq->setInterval( mode, begin, end );
                        }
                        break;
                    default :
                        break;
                }
            }

            return false;
        }

    private:

        osg::ref_ptr<osg::Sequence> _seq;
};

int
main( int    argc,
      char** argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc, argv );

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
    // root
    osg::Group*       rootNode = new osg::Group;

    // create info display
    const char*       text[] = {
        "osg::Sequence Mini-Howto",
        "- can be used for simple flip-book-style animation",
        "- is subclassed from osg::Switch",
        "- assigns a display duration to each child",
        "- can loop or swing through an interval of it's children",
        "- can repeat the interval a number of times or indefinitely",
        "- press 's' to start/pause/resume",
        "- press 'l' to toggle loop/swing mode",
        NULL
    };
    rootNode->addChild( createHUD( createTextGroup( text ) ) );

    // add sequence of models from command line
    osg::Sequence* seq = createSequence( arguments );
    rootNode->addChild( seq );

    // add model to viewer.
    viewer.setSceneData( rootNode );

    // add event handler to control sequence
    viewer.addEventHandler( new SequenceEventHandler( seq ) );
    return viewer.run();
}
