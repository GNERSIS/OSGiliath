/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgrobot example application
 */
/*
    The code below is to show how a hierarchy of objects can be made within a scenegraph.
    In other words, how there can be a parent/child relationship between objects such
    that when a parent is rotated or translated, the children move is respect to it's
    parent movement.  A robotic arm is used in this example because this is what I'm
    using OSG for.

    To rotate the joints use the following table to rotate each respective joint.

    joint    key        rotation
    1        q        + 1 degree
    1        a        - 1 degree
    2        w        + 1 degree
    2        s        - 1 degree
    3        e        + 1 degree
    3        d        - 1 degree
    4        r        + 1 degree
    4        f        - 1 degree
    5        t        + 1 degree
    5        g        - 1 degree
    6        y        + 1 degree
    6        h        - 1 degree

    In many robotics projects, the x and z axis are only used to define the positions
    of the joints of a robotic arm.  By not using the y-axis, the mathematics of
    inverse kinematics becomes easier (and in some cases possible).  All joint rotations
    are rotated about the z-axis.  If you toggle the variable 'showAxis' to true, an
    x (color red), y (color green), and z (color blue) axis will display.  Using the
    thumb of the right hand to point in the same direction of the z-axis, use the right
    hand rule to define a positive angle of rotation.

    The robotic arm in general follows the algorithm
    rotate about the x axis, translate along the x-axis, translate along the z axis,
    and then rotate about the z-axis. There is 1 exception in the method buildJoint3(...)
    where it was necessary to add a 'cheat' to get the joints to align properly.  In a
    real robotics project such as this, the tube (see buildTube2(...) method) would not
    be a part of the mathematics and thus, not have to be drawn and would not require
    the cheat.  To learn moue about positioning joints using only the x and z axis,
    web search 'Denavit-Hartenberg'.  wikipedia has comments at the URL...
    http://en.wikipedia.org/wiki/Robotics_conventions#Denavit-Hartenberg_link_frame_convention_.28DH.29

    Lessons learned with osg...
    When starting this work I was very new to OSG.  When I first placed a cylinder
    in a scene and rotated the cylinder about the z and x axis, I noticed that the
   rotation is about the barycenter of the object for both axis.  I wanted a rotation
   about the z-axis to be along the length of the tube (which OSG gives for free -- not
   always true with all scenegraphs) while the rotation about the x and y axis about the
    beginning (or start position) of the tube.  Knowing what I know about barycenters
    I positioned the shape along the -height/2 (see example 1 below)
    -- believing the offset -height/2 would place the axis of rotation at the start of
    the tube.  I was certainly wrong about this and found out the equation needs to
    be height/2.

    ...
    example 1
    joint->addDrawable(new osg::ShapeDrawable(new
   osg::Capsule(osg::vec3(0.0f,0.0f,height/2),radius,height),hints));
    ...



    author: James Moliere
    date written: 10/12/2008
*/

#include <iostream>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/maths/Math.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/nodes/PositionAttitudeTransform.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/Material.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgViewer/core/Viewer.hpp>

float jointAngle1;
float jointAngle2;
float jointAngle3;
float jointAngle4;
float jointAngle5;
float jointAngle6;
float EndEffector;

void
createAxis( osg::Transform* previousJoint );

osg::MatrixTransform* joint1;
osg::MatrixTransform* joint2;
osg::MatrixTransform* joint3;
osg::MatrixTransform* joint4;
osg::MatrixTransform* joint5;
osg::MatrixTransform* joint6;

osg::MatrixTransform*
buildJoint1( osg::MatrixTransform* previousJoint );
osg::MatrixTransform*
buildJoint2( osg::MatrixTransform* previousJoint );
osg::MatrixTransform*
buildTube2( osg::MatrixTransform* previousJoint );
osg::MatrixTransform*
buildJoint3( osg::MatrixTransform* previousJoint );
osg::MatrixTransform*
buildJoint4( osg::MatrixTransform* previousJoint );
osg::MatrixTransform*
buildTube5( osg::MatrixTransform* previousJoint );
osg::MatrixTransform*
buildJoint5( osg::MatrixTransform* previousJoint );
osg::MatrixTransform*
buildJoint6( osg::MatrixTransform* previousJoint );
osg::MatrixTransform*
buildEndEffector();
osg::TessellationHints* hints = new osg::TessellationHints;

