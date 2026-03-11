/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Base class for uniform variables. Provides name, type identity,
 * and dirty-flag infrastructure shared by all Uniform specializations.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>
#include <osg/rendering/GLExtensions.hpp>
#include <string>

namespace osg
{

    /** BaseUniform base class for encapsulating glUniform values */
    class OSG_EXPORT UniformBase : public osg::Inherit<Object, UniformBase>
    {
        public:

            UniformBase();

            UniformBase( const std::string& name );

            UniformBase( const UniformBase& rhs,
                         const CopyOp&      copyop );

            OSG_REGISTER_TYPE( osg,
                               UniformBase )

            virtual const UniformBase*
            asUniformBase() const
            {
                return this;
            }

            virtual UniformBase*
            asUniformBase()
            {
                return this;
            }

            /** Set the name of the glUniform, ensuring it is only set once.*/
            virtual void
            setName( const std::string& name );

            /** Set the name of the glUniform appending [unit] such as osg_Light[0].*/
            void
            setName( const std::string& baseName,
                     unsigned int       unit );

            inline unsigned int
            getNameID() const noexcept
            {
                return _nameID;
            }

            /** return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            virtual int
            compare( const UniformBase& rhs ) const;
            virtual int
            compareData( const UniformBase& rhs ) const;

            bool
            operator<( const UniformBase& rhs ) const
            {
                return compare( rhs ) < 0;
            }

            bool
            operator==( const UniformBase& rhs ) const
            {
                return compare( rhs ) == 0;
            }

            bool
            operator!=( const UniformBase& rhs ) const
            {
                return compare( rhs ) != 0;
            }

            /** A vector of osg::StateSet pointers which is used to store the parent(s)
             * of this Uniform, the parents could be osg::Node or osg::Drawable.*/
            typedef std::vector<StateSet*> ParentList;

            /** Get the parent list of this Uniform. */
            inline const ParentList&
            getParents() const noexcept
            {
                return _parents;
            }

            /** Get the a copy of parent list of node. A copy is returned to
             * prevent modification of the parent list.*/
            inline ParentList
            getParents()
            {
                return _parents;
            }

            inline StateSet*
            getParent( unsigned int i ) noexcept
            {
                return _parents[i];
            }

            /**
             * Get a single const parent of this Uniform.
             * @param i index of the parent to get.
             * @return the parent i.
             */
            inline const StateSet*
            getParent( unsigned int i ) const noexcept
            {
                return _parents[i];
            }

            /**
             * Get the number of parents of this Uniform.
             * @return the number of parents of this Uniform.
             */
            inline unsigned int
            getNumParents() const noexcept
            {
                return static_cast<unsigned int>( _parents.size() );
            }

            /** Set the UpdateCallback which allows users to attach customize the
             * updating of an object during the update traversal.*/
            void
            setUpdateCallback( UniformCallback* uc );

            /** Get the non const UpdateCallback.*/
            UniformCallback*
            getUpdateCallback() noexcept
            {
                return _updateCallback.get();
            }

            /** Get the const UpdateCallback.*/
            const UniformCallback*
            getUpdateCallback() const noexcept
            {
                return _updateCallback.get();
            }

            /** Set the EventCallback which allows users to attach customize the updating
             * of an object during the Event traversal.*/
            void
            setEventCallback( UniformCallback* ec );

            /** Get the non const EventCallback.*/
            UniformCallback*
            getEventCallback() noexcept
            {
                return _eventCallback.get();
            }

            /** Get the const EventCallback.*/
            const UniformCallback*
            getEventCallback() const noexcept
            {
                return _eventCallback.get();
            }

            /** Increment the modified count on the Uniform so Programs watching it know
             * it update themselves. NOTE: automatically called during
             * osg::Uniform::set*(); you must call if modifying the internal data array
             * directly. */
            inline void
            dirty() noexcept
            {
                ++_modifiedCount;
            }

            inline void
            setModifiedCount( unsigned int mc ) noexcept
            {
                _modifiedCount = mc;
            }

            inline unsigned int
            getModifiedCount() const noexcept
            {
                return _modifiedCount;
            }

            virtual void
            apply( const GLExtensions* ext,
                   GLint               location ) const;

        protected:

            virtual ~UniformBase();

            void
            addParent( osg::StateSet* object );
            void
            removeParent( osg::StateSet* object );

            friend class osg::StateSet;

            ParentList               _parents;

            unsigned int             _nameID;
            unsigned int             _modifiedCount;

            ref_ptr<UniformCallback> _updateCallback;
            ref_ptr<UniformCallback> _eventCallback;
    };

}
