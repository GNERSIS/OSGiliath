/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * BaseDotVisitor, derived from DualModeVisitor.
 * Provides: setOptions, run, apply, apply, apply.
 */
#pragma once
#ifndef __DOTBASEVISITOR_H__
    #define __DOTBASEVISITOR_H__

    #ifndef __cplusplus
        #error "this is a c++ - header!"
    #endif

    #include <osg/core/ref_ptr.hpp>
    #include <osg/geometry/Drawable.hpp>
    #include <osg/nodes/Geode.hpp>
    #include <osg/nodes/Group.hpp>
    #include <osg/traversal/NodeVisitor.hpp>
    #include <osgDB/registry/Options.hpp>
    #include <sstream>

namespace osgDot
{

    class BaseDotVisitor : public osg::DualModeVisitor
    {
        public:

            typedef std::map<osg::Object*, int> ObjectMap;

        public:

            BaseDotVisitor();

            virtual ~BaseDotVisitor();

            void
            setOptions( const osgDB::Options* options );

            bool
            run( const osg::Node& root,
                 std::ostream*    ostream );

            virtual void
            apply( osg::Node& node );

            virtual void
            apply( osg::Drawable& node );

            virtual void
            apply( osg::Group& node );

        protected:

            void
            handleNodeAndTraverse( osg::Node& node,
                                   int        id );

            virtual void
            handle( osg::StateSet& stateset,
                    int            id );
            virtual void
            handle( osg::Drawable& drawable,
                    int            id );
            virtual void
            handle( osg::Node& node,
                    int        id );
            virtual void
            handle( osg::Group& node,
                    int         id );

            virtual void
            handle( osg::Node&     node,
                    osg::StateSet& stateset,
                    int            parentID,
                    int            childID );
            virtual void
            handle( osg::Drawable& drawable,
                    osg::StateSet& stateset,
                    int            parentID,
                    int            childID );
            virtual void
                                         handle( osg::Group& parent,
                                                 osg::Node&  child,
                                                 int         parentID,
                                                 int         childID );

            osg::ref_ptr<osgDB::Options> _options;

            std::string                  _rankdir;

            std::stringstream            _nodes;
            std::stringstream            _edges;

        private:

            bool
                      getOrCreateId( osg::Object* object,
                                     int&         id );

            ObjectMap _objectMap;
    };

}    // namespace osgDot

#endif    // __DOTBASEVISITOR_H__