bool                    showAxis;

class KeyboardEventHandler : public osgGA::GUIEventHandler
{
    public:

        KeyboardEventHandler()
        {
        }

        static void
        rotate( float                 angle,
                osg::MatrixTransform* joint )
        {
            osg::dmat4 zRot;
            zRot = osg::dmat4(
                osg::rotate( osg::quat( angle, osg::vec3( 0.0F, 0.0F, 1.0F ) ) )
            );
            joint->setMatrix( joint->getMatrix() * zRot );
        }

        virtual bool
        handle( const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter& )
        {
            switch( ea.getEventType() )
            {
                case( osgGA::GUIEventAdapter::KEYDOWN ) :
                    {
                        switch( ea.getKey() )
                        {
                            case 'q' :
                                rotate( osg::PI / 180, joint1 );
                                return true;
                            case 'a' :
                                rotate( -osg::PI / 180, joint1 );
                                return true;
                            case 'w' :
                                rotate( osg::PI / 180, joint2 );
                                return true;
                            case 's' :
                                rotate( -osg::PI / 180, joint2 );
                                return true;
                            case 'e' :
                                rotate( osg::PI / 180, joint3 );
                                return true;
                            case 'd' :
                                rotate( -osg::PI / 180, joint3 );
                                return true;
                            case 'r' :
                                rotate( osg::PI / 180, joint4 );
                                return true;
                            case 'f' :
                                rotate( -osg::PI / 180, joint4 );
                                return true;
                            case 't' :
                                rotate( osg::PI / 180, joint5 );
                                return true;
                            case 'g' :
                                rotate( -osg::PI / 180, joint5 );
                                return true;
                            case 'y' :
                                rotate( osg::PI / 180, joint6 );
                                return true;
                            case 'h' :
                                rotate( -osg::PI / 180, joint6 );
                                return true;
                        }
                    }
                default :
                    break;
            }
            // return false to allow mouse manipulation
            return false;
        }
};

osg::Group*
createShapes()
{
    osg::Group*           group     = new osg::Group();
    osg::MatrixTransform* transform = new osg::MatrixTransform();
    group->addChild( transform );

    joint1                      = buildJoint1( transform );
    joint2                      = buildJoint2( joint1 );
    osg::MatrixTransform* tube2 = buildTube2( joint2 );
    joint3                      = buildJoint3( tube2 );
    joint4                      = buildJoint4( joint3 );
    osg::MatrixTransform* tube5 = buildTube5( joint4 );
    joint5                      = buildJoint5( tube5 );
    joint6                      = buildJoint6( joint5 );
    joint6->addChild( buildEndEffector() );
    return group;
}

osg::MatrixTransform*
buildJoint1( osg::MatrixTransform* previousJoint )
{
    osg::MatrixTransform* xTransform = new osg::MatrixTransform();
    previousJoint->addChild( xTransform );
    float       radius = 6.7640F;
    float       height = 45.0F;
    osg::Geode* joint  = new osg::Geode();
    xTransform->addChild( joint );
    joint->addDrawable( new osg::ShapeDrawable(
        new osg::Cylinder( osg::vec3( 0.0F, 0.0F, height / 2 ), radius, height ),
        hints
    ) );

    osg::MatrixTransform* zTransform = new osg::MatrixTransform();
    xTransform->addChild( zTransform );
    osg::dmat4 zTrans = osg::translate( 0.0, 0.0, ( double )height );
    osg::dmat4 zRot   = osg::rotate( ( double )jointAngle1, 0.0, 0.0, 1.0 );
    zTransform->setMatrix( zRot * zTrans );
    return zTransform;
}

