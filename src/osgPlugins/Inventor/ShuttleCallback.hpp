#pragma once

#include <osg/nodes/Node.hpp>
#include <osgUtil/Export.hpp>

// Callback for handling the SoShuttle node
class ShuttleCallback : public osg::NodeCallback
{
    public:

        ShuttleCallback( const osg::vec3& startPos,
                         const osg::vec3& endPos,
                         float            frequency );

        virtual void
        operator()( osg::Node*        node,
                    osg::NodeVisitor* nv );

    protected:

        osg::vec3    _startPos;
        osg::vec3    _endPos;
        float        _frequency;

        unsigned int _previousTraversalNumber;
        double       _previousTime;
        float        _angle;
};
