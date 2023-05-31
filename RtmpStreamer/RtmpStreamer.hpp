#ifndef RtmpStreamer_hpp
#define RtmpStreamer_hpp

#include <stdio.h>
#include <gst/gst.h>

static GMainLoop *loop;

typedef struct _CustomData {
    GstElement *pipeline;
    GstElement *audio_src;
    GstElement *video_src;
    GstElement *time_overlay;
    GstElement *clock_overlay;
    GstElement *file_src;
    GstElement *mp4_demux;
    GstElement *video_queue_in;
    GstElement *video_decoder;
    GstElement *video_encoder;
    GstElement *video_filter_in;
    GstElement *video_parse_in;
    GstElement *video_queue_out;
    GstElement *audio_queue_in;
    GstElement *audio_decoder;
    GstElement *audio_convert;
    GstElement *audio_encoder;
    GstElement *audio_parse;
    GstElement *audio_queue_out;
    GstElement *flv_muxer;
    GstElement *rtmp_sink;
} CustomData;

class RtmpStreamer{
private:
    CustomData *data = nullptr;
    GstBus *bus = nullptr;
    //handlers
    static void pad_added_handler(GstElement *src, GstPad *new_pad, CustomData *data);
    static gboolean eos_cb (GstBus * bus, GstMessage * message, CustomData *data);
    
    static void build_test_src(const gchar *out, CustomData *data);
    static void build_file_src(const gchar *filepath, const gchar *out, CustomData *data);
public:
    RtmpStreamer(const gchar *file_path, const gchar *rtmp_address);
    ~RtmpStreamer();
    
    void Play();
    void Pause();
    void Stop();
    void Lounch();
};

#endif /* RtmpStreamer_hpp */
