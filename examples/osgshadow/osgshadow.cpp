/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgshadow example application
 */
#include <iostream>
#include <osg/core/ArgumentParser.hpp>
#include <osg/core/io_utils.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osg/traversal/ComputeBoundsVisitor.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/io/WriteFile.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgGA/manipulators/AnimationPathManipulator.hpp>
#include <osgGA/manipulators/DriveManipulator.hpp>
#include <osgGA/manipulators/FlightManipulator.hpp>
#include <osgGA/manipulators/KeySwitchMatrixManipulator.hpp>
#include <osgGA/manipulators/TerrainManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgShadow/LightSpacePerspectiveShadowMap.hpp>
#include <osgShadow/ParallelSplitShadowMap.hpp>
#include <osgShadow/ShadowedScene.hpp>
#include <osgShadow/ShadowMap.hpp>
#include <osgShadow/ShadowTexture.hpp>
#include <osgShadow/SoftShadowMap.hpp>
#include <osgShadow/StandardShadowMap.hpp>
#include <osgShadow/ViewDependentShadowMap.hpp>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

// for the grid data..
#include "terrain_coords.hpp"
// for the model number four - island scene
#include "IslandScene.hpp"

#include <osg/rendering/HeadlessCapture.hpp>

class ChangeFOVHandler : public osgGA::GUIEventHandler
{
    public:

        ChangeFOVHandler( osg::Camera* camera ) :
            _camera( camera )
        {
            double fovy, aspectRatio, zNear, zFar;
            osg::getPerspective( _camera->getProjectionMatrix(),
                                 fovy,
                                 aspectRatio,
                                 zNear,
                                 zFar );
            std::cout << "FOV is " << fovy << std::endl;
        }

        /** Deprecated, Handle events, return true if handled, false otherwise. */
        virtual bool
        handle( const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter& /*aa*/ )
        {
            if( ea.getEventType() == osgGA::GUIEventAdapter::KEYUP )
            {
                if( ea.getKey() == '-' || ea.getKey() == '=' || ea.getKey() == '0' )
                {
                    double fovy, aspectRatio, zNear, zFar;
                    osg::getPerspective( _camera->getProjectionMatrix(),
                                         fovy,
                                         aspectRatio,
                                         zNear,
                                         zFar );

                    if( ea.getKey() == '-' )
                    {
                        fovy -= 5.0;
                    }

                    if( ea.getKey() == '=' )
                    {
                        fovy += 5.0;
                    }

                    if( ea.getKey() == '0' )
                    {
                        fovy = 45.0;
                    }

                    std::cout << "Setting FOV to " << fovy << std::endl;
                    _camera->setProjectionMatrix(
                        osg::perspective( fovy, aspectRatio, zNear, zFar )
                    );

                    return true;
                }
            }

            return false;
        }

        osg::ref_ptr<osg::Camera> _camera;
};

class DumpShadowVolumesHandler : public osgGA::GUIEventHandler
{
    public:

        DumpShadowVolumesHandler()
        {
            set( false );
        }

        bool
        get()
        {
            return _value;
        }

        void
        set( bool value )
        {
            _value = value;
        }

        /** Deprecated, Handle events, return true if handled, false otherwise. */
        virtual bool
        handle( const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter& /*aa*/ )
        {
            if( ea.getEventType() == osgGA::GUIEventAdapter::KEYUP )
            {
                if( ea.getKey() == 'D' )
                {
                    set( true );
                    return true;
                }
            }

            return false;
        }

        bool _value;
};

class LightAnimationHandler : public osgGA::GUIEventHandler
{
    public:

        LightAnimationHandler( bool flag = true ) :
            _animating( flag )
        {
        }

        void
        setAnimating( bool flag )
        {
            _animating = flag;
        }

        bool
        getAnimating() const
        {
            return _animating;
        }

        /** Deprecated, Handle events, return true if handled, false otherwise. */
        virtual bool
        handle( const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter& /*aa*/ )
        {
            if( ea.getEventType() == osgGA::GUIEventAdapter::KEYUP )
            {
                if( ea.getKey() == 'p' )
                {
                    _animating = !_animating;
                    return true;
                }
            }

            return false;
        }

        bool _animating;
};

static int ReceivesShadowTraversalMask = 0X1;
static int CastsShadowTraversalMask    = 0X2;

namespace ModelOne
{

    enum Faces
    {
        FRONT_FACE  = 1,
        BACK_FACE   = 2,
        LEFT_FACE   = 4,
        RIGHT_FACE  = 8,
        TOP_FACE    = 16,
        BOTTOM_FACE = 32,
    };

    osg::Node*
    createCube( unsigned int mask )
    {
        osg::Geode*    geode    = new osg::Geode;

        osg::Geometry* geometry = new osg::Geometry;
        geode->addDrawable( geometry );

        osg::Vec3Array* vertices = new osg::Vec3Array;
        geometry->setVertexArray( vertices );

        osg::Vec3Array* normals = new osg::Vec3Array;
        geometry->setNormalArray( normals, osg::Array::BIND_PER_VERTEX );

        osg::Vec4Array* colours = new osg::Vec4Array;
        geometry->setColorArray( colours, osg::Array::BIND_OVERALL );
        colours->push_back( osg::vec4( 1.0F, 1.0F, 1.0F, 1.0F ) );

        osg::vec3 origin( 0.0F, 0.0F, 0.0F );
        osg::vec3 dx( 2.0F, 0.0F, 0.0F );
        osg::vec3 dy( 0.0F, 1.0F, 0.0F );
        osg::vec3 dz( 0.0F, 0.0F, 1.0F );

        osg::vec3 px( 1.0F, 0.0, 0.0F );
        osg::vec3 nx( -1.0F, 0.0, 0.0F );
        osg::vec3 py( 0.0F, 1.0F, 0.0F );
        osg::vec3 ny( 0.0F, -1.0F, 0.0F );
        osg::vec3 pz( 0.0F, 0.0F, 1.0F );
        osg::vec3 nz( 0.0F, 0.0F, -1.0F );

        if( mask & FRONT_FACE )
        {
            // front face
            vertices->push_back( origin );
            vertices->push_back( origin + dx );
            vertices->push_back( origin + dx + dz );
            vertices->push_back( origin + dz );
            normals->push_back( ny );
            normals->push_back( ny );
            normals->push_back( ny );
            normals->push_back( ny );
        }

        if( mask & BACK_FACE )
        {
            // back face
            vertices->push_back( origin + dy );
            vertices->push_back( origin + dy + dz );
            vertices->push_back( origin + dy + dx + dz );
            vertices->push_back( origin + dy + dx );
            normals->push_back( py );
            normals->push_back( py );
            normals->push_back( py );
            normals->push_back( py );
        }

        if( mask & LEFT_FACE )
        {
            // left face
            vertices->push_back( origin + dy );
            vertices->push_back( origin );
            vertices->push_back( origin + dz );
            vertices->push_back( origin + dy + dz );
            normals->push_back( nx );
            normals->push_back( nx );
            normals->push_back( nx );
            normals->push_back( nx );
        }

        if( mask & RIGHT_FACE )
        {
            // right face
            vertices->push_back( origin + dx + dy );
            vertices->push_back( origin + dx + dy + dz );
            vertices->push_back( origin + dx + dz );
            vertices->push_back( origin + dx );
            normals->push_back( px );
            normals->push_back( px );
            normals->push_back( px );
            normals->push_back( px );
        }

        if( mask & TOP_FACE )
        {
            // top face
            vertices->push_back( origin + dz );
            vertices->push_back( origin + dz + dx );
            vertices->push_back( origin + dz + dx + dy );
            vertices->push_back( origin + dz + dy );
            normals->push_back( pz );
            normals->push_back( pz );
            normals->push_back( pz );
            normals->push_back( pz );
        }

        if( mask & BOTTOM_FACE )
        {
            // bottom face
            vertices->push_back( origin );
            vertices->push_back( origin + dy );
            vertices->push_back( origin + dx + dy );
            vertices->push_back( origin + dx );
            normals->push_back( nz );
            normals->push_back( nz );
            normals->push_back( nz );
            normals->push_back( nz );
        }

        osg::DrawElementsUShort* indices = new osg::DrawElementsUShort( GL_TRIANGLES );
        for( unsigned int i = 0; i < vertices->size() / 4; ++i )
        {
            indices->push_back( i * 4 + 0 );
            indices->push_back( i * 4 + 1 );
            indices->push_back( i * 4 + 3 );
            indices->push_back( i * 4 + 1 );
            indices->push_back( i * 4 + 2 );
            indices->push_back( i * 4 + 3 );
        }
        geometry->addPrimitiveSet( indices );

        return geode;
    }