osg::MatrixTransform*
buildJoint2( osg::MatrixTransform* previousJoint )
{
    if( showAxis )
    {
        createAxis( previousJoint );
    }
    double                height     = 17.6;
    double                radius     = 4.45633;
    osg::MatrixTransform* xTransform = new osg::MatrixTransform();
    previousJoint->addChild( xTransform );
    osg::dmat4 xRot = osg::rotate( osg::PI * 0.5, 1.0, 0.0, 0.0 );
    xTransform->setMatrix( xRot );
    osg::Geode* joint = new osg::Geode();
    joint->addDrawable( new osg::ShapeDrawable(
        new osg::Capsule( osg::vec3( 0.0F, 0.0F, ( float )( height / 2 ) ),
                          ( float )radius,
                          ( float )height ),
        hints
    ) );
    xTransform->addChild( joint );

    osg::MatrixTransform* zTransform = new osg::MatrixTransform();
    osg::dmat4            zTrans     = osg::translate( 0.0, 0.0, height );
    osg::dmat4            zRot =
        osg::rotate( osg::PI * 0.5 + ( double )jointAngle2, 0.0, 0.0, 1.0 );
    zTransform->setMatrix( zRot * zTrans );
    xTransform->addChild( zTransform );
    return zTransform;
}

osg::MatrixTransform*
buildTube2( osg::MatrixTransform* previousJoint )
{
    if( showAxis )
    {
        createAxis( previousJoint );
    }
    double                height     = 17.6;
    double                radius     = 4.45633;
    osg::MatrixTransform* xTransform = new osg::MatrixTransform();
    previousJoint->addChild( xTransform );
    osg::dmat4 xRot = osg::rotate( osg::PI * 0.5, 1.0, 0.0, 0.0 );
    xTransform->setMatrix( xRot );
    osg::Geode* tube3 = new osg::Geode();
    xTransform->addChild( tube3 );
    tube3->addDrawable( new osg::ShapeDrawable(
        new osg::Capsule( osg::vec3( 0.0F, 0.0F, height / 2 ), radius, height ),
        hints
    ) );

    osg::MatrixTransform* zTransform = new osg::MatrixTransform();
    xTransform->addChild( zTransform );
    osg::dmat4 zTrans = osg::translate( 0.0, 0.0, static_cast<double>( height ) );
    zTransform->setMatrix( zTrans );
    return zTransform;
}

osg::MatrixTransform*
buildJoint3( osg::MatrixTransform* previousJoint )
{
    double                height     = 7.5;
    double                radius     = 4.45633;
    osg::Geode*           joint      = new osg::Geode();
    osg::MatrixTransform* xTransform = new osg::MatrixTransform();
    previousJoint->addChild( xTransform );
    osg::dmat4 xRot = osg::rotate( -osg::PI * 0.5, 1.0, 0.0, 0.0 );
    xTransform->setMatrix( xRot );

    osg::MatrixTransform* zCheat      = new osg::MatrixTransform();
    osg::dmat4            zTransCheat = osg::translate( 0.0, 0.0, -height );
    zCheat->setMatrix( zTransCheat );
    xTransform->addChild( zCheat );

    osg::ShapeDrawable* shape = new osg::ShapeDrawable(
        new osg::Capsule( osg::vec3( 0.0F, 0.0F, height / 2 ), radius, height ),
        hints
    );
    joint->addDrawable( shape );
    zCheat->addChild( joint );

    osg::MatrixTransform* zTransform = new osg::MatrixTransform();
    zCheat->addChild( zTransform );
    osg::dmat4 zRot   = osg::rotate( ( double )jointAngle3, 0.0, 0.0, 1.0 );
    osg::dmat4 zTrans = osg::translate( 0.0, 0.0, 0.0 );
    zTransform->setMatrix( zRot * zTrans );
    if( showAxis )
    {
        createAxis( zTransform );
    }
    return zTransform;
}

