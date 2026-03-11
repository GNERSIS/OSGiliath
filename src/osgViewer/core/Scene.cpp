/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Shared scene data holder. Wraps the scene graph root with
 * DatabasePager, ImagePager, and update visitor references.
 */
#include <osgViewer/core/Scene.hpp>

#include <osgGA/events/EventVisitor.hpp>

using namespace osgViewer;

namespace osgViewer
{

    struct SceneSingleton
    {
            SceneSingleton()
            {
            }

            inline void
            add( Scene* scene )
            {
                std::lock_guard<std::mutex> lock( _mutex );
                _cache.push_back( scene );
            }

            inline void
            remove( Scene* scene )
            {
                std::lock_guard<std::mutex> lock( _mutex );
                for( SceneCache::iterator itr = _cache.begin(); itr != _cache.end();
                     ++itr )
                {
                    if( scene == itr->get() )
                    {
                        _cache.erase( itr );
                        break;
                    }
                }
            }

            inline Scene*
            getScene( osg::Node* node )
            {
                std::lock_guard<std::mutex> lock( _mutex );
                for( SceneCache::iterator itr = _cache.begin(); itr != _cache.end();
                     ++itr )
                {
                    Scene* scene = itr->get();
                    if( scene && scene->getSceneData() == node )
                    {
                        return scene;
                    }
                }
                return 0;
            }

            typedef std::vector<osg::observer_ptr<Scene>> SceneCache;
            SceneCache                                    _cache;
            std::mutex                                    _mutex;
    };

    static SceneSingleton&
    getSceneSingleton()
    {
        static SceneSingleton s_sceneSingleton;
        return s_sceneSingleton;
    }

    // Use a proxy to force the initialization of the SceneSingleton during static
    // initialization
    OSG_INIT_SINGLETON_PROXY( SceneSingletonProxy,
                              getSceneSingleton() )

}

Scene::Scene() :
    osg::Referenced( true )
{
    setDatabasePager( osgDB::DatabasePager::create() );
    setImagePager( new osgDB::ImagePager );
    getSceneSingleton().add( this );
}

Scene::~Scene()
{
    getSceneSingleton().remove( this );
}

void
Scene::setSceneData( osg::Node* node )
{
    _sceneData = node;
}

osg::Node*
Scene::getSceneData()
{
    return _sceneData.get();
}

const osg::Node*
Scene::getSceneData() const
{
    return _sceneData.get();
}

void
Scene::setDatabasePager( osgDB::DatabasePager* dp )
{
    _databasePager = dp;
}

void
Scene::setImagePager( osgDB::ImagePager* ip )
{
    _imagePager = ip;
}

bool
Scene::requiresUpdateSceneGraph() const
{
    // check if the database pager needs to update the scene
    if( getDatabasePager()->requiresUpdateSceneGraph() )
    {
        return true;
    }

    // check if the image pager needs to update the scene
    if( getImagePager()->requiresUpdateSceneGraph() )
    {
        return true;
    }

    // check if scene graph needs update traversal
    if( _sceneData.valid() &&
        ( _sceneData->getUpdateCallback() ||
          ( _sceneData->getNumChildrenRequiringUpdateTraversal() > 0 ) ) )
    {
        return true;
    }

    return false;
}

void
Scene::updateSceneGraph( osg::NodeVisitor& updateVisitor )
{
    if( !_sceneData )
    {
        return;
    }

    if( getDatabasePager() )
    {
        // synchronize changes required by the DatabasePager thread to the scene graph
        getDatabasePager()->updateSceneGraph( *updateVisitor.getFrameStamp() );
    }

    if( getImagePager() )
    {
        // synchronize changes required by the DatabasePager thread to the scene graph
        getImagePager()->updateSceneGraph( *( updateVisitor.getFrameStamp() ) );
    }

    if( getSceneData() )
    {
        updateVisitor.setImageRequestHandler( getImagePager() );
        getSceneData()->accept( updateVisitor );
    }
}

bool
Scene::requiresRedraw() const
{
    // check if the database pager needs a redraw
    if( getDatabasePager()->requiresRedraw() )
    {
        return true;
    }

    return false;
}

Scene*
Scene::getScene( osg::Node* node )
{
    return getSceneSingleton().getScene( node );
    return 0;
}

Scene*
Scene::getOrCreateScene( osg::Node* node )
{
    if( !node )
    {
        return 0;
    }

    osgViewer::Scene* scene = getScene( node );
    if( !scene )
    {
        scene = new Scene;
        scene->setSceneData( node );
    }

    return scene;
}
