#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/state/Uniform.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osgAnimation/core/AnimationUpdateCallback.hpp>
#include <osgAnimation/core/Export.hpp>

namespace osgAnimation
{

    template<typename T>
    class UpdateUniform : public AnimationUpdateCallback<osg::UniformCallback>
    {
        protected:

            osg::ref_ptr<TemplateTarget<T>> _uniformTarget;

        public:

            UpdateUniform( const std::string& aName = "" ) :
                AnimationUpdateCallback<osg::UniformCallback>( aName )
            {
                _uniformTarget =
                    new TemplateTarget<T>();    // NOTE: initial value is undefined
            }

            UpdateUniform( const UpdateUniform& updateuniform,
                           const osg::CopyOp&   copyop ) :
                AnimationUpdateCallback<osg::UniformCallback>( updateuniform,
                                                               copyop )
            {
                _uniformTarget =
                    new TemplateTarget<T>( *( updateuniform._uniformTarget ) );
            }

            OSG_REGISTER_TYPE( osgAnimation,
                               UpdateUniform<T> )

            /** Callback method called by the NodeVisitor when visiting a node.*/
            virtual void
            operator()( osg::Uniform*     uniform,
                        osg::NodeVisitor* nv )
            {
                if( nv && nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR )
                {
                    update( *uniform );
                }

                traverse( uniform, nv );
            }

            bool
            link( Channel* channel )
            {
                if( channel->getName().find( "uniform" ) != std::string::npos )
                {
                    return channel->setTarget( _uniformTarget.get() );
                }
                else
                {
                    OSG_WARN << "Channel " << channel->getName()
                             << " does not contain a valid symbolic name for this class "
                             << className() << std::endl;
                }

                return false;
            }

            void
            update( osg::Uniform& uniform )
            {
                T value = _uniformTarget->getValue();
                uniform.set( value );
            }
    };

    // float
    struct UpdateFloatUniform : public UpdateUniform<float>
    {
            UpdateFloatUniform( const std::string& aName = "" ) :
                UpdateUniform<float>( aName )
            {
            }

            UpdateFloatUniform( const UpdateFloatUniform& ufu,
                                const osg::CopyOp&        copyop ) :
                osg::Object( ufu,
                             copyop ),    // copy name
                UpdateUniform<float>( ufu,
                                      copyop )
            {
            }

            OSG_REGISTER_TYPE( osgAnimation,
                               UpdateFloatUniform )
    };

    // vec2
    struct UpdateVec2fUniform : public UpdateUniform<osg::vec2>
    {
            UpdateVec2fUniform( const std::string& aName = "" ) :
                UpdateUniform<osg::vec2>( aName )
            {
            }

            UpdateVec2fUniform( const UpdateVec2fUniform& uv2fu,
                                const osg::CopyOp&        copyop ) :
                osg::Object( uv2fu,
                             copyop ),    // copy name
                UpdateUniform<osg::vec2>( uv2fu,
                                          copyop )
            {
            }

            OSG_REGISTER_TYPE( osgAnimation,
                               UpdateVec2fUniform )
    };

    // vec3
    struct UpdateVec3fUniform : public UpdateUniform<osg::vec3>
    {
            UpdateVec3fUniform( const std::string& aName = "" ) :
                UpdateUniform<osg::vec3>( aName )
            {
            }

            UpdateVec3fUniform( const UpdateVec3fUniform& uv3fu,
                                const osg::CopyOp&        copyop ) :
                osg::Object( uv3fu,
                             copyop ),    // copy name
                UpdateUniform<osg::vec3>( uv3fu,
                                          copyop )
            {
            }

            OSG_REGISTER_TYPE( osgAnimation,
                               UpdateVec3fUniform )
    };

    // vec4
    struct UpdateVec4fUniform : public UpdateUniform<osg::vec4>
    {
            UpdateVec4fUniform( const std::string& aName = "" ) :
                UpdateUniform<osg::vec4>( aName )
            {
            }

            UpdateVec4fUniform( const UpdateVec4fUniform& uv4fu,
                                const osg::CopyOp&        copyop ) :
                osg::Object( uv4fu,
                             copyop ),    // copy name
                UpdateUniform<osg::vec4>( uv4fu,
                                          copyop )
            {
            }

            OSG_REGISTER_TYPE( osgAnimation,
                               UpdateVec4fUniform )
    };

    // mat4
    struct UpdateMatrixfUniform : public UpdateUniform<osg::mat4>
    {
            UpdateMatrixfUniform( const std::string& aName = "" ) :
                UpdateUniform<osg::mat4>( aName )
            {
            }

            UpdateMatrixfUniform( const UpdateMatrixfUniform& umfu,
                                  const osg::CopyOp&          copyop ) :
                osg::Object( umfu,
                             copyop ),    // copy name
                UpdateUniform<osg::mat4>( umfu,
                                          copyop )
            {
            }

            OSG_REGISTER_TYPE( osgAnimation,
                               UpdateMatrixfUniform )
    };

}