osg::MatrixTransform*
buildJoint4( osg::MatrixTransform* previousJoint )
{
    double      height = 17.5;
    double      radius = 2.86479;
    osg::Geode* tube4  = new osg::Geode();
    tube4->addDrawable( new osg::ShapeDrawable(
        new osg::Capsule( osg::vec3( 0.0F, 0.0F, height / 2 ), radius, height ),
        hints
    ) );
    osg::MatrixTransform* xTransform = new osg::MatrixTransform();
    // if (showAxis)
    //{
    // createAxis(xTransform);
    //}
    previousJoint->addChild( xTransform );
    xTransform->addChild( tube4 );
    osg::dmat4 xRot = osg::rotate( osg::PI * 0.5, 1.0, 0.0, 0.0 );
    xTransform->setMatrix( xRot );
    height            = 7.5;
    osg::dmat4 zTrans = osg::translate( 0.0, 0.0, 17.5 );
    osg::dmat4 zRot =
        osg::rotate( ( double )jointAngle4 - osg::PI * 0.5, 0.0, 0.0, 1.0 );

    osg::MatrixTransform* zTransform = new osg::MatrixTransform();
    zTransform->setMatrix( zRot * zTrans );
    xTransform->addChild( zTransform );
    return zTransform;
}

osg::MatrixTransform*
buildTube5( osg::MatrixTransform* previousJoint )
{
    if( showAxis )
    {
        createAxis( previousJoint );
    }
    double height             = 7.5;
    double radius             = 2.86479;

    height                    = 15.0;
    osg::Geode*         joint = new osg::Geode();
    osg::ShapeDrawable* shape = new osg::ShapeDrawable(
        new osg::Cylinder( osg::vec3( 0.0F, 0.0F, height / 2 ), radius, height ),
        hints
    );
    joint->addDrawable( shape );
    osg::MatrixTransform* xTransform = new osg::MatrixTransform();
    previousJoint->addChild( xTransform );
    xTransform->addChild( joint );

    osg::MatrixTransform* zTransform = new osg::MatrixTransform();
    osg::dmat4            zTrans     = osg::translate( 0.0, 0.0, ( double )height );
    zTransform->setMatrix( zTrans );
    xTransform->addChild( zTransform );
    return zTransform;
}

osg::MatrixTransform*
buildJoint5( osg::MatrixTransform* previousJoint )
{
    double              radius = 2.86479;
    double              height = 2.86479 * 2;
    osg::Geode*         joint  = new osg::Geode();
    osg::ShapeDrawable* shape  = new osg::ShapeDrawable(
        new osg::Cylinder( osg::vec3( 0.0F, 0.0F, 0.0F ), radius, height ),
        hints
    );
    joint->addDrawable( shape );
    osg::MatrixTransform* xTransform = new osg::MatrixTransform();
    previousJoint->addChild( xTransform );
    osg::dmat4 xRot = osg::rotate( -osg::PI * 0.5, 1.0, 0.0, 0.0 );
    xTransform->setMatrix( xRot );
    xTransform->addChild( joint );
    if( showAxis )
    {
        createAxis( xTransform );
    }

    osg::MatrixTransform* zTransform = new osg::MatrixTransform();
    osg::dmat4            zRot   = osg::rotate( ( double )jointAngle5, 0.0, 0.0, 1.0 );
    osg::dmat4            zTrans = osg::translate( 0.0, 0.0, 0.0 );
    zTransform->setMatrix( zRot * zTrans );
    xTransform->addChild( zTransform );
    return zTransform;
}

osg::MatrixTransform*
buildJoint6( osg::MatrixTransform* previousJoint )
{
    double      height = 3.0;
    double      radius = 1.0;
    osg::Geode* joint  = new osg::Geode();
    joint->addDrawable( new osg::ShapeDrawable(
        new osg::Cylinder( osg::vec3( 0.0F, 0.0F, height / 2 ), radius, height ),
        hints
    ) );
    osg::MatrixTransform* xTransform = new osg::MatrixTransform();
    osg::dmat4            xRot       = osg::rotate( osg::PI * 0.5, 1.0, 0.0, 0.0 );
    xTransform->setMatrix( xRot );
    xTransform->addChild( joint );
    previousJoint->addChild( xTransform );
    if( showAxis )
    {
        createAxis( xTransform );
    }
    return xTransform;
}

