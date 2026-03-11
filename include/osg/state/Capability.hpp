/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Generic GL capability toggle attribute. Wraps glEnable/glDisable
 * for capabilities not covered by specialized state attributes.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/GL>
#include <osg/state/StateAttribute.hpp>

namespace osg
{

    class OSG_EXPORT Capability : public osg::Inherit<osg::StateAttribute, Capability>
    {
        public:

            Capability();

            Capability( GLenum capability ) :
                _capability( capability )
            {
            }

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            Capability( const Capability& cap,
                        const CopyOp&     copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( cap,
                         copyop ),
                _capability( cap._capability )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               Capability )

            /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            virtual int
            compare( const StateAttribute& sa ) const
            {
                // Check for equal types, then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( Capability, sa )

                    COMPARE_StateAttribute_Parameter( _capability );

                return 0;
            }

            /** Return the Type identifier of the attribute's class type.*/
            virtual Type
            getType() const
            {
                return Type::CAPABILITY + _capability;
            }

            void
            setCapability( GLenum capability )
            {
                _capability = capability;
            }

            GLenum
            getCapability() const
            {
                return _capability;
            }

        protected:

            virtual ~Capability();

            GLenum _capability;
    };

    /** Encapsulates glEnablei/glDisablei
     */
    class OSG_EXPORT Capabilityi : public osg::Inherit<osg::Capability, Capabilityi>
    {
        public:

            Capabilityi();

            Capabilityi( GLenum       capability,
                         unsigned int buf ) :
                Inherit( capability ),
                _index( buf )
            {
            }

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            Capabilityi( const Capabilityi& cap,
                         const CopyOp&      copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( cap,
                         copyop ),
                _index( cap._index )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               Capabilityi )

            /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            virtual int
            compare( const StateAttribute& sa ) const
            {
                // Check for equal types, then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( Capabilityi, sa )

                    COMPARE_StateAttribute_Parameter( _index );
                COMPARE_StateAttribute_Parameter( _capability );

                return 0;
            }

            /** Return the member identifier within the attribute's class type. Used for
             * light number/clip plane number etc.*/
            virtual unsigned int
            getMember() const
            {
                return _index;
            }

            /** Set the renderbuffer index of the Enablei. */
            void
            setIndex( unsigned int buf )
            {
                _index = buf;
            }

            /** Get the renderbuffer index of the Enablei. */
            unsigned int
            getIndex() const
            {
                return _index;
            }

        protected:

            virtual ~Capabilityi();

            unsigned int _index;
    };

    class OSG_EXPORT Enablei : public osg::Inherit<Capabilityi, Enablei>
    {
        public:

            Enablei()
            {
            }

            Enablei( unsigned int buf,
                     GLenum       capability ) :
                Inherit( buf,
                         capability )
            {
            }

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            Enablei( const Enablei& ei,
                     const CopyOp&  copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( ei,
                         copyop )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               Enablei )

            virtual void
            apply( State& ) const;

        protected:

            virtual ~Enablei()
            {
            }
    };

    class OSG_EXPORT Disablei : public osg::Inherit<Capabilityi, Disablei>
    {
        public:

            Disablei()
            {
            }

            Disablei( unsigned int buf,
                      GLenum       capability ) :
                Inherit( buf,
                         capability )
            {
            }

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            Disablei( const Disablei& ei,
                      const CopyOp&   copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( ei,
                         copyop )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               Disablei )

            virtual void
            apply( State& ) const;

        protected:

            virtual ~Disablei()
            {
            }
    };

}
