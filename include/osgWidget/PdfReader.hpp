/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * PDF document viewer widget. Renders PDF pages as
 * textures within the widget system.
 */
#pragma once

#include <osg/images/Image.hpp>
#include <osg/nodes/Geode.hpp>
#include <osgWidget/Export.hpp>

namespace osgWidget
{

    /** Hints structure that can be passed to PdfReader and VncClient classes to help
     * guide them on what geometry to build.*/
    struct GeometryHints
    {
            enum AspectRatioPolicy
            {
                RESIZE_HEIGHT_TO_MAINTAINCE_ASPECT_RATIO,
                RESIZE_WIDTH_TO_MAINTAINCE_ASPECT_RATIO,
                IGNORE_DOCUMENT_ASPECT_RATIO
            };

            GeometryHints() :
                position( 0.0F,
                          0.0F,
                          0.0F ),
                widthVec( 1.0F,
                          0.0F,
                          0.0F ),
                heightVec( 0.0F,
                           1.0F,
                           0.0F ),
                backgroundColor( 1.0F,
                                 1.0F,
                                 1.0F,
                                 1.0F ),
                aspectRatioPolicy( RESIZE_HEIGHT_TO_MAINTAINCE_ASPECT_RATIO ),
                widthResolution( 1'024 ),
                heightResolution( 1'024 )
            {
            }

            GeometryHints( const osg::vec3&  pos,
                           const osg::vec3&  wVec,
                           const osg::vec3&  hVec,
                           const osg::vec4&  bColor,
                           AspectRatioPolicy asp =
                               RESIZE_HEIGHT_TO_MAINTAINCE_ASPECT_RATIO,
                           unsigned int wRes = 1'024,
                           unsigned int hRes = 1'024 ) :
                position( pos ),
                widthVec( wVec ),
                heightVec( hVec ),
                backgroundColor( bColor ),
                aspectRatioPolicy( asp ),
                widthResolution( wRes ),
                heightResolution( hRes )
            {
            }

            osg::vec3         position;
            osg::vec3         widthVec;
            osg::vec3         heightVec;

            osg::vec4         backgroundColor;

            AspectRatioPolicy aspectRatioPolicy;

            unsigned int      widthResolution;
            unsigned int      heightResolution;
    };

    /** Pure virtual base class for interfacing with implementation of PDF reader.*/
    class PdfImage : public osg::Image
    {
        public:

            PdfImage() :
                _backgroundColor( 1.0F,
                                  1.0F,
                                  1.0F,
                                  1.0F ),
                _pageNum( 0 ),
                _nextPageKeyEvent( 'n' ),
                _previousPageKeyEvent( 'p' )
            {
            }

            void
            setBackgroundColor( const osg::vec4& backgroundColor )
            {
                _backgroundColor = backgroundColor;
            }

            const osg::vec4&
            getBackgroundColor() const
            {
                return _backgroundColor;
            }

            int
            getPageNum() const
            {
                return _pageNum;
            }

            virtual int
            getNumOfPages() = 0;

            virtual bool
            page( int pageNum ) = 0;

            bool
            previous()
            {
                return page( _pageNum - 1 );
            }

            bool
            next()
            {
                return page( _pageNum + 1 );
            }

            void
            setNextPageKeyEvent( int key )
            {
                _nextPageKeyEvent = key;
            }

            int
            getNextPageKeyEvent() const
            {
                return _nextPageKeyEvent;
            }

            void
            setPreviousPageKeyEvent( int key )
            {
                _previousPageKeyEvent = key;
            }

            int
            getPreviousPageKeyEvent() const
            {
                return _previousPageKeyEvent;
            }

        protected:

            virtual ~PdfImage()
            {
            }

            osg::vec4 _backgroundColor;

            int       _pageNum;
            int       _nextPageKeyEvent;
            int       _previousPageKeyEvent;
    };

    /** Convenience class that provides a interactive quad that can be placed directly in
     * the scene.*/
    class OSGWIDGET_EXPORT PdfReader : public osg::Geode
    {
        public:

            PdfReader()
            {
            }

            PdfReader( const std::string&   filename,
                       const GeometryHints& hints = GeometryHints() );

            bool
            assign( PdfImage*            pdfImage,
                    const GeometryHints& hints = GeometryHints() );

            bool
            open( const std::string&   filename,
                  const GeometryHints& hints = GeometryHints() );

            bool
            page( int pageNum );

            bool
            previous();

            bool
            next();

        protected:

            osg::ref_ptr<PdfImage> _pdfImage;
    };

}
