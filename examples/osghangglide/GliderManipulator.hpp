/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * GliderManipulator example application
 */
#ifndef OSGGA_GliderMANIPULATOR
    #define OSGGA_GliderMANIPULATOR 1

    #include <osg/maths/quat.hpp>
    #include <osgGA/manipulators/CameraManipulator.hpp>

/**
GliderManipulator is a CameraManipulator which provides Glider simulator-like
updating of the camera position & orientation. By default, the left mouse
button accelerates, the right mouse button decelerates, and the middle mouse
button (or left and right simultaneously) stops dead.
*/

class GliderManipulator : public osgGA::CameraManipulator
{
    public:

        GliderManipulator();

        virtual const char*
        className() const
        {
            return "Glider";
        }

        /** set the position of the matrix manipulator using a 4x4 dmat4.*/
        virtual void
        setByMatrix( const osg::dmat4& matrix );

        /** set the position of the matrix manipulator using a 4x4 dmat4.*/
        virtual void
        setByInverseMatrix( const osg::dmat4& matrix )
        {
            setByMatrix( osg::inverse( matrix ) );
        }

        /** get the position of the manipulator as 4x4 dmat4.*/
        virtual osg::dmat4
        getMatrix() const;

        /** get the position of the manipulator as a inverse matrix of the manipulator,
         * typically used as a model view matrix.*/
        virtual osg::dmat4
        getInverseMatrix() const;

        virtual void
        setNode( osg::Node* );

        virtual const osg::Node*
        getNode() const;

        virtual osg::Node*
        getNode();

        virtual void
        home( const osgGA::GUIEventAdapter& ea,
              osgGA::GUIActionAdapter&      us );

        virtual void
        init( const osgGA::GUIEventAdapter& ea,
              osgGA::GUIActionAdapter&      us );

        virtual bool
        handle( const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter&      us );

        /** Get the keyboard and mouse usage of this manipulator.*/
        virtual void
        getUsage( osg::ApplicationUsage& usage ) const;

        enum YawControlMode
        {
            YAW_AUTOMATICALLY_WHEN_BANKED,
            NO_AUTOMATIC_YAW,
        };

        /**        Configure the Yaw control for the Glider model.        */
        void
        setYawControlMode( YawControlMode ycm )
        {
            _yawMode = ycm;
        }

    protected:

        virtual ~GliderManipulator();

        /** Reset the internal GUIEvent stack.*/
        void
        flushMouseEventStack();
        /** Add the current mouse GUIEvent to internal stack.*/
        void
        addMouseEvent( const osgGA::GUIEventAdapter& ea );

        void
        computePosition( const osg::vec3& eye,
                         const osg::vec3& lv,
                         const osg::vec3& up );

        /** For the give mouse movement calculate the movement of the camera.
            Return true is camera has moved and a redraw is required.*/
        bool
                                                   calcMovement();

        // Internal event stack comprising last three mouse events.
        osg::ref_ptr<const osgGA::GUIEventAdapter> _ga_t1;
        osg::ref_ptr<const osgGA::GUIEventAdapter> _ga_t0;

        osg::ref_ptr<osg::Node>                    _node;

        float                                      _modelScale;
        float                                      _velocity;

        YawControlMode                             _yawMode;

        osg::vec3                                  _eye;
        osg::quat                                  _rotation;
        float                                      _distance;
};

#endif
