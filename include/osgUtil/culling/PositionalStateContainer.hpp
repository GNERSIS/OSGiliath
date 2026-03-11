/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Collects positional state (lights) encountered during culling
 * for application during the draw traversal.
 */
#pragma once

#include <osg/core/Object.hpp>
#include <osg/lighting/Light.hpp>
#include <osg/state/State.hpp>
#include <osgUtil/culling/RenderLeaf.hpp>
#include <osgUtil/culling/StateGraph.hpp>

namespace osgUtil
{

    /**
     * PositionalStateContainer base class. Used in RenderStage class.
     */
    class OSGUTIL_EXPORT PositionalStateContainer : public osg::Object
    {
        public:

            PositionalStateContainer();

            virtual osg::Object*
            cloneType() const
            {
                return new PositionalStateContainer();
            }

            virtual osg::Object*
            clone( const osg::CopyOp& ) const
            {
                return new PositionalStateContainer();
            }    // note only implements a clone of type.

            virtual bool
            isSameKindAs( const osg::Object* obj ) const
            {
                return dynamic_cast<const PositionalStateContainer*>( obj ) != 0L;
            }

            virtual const char*
            libraryName() const
            {
                return "osgUtil";
            }

            virtual const char*
            className() const
            {
                return "PositionalStateContainer";
            }

            virtual void
            reset();

            typedef std::pair<osg::ref_ptr<const osg::StateAttribute>,
                              osg::ref_ptr<osg::RefMatrix>>
                                                           AttrMatrixPair;
            typedef std::vector<AttrMatrixPair>            AttrMatrixList;
            typedef std::map<unsigned int, AttrMatrixList> TexUnitAttrMatrixListMap;

            AttrMatrixList&
            getAttrMatrixList()
            {
                return _attrList;
            }

            virtual void
            addPositionedAttribute( osg::RefMatrix*            matrix,
                                    const osg::StateAttribute* attr )
            {
                _attrList.push_back( AttrMatrixPair( attr, matrix ) );
            }

            TexUnitAttrMatrixListMap&
            getTexUnitAttrMatrixListMap()
            {
                return _texAttrListMap;
            }

            virtual void
            addPositionedTextureAttribute( unsigned int               textureUnit,
                                           osg::RefMatrix*            matrix,
                                           const osg::StateAttribute* attr )
            {
                _texAttrListMap[textureUnit].push_back( AttrMatrixPair( attr, matrix ) );
            }

            virtual void
            draw( osg::State&       state,
                  RenderLeaf*&      previous,
                  const osg::dmat4* postMultMatrix = 0 );

        public:

            AttrMatrixList           _attrList;
            TexUnitAttrMatrixListMap _texAttrListMap;

        protected:

            virtual ~PositionalStateContainer();
    };

}
