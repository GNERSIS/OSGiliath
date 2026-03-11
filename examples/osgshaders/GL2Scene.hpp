/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * GL2Scene example application
 */
/* file:	examples/osgglsl/GL2Scene.h
 * author:	Mike Weiblen 2005-03-30
 *
 * See http://www.3dlabs.com/opengl2/ for more information regarding
 * the OpenGL Shading Language.
 */

#include <osg/core/ref_ptr.hpp>
#include <osg/core/Referenced.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Node.hpp>
#include <osg/state/Program.hpp>

class GL2Scene : public osg::Referenced
{
    public:

        GL2Scene();

        osg::ref_ptr<osg::Group>
        getRootNode()
        {
            return _rootNode;
        }

        void
        reloadShaderSource();
        void
        toggleShaderEnable();

    protected:

        ~GL2Scene();

    private: /*methods*/

        osg::ref_ptr<osg::Group>
        buildScene();

    private: /*data*/

        osg::ref_ptr<osg::Group>                _rootNode;
        std::vector<osg::ref_ptr<osg::Program>> _programList;
        bool                                    _shadersEnabled;
};

typedef osg::ref_ptr<GL2Scene> GL2ScenePtr;

/*EOF*/
