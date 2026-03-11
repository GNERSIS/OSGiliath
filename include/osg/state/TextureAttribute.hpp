/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Base class for texture-related state attributes. Provides
 * common texture unit binding and texture target type identity.
 */
#pragma once

#include <osg/state/StateAttribute.hpp>

namespace osg
{

    // forward declare
    class StateSet;

    class TextureAttribute : public StateAttribute
    {
        public:

            TextureAttribute() :
                _textureUnit( 0 )
            {
            }

            TextureAttribute( const TextureAttribute& ta,
                              const CopyOp&           copyop = CopyOp::SHALLOW_COPY ) :
                StateAttribute( ta,
                                copyop ),
                _textureUnit( 0 )
            {
            }

            virtual bool
            isTextureAttribute() const
            {
                return true;
            }

            virtual unsigned int
            getMember() const
            {
                return _textureUnit;
            }

        protected:

            virtual ~TextureAttribute()
            {
            }

            // called when the TextureAttribute is assigned to a StateSet;
            virtual void
            setTextureUnit( unsigned int unit )
            {
                if( unit != _textureUnit )
                {
                    _textureUnit = unit;

                    // configure the uniform names to reflect new texture unit value
                    configureUniformNames();
                }
            }

            // TextureAttribute subclasses that manage uniforms will need to override
            // configureUniformNames() and adjust uniform names to reflect texture unit
            // that the attribute is assigned to.
            virtual void
            configureUniformNames()
            {
            }

            friend class StateSet;

            unsigned int _textureUnit;
    };

}
