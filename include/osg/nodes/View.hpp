/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Base view class holding camera, scene graph root, and light.
 * Foundation for osgViewer::View with master/slave cameras.
 */
#pragma once

#include <mutex>
#include <osg/core/Inherit.hpp>
#include <osg/core/Stats.hpp>
#include <osg/lighting/Light.hpp>
#include <osg/nodes/Camera.hpp>

namespace osg
{

    /** View - maintains a master camera view and a list of slave cameras that are
     * relative to this master camera. Note, if no slave cameras are attached to the view
     * then the master camera does both the control and implementation of the rendering
     * of the scene, but if slave cameras are present then the master controls the view
     * onto the scene, while the slaves implement the rendering of the scene.
     */
    class OSG_EXPORT View : public virtual osg::Object
    {
        public:

            View();

            View( const osg::View&   view,
                  const osg::CopyOp& copyop = CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osg,
                               View )

            osg::Object*
            cloneType() const override
            {
                return new View();
            }

            osg::Object*
            clone( const osg::CopyOp& copyop ) const override
            {
                return new View( *this, copyop );
            }

            bool
            isSameKindAs( const osg::Object* obj ) const override
            {
                return dynamic_cast<const View*>( obj ) != nullptr;
            }

            bool
            is_compatible( const std::type_info& type ) const noexcept override
            {
                return typeid( View ) == type || osg::Object::is_compatible( type );
            }

            const std::type_info&
            type_info() const noexcept override
            {
                return typeid( View );
            }

            std::size_t
            sizeofObject() const noexcept override
            {
                return sizeof( View );
            }

            const char*
            libraryName() const override
            {
                return _s_libraryName();
            }

            const char*
            className() const override
            {
                return _s_className();
            }

            /** Take all the settings, Camera and Slaves from the passed in view, leaving
             * it empty. */
            virtual void
            take( View& rhs );

            /** Set the Stats object used to collect various frame related timing and
             * scene graph stats.*/
            void
            setStats( osg::Stats* stats )
            {
                _stats = stats;
            }

            /** Get the Viewers Stats object.*/
            osg::Stats*
            getStats()
            {
                return _stats.get();
            }

            /** Get the Viewers Stats object.*/
            const osg::Stats*
            getStats() const
            {
                return _stats.get();
            }

            /** Options for controlling the global lighting used for the view.*/
            enum LightingMode
            {
                NO_LIGHT,
                HEADLIGHT,
                SKY_LIGHT,
            };

            /** Set the global lighting to use for this view.
             * Defaults to headlight. */
            void
            setLightingMode( LightingMode lightingMode );

            /** Get the global lighting used for this view.*/
            LightingMode
            getLightingMode() const
            {
                return _lightingMode;
            }

            /** Get the global light.*/
            void
            setLight( osg::Light* light )
            {
                _light = light;
            }

            /** Get the global lighting if assigned.*/
            osg::Light*
            getLight()
            {
                return _light.get();
            }

            /** Get the const global lighting if assigned.*/
            const osg::Light*
            getLight() const
            {
                return _light.get();
            }

            /** Set the master camera of the view. */
            void
            setCamera( osg::Camera* camera );

            /** Get the master camera of the view. */
            osg::Camera*
            getCamera()
            {
                return _camera.get();
            }

            /** Get the const master camera of the view. */
            const osg::Camera*
            getCamera() const
            {
                return _camera.get();
            }

            /** Set the frame stamp of the view. */
            void
            setFrameStamp( osg::FrameStamp* fs )
            {
                _frameStamp = fs;
            }

            /** Get the frame stamp of the view. */
            osg::FrameStamp*
            getFrameStamp()
            {
                return _frameStamp.get();
            }

            /** Get the frame stamp of the view. */
            const osg::FrameStamp*
            getFrameStamp() const
            {
                return _frameStamp.get();
            }

