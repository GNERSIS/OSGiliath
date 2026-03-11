#pragma once

// #define DEBUG 1
#ifdef DEBUG
    #include <iostream>
#endif

#include <osg/geometry/Geometry.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <stack>

class Normals : public osg::Geode
{
    public:

        enum Mode
        {
            SurfaceNormals,
            VertexNormals,
        };

        Normals( osg::Node* node,
                 float      scale = 1.0,
                 Mode       mode  = SurfaceNormals );

    private:

        class MakeNormalsVisitor : public osg::DualModeVisitor
        {
            public:

                MakeNormalsVisitor( float normalScale,
                                    Mode  mode );

                void
                setMode( Mode mode )
                {
                    _mode = mode;
                }

                virtual void
                apply( osg::MatrixTransform& tx );

                virtual void
                apply( osg::Geode& geode );

                osg::Vec3Array*
                getCoords()
                {
                    return _local_coords.get();
                }

            private:

                osg::ref_ptr<osg::Vec3Array> _local_coords;
                float                        _normal_scale;
                Mode                         _mode;
                osg::dmat4                   _mat;
                std::stack<osg::dmat4>       _matStack;

                void
                _processPrimitive( unsigned int             nv,
                                   osg::Vec3Array::iterator coords,
                                   osg::Vec3Array::iterator normals,
                                   osg::Array::Binding      binding );
        };

#ifdef DEBUG
        static void
        _printPrimitiveType( osg::PrimitiveSet* pset );
#endif
};

class SurfaceNormals : public Normals
{
    public:

        SurfaceNormals( osg::Node* node,
                        float      scale = 1.0 ) :
            Normals( node,
                     scale,
                     Normals::SurfaceNormals )
        {
        }
};

class VertexNormals : public Normals
{
    public:

        VertexNormals( osg::Node* node,
                       float      scale = 1.0 ) :
            Normals( node,
                     scale,
                     Normals::VertexNormals )
        {
        }
};
