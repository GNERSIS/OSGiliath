#ifndef UPDATEPROPERTY_H
#define UPDATEPROPERTY_H

#include <osg/core/Inherit.hpp>
#include <osg/maths/compat.hpp>
#include <osg/traversal/AnimationPath.hpp>
#include <osgDB/io/FileNameUtils.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/io/WriteFile.hpp>
#include <osgViewer/core/Viewer.hpp>

namespace gsc
{

    class UpdateProperty : public osg::Inherit<osg::Object, UpdateProperty>
    {
        public:

            OSG_REGISTER_TYPE( gsc,
                               UpdateProperty )

            UpdateProperty()
            {
            }

            UpdateProperty( const UpdateProperty& up,
                            const osg::CopyOp&    copyop = osg::CopyOp::SHALLOW_COPY ) :
                Inherit( up,
                         copyop )
            {
            }

            virtual void
            update( osgViewer::View* view )
            {
            }

        protected:

            virtual ~UpdateProperty()
            {
            }
    };

}

#endif
