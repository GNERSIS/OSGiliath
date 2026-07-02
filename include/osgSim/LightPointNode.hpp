/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Point-light rendering node for airport/runway lights.
 * Renders large numbers of attenuated light points efficiently.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/maths/box.hpp>
#include <osg/maths/quat.hpp>
#include <osg/maths/vec4.hpp>
#include <osg/nodes/Node.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osgSim/Export.hpp>
#include <osgSim/LightPoint.hpp>
#include <osgSim/LightPointSystem.hpp>
#include <set>
#include <vector>

namespace osgSim
{

    class OSGSIM_EXPORT LightPointNode : public osg::Inherit<osg::Node, LightPointNode>
    {
        public:

            typedef std::vector<LightPoint> LightPointList;

            LightPointNode();

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            LightPointNode( const LightPointNode&,
                            const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgSim,
                               LightPointNode )

            virtual void
            traverse( osg::NodeVisitor& nv );

            unsigned int
            getNumLightPoints() const
            {
                return static_cast<unsigned int>( _lightPointList.size() );
            }

            unsigned int
            addLightPoint( const LightPoint& lp );

            void
            removeLightPoint( unsigned int pos );

            LightPoint&
            getLightPoint( unsigned int pos )
            {
                return _lightPointList[pos];
            }

            const LightPoint&
            getLightPoint( unsigned int pos ) const
            {
                return _lightPointList[pos];
            }

            void
            setLightPointList( const LightPointList& lpl )
            {
                _lightPointList = lpl;
            }

            LightPointList&
            getLightPointList()
            {
                return _lightPointList;
            }

            const LightPointList&
            getLightPointList() const
            {
                return _lightPointList;
            }

            void
            setMinPixelSize( float minPixelSize )
            {
                _minPixelSize = minPixelSize;
            }

            float
            getMinPixelSize() const
            {
                return _minPixelSize;
            }

            void
            setMaxPixelSize( float maxPixelSize )
            {
                _maxPixelSize = maxPixelSize;
            }

            float
            getMaxPixelSize() const
            {
                return _maxPixelSize;
            }

            void
            setMaxVisibleDistance2( float maxVisibleDistance2 )
            {
                _maxVisibleDistance2 = maxVisibleDistance2;
            }

            float
            getMaxVisibleDistance2() const
            {
                return _maxVisibleDistance2;
            }

            void
            setLightPointSystem( osgSim::LightPointSystem* lps )
            {
                _lightSystem = lps;
            }

            osgSim::LightPointSystem*
            getLightPointSystem()
            {
                return _lightSystem.get();
            }

            const osgSim::LightPointSystem*
            getLightPointSystem() const
            {
                return _lightSystem.get();
            }

            void
            setPointSprite( bool enable = true )
            {
                _pointSprites = enable;
            }

            bool
            getPointSprite() const
            {
                return _pointSprites;
            }

            virtual osg::sphere
            computeBound() const;

        protected:

            ~LightPointNode()
            {
            }

            // used to cache the bounding box of the lightpoints as a tighter
            // view frustum check.
            mutable osg::box                       _bbox;

            LightPointList                         _lightPointList;

            float                                  _minPixelSize;
            float                                  _maxPixelSize;
            float                                  _maxVisibleDistance2;

            osg::ref_ptr<osgSim::LightPointSystem> _lightSystem;

            bool                                   _pointSprites;
    };

}
