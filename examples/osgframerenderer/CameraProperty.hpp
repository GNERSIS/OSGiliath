#ifndef CAMERAPROPERTY_H
#define CAMERAPROPERTY_H

#include "UpdateProperty.hpp"

#include <osg/core/Inherit.hpp>
#include <osg/maths/compat.hpp>
#include <osg/traversal/AnimationPath.hpp>

namespace gsc
{

    class CameraProperty : public osg::Inherit<gsc::UpdateProperty, CameraProperty>
    {
        public:

            OSG_REGISTER_TYPE( gsc,
                               CameraProperty )

            CameraProperty() :
                _center( 0.0,
                         0.0,
                         0.0 ),
                _eye( 0.0,
                      -1.0,
                      0.0 ),
                _up( 0.0,
                     0.0,
                     1.0 ),
                _rotationCenter( 0.0,
                                 0.0,
                                 0.0 ),
                _rotationAxis( 0.0,
                               0.0,
                               1.0 ),
                _rotationSpeed( 0.0 )
            {
            }

            CameraProperty( const CameraProperty& cp,
                            const osg::CopyOp&    copyop = osg::CopyOp::SHALLOW_COPY ) :
                Inherit( cp,
                         copyop ),
                _center( cp._center ),
                _eye( cp._eye ),
                _up( cp._up ),
                _rotationCenter( cp._rotationCenter ),
                _rotationAxis( cp._rotationAxis ),
                _rotationSpeed( cp._rotationSpeed )
            {
            }

            void
            setToModel( const osg::Node* node );

            void
            setCenter( const osg::dvec3& center )
            {
                _center = center;
            }

            const osg::dvec3&
            getCenter() const
            {
                return _center;
            }

            void
            setEyePoint( const osg::dvec3& eye )
            {
                _eye = eye;
            }

            const osg::dvec3&
            getEyePoint() const
            {
                return _eye;
            }

            void
            setUpVector( const osg::dvec3& up )
            {
                _up = up;
            }

            const osg::dvec3&
            getUpVector() const
            {
                return _up;
            }

            void
            setRotationCenter( const osg::dvec3& center )
            {
                _rotationCenter = center;
            }

            const osg::dvec3&
            getRotationCenter() const
            {
                return _rotationCenter;
            }

            void
            setRotationAxis( const osg::dvec3& axis )
            {
                _rotationAxis = axis;
            }

            const osg::dvec3&
            getRotationAxis() const
            {
                return _rotationAxis;
            }

            void
            setRotationSpeed( double speed )
            {
                _rotationSpeed = speed;
            }

            double
            getRotationSpeed() const
            {
                return _rotationSpeed;
            }

            virtual void
            update( osgViewer::View* view );

        protected:

            virtual ~CameraProperty()
            {
            }

            osg::dvec3 _center;
            osg::dvec3 _eye;
            osg::dvec3 _up;
            osg::dvec3 _rotationCenter;
            osg::dvec3 _rotationAxis;
            double     _rotationSpeed;
    };

}

#endif
