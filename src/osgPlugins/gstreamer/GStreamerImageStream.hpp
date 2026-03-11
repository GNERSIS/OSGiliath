#pragma once

#include <gst/app/gstappsink.h>
#include <gst/pbutils/pbutils.h>
#include <osg/core/Inherit.hpp>
#include <osg/textures/ImageStream.hpp>
#include <osg/threading/Thread.hpp>

namespace osgGStreamer
{

    class GStreamerImageStream
        : public osg::Inherit<osg::ImageStream, GStreamerImageStream>,
          public osg::Thread
    {
        public:

            OSG_REGISTER_TYPE( osgGStreamer,
                               GStreamerImageStream )

            GStreamerImageStream();
            GStreamerImageStream( const GStreamerImageStream& image,
                                  const osg::CopyOp&          copyop =
                                      osg::CopyOp::SHALLOW_COPY );

            bool
            open( const std::string& filename );

            virtual void
            play();
            virtual void
            pause();
            virtual void
            rewind();
            virtual void
            seek( double time );

        private:

            virtual ~GStreamerImageStream();

            virtual void
            run();

            static gboolean
            on_message( GstBus*               bus,
                        GstMessage*           message,
                        GStreamerImageStream* user_data );

            static GstFlowReturn
            on_new_sample( GstAppSink*           appsink,
                           GStreamerImageStream* user_data );
            static GstFlowReturn
                           on_new_preroll( GstAppSink*           appsink,
                                           GStreamerImageStream* user_data );

            GMainLoop*     _loop;
            GstElement*    _pipeline;

            unsigned char* _internal_buffer;

            int            _width;
            int            _height;
    };

}
