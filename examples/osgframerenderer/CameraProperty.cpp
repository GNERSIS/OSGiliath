#include "CameraProperty.hpp"

#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/maths/transform.hpp>

using namespace gsc;

void
CameraProperty::setToModel( const osg::Node* node )
{
    osg::sphere bs   = node->getBound();

    double      dist = osg::DisplaySettings::instance()->getScreenDistance();

    OSG_NOTICE << "Node name " << node->getName() << std::endl;

#if 1
    if( node->getName().find( "Presentation" ) == std::string::npos )
    {
        double screenWidth    = osg::DisplaySettings::instance()->getScreenWidth();
        double screenHeight   = osg::DisplaySettings::instance()->getScreenHeight();
        double screenDistance = osg::DisplaySettings::instance()->getScreenDistance();

        double vfov           = atan2( screenHeight / 2.0, screenDistance ) * 2.0;
        double hfov           = atan2( screenWidth / 2.0, screenDistance ) * 2.0;
        double viewAngle      = vfov < hfov ? vfov : hfov;

        dist                  = bs.radius / sin( viewAngle * 0.5 );
    }
#endif

    _center         = bs.center;
    _eye            = _center - osg::dvec3( 0.0, dist, 0.0 );
    _up             = osg::dvec3( 0.0, 0.0, 1.0 );

    _rotationCenter = _center;
    _rotationAxis   = osg::dvec3( 0.0, 0.0, 1.0 );
    _rotationSpeed  = 0.0;
}

void
CameraProperty::update( osgViewer::View* view )
{
    osg::Camera*     camera = view->getCamera();
    osg::FrameStamp* fs     = view->getFrameStamp();

    osg::dmat4       matrix = osg::lookAt( _eye, _center, _up );

    if( _rotationSpeed != 0.0 )
    {
        osg::preMult( matrix,
                      osg::translate( -_rotationCenter ) *
                          osg::rotate( osg::radians( _rotationSpeed *
                                                     fs->getSimulationTime() ),
                                       _rotationAxis ) *
                          osg::translate( _rotationCenter ) );
    }

    camera->setViewMatrix( matrix );

    // set the fusion distance up so that the left and right eye images are co-incedent
    // on the image plane at the center of ration.
    view->setFusionDistance( osgUtil::SceneView::USE_FUSION_DISTANCE_VALUE,
                             osg::length( _center - _eye ) );
    // view->setFusionDistance(osgUtil::SceneView::PROPORTIONAL_TO_SCREEN_DISTANCE, 1.0);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Serialization support
//
REGISTER_OBJECT_WRAPPER( gsc_CameraProperty,
                         new gsc::CameraProperty,
                         gsc::CameraProperty,
                         "osg::Object gsc::CameraProperty" )
{
    ADD_VEC3D_SERIALIZER( Center, osg::dvec3( 0.0, 0.0, 0.0 ) );
    ADD_VEC3D_SERIALIZER( EyePoint, osg::dvec3( 0.0, -1.0, 0.0 ) );
    ADD_VEC3D_SERIALIZER( UpVector, osg::dvec3( 0.0, 0.0, 1.0 ) );
    ADD_VEC3D_SERIALIZER( RotationCenter, osg::dvec3( 0.0, 0.0, 0.0 ) );
    ADD_VEC3D_SERIALIZER( RotationAxis, osg::dvec3( 0.0, 0.0, 1.0 ) );
    ADD_DOUBLE_SERIALIZER( RotationSpeed, 0.0 );
}
