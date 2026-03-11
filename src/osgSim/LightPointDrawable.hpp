/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * LightPointDrawable, derived from Drawable.
 * Provides: cloneType, clone, isSameKindAs, className, ColorPosition, ColorPosition.
 */
#pragma once

#include <osg/core/Endian.hpp>
#include <osg/geometry/Drawable.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>
#include <osg/state/BlendFunc.hpp>
#include <osg/state/ColorMask.hpp>
#include <osg/state/Depth.hpp>
#include <osg/state/Point.hpp>
#include <osgSim/Export>
#include <vector>

namespace osgSim
{

    class OSGSIM_EXPORT LightPointDrawable : public osg::Drawable
    {
        public:

            LightPointDrawable();

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            LightPointDrawable( const LightPointDrawable&,
                                const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            virtual osg::Object*
            cloneType() const
            {
                return new LightPointDrawable();
            }

            virtual osg::Object*
            clone( const osg::CopyOp& ) const
            {
                return new LightPointDrawable();
            }

            virtual bool
            isSameKindAs( const osg::Object* obj ) const
            {
                return dynamic_cast<const LightPointDrawable*>( obj ) != NULL;
            }

            virtual const char*
            className() const
            {
                return "LightPointDrawable";
            }

            // typedef std::pair<unsigned int,osg::vec3> ColorPosition;
            struct ColorPosition
            {
                    unsigned int first;
                    osg::vec3    second;

                    ColorPosition()
                    {
                    }

                    ColorPosition( unsigned int     f,
                                   const osg::vec3& s ) :
                        first( f ),
                        second( s )
                    {
                    }
            };

            void
            reset();

            inline unsigned int
            colorToRGBA( const osg::vec4& color ) const
            {
                return _endian == osg::BigEndian ? osg::asABGR( color )
                                                 : osg::asRGBA( color );
            }

            inline void
            addOpaqueLightPoint( unsigned int     pointSize,
                                 const osg::vec3& position,
                                 const osg::vec4& color )
            {
                if( pointSize >= _sizedOpaqueLightPointList.size() )
                {
                    _sizedOpaqueLightPointList.resize( pointSize + 1 );
                }
                _sizedOpaqueLightPointList[pointSize].push_back(
                    ColorPosition( colorToRGBA( color ), position )
                );
            }

            inline void
            addAdditiveLightPoint( unsigned int     pointSize,
                                   const osg::vec3& position,
                                   const osg::vec4& color )
            {
                if( pointSize >= _sizedAdditiveLightPointList.size() )
                {
                    _sizedAdditiveLightPointList.resize( pointSize + 1 );
                }
                _sizedAdditiveLightPointList[pointSize].push_back(
                    ColorPosition( colorToRGBA( color ), position )
                );
            }

            inline void
            addBlendedLightPoint( unsigned int     pointSize,
                                  const osg::vec3& position,
                                  const osg::vec4& color )
            {
                if( pointSize >= _sizedBlendedLightPointList.size() )
                {
                    _sizedBlendedLightPointList.resize( pointSize + 1 );
                }
                _sizedBlendedLightPointList[pointSize].push_back(
                    ColorPosition( colorToRGBA( color ), position )
                );
            }

            /** draw LightPoints. */
            virtual void
            drawImplementation( osg::RenderInfo& renderInfo ) const;

            void
            setSimulationTime( double time )
            {
                _simulationTime         = time;
                _simulationTimeInterval = 0.0;
            }

            void
            updateSimulationTime( double time )
            {
                _simulationTimeInterval = osg::clampAbove( time - _simulationTime, 0.0 );
                _simulationTime         = time;
            }

            double
            getSimulationTime() const
            {
                return _simulationTime;
            }

            double
            getSimulationTimeInterval() const
            {
                return _simulationTimeInterval;
            }

            virtual osg::box
            computeBoundingBox() const;

        protected:

            virtual ~LightPointDrawable();

            osg::Endian                         _endian;

            double                              _simulationTime;
            double                              _simulationTimeInterval;

            typedef std::vector<ColorPosition>  LightPointList;
            typedef std::vector<LightPointList> SizedLightPointList;

            SizedLightPointList                 _sizedOpaqueLightPointList;
            SizedLightPointList                 _sizedAdditiveLightPointList;
            SizedLightPointList                 _sizedBlendedLightPointList;

            osg::ref_ptr<osg::Depth>            _depthOff;
            osg::ref_ptr<osg::Depth>            _depthOn;
            osg::ref_ptr<osg::BlendFunc>        _blendOne;
            osg::ref_ptr<osg::BlendFunc>        _blendOneMinusSrcAlpha;
            osg::ref_ptr<osg::ColorMask>        _colorMaskOff;
    };

}