            /** Slave allows one to up a camera that follows the master with a local
             * offset to the project and view matrices.*/
            struct OSG_EXPORT Slave
            {
                    Slave( bool useMastersSceneData = true ) :
                        _useMastersSceneData( useMastersSceneData )
                    {
                    }

                    Slave( osg::Camera*      camera,
                           const osg::dmat4& projectionOffset,
                           const osg::dmat4& viewOffset,
                           bool              useMastersSceneData = true ) :
                        _camera( camera ),
                        _projectionOffset( projectionOffset ),
                        _viewOffset( viewOffset ),
                        _useMastersSceneData( useMastersSceneData )
                    {
                    }

                    Slave( const Slave& rhs ) :
                        _camera( rhs._camera ),
                        _projectionOffset( rhs._projectionOffset ),
                        _viewOffset( rhs._viewOffset ),
                        _useMastersSceneData( rhs._useMastersSceneData ),
                        _updateSlaveCallback( rhs._updateSlaveCallback )
                    {
                    }

                    virtual ~Slave()
                    {
                    }

                    Slave&
                    operator=( const Slave& rhs )
                    {
                        _camera              = rhs._camera;
                        _projectionOffset    = rhs._projectionOffset;
                        _viewOffset          = rhs._viewOffset;
                        _useMastersSceneData = rhs._useMastersSceneData;
                        _updateSlaveCallback = rhs._updateSlaveCallback;
                        return *this;
                    }

                    struct UpdateSlaveCallback : public virtual Referenced
                    {
                            virtual void
                            updateSlave( osg::View&        view,
                                         osg::View::Slave& slave ) = 0;
                    };

                    void
                    updateSlave( View& view )
                    {
                        if( _updateSlaveCallback.valid() )
                        {
                            _updateSlaveCallback->updateSlave( view, *this );
                        }
                        else
                        {
                            updateSlaveImplementation( view );
                        }
                    }

                    virtual void
                                              updateSlaveImplementation( View& view );

                    osg::ref_ptr<osg::Camera> _camera;
                    osg::dmat4                _projectionOffset;
                    osg::dmat4                _viewOffset;
                    bool                      _useMastersSceneData;
                    osg::ref_ptr<UpdateSlaveCallback> _updateSlaveCallback;
            };

            bool
            addSlave( osg::Camera* camera,
                      bool         useMastersSceneData = true )
            {
                return addSlave( camera,
                                 osg::dmat4(),
                                 osg::dmat4(),
                                 useMastersSceneData );
            }

            bool
            addSlave( osg::Camera*      camera,
                      const osg::dmat4& projectionOffset,
                      const osg::dmat4& viewOffset,
                      bool              useMastersSceneData = true );

            bool
            removeSlave( unsigned int pos );

            unsigned int
            getNumSlaves() const
            {
                return static_cast<unsigned int>( _slaves.size() );
            }

            Slave&
            getSlave( unsigned int pos )
            {
                return _slaves[pos];
            }

            const Slave&
            getSlave( unsigned int pos ) const
            {
                return _slaves[pos];
            }

            unsigned int
            findSlaveIndexForCamera( osg::Camera* camera ) const;

            Slave*
            findSlaveForCamera( osg::Camera* camera );

            void
            updateSlaves();

            void
            resizeGLObjectBuffers( unsigned int maxSize ) override;
            void
            releaseGLObjects( osg::State* = 0 ) const override;

        protected:

            virtual ~View();

            virtual osg::GraphicsOperation*
            createRenderer( osg::Camera* )
            {
                return 0;
            }

            osg::ref_ptr<osg::Stats>      _stats;

            LightingMode                  _lightingMode;
            osg::ref_ptr<osg::Light>      _light;

            osg::ref_ptr<osg::Camera>     _camera;

            typedef std::vector<Slave>    Slaves;
            Slaves                        _slaves;

            osg::ref_ptr<osg::FrameStamp> _frameStamp;
    };

}
