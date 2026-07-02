/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Distance-fading text that blends out as the camera moves away.
 * Used for annotations that declutter at distance.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osgText/Text.hpp>

namespace osgText
{

    class OSGTEXT_EXPORT FadeText : public osg::Inherit<osgText::Text, FadeText>
    {
        public:

            FadeText();
            FadeText( const Text&        text,
                      const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE(
                osgText,
                FadeText
            ) /** Set the speed that the alpha value changes as the text is occluded or
                 becomes visible.*/

            void
            setFadeSpeed( float fadeSpeed )
            {
                _fadeSpeed = fadeSpeed;
            }

            /** Get the speed that the alpha value changes.*/
            float
            getFadeSpeed() const
            {
                return _fadeSpeed;
            }

            /** Draw the text.*/
            virtual void
            drawImplementation( osg::RenderInfo& renderInfo ) const;

        protected:

            virtual ~FadeText()
            {
            }

            void
            init();

            struct FadeTextUpdateCallback;
            friend struct FadeTextUpdateCallback;

            typedef std::map<osg::View*, osg::vec4> ViewBlendColourMap;

            ViewBlendColourMap&
            getViewBlendColourMap()
            {
                return _viewBlendColourMap;
            }

            const ViewBlendColourMap&
            getViewBlendColourMap() const
            {
                return _viewBlendColourMap;
            }

            float                      _fadeSpeed;

            mutable ViewBlendColourMap _viewBlendColourMap;
    };

}
