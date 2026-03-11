/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Leaf node containing Drawable objects for rendering.
 * Groups geometry, shapes, and text under a single scene graph node.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/geometry/Drawable.hpp>
#include <osg/nodes/Group.hpp>

namespace osg
{

    /** A \c Geode is a "geometry node", that is, a leaf node on the scene graph
     * that can have "renderable things" attached to it. In OSG, renderable things
     * are represented by objects from the \c Drawable class, so a \c Geode is a
     * \c Node whose purpose is grouping <tt>Drawable</tt>s.
     */
    class OSG_EXPORT Geode : public osg::Inherit<Group, Geode>
    {
        public:

            OSG_REGISTER_TYPE( osg,
                               Geode )

            Geode();

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            Geode( const Geode&,
                   const CopyOp& copyop = CopyOp::SHALLOW_COPY );

            virtual Geode*
            asGeode()
            {
                return this;
            }

            virtual const Geode*
            asGeode() const
            {
                return this;
            }

            /** Add a \c Drawable to the \c Geode.
             * If \c drawable is not \c NULL and is not contained in the \c Geode
             * then increment its reference count, add it to the drawables list and
             * dirty the bounding sphere to force it to be recomputed on the next
             * call to \c getBound().
             * @param drawable The \c Drawable to be added to the \c Geode.
             * @return  \c true for success; \c false otherwise.
             */
            virtual bool
            addDrawable( Drawable* drawable );

            template<class T>
            bool
            addDrawable( const ref_ptr<T>& drawable )
            {
                return addDrawable( drawable.get() );
            }

            /** Remove a \c Drawable from the \c Geode.
             * Equivalent to <tt>removeDrawable(getDrawableIndex(drawable)</tt>.
             * @param drawable The drawable to be removed.
             * @return \c true if at least one \c Drawable was removed. \c false
             *         otherwise.
             */
            virtual bool
            removeDrawable( Drawable* drawable );

            template<class T>
            bool
            removeDrawable( const ref_ptr<T>& drawable )
            {
                return removeDrawable( drawable.get() );
            }

            /** Remove <tt>Drawable</tt>(s) from the specified position in
             * <tt>Geode</tt>'s drawable list.
             * @param i The index of the first \c Drawable to remove.
             * @param numDrawablesToRemove The number of <tt>Drawable</tt> to
             *        remove.
             * @return \c true if at least one \c Drawable was removed. \c false
             *         otherwise.
             */
            virtual bool
            removeDrawables( unsigned int i,
                             unsigned int numDrawablesToRemove = 1 );

            /** Replace specified Drawable with another Drawable.
             * Equivalent to <tt>setDrawable(getDrawableIndex(origDraw),newDraw)</tt>,
             * see docs for \c setDrawable() for further details on implementation.
             */
            virtual bool
            replaceDrawable( Drawable* origDraw,
                             Drawable* newDraw );

            template<class T,
                     class R>
            bool
            replaceDrawable( const ref_ptr<T>& origDraw,
                             const ref_ptr<R>& newDraw )
            {
                return replaceDrawable( origDraw.get(), newDraw.get() );
            }

            /** Set \c Drawable at position \c i.
             * Decrement the reference count origGSet and increments the
             * reference count of newGset, and dirty the bounding sphere
             * to force it to recompute on next getBound() and returns true.
             * If origDrawable is not found then return false and do not
             * add newGset.  If newGset is NULL then return false and do
             * not remove origGset.
             * @return \c true if set correctly, \c false on failure
             *         (if node==NULL || i is out of range).
             */
            virtual bool
            setDrawable( unsigned int i,
                         Drawable*    drawable );

            template<class T>
            bool
            setDrawable( unsigned int      i,
                         const ref_ptr<T>& drawable )
            {
                return setDrawable( i, drawable.get() );
            }

            /** Return the number of <tt>Drawable</tt>s currently attached to the
             * \c Geode.
             */
            inline unsigned int
            getNumDrawables() const
            {
                return getNumChildren();
            }

            /** Return the \c Drawable at position \c i.*/
            inline Drawable*
            getDrawable( unsigned int i )
            {
                return _children[i].valid() ? _children[i]->asDrawable() : 0;
            }

            /** Return the \c Drawable at position \c i.*/
            inline const Drawable*
            getDrawable( unsigned int i ) const
            {
                return _children[i].valid() ? _children[i]->asDrawable() : 0;
            }

            /** Return \c true if a given \c Drawable is contained within \c Geode.*/
            inline bool
            containsDrawable( const Drawable* drawable ) const
            {
                for( NodeList::const_iterator itr = _children.begin();
                     itr != _children.end();
                     ++itr )
                {
                    if( itr->get() == drawable )
                    {
                        return true;
                    }
                }
                return false;
            }

            template<class T>
            bool
            containsDrawable( const ref_ptr<T>& drawable ) const
            {
                return containsDrawable( drawable.get() );
            }

            /** Get the index number of \c drawable.
             * @return A value between 0 and <tt>getNumDrawables()-1</tt> if
             *         \c drawable is found; if not found, then
             *         <tt>getNumDrawables()</tt> is returned.
             */
            inline unsigned int
            getDrawableIndex( const Drawable* drawable ) const
            {
                return getChildIndex( drawable );
            }

            template<class T>
            unsigned int
            getDrawableIndex( const ref_ptr<T>& drawable ) const
            {
                return getDrawableIndex( drawable.get() );
            }

            /** Compile OpenGL Display List for each drawable.*/
            void
            compileDrawables( RenderInfo& renderInfo );

            /** Return the Geode's bounding box, which is the union of all the
             * bounding boxes of the geode's drawables.*/
            inline const box&
            getBoundingBox() const
            {
                if( !_boundingSphereComputed )
                {
                    getBound();
                }
                return _bbox;
            }

            virtual sphere
            computeBound() const;

        protected:

            virtual ~Geode();

            mutable osg::box _bbox;
    };

}
