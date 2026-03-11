/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * SimpleDotVisitor, derived from BaseDotVisitor.
 */
#pragma once
#ifndef __SIMPLEDOTVISITOR_H__
    #define __SIMPLEDOTVISITOR_H__

    #ifndef __cplusplus
        #error "this is a c++ - header!"
    #endif

    #include "BaseDotVisitor.hpp"

namespace osgDot
{

    class SimpleDotVisitor : public BaseDotVisitor
    {
        public:

            SimpleDotVisitor();

            virtual ~SimpleDotVisitor();

        protected:

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

            virtual void
            drawNode( int                id,
                      const std::string& shape,
                      const std::string& style,
                      const std::string& label,
                      const std::string& color,
                      const std::string& fillColor );

            virtual void
            drawEdge( int                sourceId,
                      int                sinkId,
                      const std::string& style );
    };

}    // namespace osgDot

#endif    // __SIMPLEDOTVISITOR_H__