    class SwitchHandler : public osgGA::GUIEventHandler
    {
        public:

            SwitchHandler() :
                _childNum( 0 )
            {
            }

            virtual bool
            handle( const osgGA::GUIEventAdapter& ea,
                    osgGA::GUIActionAdapter& /*aa*/,
                    osg::Object* object,
                    osg::NodeVisitor* /*nv*/ )
            {
                osg::Switch* sw = dynamic_cast<osg::Switch*>( object );
                if( !sw )
                {
                    return false;
                }

                if( ea.getHandled() )
                {
                    return false;
                }

                switch( ea.getEventType() )
                {
                    case( osgGA::GUIEventAdapter::KEYDOWN ) :
                        {
                            if( ea.getKey() == 'n' )
                            {
                                ++_childNum;
                                if( _childNum >= sw->getNumChildren() )
                                {
                                    _childNum = 0;
                                }

                                sw->setSingleChildOn( _childNum );
                                return true;
                            }
                            break;
                        }
                    default :
                        break;
                }
                return false;
            }

        protected:

            virtual ~SwitchHandler()
            {
            }

            unsigned int _childNum;
    };

    osg::Node*
    createModel( osg::ArgumentParser& /*arguments*/ )
    {
        osg::Switch* sw = new osg::Switch;
        sw->setEventCallback( new ModelOne::SwitchHandler );

        sw->addChild( ModelOne::createCube( ModelOne::FRONT_FACE ), true );
        sw->addChild( ModelOne::createCube( ModelOne::FRONT_FACE | ModelOne::BACK_FACE ),
                      false );
        sw->addChild( ModelOne::createCube( ModelOne::FRONT_FACE |
                                            ModelOne::BACK_FACE |
                                            ModelOne::LEFT_FACE ),
                      false );
        sw->addChild( ModelOne::createCube( ModelOne::FRONT_FACE |
                                            ModelOne::BACK_FACE |
                                            ModelOne::LEFT_FACE |
                                            ModelOne::RIGHT_FACE ),
                      false );
        sw->addChild( ModelOne::createCube( ModelOne::FRONT_FACE |
                                            ModelOne::BACK_FACE |
                                            ModelOne::LEFT_FACE |
                                            ModelOne::RIGHT_FACE |
                                            ModelOne::TOP_FACE ),
                      false );
        sw->addChild( ModelOne::createCube( ModelOne::FRONT_FACE |
                                            ModelOne::BACK_FACE |
                                            ModelOne::LEFT_FACE |
                                            ModelOne::RIGHT_FACE |
                                            ModelOne::TOP_FACE |
                                            ModelOne::BOTTOM_FACE ),
                      false );

        return sw;
    }

}

namespace ModelTwo
{

    osg::AnimationPath*
    createAnimationPath( const osg::vec3& center,
                         float            radius,
                         double           looptime )
    {
        // set up the animation path
        osg::AnimationPath* animationPath = new osg::AnimationPath;
        animationPath->setLoopMode( osg::AnimationPath::LOOP );

        int    numSamples = 40;
        float  yaw        = 0.0F;
        float  yaw_delta  = 2.0F * osg::PI / ( ( float )numSamples - 1.0F );
        float  roll       = osg::radians( 30.0F );

        double time       = 0.0F;
        double time_delta = looptime / ( double )numSamples;
        for( int i = 0; i < numSamples; ++i )
        {
            osg::vec3 position(
                center + osg::vec3( sinf( yaw ) * radius, cosf( yaw ) * radius, 0.0F )
            );
            osg::quat rotation( osg::quat( roll, osg::vec3( 0.0, 1.0, 0.0 ) ) *
                                osg::quat( -( yaw + osg::radians( 90.0F ) ),
                                           osg::vec3( 0.0, 0.0, 1.0 ) ) );

            animationPath->insert(
                time,
                osg::AnimationPath::ControlPoint( osg::dvec3( position ), rotation )
            );

            yaw  += yaw_delta;
            time += time_delta;
        }
        return animationPath;
    }

