/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Node whose children are loaded on demand from external files
 * via the DatabasePager. Used for deferred scene loading.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/nodes/Group.hpp>

namespace osg
{

    /** ProxyNode.
     */
    class OSG_EXPORT ProxyNode : public osg::Inherit<Group, ProxyNode>
    {
        public:

            OSG_REGISTER_TYPE( osg,
                               ProxyNode )

            ProxyNode();

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            ProxyNode( const ProxyNode&,
                       const CopyOp& copyop = CopyOp::SHALLOW_COPY );

            typedef osg::sphere::center_type vec_type;
            typedef osg::sphere::value_type  value_type;

            virtual void
            traverse( NodeVisitor& nv ) override;
            virtual void
            traverse( ConstNodeVisitor& nv ) const override;

            using osg::Group::addChild;

            virtual bool
            addChild( Node* child ) override;

            virtual bool
            addChild( Node*              child,
                      const std::string& filename );

            template<class T>
            bool
            addChild( const ref_ptr<T>&  child,
                      const std::string& filename )
            {
                return addChild( child.get(), filename );
            }

            virtual bool
            removeChildren( unsigned int pos,
                            unsigned int numChildrenToRemove ) override;

            /** Set the optional database osgDB::Options object to use when loading
             * children.*/
            void
            setDatabaseOptions( osg::Referenced* options )
            {
                _databaseOptions = options;
            }

            /** Get the optional database osgDB::Options object used when loading
             * children.*/
            osg::Referenced*
            getDatabaseOptions()
            {
                return _databaseOptions.get();
            }

            /** Get the optional database osgDB::Options object used when loading
             * children.*/
            const osg::Referenced*
            getDatabaseOptions() const
            {
                return _databaseOptions.get();
            }

            /** Set the database path to prepend to children's filenames.*/
            void
            setDatabasePath( const std::string& path );

            /** Get the database path used to prepend to children's filenames.*/
            inline const std::string&
            getDatabasePath() const
            {
                return _databasePath;
            }

            void
            setFileName( unsigned int       childNo,
                         const std::string& filename )
            {
                expandFileNameListTo( childNo );
                _filenameList[childNo].first = filename;
            }

            const std::string&
            getFileName( unsigned int childNo ) const
            {
                return _filenameList[childNo].first;
            }

            unsigned int
            getNumFileNames() const
            {
                return static_cast<unsigned int>( _filenameList.size() );
            }

            /** Return the DatabaseRequest object used by the DatabasePager to keep track
             * of file load requests being carried out on behalf of the DatabasePager.
             * Note, in normal OSG usage you should not set this value yourself, as this
             * will be managed by the osgDB::DatabasePager.*/
            osg::ref_ptr<osg::Referenced>&
            getDatabaseRequest( unsigned int childNo )
            {
                return _filenameList[childNo].second;
            }

            /** Return the const DatabaseRequest object.*/
            const osg::ref_ptr<osg::Referenced>&
            getDatabaseRequest( unsigned int childNo ) const
            {
                return _filenameList[childNo].second;
            }

            /** Modes which control how the center of object should be determined when
             * computing which child is active.*/
            enum CenterMode
            {
                USE_BOUNDING_SPHERE_CENTER,
                USER_DEFINED_CENTER,
                UNION_OF_BOUNDING_SPHERE_AND_USER_DEFINED,
            };

            /** Set how the center of object should be determined when computing which
             * child is active.*/
            void
            setCenterMode( CenterMode mode )
            {
                _centerMode = mode;
            }

            /** Get how the center of object should be determined when computing which
             * child is active.*/
            CenterMode
            getCenterMode() const
            {
                return _centerMode;
            }

            /** Modes which control how the proxynode external reference are loaded.*/
            enum LoadingExternalReferenceMode
            {
                LOAD_IMMEDIATELY,
                DEFER_LOADING_TO_DATABASE_PAGER,
                NO_AUTOMATIC_LOADING,
            };

            /** Set how the child loading is done.*/
            void
            setLoadingExternalReferenceMode( LoadingExternalReferenceMode mode )
            {
                _loadingExtReference = mode;
            }

            /** Get the loading mode.*/
            LoadingExternalReferenceMode
            getLoadingExternalReferenceMode() const
            {
                return _loadingExtReference;
            }

            /** Sets the object-space point which defines the center of the
               osg::ProxyNode. Center is affected by any transforms in the hierarchy
               above the osg::ProxyNode.*/
            inline void
            setCenter( const vec_type& center )
            {
                if( _centerMode != UNION_OF_BOUNDING_SPHERE_AND_USER_DEFINED )
                {
                    _centerMode = USER_DEFINED_CENTER;
                }
                _userDefinedCenter = center;
            }

            /** Return the ProxyNode center point. */
            inline const vec_type&
            getCenter() const
            {
                if( ( _centerMode == USER_DEFINED_CENTER ) ||
                    ( _centerMode == UNION_OF_BOUNDING_SPHERE_AND_USER_DEFINED ) )
                {
                    return _userDefinedCenter;
                }
                else
                {
                    return getBound().center;
                }
            }

            /** Set the object-space reference radius of the volume enclosed by the
             * ProxyNode. Used to determine the bounding sphere of the ProxyNode in the
             * absence of any children.*/
            inline void
            setRadius( value_type radius )
            {
                _radius = radius;
            }

            /** Get the object-space radius of the volume enclosed by the ProxyNode.*/
            inline value_type
            getRadius() const
            {
                return _radius;
            }

            virtual sphere
            computeBound() const override;

        protected:

            virtual ~ProxyNode()
            {
            }

            void
            expandFileNameListTo( unsigned int pos );

            typedef std::pair<std::string, osg::ref_ptr<osg::Referenced>>
                                                             FileNameDatabaseRequestPair;
            typedef std::vector<FileNameDatabaseRequestPair> FileNameDatabaseRequestList;

            FileNameDatabaseRequestList                      _filenameList;
            ref_ptr<Referenced>                              _databaseOptions;
            std::string                                      _databasePath;

            LoadingExternalReferenceMode                     _loadingExtReference;

            CenterMode                                       _centerMode;
            vec_type                                         _userDefinedCenter;
            value_type                                       _radius;
    };

}
