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

    gboolean build_test_src(const gchar *out){
        if (!pipeline){
            g_print("no pipeline");
            return false;
        }
        
        video_src = gst_element_factory_make ("videotestsrc", "video_src");
        time_overlay = gst_element_factory_make ("timeoverlay", "time_overlay");
        clock_overlay = gst_element_factory_make ("clockoverlay", "clock_overlay");
        video_filter_in = gst_element_factory_make ("capsfilter", "video_filter_in");
        video_encoder = gst_element_factory_make ("x264enc", "video_encoder");
        video_queue_out = gst_element_factory_make ("queue", "video_queue_out");
        audio_src = gst_element_factory_make ("audiotestsrc", "audio_src");
        audio_encoder = gst_element_factory_make ("avenc_aac", "audio_encoder");
        audio_queue_out = gst_element_factory_make ("queue", "audio_queue_out");
        flv_muxer = gst_element_factory_make ("flvmux", "mux");
        rtmp_sink = gst_element_factory_make ("rtmpsink", "rtmp_sink");
        
        gst_bin_add_many (GST_BIN (pipeline),
                          video_src,
                          time_overlay,
                          clock_overlay,
                          video_filter_in,
                          video_encoder,
                          video_queue_out,
                          audio_src,
                          audio_encoder,
                          audio_queue_out,
                          flv_muxer,
                          rtmp_sink,NULL);
        
        g_object_set (G_OBJECT(video_src),
                      "do-timestamp",true,
                      "is-live", true,
                      NULL);
        g_object_set (G_OBJECT(audio_src),
                      "is-live", true,
                      "do-timestamp",true,
                      NULL);
        g_object_set (G_OBJECT(time_overlay),
                      "halignment", 2,
                      NULL);
        g_object_set (G_OBJECT (video_encoder),
                      "bitrate", 3000,
                      "speed-preset", 1,
                      "tune", 4,
                      NULL);
        g_object_set (G_OBJECT (audio_encoder),
                      "bitrate", 128,
                      NULL);
        g_object_set (G_OBJECT (rtmp_sink),
                      "location", out,
                      "async", true,
                      NULL);
        g_object_set (G_OBJECT(flv_muxer),
                      "streamable", true,
                      NULL);
        
        GstCaps* filtercaps_in = gst_caps_from_string("video/x-raw,format=I420,width=1920,height=1080");
        g_object_set (G_OBJECT (video_filter_in), "caps", filtercaps_in, NULL);
        gst_caps_unref(filtercaps_in);
        
        if (!video_src ||
            !video_filter_in ||
            !time_overlay ||
            !clock_overlay ||
            !video_encoder ||
            !video_queue_out ||
            !audio_src ||
            !audio_encoder ||
            !audio_queue_out ||
            !flv_muxer ||
            !rtmp_sink){
            g_print("create elements troubles");
            return false;
        }
        
        if (!gst_element_link_many(flv_muxer,
                                   rtmp_sink, NULL)){
            g_print("flv link failure");
            return false;
        }
        
        if (!gst_element_link_many(video_src,
                                   time_overlay,
                                   clock_overlay,
                                   video_filter_in,
                                   video_encoder,
                                   video_queue_out,
                                   NULL)){
            g_print("video link failure");
            return false;
        }
        
        if (!gst_element_link_many(audio_src,
                                   audio_encoder,
                                   audio_queue_out,NULL)){
            g_print("audio link failure");
            return false;
        }
        
        gst_element_link(video_queue_out, flv_muxer);
        gst_element_link(audio_queue_out, flv_muxer);
        return true;
    }
    
    gboolean build_file_src(const gchar *filepath, const gchar *out){
        if (!pipeline){
            g_print("no pipeline");
            return false;
        }
        
        file_src = gst_element_factory_make ("filesrc", "file_src");
        mp4_demux = gst_element_factory_make ("qtdemux", "demux");
        video_queue_in = gst_element_factory_make ("queue", "video_queue_in");
        video_parse_in = gst_element_factory_make ("h264parse", "video_parse_in");
        video_decoder = gst_element_factory_make ("avdec_h264", "video_decoder");
        time_overlay = gst_element_factory_make ("timeoverlay", "time_overlay");
        clock_overlay = gst_element_factory_make ("clockoverlay", "clock_overlay");
        video_encoder = gst_element_factory_make ("x264enc", "video_encoder");
        video_filter_in = gst_element_factory_make("capsfilter", "video_filter_in");
        video_queue_out = gst_element_factory_make ("queue", "video_queue_out");
        
        audio_queue_in = gst_element_factory_make ("queue", "audio_queue_in");
        audio_parse = gst_element_factory_make ("aacparse", "audio_parse_in");
        audio_decoder = gst_element_factory_make ("avdec_aac", "audio_decoder");
        audio_convert = gst_element_factory_make ("audioconvert", "audio_convert");
        audio_encoder = gst_element_factory_make ("avenc_aac", "audio_encoder");
        audio_queue_out = gst_element_factory_make ("queue", "audio_queue_out");

        flv_muxer = gst_element_factory_make ("flvmux", "mux");
        rtmp_sink = gst_element_factory_make ("rtmpsink", "rtmp_sink");
        
        if (!file_src ||
            !mp4_demux ||
            
            !video_queue_in ||
            !video_parse_in ||
            !video_decoder ||
            !time_overlay ||
            !clock_overlay ||
            !video_encoder ||
            !video_filter_in ||
            !video_queue_out ||
            
            !audio_queue_in ||
            !audio_parse ||
            !audio_decoder ||
            !audio_convert ||
            !audio_encoder ||
            !audio_queue_out ||
            
            !flv_muxer ||
            !rtmp_sink){
            g_print("some element wasn't build");
            return false;
        }
        
        // setup
        g_object_set (G_OBJECT (file_src),
                      "location", filepath,NULL);
        g_object_set (G_OBJECT (video_encoder),
                      "bitrate", 3000,
                      "speed-preset", 1,
                      "tune", 4,
                      NULL);
        g_object_set (G_OBJECT (audio_encoder),
                      "bitrate", 128,
                      NULL);
        g_object_set (G_OBJECT (rtmp_sink), "location", out, NULL);
        g_object_set (G_OBJECT(time_overlay),
                      "halignment", 2,
                      NULL);
        g_object_set (G_OBJECT(flv_muxer),
                      "streamable", true,
                      NULL);
        
        GstCaps* filtercaps_in = gst_caps_from_string("video/x-raw,format=I420");
        g_object_set (G_OBJECT (video_filter_in), "caps", filtercaps_in, NULL);
        gst_caps_unref(filtercaps_in);
        
        gst_bin_add_many (GST_BIN (pipeline),
                          file_src,
                          mp4_demux ,
                          
                          video_queue_in,
                          video_parse_in,
                          video_decoder,
                          video_filter_in,
                          time_overlay,
                          clock_overlay,
                          video_encoder,
                          video_queue_out,
                          
                          audio_queue_in,
                          audio_parse,
                          audio_decoder,
                          audio_convert,
                          audio_encoder,
                          audio_queue_out,
                          
                          flv_muxer,
                          rtmp_sink,
                          NULL);
        
        if (!gst_element_link_many(file_src, mp4_demux, NULL)){
            g_print("src link failure");
            return false;
        }
        
        if (!gst_element_link_many(video_queue_in,
                                   video_parse_in,
                                   video_decoder,
                                   video_filter_in,
                                   time_overlay,
                                   clock_overlay,
                                   video_encoder,
                                   video_queue_out,
                                   NULL)){
            g_print("video link failure");
            return false;
        }
        
        if (!gst_element_link_many(audio_queue_in,
                                   audio_parse,
                                   audio_decoder,
                                   audio_convert,
                                   audio_encoder,
                                   audio_queue_out,
                                   NULL)){
            g_print("audio link failure");
            return false;
        }
        
        if (!gst_element_link_many(flv_muxer,
                                   rtmp_sink, NULL)){
            g_print("flv link failure");
            return false;
        }
        if (!gst_element_link_many(video_queue_out, flv_muxer, NULL)){
            g_print("video to flv link failure");
            return false;
        }
        
        if (!gst_element_link_many(audio_queue_out, flv_muxer, NULL)){
            g_print("audio to flv link failure");
            return false;
        }
        // handler for pads of MP4
        g_signal_connect (mp4_demux, "pad-added", G_CALLBACK (pad_added_handler), this);
        return true;
    }
    
    static void pad_added_handler(GstElement *src, GstPad *new_pad, _CustomData *data){
        GstPad *sink_pad_audio = gst_element_get_static_pad (data->audio_queue_in, "sink");
        GstPad *sink_pad_video = gst_element_get_static_pad (data->video_queue_in, "sink");

        GstPadLinkReturn ret;
        GstCaps *new_pad_caps = NULL;
        GstStructure *new_pad_struct = NULL;
        const gchar *new_pad_type = NULL;

        new_pad_caps = gst_pad_get_current_caps (new_pad);
        new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
        new_pad_type = gst_structure_get_name (new_pad_struct);

        if (g_str_has_prefix (new_pad_type,"audio/mpeg"))
        {
            ret = gst_pad_link (new_pad, sink_pad_audio);
        }
        else if (g_str_has_prefix (new_pad_type, "video/x-h264"))
        {
            ret = gst_pad_link (new_pad, sink_pad_video);
        }
        else {
            // ignore
            goto exit;
        }
    exit:
        if (new_pad_caps != NULL)
            gst_caps_unref (new_pad_caps);
        gst_object_unref (sink_pad_audio);
        gst_object_unref (sink_pad_video);
    }
    static gboolean eos_cb (GstBus * bus, GstMessage * message, _CustomData *data){
        g_print("EOS");
        g_main_loop_quit (loop);
        return true;
    }
    
} CustomData;

class RtmpStreamer{
private:
    CustomData *data = nullptr;
    GstBus *bus = nullptr;
public:
    RtmpStreamer(const gchar *file_path, const gchar *rtmp_address);
    ~RtmpStreamer();
    
    void Play();
    void Pause();
    void Stop();
    void Launch();
};

#endif /* RtmpStreamer_hpp */