    osg::Node*
    createBase( const osg::vec3& center,
                float            radius )
    {

        osg::Geode*              geode = new osg::Geode;

        // set up the texture of the base.
        osg::StateSet*           stateset = new osg::StateSet();
        osg::ref_ptr<osg::Image> image    = osgDB::readRefImageFile( "Images/lz.rgb" );
        if( image )
        {
            osg::Texture2D* texture = new osg::Texture2D;
            texture->setImage( image );
            stateset->setTextureAttributeAndModes( 0, texture, osg::StateAttribute::ON );
        }

        geode->setStateSet( stateset );

        osg::HeightField* grid = new osg::HeightField;
        grid->allocate( 38, 39 );
        grid->setOrigin( center + osg::vec3( -radius, -radius, 0.0F ) );
        grid->setXInterval( radius * 2.0F / ( float )( 38 - 1 ) );
        grid->setYInterval( radius * 2.0F / ( float )( 39 - 1 ) );

        float        minHeight = FLT_MAX;
        float        maxHeight = -FLT_MAX;

        unsigned int r;
        for( r = 0; r < 39; ++r )
        {
            for( unsigned int c = 0; c < 38; ++c )
            {
                float h = vertex[r + c * 39][2];
                if( h > maxHeight )
                {
                    maxHeight = h;
                }
                if( h < minHeight )
                {
                    minHeight = h;
                }
            }
        }

        float hieghtScale  = radius * 0.5F / ( maxHeight - minHeight );
        float hieghtOffset = -( minHeight + maxHeight ) * 0.5F;

        for( r = 0; r < 39; ++r )
        {
            for( unsigned int c = 0; c < 38; ++c )
            {
                float h = vertex[r + c * 39][2];
                grid->setHeight( c, r, ( h + hieghtOffset ) * hieghtScale );
            }
        }

        geode->addDrawable( new osg::ShapeDrawable( grid ) );

        osg::Group* group = new osg::Group;
        group->addChild( geode );

        return group;
    }

    osg::Node*
    createMovingModel( const osg::vec3& center,
                       float            radius )
    {
        float               animationLength = 10.0F;

        osg::AnimationPath* animationPath =
            createAnimationPath( center, radius, animationLength );

        osg::Group*             model  = new osg::Group;

        osg::ref_ptr<osg::Node> cessna = osgDB::readRefNodeFile( "damaged_helmet.glb" );
        if( cessna )
        {
            const osg::sphere&    bs         = cessna->getBound();

            float                 size       = radius / bs.radius * 0.3F;
            osg::MatrixTransform* positioned = new osg::MatrixTransform;
            positioned->setDataVariance( osg::Object::DataVariance::STATIC );
            positioned->setMatrix(
                osg::dmat4( osg::rotate( osg::radians( 180.0F ), 0.0F, 0.0F, 2.0F ) *
                            osg::scale( size, size, size ) *
                            osg::translate( -bs.center ) )
            );

            positioned->addChild( cessna );

            osg::MatrixTransform* xform = new osg::MatrixTransform;
            xform->setUpdateCallback(
                new osg::AnimationPathCallback( animationPath, 0.0F, 2.0 )
            );
            xform->addChild( positioned );

            model->addChild( xform );
        }

        return model;
    }

    osg::Node*
    createModel( osg::ArgumentParser& /*arguments*/ )
    {
        osg::vec3  center( 0.0F, 0.0F, 0.0F );
        float      radius = 100.0F;
        // the shadower model
        osg::Node* shadower = createMovingModel( center, radius * 0.5F );
        shadower->setNodeMask( CastsShadowTraversalMask );

        // the shadowed model
        osg::Node* shadowed =
            createBase( center - osg::vec3( 0.0F, 0.0F, radius * 0.25 ), radius );
        shadowed->setNodeMask( ReceivesShadowTraversalMask );

        osg::Group* group = new osg::Group;

        group->addChild( shadowed );
        group->addChild( shadower );

        return group;
    }

}

namespace ModelThree
{

    osg::Group*
    createModel( osg::ArgumentParser& arguments )
    {
        osg::Group*              scene   = new osg::Group;

        osg::ref_ptr<osg::Geode> geode_1 = new osg::Geode;
        scene->addChild( geode_1.get() );

        osg::ref_ptr<osg::Geode>           geode_2     = new osg::Geode;
        osg::ref_ptr<osg::MatrixTransform> transform_2 = new osg::MatrixTransform;
        transform_2->addChild( geode_2.get() );
        // transform_2->setUpdateCallback(new osg::AnimationPathCallback(osg::vec3(0, 0,
        // 0), osg::dvec3(0.0,0.0,1.0), osg::radians(45.0f)));
        scene->addChild( transform_2.get() );

        osg::ref_ptr<osg::Geode>           geode_3     = new osg::Geode;
        osg::ref_ptr<osg::MatrixTransform> transform_3 = new osg::MatrixTransform;
        transform_3->addChild( geode_3.get() );
        // transform_3->setUpdateCallback(new osg::AnimationPathCallback(osg::vec3(0, 0,
        // 0), osg::dvec3(0.0,0.0,1.0), osg::radians(-22.5f)));
        scene->addChild( transform_3.get() );

        const float                          radius = 0.8F;
        const float                          height = 1.0F;
        osg::ref_ptr<osg::TessellationHints> hints  = new osg::TessellationHints;
        hints->setDetailRatio( 2.0F );
        osg::ref_ptr<osg::ShapeDrawable> shape;
        shape = new osg::ShapeDrawable(
            new osg::Box( osg::vec3( 0.0F, 0.0F, -2.0F ), 10, 10.0F, 0.1F ),
            hints.get()
        );
        shape->setColor( osg::vec4( 0.5F, 0.5F, 0.7F, 1.0F ) );
        shape->setName( "base box" );
        geode_1->addDrawable( shape.get() );

        shape = new osg::ShapeDrawable( new osg::Sphere( osg::vec3( 0.0F, 0.0F, 0.0F ),
                                                         radius * 2 ),
                                        hints.get() );
        shape->setColor( osg::vec4( 0.8F, 0.8F, 0.8F, 1.0F ) );
        shape->setName( "center sphere" );
        geode_1->addDrawable( shape.get() );

        shape = new osg::ShapeDrawable( new osg::Sphere( osg::vec3( -3.0F, 0.0F, 0.0F ),
                                                         radius ),
                                        hints.get() );
        shape->setColor( osg::vec4( 0.6F, 0.8F, 0.8F, 1.0F ) );
        shape->setName( "cyan sphere" );
        geode_2->addDrawable( shape.get() );

        shape = new osg::ShapeDrawable( new osg::Box( osg::vec3( 3.0F, 0.0F, 0.0F ),
                                                      2 * radius ),
                                        hints.get() );
        shape->setColor( osg::vec4( 0.4F, 0.9F, 0.3F, 1.0F ) );
        shape->setName( "green box" );
        geode_2->addDrawable( shape.get() );

        shape = new osg::ShapeDrawable(
            new osg::Cone( osg::vec3( 0.0F, -3.0F, 0.0F ), radius, height ),
            hints.get()
        );
        shape->setColor( osg::vec4( 0.2F, 0.5F, 0.7F, 1.0F ) );
        shape->setName( "blue cone" );
        geode_2->addDrawable( shape.get() );

        shape = new osg::ShapeDrawable(
            new osg::Cylinder( osg::vec3( 0.0F, 3.0F, 0.0F ), radius, height ),
            hints.get()
        );
        shape->setColor( osg::vec4( 1.0F, 0.3F, 0.3F, 1.0F ) );
        shape->setName( "red cylinder" );
        geode_2->addDrawable( shape.get() );

        shape = new osg::ShapeDrawable(
            new osg::Box( osg::vec3( 0.0F, 0.0F, 3.0F ), 2.0F, 2.0F, 0.1F ),
            hints.get()
        );
        shape->setColor( osg::vec4( 0.8F, 0.8F, 0.4F, 1.0F ) );
        shape->setName( "rotating box" );
        geode_3->addDrawable( shape.get() );

        // material
        osg::ref_ptr<osg::Material> matirial = new osg::Material;
        matirial->setColorMode( osg::Material::DIFFUSE );
        matirial->setAmbient( osg::Material::FRONT_AND_BACK, osg::vec4( 0, 0, 0, 1 ) );
        matirial->setSpecular( osg::Material::FRONT_AND_BACK, osg::vec4( 1, 1, 1, 1 ) );
        matirial->setShininess( osg::Material::FRONT_AND_BACK, 64.0F );
        scene->getOrCreateStateSet()->setAttributeAndModes( matirial.get(),
                                                            osg::StateAttribute::ON );

        bool withBaseTexture = true;
        while( arguments.read( "--with-base-texture" ) )
        {
            withBaseTexture = true;
        }
        while( arguments.read( "--no-base-texture" ) )
        {
            withBaseTexture = false;
        }

        if( withBaseTexture )
        {
            scene->getOrCreateStateSet()->setTextureAttributeAndModes(
                0,
                new osg::Texture2D( osgDB::readRefImageFile( "Images/lz.rgb" ) ),
                osg::StateAttribute::ON
            );
        }

        return scene;
    }

}

