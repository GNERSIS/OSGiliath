#ifndef CAMERAPATHPROPERTY_H
#define CAMERAPATHPROPERTY_H

#include "UpdateProperty.hpp"

#include <osg/core/Inherit.hpp>
#include <osg/maths/compat.hpp>
#include <osg/traversal/AnimationPath.hpp>

namespace gsc
{

    class CameraPathProperty
        : public osg::Inherit<gsc::UpdateProperty, CameraPathProperty>
    {
        public:

            OSG_REGISTER_TYPE( gsc,
                               CameraPathProperty )

            CameraPathProperty()
            {
            }

            CameraPathProperty( const std::string& filename )
            {
                setAnimationPathFileName( filename );
            }

            CameraPathProperty( const CameraPathProperty& cpp,
                                const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY ) :
                Inherit( cpp,
                         copyop )
            {
            }

            void
            setAnimationPathFileName( const std::string& filename )
            {
                _filename = filename;
                loadAnimationPath();
            }

            const std::string&
            getAnimationPathFileName() const
            {
                return _filename;
            }

            void
            setAnimationPath( osg::AnimationPath* ap )
            {
                _animationPath = ap;
            }

            osg::AnimationPath*
            getAnimationPath()
            {
                return _animationPath.get();
            }

            const osg::AnimationPath*
            getAnimationPath() const
            {
                return _animationPath.get();
            }

            bool
            getTimeRange( double& startTime,
                          double& endTime ) const;

            void
            resetTimeRange( double startTime,
                            double endTime );

            virtual void
            update( osgViewer::View* view );

        protected:

            virtual ~CameraPathProperty()
            {
            }

            void
                                             loadAnimationPath();

            std::string                      _filename;
            osg::ref_ptr<osg::AnimationPath> _animationPath;
    };

}

#endif
