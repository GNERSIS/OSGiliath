/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * TileMapper, derived from DualModeVisitor.
 * Provides: _containsGeode, getEyePoint, getEyeLocal, getDistanceToEyePoint, getDistanceFromEyePoint, apply.
 */
/*    Dec 2010 - TileMapper was fixed and simplified
    Nick
*/

#pragma once

#include "trpage_sys.h"
#include "trpage_read.h"

#include <osg/CullStack>
#include <osg/NodeVisitor>

#include <set>

namespace txp
{

struct TileIdentifier : public osg::Referenced
{
    TileIdentifier():
        x(-1),
        y(-1),
        lod(-1)
    {}

    TileIdentifier(int ax, int ay, int alod):
        x(ax),
        y(ay),
        lod(alod)
    {}

    TileIdentifier(const TileIdentifier& rhs):
        osg::Referenced(),
        x(rhs.x),
        y(rhs.y),
        lod(rhs.lod)
    {}

    TileIdentifier& operator = (const TileIdentifier& rhs)
    {
        if (this==&rhs) return *this;
        x = rhs.x;
        y = rhs.y;
        lod = rhs.lod;
        return *this;
    }

    void set(int ax, int ay, int alod)
    {
        x = ax;
        y = ay;
        lod = alod;
    }

    bool operator < (const TileIdentifier& rhs) const
    {
        if (lod<rhs.lod)
            return true;
        if (lod>rhs.lod)
            return false;
        if (x<rhs.x)
            return true;
        if (x>rhs.x)
            return false;
        if (y<rhs.y)
            return true;
        if (y>rhs.y)
            return false;
        return false;
    }

    int x,y,lod;

};

class TileMapper : public osg::DualModeVisitor, public osg::CullStack
{
public:

    typedef osg::dmat4::value_type value_type;


    TileMapper():
        osg::DualModeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN),
        _containsGeode(false) {}


    virtual osg::vec3 getEyePoint() const
    {
        return getEyeLocal();
    }
    virtual float getDistanceToEyePoint(const osg::vec3& pos, bool withLODScale) const;
    virtual float getDistanceFromEyePoint(const osg::vec3& pos, bool withLODScale) const;

    virtual void apply(osg::Node& node);
    virtual void apply(osg::Group& node);
    virtual void apply(osg::Geode& node);
    virtual void apply(osg::PagedLOD& node);

    void insertTile(const TileIdentifier& tid);

    bool isTileNeighbourALowerLODLevel(const TileIdentifier& tid, int dx, int dy) const;

protected:

    typedef std::map< TileIdentifier, int>        TileMap;
    TileMap                                        _tileMap;
    bool                                        _containsGeode;

};

} // namespace
