/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Base class for all OSG objects. Provides reference counting,
 * cloning, naming, user data, and data variance tracking.
 */
#include <osg/core/Object.hpp>

#include <osg/core/UserDataContainer.hpp>

namespace osg
{

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Object
    //
    Object::Object( const Object& obj,
                    const CopyOp& copyop ) :
        Referenced(),
        _name( obj._name ),
        _dataVariance( obj._dataVariance ),
        _userDataContainer( 0 )
    {
        if( obj._userDataContainer )
        {
            if( copyop.getCopyFlags() & osg::CopyOp::DEEP_COPY_USERDATA )
            {
                setUserDataContainer( osg::clone( obj._userDataContainer, copyop ) );
            }
            else
            {
                setUserDataContainer( obj._userDataContainer );
            }
        }
    }

    Object::~Object()
    {
        if( _userDataContainer )
        {
            _userDataContainer->unref();
        }
    }

    void
    Object::setThreadSafeRefUnref( bool threadSafe )
    {
        Referenced::setThreadSafeRefUnref( threadSafe );
        if( _userDataContainer )
        {
            _userDataContainer->setThreadSafeRefUnref( threadSafe );
        }
    }

    void
    Object::setUserDataContainer( osg::UserDataContainer* udc )
    {
        if( _userDataContainer == udc )
        {
            return;
        }

        if( _userDataContainer )
        {
            _userDataContainer->unref();
        }

        _userDataContainer = udc;

        if( _userDataContainer )
        {
            _userDataContainer->ref();
        }
    }

    osg::UserDataContainer*
    Object::getOrCreateUserDataContainer()
    {
        if( !_userDataContainer )
        {
            setUserDataContainer( new DefaultUserDataContainer() );
        }
        return _userDataContainer;
    }

    void
    Object::setUserData( Referenced* obj )
    {
        if( getUserData() == obj )
        {
            return;
        }

        getOrCreateUserDataContainer()->setUserData( obj );
    }

    Referenced*
    Object::getUserData()
    {
        return _userDataContainer ? _userDataContainer->getUserData() : 0;
    }

    const Referenced*
    Object::getUserData() const
    {
        return _userDataContainer ? _userDataContainer->getUserData() : 0;
    }

}    // end of namespace osg
