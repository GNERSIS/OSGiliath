/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * ImageReaderWriter example application
 */
#ifndef IMAGEREADERWRITER_H
#define IMAGEREADERWRITER_H

#include "PhotoArchive.hpp"

#include <mutex>
#include <osgDB/io/ImageOptions.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/io/WriteFile.hpp>

#define SERIALIZER() std::lock_guard<std::recursive_mutex> lock( _serializerMutex )

class ImageReaderWriter : public osgDB::ReaderWriter
{
    public:

        ImageReaderWriter();

        virtual const char*
        className() const
        {
            return "ImageReader";
        }

        void
        addPhotoArchive( PhotoArchive* archive )
        {
            _photoArchiveList.push_back( archive );
        }

        std::string
        insertReference( const std::string& fileName,
                         unsigned int       res,
                         float              width,
                         float              height,
                         bool               backPage )
        {
            SERIALIZER();
            return const_cast<ImageReaderWriter*>( this )
                ->local_insertReference( fileName, res, width, height, backPage );
        }

        virtual ReadResult
        readNode( const std::string& fileName,
                  const Options*     options ) const
        {
            SERIALIZER();
            return const_cast<ImageReaderWriter*>( this )->local_readNode( fileName,
                                                                           options );
        }

    protected:

        std::string
        local_insertReference( const std::string& fileName,
                               unsigned int       res,
                               float              width,
                               float              height,
                               bool               backPage );

        ReadResult
                                     local_readNode( const std::string& fileName,
                                                     const Options* );

        mutable std::recursive_mutex _serializerMutex;

        struct DataReference
        {
                DataReference();
                DataReference( const std::string& fileName,
                               unsigned int       res,
                               float              width,
                               float              height,
                               bool               backPage );
                DataReference( const DataReference& rhs );
                DataReference&
                             operator=( const DataReference& ) = default;

                std::string  _fileName;
                unsigned int _resolutionX;
                unsigned int _resolutionY;
                osg::vec3    _center;
                osg::vec3    _maximumWidth;
                osg::vec3    _maximumHeight;
                unsigned int _numPointsAcross;
                unsigned int _numPointsUp;
                bool         _backPage;
        };

        osg::ref_ptr<osg::Image>
        readImage_Archive( DataReference& dr,
                           float&         s,
                           float&         t );

        osg::ref_ptr<osg::Image>
        readImage_DynamicSampling( DataReference& dr,
                                   float&         s,
                                   float&         t );

        typedef std::map<std::string, DataReference>    DataReferenceMap;
        typedef std::vector<osg::ref_ptr<PhotoArchive>> PhotoArchiveList;

        DataReferenceMap                                _dataReferences;
        PhotoArchiveList                                _photoArchiveList;
};

#endif