namespace ModelFive
{

    struct UseVBOVisitor : public osg::DualModeVisitor
    {
            UseVBOVisitor() :
                osg::DualModeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
            {
            }

            virtual void
            apply( osg::Geode& geode )
            {
                for( unsigned int i = 0; i < geode.getNumDrawables(); ++i )
                {
                    osg::Geometry* geometry =
                        dynamic_cast<osg::Geometry*>( geode.getDrawable( i ) );
                    if( geometry )
                    {
                        geometry->setUseVertexBufferObjects( true );
                    }
                }
            }
    };

    osg::AnimationPathCallback*
    createAnimationPathCallback( float radius,
                                 float time )
    {
        osg::ref_ptr<osg::AnimationPath> path = new osg::AnimationPath;
        path->setLoopMode( osg::AnimationPath::LOOP );

        unsigned int numSamples = 32;
        float        delta_yaw  = 2.0F * osg::PI / ( ( float )numSamples - 1.0F );
        float        delta_time = time / ( float )numSamples;
        for( unsigned int i = 0; i < numSamples; ++i )
        {
            float     yaw = delta_yaw * ( float )i;
            osg::vec3 pos( sinf( yaw ) * radius, cosf( yaw ) * radius, 0.0F );
            osg::quat rot( -yaw, osg::vec3( 0.0F, 0.0F, 1.0F ) );
            path->insert( delta_time * ( float )i,
                          osg::AnimationPath::ControlPoint( osg::dvec3( pos ), rot ) );
        }

        osg::ref_ptr<osg::AnimationPathCallback> apcb = new osg::AnimationPathCallback;
        apcb->setAnimationPath( path.get() );
        return apcb.release();
    }

    osg::Group*
    createModel( osg::ArgumentParser& /*arguments*/ )
    {
        // Set the ground (only receives shadow)
        osg::ref_ptr<osg::MatrixTransform> groundNode = new osg::MatrixTransform;
        groundNode->addChild( osgDB::readRefNodeFile( "duck.glb" ) );
        groundNode->setMatrix( osg::dmat4( osg::translate( 200.0F, 200.0F, -200.0F ) ) );
        groundNode->setNodeMask( ReceivesShadowTraversalMask );

        // Set the cessna (only casts shadow)
        osg::ref_ptr<osg::MatrixTransform> cessnaNode = new osg::MatrixTransform;
        cessnaNode->addChild( osgDB::readRefNodeFile( "damaged_helmet.glb" ) );
        cessnaNode->addUpdateCallback( createAnimationPathCallback( 50.0F, 6.0F ) );
        cessnaNode->setNodeMask( CastsShadowTraversalMask );

        // cessna is really poorly optimized so fix this by optimizing the mesh and use
        // VBO's.
        osgUtil::Optimizer optimizer;
        optimizer.optimize( cessnaNode.get(),
                            osgUtil::Optimizer::INDEX_MESH |
                                osgUtil::Optimizer::VERTEX_POSTTRANSFORM |
                                osgUtil::Optimizer::VERTEX_PRETRANSFORM );

        UseVBOVisitor useVBOVisitor;
        cessnaNode->accept( useVBOVisitor );

        osg::ref_ptr<osg::Group> shadowRoot = new osg::Group;
        shadowRoot->addChild( groundNode.get() );
        for( unsigned int i = 0; i < 10; ++i )
        {
            for( unsigned int j = 0; j < 10; ++j )
            {
                osg::ref_ptr<osg::MatrixTransform> cessnaInstance =
                    new osg::MatrixTransform;
                cessnaInstance->setMatrix(
                    osg::dmat4( osg::translate( ( float )i * 50.0F - 25.0F,
                                                ( float )j * 50.0F - 25.0F,
                                                0.0F ) )
                );
                cessnaInstance->addChild( cessnaNode.get() );
                shadowRoot->addChild( cessnaInstance.get() );
            }
        }

        return shadowRoot.release();
    }

}

osg::Node*
createTestModel( osg::ArgumentParser& arguments )
{
    if( arguments.read( "-1" ) )
    {
        return ModelOne::createModel( arguments );
    }
    else if( arguments.read( "-2" ) )
    {
        return ModelTwo::createModel( arguments );
    }
    else if( arguments.read( "-4" ) )
    {
        return ModelFour::createModel( arguments );
    }
    else if( arguments.read( "-5" ) )
    {
        return ModelFive::createModel( arguments );
    }
    else /*if (arguments.read("-3"))*/
    {
        return ModelThree::createModel( arguments );
    }
}

