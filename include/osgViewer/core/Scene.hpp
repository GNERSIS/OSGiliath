/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Shared scene data holder. Wraps the scene graph root with
 * DatabasePager, ImagePager, and update visitor references.
 */
#pragma once

#include <list>
#include <osgDB/cache/DatabasePager.hpp>
#include <osgDB/cache/ImagePager.hpp>
#include <osgGA/events/EventVisitor.hpp>
#include <osgGA/events/GUIEventHandler.hpp>
#include <osgViewer/core/Export.hpp>

namespace osgViewer
{

    /** Scene holds the higher level reference to a single scene graph.*/
    class OSGVIEWER_EXPORT Scene : public osg::Referenced
    {
        public:

            virtual const char*
            className() const
            {
                return "Scene";
            }

            void
            setSceneData( osg::Node* node );
            osg::Node*
            getSceneData();
            const osg::Node*
            getSceneData() const;

            void
            setDatabasePager( osgDB::DatabasePager* dp );

            osgDB::DatabasePager*
            getDatabasePager()
            {
                return _databasePager.get();
            }

            const osgDB::DatabasePager*
            getDatabasePager() const
            {
                return _databasePager.get();
            }

            void
            setImagePager( osgDB::ImagePager* ip );

            osgDB::ImagePager*
            getImagePager()
            {
                return _imagePager.get();
            }

            const osgDB::ImagePager*
            getImagePager() const
            {
                return _imagePager.get();
            }

            virtual bool
            requiresUpdateSceneGraph() const;

            virtual void
            updateSceneGraph( osg::NodeVisitor& updateVisitor );

            virtual bool
            requiresRedraw() const;

            /** Get the Scene object that has the specified node assigned to it.
             * return 0 if no Scene has yet been assigned the specified node.*/
            static Scene*
            getScene( osg::Node* node );

        protected:

            Scene();
            virtual ~Scene();

            /** Get the Scene object that has the specified node assigned to it.
             * or return a new Scene if no Scene has yet been assigned the specified
             * node.*/
            static Scene*
            getOrCreateScene( osg::Node* node );

            friend class View;

            osg::ref_ptr<osg::Node>            _sceneData;

            osg::ref_ptr<osgDB::DatabasePager> _databasePager;
            osg::ref_ptr<osgDB::ImagePager>    _imagePager;
    };

}
