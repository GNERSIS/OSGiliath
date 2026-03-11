#ifndef _ORIENTATION_CONVERTER_H
#define _ORIENTATION_CONVERTER_H

#include <osg/maths/Matrix.hpp>
#include <osg/maths/Vec3.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/Node.hpp>

class OrientationConverter
{
    public:

        OrientationConverter( void );
        void
        setRotation( const osg::vec3& from,
                     const osg::vec3& to );
        void
        setRotation( float            degrees,
                     const osg::vec3& axis );
        void
        setTranslation( const osg::vec3& trans );
        void
        setScale( const osg::vec3& trans );
        void
        useWorldFrame( bool worldFrame );

        /** return the root of the updated subgraph as the subgraph
         * the node passed in my flatten during optimization.*/
        osg::Node*
        convert( osg::Node* node );

    private:

        OrientationConverter( const OrientationConverter& )
        {
        }

        OrientationConverter&
        operator=( const OrientationConverter& )
        {
            return *this;
        }

        osg::dmat4 R, T, S;
        bool       _trans_set;
        bool       _use_world_frame;
};
#endif
