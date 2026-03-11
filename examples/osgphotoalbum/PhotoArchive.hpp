/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * PhotoArchive example application
 */
#ifndef PHOTOARCHIVE_H
#define PHOTOARCHIVE_H

#include <osg/images/Image.hpp>
#include <osg/maths/compat.hpp>

class PhotoArchive : public osg::Referenced
{
    public:

        static PhotoArchive*
        open( const std::string& filename )
        {
            osg::ref_ptr<PhotoArchive> archive = new PhotoArchive( filename );
            if( !archive->empty() )
            {
                return archive.release();
            }
            else
            {
                return 0;
            }
        }

        typedef std::vector<std::string> FileNameList;

        bool
        empty()
        {
            return _photoIndex.empty();
        }

        void
        getImageFileNameList( FileNameList& filenameList );

        static void
        buildArchive( const std::string&  filename,
                      const FileNameList& imageList,
                      unsigned int        thumbnailSize = 256,
                      unsigned int        maximumSize   = 1'024,
                      bool                compressed    = true );

        osg::ref_ptr<osg::Image>
        readImage( const std::string& filename,
                   unsigned int       target_s,
                   unsigned           target_t,
                   float&             original_s,
                   float&             original_t );

    protected:

        PhotoArchive( const std::string& filename );

        virtual ~PhotoArchive()
        {
        }

        bool
        readPhotoIndex( const std::string& filename );

        struct PhotoHeader
        {
                PhotoHeader() :
                    original_s( 0 ),
                    original_t( 0 ),
                    thumbnail_s( 0 ),
                    thumbnail_t( 0 ),
                    thumbnail_position( 0 ),
                    fullsize_s( 0 ),
                    fullsize_t( 0 ),
                    fullsize_position( 0 )
                {
                    filename[0] = '\0';
                }

                char         filename[256];
                unsigned int original_s;
                unsigned int original_t;

                unsigned int thumbnail_s;
                unsigned int thumbnail_t;
                unsigned int thumbnail_position;

                unsigned int fullsize_s;
                unsigned int fullsize_t;
                unsigned int fullsize_position;
        };

        struct ImageHeader
        {
                ImageHeader() :
                    s( 0 ),
                    t( 0 ),
                    internalTextureformat( 0 ),
                    pixelFormat( 0 ),
                    type( 0 ),
                    size( 0 )
                {
                }

                unsigned int s;
                unsigned int t;
                GLint        internalTextureformat;
                GLenum       pixelFormat;
                GLenum       type;
                unsigned int size;
        };

        typedef std::vector<PhotoHeader> PhotoIndexList;

        std::string                      _archiveFileName;
        PhotoIndexList                   _photoIndex;
};

#endif
