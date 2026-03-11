#ifndef _VIRTUAL_PROGRAM__
    #define _VIRTUAL_PROGRAM__ 1

    #include <map>
    #include <osg/core/Inherit.hpp>
    #include <osg/state/Program.hpp>
    #include <osg/state/Shader.hpp>
    #include <string>

////////////////////////////////////////////////////////////////////////////////
namespace osgCandidate
{

    ////////////////////////////////////////////////////////////////////////////////
    class VirtualProgram : public osg::Inherit<osg::Program, VirtualProgram>
    {
        public:

            OSG_REGISTER_TYPE( osgCandidate,
                               VirtualProgram )

            VirtualProgram( unsigned int mask = 0XFF'FF'FF'FFUL );

            virtual ~VirtualProgram( void );

            VirtualProgram( const VirtualProgram& VirtualProgram,
                            const osg::CopyOp&    copyop = osg::CopyOp::SHALLOW_COPY );

            Type
            getType() const override
            {
                return Type::PROGRAM;
            }

            /** return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs.*/
            virtual int
            compare( const StateAttribute& sa ) const
            {
                // check the types are equal and then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( VirtualProgram, sa )

                    // compare each parameter in turn against the rhs.
                    COMPARE_StateAttribute_Parameter( _mask )
                        COMPARE_StateAttribute_Parameter(
                            _shaderMap
                        ) return 0;    // passed all the above comparison macros, must be
                                       // equal.
            }

            /** If enabled, activate our program in the GL pipeline,
             * performing any rebuild operations that might be pending. */
            virtual void
            apply( osg::State& state ) const;

            osg::Shader*
            getShader( const std::string& shaderSemantic,
                       osg::Shader::Type  type );

            osg::Shader*
            setShader( const std::string& shaderSemantic,
                       osg::Shader*       shader );

        protected:

            typedef std::vector<osg::ref_ptr<osg::Shader>>              ShaderList;
            typedef std::pair<std::string, osg::Shader::Type>           ShaderSemantic;
            typedef std::map<ShaderSemantic, osg::ref_ptr<osg::Shader>> ShaderMap;
            typedef std::map<ShaderList, osg::ref_ptr<osg::Program>>    ProgramMap;

            mutable ProgramMap                                          _programMap;
            ShaderMap                                                   _shaderMap;
            unsigned int                                                _mask;
    };    // class VirtualProgram

    ////////////////////////////////////////////////////////////////////////////////

}    // namespace osgCandidate

////////////////////////////////////////////////////////////////////////////////
#endif