osg::MatrixTransform*
buildEndEffector()
{
    osg::MatrixTransform* mt = new osg::MatrixTransform();
    osg::dmat4            m;
    double                length = 17.0;
    m                            = osg::translate( 0.0, 0.0, length / 2 );
    mt->setMatrix( m );
    osg::Geode*         geode_3 = new osg::Geode;
    osg::ShapeDrawable* shape1  = new osg::ShapeDrawable(
        new osg::Box( osg::vec3( -EndEffector, 0.0F, 0.0F ), .5, 1.5F, length ),
        hints
    );
    osg::ShapeDrawable* shape2 = new osg::ShapeDrawable(
        new osg::Box( osg::vec3( EndEffector, 0.0F, 0.0F ), .5, 1.5F, length ),
        hints
    );
    shape1->setColor( osg::vec4( 0.8F, 0.8F, 0.4F, 1.0F ) );
    shape2->setColor( osg::vec4( 0.8F, 0.8F, 0.4F, 1.0F ) );
    geode_3->addDrawable( shape1 );
    geode_3->addDrawable( shape2 );
    mt->addChild( geode_3 );
    return mt;
}

void
createAxis( osg::Transform* previousJoint )
{
    double                height = 12.0;
    double                radius = .5;

    osg::MatrixTransform* zmt    = new osg::MatrixTransform();

    previousJoint->addChild( zmt );
    osg::ShapeDrawable* zShape = new osg::ShapeDrawable(
        new osg::Cylinder( osg::vec3( 0.0F, 0.0F, height / 2 ), radius, height ),
        hints
    );
    osg::ShapeDrawable* zCone = new osg::ShapeDrawable(
        new osg::Cone( osg::vec3( 0.0F, 0.0F, 1.0 ), radius + 1.0, 2.0 ),
        hints
    );

    osg::MatrixTransform* zmtCone = new osg::MatrixTransform();
    osg::Geode*           zgCone  = new osg::Geode;

    zmtCone->setMatrix( osg::translate( 0.0, 0.0, ( double )height ) );
    previousJoint->addChild( zmtCone );

    zShape->setColor( osg::vec4( 0.0F, 0.0F, 1.0F, 1.0F ) );
    zCone->setColor( osg::vec4( 0.0F, 0.0F, 1.0F, 1.0F ) );
    osg::Geode* z = new osg::Geode;
    z->addDrawable( zShape );
    zgCone->addDrawable( zCone );
    zmtCone->addChild( zgCone );
    zmt->addChild( z );

    osg::MatrixTransform* mt = new osg::MatrixTransform();
    previousJoint->addChild( mt );

    osg::dmat4 xMatrix = osg::rotate( -osg::PI * 0.5, 0.0, 1.0, 0.0 );
    mt->setMatrix( xMatrix );

    osg::ShapeDrawable* xShape = new osg::ShapeDrawable(
        new osg::Cylinder( osg::vec3( 0.0F, 0.0F, height / 2 ), radius, height ),
        hints
    );
    xShape->setColor( osg::vec4( 1.0F, 0.0F, 0.0F, 1.0F ) );
    osg::Geode* x = new osg::Geode;
    x->addDrawable( xShape );
    mt->addChild( x );

    osg::MatrixTransform* yMt = new osg::MatrixTransform();
    previousJoint->addChild( yMt );
    osg::dmat4 yMatrix = osg::rotate( osg::PI * 0.5, 1.0, 0.0, 0.0 );
    yMt->setMatrix( yMatrix );

    osg::ShapeDrawable* yShape = new osg::ShapeDrawable(
        new osg::Cylinder( osg::vec3( 0.0F, 0.0F, height / 2 ), radius, height ),
        hints
    );
    yShape->setColor( osg::vec4( 0.0F, 1.0F, 0.0F, 1.0F ) );
    osg::Geode* y = new osg::Geode;
    y->addDrawable( yShape );
    yMt->addChild( y );
}

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    hints->setDetailRatio( 0.5F );
    showAxis    = true;
    jointAngle1 = 0.0;
    jointAngle2 = 0.0;
    jointAngle3 = 0.0;
    jointAngle4 = 0.0;
    jointAngle5 = 0.0;
    jointAngle6 = 0.0;
    EndEffector = 1.0;
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
    viewer.addEventHandler( new KeyboardEventHandler() );

    // add model to viewer.
    viewer.setSceneData( createShapes() );
    return viewer.run();
}
