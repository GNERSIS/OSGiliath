/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * dxfBlock, derived from Referenced.
 * Provides: getName, assign, getEntityList, getPosition.
 */
#pragma once

#include <map>
#include <osg/core/ref_ptr.hpp>
#include <osg/core/Referenced.hpp>
#include <osg/maths/vec3.hpp>
#include <string>
#include <vector>

class dxfFile;
class codeValue;
class dxfEntity;

typedef std::vector<osg::ref_ptr<dxfEntity>> EntityList;

class dxfBlock : public osg::Referenced
{
    public:

        dxfBlock() :
            _currentEntity( NULL )
        {
        }

        virtual ~dxfBlock()
        {
        }

        inline const std::string&
        getName() const
        {
            return _name;
        }

        virtual void
        assign( dxfFile*   dxf,
                codeValue& cv );

        EntityList&
        getEntityList()
        {
            return _entityList;
        }

        const osg::dvec3&
        getPosition() const;

    protected:

        EntityList  _entityList;
        dxfEntity*  _currentEntity;
        std::string _name;
        osg::dvec3  _position;
};