int
main( int    argc,
      char** argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc, argv );

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(
        arguments.getApplicationName() +
        " is the example which demonstrates using of GL_ARB_shadow extension "
        "implemented in osg::Texture class"
    );
    arguments.getApplicationUsage()->setCommandLineUsage(
        arguments.getApplicationName()
    );
    arguments.getApplicationUsage()->addCommandLineOption( "-h or --help",
                                                           "Display this information" );
    arguments.getApplicationUsage()->addCommandLineOption( "--positionalLight",
                                                           "Use a positional light." );
    arguments.getApplicationUsage()->addCommandLineOption( "--directionalLight",
                                                           "Use a direction light." );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--noUpdate",
        "Disable the updating the of light source."
    );

    arguments.getApplicationUsage()->addCommandLineOption(
        "--castsShadowMask",
        "Override default castsShadowMask (default - 0x2)"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--receivesShadowMask",
        "Override default receivesShadowMask (default - 0x1)"
    );

    arguments.getApplicationUsage()->addCommandLineOption(
        "--base",
        "Add a base geometry to test shadows."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--ssm",
        "Select SoftShadowMap implementation."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--sm",
        "Select ShadowMap implementation."
    );

    arguments.getApplicationUsage()->addCommandLineOption(
        "--pssm",
        "Select ParallelSplitShadowMap implementation."
    );    // ADEGLI
    arguments.getApplicationUsage()->addCommandLineOption(
        "--mapcount",
        "ParallelSplitShadowMap texture count."
    );    // ADEGLI
    arguments.getApplicationUsage()->addCommandLineOption(
        "--mapres",
        "ParallelSplitShadowMap texture resolution."
    );    // ADEGLI
    arguments.getApplicationUsage()->addCommandLineOption(
        "--debug-color",
        "ParallelSplitShadowMap display debugging color (only the first 3 maps are "
        "color r=0,g=1,b=2."
    );    // ADEGLI
    arguments.getApplicationUsage()->addCommandLineOption(
        "--minNearSplit",
        "ParallelSplitShadowMap shadow map near offset."
    );    // ADEGLI
    arguments.getApplicationUsage()->addCommandLineOption(
        "--maxFarDist",
        "ParallelSplitShadowMap max far distance to shadow."
    );    // ADEGLI
    arguments.getApplicationUsage()->addCommandLineOption(
        "--moveVCamFactor",
        "ParallelSplitShadowMap move the virtual frustum behind the real camera, (also "
        "back ground object can cast shadow)."
    );    // ADEGLI
    arguments.getApplicationUsage()->addCommandLineOption(
        "--PolyOffset-Factor",
        "ParallelSplitShadowMap set PolygonOffset factor."
    );    // ADEGLI
    arguments.getApplicationUsage()->addCommandLineOption(
        "--PolyOffset-Unit",
        "ParallelSplitShadowMap set PolygonOffset unit."
    );    // ADEGLI

    arguments.getApplicationUsage()->addCommandLineOption(
        "--lispsm",
        "Select LightSpacePerspectiveShadowMap implementation."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--msm",
        "Select MinimalShadowMap implementation."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--ViewBounds",
        "MSM, LiSPSM optimize shadow for view frustum (weakest option)"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--CullBounds",
        "MSM, LiSPSM optimize shadow for bounds of culled objects in view frustum "
        "(better option)."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--DrawBounds",
        "MSM, LiSPSM optimize shadow for bounds of predrawn pixels in view frustum "
        "(best & default)."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--mapres",
        "MSM, LiSPSM & texture resolution."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--maxFarDist",
        "MSM, LiSPSM max far distance to shadow."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--moveVCamFactor",
        "MSM, LiSPSM move the virtual frustum behind the real camera, (also back ground "
        "object can cast shadow)."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--minLightMargin",
        "MSM, LiSPSM the same as --moveVCamFactor."
    );

    arguments.getApplicationUsage()->addCommandLineOption( "-1", "Use test model one." );
    arguments.getApplicationUsage()->addCommandLineOption( "-2", "Use test model two." );
    arguments.getApplicationUsage()->addCommandLineOption(
        "-3",
        "Use test model three (default)."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "-4",
        "Use test model four - island scene."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--two-sided",
        "Use two-sided stencil extension for shadow volumes."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--two-pass",
        "Use two-pass stencil for shadow volumes."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--near-far-mode",
        "COMPUTE_NEAR_USING_PRIMITIVES, COMPUTE_NEAR_FAR_USING_PRIMITIVES, "
        "COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES, DO_NOT_COMPUTE_NEAR_FAR"
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--max-shadow-distance",
        "<float> Maximum distance that the shadow map should extend from the eye point."
    );

    // construct the viewer.
    // Headless capture: early check before viewer construction
    {
        std::string headlessOutput;
        if( arguments.read( "--headless", headlessOutput ) )
        {
            auto node = osgDB::readRefNodeFiles( arguments );
            if( !node )
            {
                node = osgDB::readRefNodeFile( "damaged_helmet.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    osgViewer::Viewer viewer( arguments );

    // if user request help write it out to cout.
    if( arguments.read( "-h" ) || arguments.read( "--help" ) )
    {
        arguments.getApplicationUsage()->write( std::cout );
        return 1;
    }

    double zNear = 1.0, zMid = 10.0, zFar = 1000.0;
    if( arguments.read( "--depth-partition", zNear, zMid, zFar ) )
    {
        // set up depth partitioning
        osg::ref_ptr<osgViewer::DepthPartitionSettings> dps =
            new osgViewer::DepthPartitionSettings;
        dps->_mode  = osgViewer::DepthPartitionSettings::FIXED_RANGE;
        dps->_zNear = zNear;
        dps->_zMid  = zMid;
        dps->_zFar  = zFar;
        viewer.setUpDepthPartition( dps.get() );
    }

    if( arguments.read( "--dp" ) )
    {
        // set up depth partitioning
        viewer.setUpDepthPartition();
    }

    float fov = 0.0;
    while( arguments.read( "--fov", fov ) )
    {
    }

    osg::vec4 lightpos( 0.0, 0.0, 1, 0.0 );
    bool      spotlight = false;
    while( arguments.read( "--positionalLight" ) )
    {
        lightpos.set( 0.5, 0.5, 1.5, 1.0 );
    }
    while( arguments.read( "--directionalLight" ) )
    {
        lightpos.set( 0.0, 0.0, 1, 0.0 );
    }
    while( arguments.read( "--spotLight" ) )
    {
        lightpos.set( 0.5, 0.5, 1.5, 1.0 );
        spotlight = true;
    }

    bool      keepLightPos = false;
    osg::vec3 spotLookat( 0.0, 0.0, 0.0 );
    while(
        arguments.read( "--light-pos", lightpos.x, lightpos.y, lightpos.z, lightpos.w )
    )
    {
        keepLightPos = true;
    }
    while( arguments.read( "--light-pos", lightpos.x, lightpos.y, lightpos.z ) )
    {
        lightpos.w   = 1.0;
        keepLightPos = true;
    }
    while( arguments.read( "--light-dir", lightpos.x, lightpos.y, lightpos.z ) )
    {
        lightpos.w   = 0.0;
        keepLightPos = true;
    }
    while( arguments.read( "--spot-lookat", spotLookat.x, spotLookat.y, spotLookat.z ) )
    {
    }

    while( arguments.read( "--castsShadowMask", CastsShadowTraversalMask ) )
        ;
    while( arguments.read( "--receivesShadowMask", ReceivesShadowTraversalMask ) )
        ;

    bool updateLightPosition = true;
    while( arguments.read( "--noUpdate" ) )
    {
        updateLightPosition = false;
    }

    // set up the camera manipulators.
    {
        osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator =
            new osgGA::KeySwitchMatrixManipulator;

        keyswitchManipulator->addMatrixManipulator( '1',
                                                    "Trackball",
                                                    new osgGA::TrackballManipulator() );
        keyswitchManipulator->addMatrixManipulator( '2',
                                                    "Flight",
                                                    new osgGA::FlightManipulator() );
        keyswitchManipulator->addMatrixManipulator( '3',
                                                    "Drive",
                                                    new osgGA::DriveManipulator() );
        keyswitchManipulator->addMatrixManipulator( '4',
                                                    "Terrain",
                                                    new osgGA::TerrainManipulator() );

        std::string pathfile;
        char        keyForAnimationPath = '5';
        while( arguments.read( "-p", pathfile ) )
        {
            osgGA::AnimationPathManipulator* apm =
                new osgGA::AnimationPathManipulator( pathfile );
            if( apm || !apm->valid() )
            {
                unsigned int num = keyswitchManipulator->getNumMatrixManipulators();
                keyswitchManipulator->addMatrixManipulator( keyForAnimationPath,
                                                            "Path",
                                                            apm );
                keyswitchManipulator->selectMatrixManipulator( num );
                ++keyForAnimationPath;
            }
        }

        viewer.setCameraManipulator( keyswitchManipulator.get() );
    }

    // add the state manipulator
    viewer.addEventHandler(
        new osgGA::StateSetManipulator( viewer.getCamera()->getOrCreateStateSet() )
    );

    // add stats
    viewer.addEventHandler( new osgViewer::StatsHandler() );

    // add the record camera path handler
    viewer.addEventHandler( new osgViewer::RecordCameraPathHandler );

    // add the window size toggle handler
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );

    // add the threading handler
    viewer.addEventHandler( new osgViewer::ThreadingHandler() );

    osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;

    osgShadow::ShadowSettings*             settings = shadowedScene->getShadowSettings();
    settings->setReceivesShadowTraversalMask( ReceivesShadowTraversalMask );
    settings->setCastsShadowTraversalMask( CastsShadowTraversalMask );

    std::string nearFarMode( "" );
    if( arguments.read( "--near-far-mode", nearFarMode ) )
    {
        if( nearFarMode == "COMPUTE_NEAR_USING_PRIMITIVES" )
        {
            settings->setComputeNearFarModeOverride(
                osg::CullSettings::COMPUTE_NEAR_USING_PRIMITIVES
            );
        }
        else if( nearFarMode == "COMPUTE_NEAR_FAR_USING_PRIMITIVES" )
        {
            settings->setComputeNearFarModeOverride(
                osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES
            );
        }
        else if( nearFarMode == "DO_NOT_COMPUTE_NEAR_FAR" )
        {
            settings->setComputeNearFarModeOverride(
                osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR
            );
        }
        else if( nearFarMode == "COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES" )
        {
            settings->setComputeNearFarModeOverride(
                osg::CullSettings::COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES
            );
        }

        OSG_NOTICE << "ComputeNearFarModeOverride set to ";
        switch( settings->getComputeNearFarModeOverride() )
        {
            case( osg::CullSettings::COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES ) :
                OSG_NOTICE << "COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES";
                break;
            case( osg::CullSettings::COMPUTE_NEAR_USING_PRIMITIVES ) :
                OSG_NOTICE << "COMPUTE_NEAR_USING_PRIMITIVES";
                break;
            case( osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES ) :
                OSG_NOTICE << "COMPUTE_NEAR_FAR_USING_PRIMITIVES";
                break;
            case( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR ) :
                OSG_NOTICE << "DO_NOT_COMPUTE_NEAR_FAR";
                break;
        }
        OSG_NOTICE << std::endl;
    }

    double distance;
    if( arguments.read( "--max-shadow-distance", distance ) )
    {
        settings->setMaximumShadowMapDistance( distance );
        OSG_NOTICE << "MaximumShadowMapDistance set to "
                   << settings->getMaximumShadowMapDistance() << std::endl;
    }

    osg::ref_ptr<osgShadow::MinimalShadowMap> msm = NULL;
    if( arguments.read( "--no-shadows" ) )
    {
        OSG_NOTICE << "Not using a ShadowTechnique" << std::endl;
        shadowedScene->setShadowTechnique( 0 );
    }
    else if( arguments.read( "--sv" ) )
    {
        OSG_NOTICE << "Warning: ShadowVolume no longer supported." << std::endl;
        return 1;
    }
    else if( arguments.read( "--st" ) )
    {
        osg::ref_ptr<osgShadow::ShadowTexture> st = new osgShadow::ShadowTexture;
        shadowedScene->setShadowTechnique( st.get() );
    }
    else if( arguments.read( "--stsm" ) )
    {
        osg::ref_ptr<osgShadow::StandardShadowMap> st = new osgShadow::StandardShadowMap;
        shadowedScene->setShadowTechnique( st.get() );
    }
    else if( arguments.read( "--pssm" ) )
    {
        int mapcount = 3;
        while( arguments.read( "--mapcount", mapcount ) )
            ;
        osg::ref_ptr<osgShadow::ParallelSplitShadowMap> pssm =
            new osgShadow::ParallelSplitShadowMap( NULL, mapcount );

        int mapres = 1'024;
        while( arguments.read( "--mapres", mapres ) )
        {
            pssm->setTextureResolution( mapres );
        }

        while( arguments.read( "--debug-color" ) )
        {
            pssm->setDebugColorOn();
        }

        int minNearSplit = 0;
        while( arguments.read( "--minNearSplit", minNearSplit ) )
        {
            if( minNearSplit > 0 )
            {
                pssm->setMinNearDistanceForSplits( minNearSplit );
                std::cout << "ParallelSplitShadowMap : setMinNearDistanceForSplits("
                          << minNearSplit << ")" << std::endl;
            }
        }

        int maxfardist = 0;
        while( arguments.read( "--maxFarDist", maxfardist ) )
        {
            if( maxfardist > 0 )
            {
                pssm->setMaxFarDistance( maxfardist );
                std::cout << "ParallelSplitShadowMap : setMaxFarDistance(" << maxfardist
                          << ")" << std::endl;
            }
        }

        int moveVCamFactor = 0;
        while( arguments.read( "--moveVCamFactor", moveVCamFactor ) )
        {
            if( maxfardist > 0 )
            {
                pssm->setMoveVCamBehindRCamFactor( moveVCamFactor );
                std::cout << "ParallelSplitShadowMap : setMoveVCamBehindRCamFactor("
                          << moveVCamFactor << ")" << std::endl;
            }
        }

        double polyoffsetfactor = pssm->getPolygonOffset().x;
        double polyoffsetunit   = pssm->getPolygonOffset().y;
        while( arguments.read( "--PolyOffset-Factor", polyoffsetfactor ) )
            ;
        while( arguments.read( "--PolyOffset-Unit", polyoffsetunit ) )
            ;
        pssm->setPolygonOffset( osg::vec2( polyoffsetfactor, polyoffsetunit ) );

        shadowedScene->setShadowTechnique( pssm.get() );
    }
    else if( arguments.read( "--ssm" ) )
    {
        osg::ref_ptr<osgShadow::SoftShadowMap> sm = new osgShadow::SoftShadowMap;
        shadowedScene->setShadowTechnique( sm.get() );
    }
    else if( arguments.read( "--vdsm" ) )
    {
        while( arguments.read( "--debugHUD" ) )
        {
            settings->setDebugDraw( true );
        }
        if( arguments.read( "--persp" ) )
        {
            settings->setShadowMapProjectionHint(
                osgShadow::ShadowSettings::PERSPECTIVE_SHADOW_MAP
            );
        }
        if( arguments.read( "--ortho" ) )
        {
            settings->setShadowMapProjectionHint(
                osgShadow::ShadowSettings::ORTHOGRAPHIC_SHADOW_MAP
            );
        }

        unsigned int unit = 1;
        if( arguments.read( "--unit", unit ) )
        {
            settings->setBaseShadowTextureUnit( unit );
        }

        double n = 0.0;
        if( arguments.read( "-n", n ) )
        {
            settings->setMinimumShadowMapNearFarRatio( n );
        }

        unsigned int numShadowMaps;
        if( arguments.read( "--num-sm", numShadowMaps ) )
        {
            settings->setNumShadowMapsPerLight( numShadowMaps );
        }

        if( arguments.read( "--parallel-split" ) || arguments.read( "--ps" ) )
        {
            settings->setMultipleShadowMapHint(
                osgShadow::ShadowSettings::PARALLEL_SPLIT
            );
        }
        if( arguments.read( "--cascaded" ) )
        {
            settings->setMultipleShadowMapHint( osgShadow::ShadowSettings::CASCADED );
        }

        int mapres = 1'024;
        while( arguments.read( "--mapres", mapres ) )
        {
            settings->setTextureSize( osg::svec2( mapres, mapres ) );
        }

        osg::ref_ptr<osgShadow::ViewDependentShadowMap> vdsm =
            new osgShadow::ViewDependentShadowMap;
        shadowedScene->setShadowTechnique( vdsm.get() );
    }
    else if( arguments.read( "--lispsm" ) )
    {
        if( arguments.read( "--ViewBounds" ) )
        {
            msm = new osgShadow::LightSpacePerspectiveShadowMapVB;
        }
        else if( arguments.read( "--CullBounds" ) )
        {
            msm = new osgShadow::LightSpacePerspectiveShadowMapCB;
        }
        else    // if( arguments.read( "--DrawBounds" ) ) // default
        {
            msm = new osgShadow::LightSpacePerspectiveShadowMapDB;
        }
    }
    else if( arguments.read( "--msm" ) )
    {
        if( arguments.read( "--ViewBounds" ) )
        {
            msm = new osgShadow::MinimalShadowMap;
        }
        else if( arguments.read( "--CullBounds" ) )
        {
            msm = new osgShadow::MinimalCullBoundsShadowMap;
        }
        else    // if( arguments.read( "--DrawBounds" ) ) // default
        {
            msm = new osgShadow::MinimalDrawBoundsShadowMap;
        }
    }
    else /* if (arguments.read("--sm")) */
    {
        osg::ref_ptr<osgShadow::ShadowMap> sm = new osgShadow::ShadowMap;
        shadowedScene->setShadowTechnique( sm.get() );

        int mapres = 1'024;
        while( arguments.read( "--mapres", mapres ) )
        {
            sm->setTextureSize( osg::svec2( mapres, mapres ) );
        }
    }

    if( msm )    // Set common MSM & LISPSM arguments
    {
        shadowedScene->setShadowTechnique( msm.get() );
        while( arguments.read( "--debugHUD" ) )
        {
            msm->setDebugDraw( true );
        }

        float        minLightMargin = 10.F;
        float        maxFarPlane    = 0;
        unsigned int texSize        = 1'024;
        unsigned int baseTexUnit    = 0;
        unsigned int shadowTexUnit  = 1;

        while( arguments.read( "--moveVCamFactor", minLightMargin ) )
            ;
        while( arguments.read( "--minLightMargin", minLightMargin ) )
            ;
        while( arguments.read( "--maxFarDist", maxFarPlane ) )
            ;
        while( arguments.read( "--mapres", texSize ) )
            ;
        while( arguments.read( "--baseTextureUnit", baseTexUnit ) )
            ;
        while( arguments.read( "--shadowTextureUnit", shadowTexUnit ) )
            ;

        msm->setMinLightMargin( minLightMargin );
        msm->setMaxFarPlane( maxFarPlane );
        msm->setTextureSize( osg::svec2( texSize, texSize ) );
        msm->setShadowTextureCoordIndex( shadowTexUnit );
        msm->setShadowTextureUnit( shadowTexUnit );
        msm->setBaseTextureCoordIndex( baseTexUnit );
        msm->setBaseTextureUnit( baseTexUnit );
    }

    OSG_INFO << "shadowedScene->getShadowTechnique()="
             << shadowedScene->getShadowTechnique() << std::endl;

    osg::ref_ptr<osg::Node> model = osgDB::readRefNodeFiles( arguments );
    if( model.valid() )
    {
        model->setNodeMask( CastsShadowTraversalMask | ReceivesShadowTraversalMask );
    }
    else
    {
        model = createTestModel( arguments );
    }

    // get the bounds of the model.
    osg::ComputeBoundsVisitor cbbv;
    model->accept( cbbv );
    osg::box bb = cbbv.getBoundingBox();

    if( lightpos.w == 1.0 && !keepLightPos )
    {
        lightpos.x = bb.xMin() + ( bb.xMax() - bb.xMin() ) * lightpos.x;
        lightpos.y = bb.yMin() + ( bb.yMax() - bb.yMin() ) * lightpos.y;
        lightpos.z = bb.zMin() + ( bb.zMax() - bb.zMin() ) * lightpos.z;
    }

    if( arguments.read( "--base" ) )
    {

        osg::Geode* geode = new osg::Geode;

        osg::vec3   widthVec( bb.radius(), 0.0F, 0.0F );
        osg::vec3   depthVec( 0.0F, bb.radius(), 0.0F );
        osg::vec3   centerBase( ( bb.xMin() + bb.xMax() ) * 0.5F,
                                ( bb.yMin() + bb.yMax() ) * 0.5F,
                                bb.zMin() - bb.radius() * 0.1F );

        geode->addDrawable( osg::createTexturedQuadGeometry(
            centerBase - widthVec * 1.5F - depthVec * 1.5F,
            widthVec * 3.0F,
            depthVec * 3.0F
        ) );

        geode->setNodeMask( shadowedScene->getReceivesShadowTraversalMask() );

        geode->getOrCreateStateSet()->setTextureAttributeAndModes(
            0,
            new osg::Texture2D( osgDB::readRefImageFile( "Images/lz.rgb" ) )
        );

        shadowedScene->addChild( geode );
    }

    osg::ref_ptr<osg::LightSource> ls = new osg::LightSource;
    ls->getLight()->setPosition( lightpos );

    if( spotlight )
    {
        osg::vec3 center   = spotLookat;
        osg::vec3 lightdir = center - osg::vec3( lightpos.x, lightpos.y, lightpos.z );
        lightdir           = osg::normalize( lightdir );
        ls->getLight()->setDirection( lightdir );
        ls->getLight()->setSpotCutoff( 25.0F );

        // set the LightSource, only for checking, there is only 1 light in the scene
        osgShadow::ShadowMap* shadowMap =
            dynamic_cast<osgShadow::ShadowMap*>( shadowedScene->getShadowTechnique() );
        if( shadowMap )
        {
            shadowMap->setLight( ls.get() );
        }
    }

    if( arguments.read( "--coloured-light" ) )
    {
        ls->getLight()->setAmbient( osg::vec4( 1.0, 0.0, 0.0, 1.0 ) );
        ls->getLight()->setDiffuse( osg::vec4( 0.0, 1.0, 0.0, 1.0 ) );
    }
    else
    {
        ls->getLight()->setAmbient( osg::vec4( 0.2, 0.2, 0.2, 1.0 ) );
        ls->getLight()->setDiffuse( osg::vec4( 0.8, 0.8, 0.8, 1.0 ) );
    }

    shadowedScene->addChild( model.get() );
    shadowedScene->addChild( ls.get() );

    viewer.setSceneData( shadowedScene.get() );

    osg::ref_ptr<DumpShadowVolumesHandler> dumpShadowVolumes =
        new DumpShadowVolumesHandler;

    viewer.addEventHandler( new ChangeFOVHandler( viewer.getCamera() ) );
    viewer.addEventHandler( dumpShadowVolumes.get() );

    // create the windows and run the threads.
    viewer.realize();

    if( fov != 0.0 )
    {
        double fovy, aspectRatio;
        osg::getPerspective( viewer.getCamera()->getProjectionMatrix(),
                             fovy,
                             aspectRatio,
                             zNear,
                             zFar );

        std::cout << "Setting FOV to " << fov << std::endl;
        viewer.getCamera()->setProjectionMatrix(
            osg::perspective( static_cast<double>( fov ), aspectRatio, zNear, zFar )
        );
    }

    // it is done after viewer.realize() so that the windows are already initialized
    if( arguments.read( "--debugHUD" ) )
    {
        osgViewer::Viewer::Windows windows;
        viewer.getWindows( windows );

        if( windows.empty() )
        {
            return 1;
        }

        osgShadow::ShadowMap* sm =
            dynamic_cast<osgShadow::ShadowMap*>( shadowedScene->getShadowTechnique() );
        if( sm )
        {
            osg::ref_ptr<osg::Camera> hudCamera = sm->makeDebugHUD();

            // set up cameras to rendering on the first window available.
            hudCamera->setGraphicsContext( windows[0] );
            hudCamera->setViewport( 0,
                                    0,
                                    windows[0]->getTraits()->width,
                                    windows[0]->getTraits()->height );

            viewer.addSlave( hudCamera.get(), false );
        }
    }

    osg::ref_ptr<LightAnimationHandler> lightAnimationHandler =
        updateLightPosition ? new LightAnimationHandler : 0;
    if( lightAnimationHandler )
    {
        viewer.addEventHandler( lightAnimationHandler.get() );
    }

    // osgDB::writeNodeFile(*group,"test.osgt");

    while( !viewer.done() )
    {
        {
            if( msm )
            {

                // If scene decorated by CoordinateSystemNode try to find localToWorld
                // and set modellingSpaceToWorld matrix to optimize scene bounds
                // computation

                osg::NodePath np = viewer.getCoordinateSystemNodePath();
                if( !np.empty() )
                {
                    osg::CoordinateSystemNode* csn =
                        dynamic_cast<osg::CoordinateSystemNode*>( np.back() );

                    if( csn )
                    {
                        osg::dvec3 pos =
                            osg::getTrans( viewer.getCameraManipulator()->getMatrix() );

                        msm->setModellingSpaceToWorldTransform(
                            csn->computeLocalCoordinateFrame( pos )
                        );
                    }
                }
            }
        }

        if( lightAnimationHandler.valid() && lightAnimationHandler->getAnimating() )
        {
            float t = viewer.getFrameStamp()->getSimulationTime();

            if( lightpos.w == 1.0 )
            {
                lightpos.set( bb.center().x + sinf( t ) * bb.radius(),
                              bb.center().y + cosf( t ) * bb.radius(),
                              bb.zMax() + bb.radius() * 3.0F,
                              1.0F );
            }
            else
            {
                lightpos.set( sinf( t ), cosf( t ), 1.0F, 0.0F );
            }
            ls->getLight()->setPosition( lightpos );

            osg::vec3 lightDir( -lightpos.x, -lightpos.y, -lightpos.z );
            if( spotlight )
            {
                lightDir = osg::vec3( bb.center().x + sinf( t ) * bb.radius() / 2.0,
                                      bb.center().y + cosf( t ) * bb.radius() / 2.0,
                                      bb.center().z ) -
                           osg::vec3( lightpos.x, lightpos.y, lightpos.z );
            }
            lightDir = osg::normalize( lightDir );
            ls->getLight()->setDirection( lightDir );
        }

        if( dumpShadowVolumes->get() )
        {
            dumpShadowVolumes->set( false );

            static int dumpFileNo = 0;
            dumpFileNo++;
            char filename[256];
            std::sprintf( filename, "shadowDump%d.osgt", dumpFileNo );

            if( msm )
            {
                msm->setDebugDump( filename );
            }
        }

        viewer.frame();
    }

    return 0;
}
