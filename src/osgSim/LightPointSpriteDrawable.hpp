/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * LightPointSpriteDrawable, derived from LightPointDrawable.
 * Provides: cloneType, clone, isSameKindAs, className, drawImplementation.
 */
#pragma once

#include "LightPointDrawable.hpp"

#include <osg/geometry/Drawable.hpp>
#include <osgSim/Export.hpp>

namespace osgSim
{

    class OSGSIM_EXPORT LightPointSpriteDrawable : public osgSim::LightPointDrawable
    {
        public:

            LightPointSpriteDrawable();

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            LightPointSpriteDrawable( const LightPointSpriteDrawable&,
                                      const osg::CopyOp& copyop =
                                          osg::CopyOp::SHALLOW_COPY );

            virtual osg::Object*
            cloneType() const
            {
                return new LightPointSpriteDrawable();
            }

            virtual osg::Object*
            clone( const osg::CopyOp& ) const
            {
                return new LightPointSpriteDrawable();
            }

            virtual bool
            isSameKindAs( const osg::Object* obj ) const
            {
                return dynamic_cast<const LightPointSpriteDrawable*>( obj ) != NULL;
            }

            virtual const char*
            className() const
            {
                return "LightPointSpriteDrawable";
            }

            /** draw LightPoints. */
            virtual void
            drawImplementation( osg::RenderInfo& renderInfo ) const;

        protected:

            virtual ~LightPointSpriteDrawable()
            {
            }
    };

}
