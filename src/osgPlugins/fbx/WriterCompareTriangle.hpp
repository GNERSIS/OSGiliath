#pragma once

#include <iostream>
#include <osg/geometry/Geometry.hpp>
#include <osg/nodes/Geode.hpp>

struct Triangle
{
        unsigned int t1;
        unsigned int t2;
        unsigned int t3;
        unsigned int material;
};

class WriterCompareTriangle
{
    public:

        WriterCompareTriangle( const osg::Geode& geode,
                               unsigned int      nbVertices );

        bool
        operator()( const std::pair<Triangle,
                                    int>& t1,
                    const std::pair<Triangle,
                                    int>& t2 ) const;

    private:

        void    // This function prevents the scene being cut into too many boxes
        setMaxMin( unsigned int& nbVerticesX,
                   unsigned int& nbVerticesY,
                   unsigned int& nbVerticesZ ) const;

        /**
         *  Cut the scene in different box to sort.
         *  \param nbVertices is the number of vertices in mesh.
         *  \param sceneBox contain the size of the scene.
         */
        void
        cutscene( int             nbVertices,
                  const osg::box& sceneBox );

        /**
         *  Find in which box those points are.
         *  \return the place of the box in the vector.
         *  \sa See cutScene() about the definition of the boxes for faces sorting.
         */
        int
                              inWhichBox( const osg::vec3::value_type x,
                                          const osg::vec3::value_type y,
                                          const osg::vec3::value_type z ) const;

        const osg::Geode&     geode;
        std::vector<osg::box> boxList;
};
