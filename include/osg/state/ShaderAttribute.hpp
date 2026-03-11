/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * State attribute combining a shader program with associated uniforms
 * and modes. Simplifies per-technique shader setup.
 */
#pragma once

#include <osg/state/Shader.hpp>
#include <osg/state/StateAttribute.hpp>
#include <osg/state/Uniform.hpp>

namespace osg
{

    class OSG_EXPORT ShaderAttribute : public StateAttribute
    {
        public:

            ShaderAttribute();

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            ShaderAttribute( const ShaderAttribute& sa,
                             const CopyOp&          copyop = CopyOp::SHALLOW_COPY );

            virtual osg::Object*
            cloneType() const;

            virtual osg::Object*
            clone( const osg::CopyOp& copyop ) const
            {
                return new ShaderAttribute( *this, copyop );
            }

            virtual bool
            isSameKindAs( const osg::Object* obj ) const
            {
                return dynamic_cast<const ShaderAttribute*>( obj ) != NULL;
            }

            virtual const char*
            libraryName() const
            {
                return "osg";
            }

            virtual const char*
            className() const
            {
                return "ShaderAttribute";
            }

            /** return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            virtual int
            compare( const StateAttribute& sa ) const;

            void
            setType( Type type );

            virtual Type
            getType() const
            {
                return _type;
            }

            unsigned int
            addShader( Shader* shader )
            {
                return _shaderComponent->addShader( shader );
            }

            void
            removeShader( unsigned int i )
            {
                _shaderComponent->removeShader( i );
            }

            unsigned int
            getNumShaders() const
            {
                return _shaderComponent->getNumShaders();
            }

            Shader*
            getShader( unsigned int i )
            {
                return _shaderComponent->getShader( i );
            }

            const Shader*
            getShader( unsigned int i ) const
            {
                return _shaderComponent->getShader( i );
            }

            unsigned int
            addUniform( Uniform* uniform );
            void
            removeUniform( unsigned int i );

            unsigned int
            getNumUniforms() const
            {
                return static_cast<unsigned int>( _uniforms.size() );
            }

            Uniform*
            getUniform( unsigned int i )
            {
                return _uniforms[i].get();
            }

            const Uniform*
            getUniform( unsigned int i ) const
            {
                return _uniforms[i].get();
            }

            virtual bool
            getModeUsage( StateAttribute::ModeUsage& usage ) const;

            virtual void
            apply( State& state ) const;

            virtual void
            compileGLObjects( State& state ) const;

            virtual void
            resizeGLObjectBuffers( unsigned int maxSize );

            virtual void
            releaseGLObjects( State* state = 0 ) const;

        protected:

            virtual ~ShaderAttribute();

            typedef std::vector<osg::ref_ptr<osg::Uniform>> Uniforms;

            Type                                            _type;
            Uniforms                                        _uniforms;
    };

}
