#include "RtmpStreamer.hpp"

#include <iostream>

RtmpStreamer::RtmpStreamer(const gchar *file_path, const gchar *rtmp_address){
    gst_init(NULL, NULL);
    data = new CustomData;
    
    data->pipeline = gst_pipeline_new ("rtmp_streamer");
    if (data->pipeline == NULL){
        g_printerr ("Could not create 'pipeline'");
        return;
    }
    
    if (!file_path){
        build_test_src(rtmp_address,data);
    } else {
        build_file_src(file_path,rtmp_address,data);
    }
    
    bus = gst_element_get_bus(data->pipeline);
    
    gst_bus_add_signal_watch(bus);
    g_object_set(GST_BIN(data->pipeline), "message-forward", TRUE, NULL);
    g_signal_connect(G_OBJECT(bus), "message::eos", G_CALLBACK(eos_cb), data);
    gst_object_unref(bus);
}

void RtmpStreamer::build_file_src(const gchar *filepath, const gchar *out, CustomData *data){
    if (!data->pipeline){
        g_print("no pipeline");
        return;
    }
    
    data->file_src = gst_element_factory_make ("filesrc", "file_src");
    data->mp4_demux = gst_element_factory_make ("qtdemux", "demux");
    data->video_queue_in = gst_element_factory_make ("queue", "video_queue_in");
    data->video_parse_in = gst_element_factory_make ("h264parse", "video_parse_in");
    data->video_decoder = gst_element_factory_make ("avdec_h264", "video_decoder");
    data->time_overlay = gst_element_factory_make ("timeoverlay", "time_overlay");
    data->clock_overlay = gst_element_factory_make ("clockoverlay", "clock_overlay");
    data->video_encoder = gst_element_factory_make ("x264enc", "video_encoder");
    data->video_filter_in = gst_element_factory_make("capsfilter", "video_filter_in");
    data->video_queue_out = gst_element_factory_make ("queue", "video_queue_out");
    
    data->audio_queue_in = gst_element_factory_make ("queue", "audio_queue_in");
    data->audio_parse = gst_element_factory_make ("aacparse", "audio_parse_in");
    data->audio_decoder = gst_element_factory_make ("avdec_aac", "audio_decoder");
    data->audio_convert = gst_element_factory_make ("audioconvert", "audio_convert");
    data->audio_encoder = gst_element_factory_make ("avenc_aac", "audio_encoder");
    data->audio_queue_out = gst_element_factory_make ("queue", "audio_queue_out");

    data->flv_muxer = gst_element_factory_make ("flvmux", "mux");
    data->rtmp_sink = gst_element_factory_make ("rtmpsink", "rtmp_sink");
    
    if (!data->file_src ||
        !data->mp4_demux ||
        
        !data->video_queue_in ||
        !data->video_parse_in ||
        !data->video_decoder ||
        !data->time_overlay ||
        !data->clock_overlay ||
        !data->video_encoder ||
        !data->video_filter_in ||
        !data->video_queue_out ||
        
        !data->audio_queue_in ||
        !data->audio_parse ||
        !data->audio_decoder ||
        !data->audio_convert ||
        !data->audio_encoder ||
        !data->audio_queue_out ||
        
        !data->flv_muxer ||
        !data->rtmp_sink){
        g_print("some element wasn't build");
        return;
    }
    
    // setup
    g_object_set (G_OBJECT (data->file_src),
                  "location", filepath,NULL);
    g_object_set (G_OBJECT (data->video_encoder),
                  "bitrate", 3000,
                  "speed-preset", 1,
                  "tune", 4,
                  NULL);
    g_object_set (G_OBJECT (data->audio_encoder),
                  "bitrate", 128,
                  NULL);
    g_object_set (G_OBJECT (data->rtmp_sink), "location", out, NULL);
    g_object_set (G_OBJECT(data->time_overlay),
                  "halignment", 2,
                  NULL);
    g_object_set (G_OBJECT(data->flv_muxer),
                  "streamable", true,
                  NULL);
    
    GstCaps* filtercaps_in = gst_caps_from_string("video/x-raw,format=I420");
    g_object_set (G_OBJECT (data->video_filter_in), "caps", filtercaps_in, NULL);
    gst_caps_unref(filtercaps_in);
    
    gst_bin_add_many (GST_BIN (data->pipeline),
                      data->file_src,
                      data->mp4_demux ,
                      
                      data->video_queue_in,
                      data->video_parse_in,
                      data->video_decoder,
                      data->video_filter_in,
                      data->time_overlay,
                      data->clock_overlay,
                      data->video_encoder,
                      data->video_queue_out,
                      
                      data->audio_queue_in,
                      data->audio_parse,
                      data->audio_decoder,
                      data->audio_convert,
                      data->audio_encoder,
                      data->audio_queue_out,
                      
                      data->flv_muxer,
                      data->rtmp_sink,
                      NULL);
    
    if (!gst_element_link_many(data->file_src, data->mp4_demux, NULL)){
        g_print("src link failure");
        return;
    }
    
    if (!gst_element_link_many(data->video_queue_in,
                               data->video_parse_in,
                               data->video_decoder,
                               data->video_filter_in,
                               data->time_overlay,
                               data->clock_overlay,
                               data->video_encoder,
                               data->video_queue_out,
                               NULL)){
        g_print("video link failure");
        return;
    }
    
    if (!gst_element_link_many(data->audio_queue_in,
                               data->audio_parse,
                               data->audio_decoder,
                               data->audio_convert,
                               data->audio_encoder,
                               data->audio_queue_out,
                               NULL)){
        g_print("audio link failure");
        return;
    }
    
    if (!gst_element_link_many(data->flv_muxer,
                               data->rtmp_sink, NULL)){
        g_print("flv link failure");
        return;
    }
    if (!gst_element_link_many(data->video_queue_out, data->flv_muxer, NULL)){
        g_print("video to flv link failure");
        return;
    }
    
    if (!gst_element_link_many(data->audio_queue_out, data->flv_muxer, NULL)){
        g_print("audio to flv link failure");
        return;
    }
    // handler for pads of MP4
    g_signal_connect (data->mp4_demux, "pad-added", G_CALLBACK (pad_added_handler), data);
}

void RtmpStreamer::build_test_src(const gchar *out, CustomData *data){
    if (!data->pipeline){
        g_print("no pipeline");
        return;
    }
    
    data->video_src = gst_element_factory_make ("videotestsrc", "video_src");
    data->time_overlay = gst_element_factory_make ("timeoverlay", "time_overlay");
    data->clock_overlay = gst_element_factory_make ("clockoverlay", "clock_overlay");
    data->video_filter_in = gst_element_factory_make ("capsfilter", "video_filter_in");
    data->video_encoder = gst_element_factory_make ("x264enc", "video_encoder");
    data->video_queue_out = gst_element_factory_make ("queue", "video_queue_out");
    data->audio_src = gst_element_factory_make ("audiotestsrc", "audio_src");
    data->audio_encoder = gst_element_factory_make ("avenc_aac", "audio_encoder");
    data->audio_queue_out = gst_element_factory_make ("queue", "audio_queue_out");
    data->flv_muxer = gst_element_factory_make ("flvmux", "mux");
    data->rtmp_sink = gst_element_factory_make ("rtmpsink", "rtmp_sink");
    
    gst_bin_add_many (GST_BIN (data->pipeline),
                      data->video_src,
                      data->time_overlay,
                      data->clock_overlay,
                      data->video_filter_in,
                      data->video_encoder,
                      data->video_queue_out,
                      data->audio_src,
                      data->audio_encoder,
                      data->audio_queue_out,
                      data->flv_muxer,
                      data->rtmp_sink,NULL);
    
    g_object_set (G_OBJECT(data->video_src),
                  "do-timestamp",true,
                  "is-live", true,
                  NULL);
    g_object_set (G_OBJECT(data->audio_src),
                  "is-live", true,
                  "do-timestamp",true,
                  NULL);
    g_object_set (G_OBJECT(data->time_overlay),
                  "halignment", 2,
                  NULL);
    g_object_set (G_OBJECT (data->video_encoder),
                  "bitrate", 3000,
                  "speed-preset", 1,
                  "tune", 4,
                  NULL);
    g_object_set (G_OBJECT (data->audio_encoder),
                  "bitrate", 128,
                  NULL);
    g_object_set (G_OBJECT (data->rtmp_sink),
                  "location", out,
                  "async", true,
                  NULL);
    g_object_set (G_OBJECT(data->flv_muxer),
                  "streamable", true,
                  NULL);
    
    GstCaps* filtercaps_in = gst_caps_from_string("video/x-raw,format=I420,width=1920,height=1080");
    g_object_set (G_OBJECT (data->video_filter_in), "caps", filtercaps_in, NULL);
    gst_caps_unref(filtercaps_in);
    
    if (!data->video_src ||
        !data->video_filter_in ||
        !data->time_overlay ||
        !data->clock_overlay ||
        !data->video_encoder ||
        !data->video_queue_out ||
        !data->audio_src ||
        !data->audio_encoder ||
        !data->audio_queue_out ||
        !data->flv_muxer ||
        !data->rtmp_sink){
        g_print("create elements troubles");
        return;
    }
    
    if (!gst_element_link_many(data->flv_muxer,
                               data->rtmp_sink, NULL)){
        g_print("flv link failure");
        return;
    }
    
    if (!gst_element_link_many(data->video_src,
                               data->time_overlay,
                               data->clock_overlay,
                               data->video_filter_in,
                               data->video_encoder,
                               data->video_queue_out,
                               NULL)){
        g_print("video link failure");
        return;
    }
    
    if (!gst_element_link_many(data->audio_src,
                               data->audio_encoder,
                               data->audio_queue_out,NULL)){
        g_print("audio link failure");
        return;
    }
    
    gst_element_link(data->video_queue_out, data->flv_muxer);
    gst_element_link(data->audio_queue_out, data->flv_muxer);
}

RtmpStreamer::~RtmpStreamer(){
    if (bus)
        gst_object_unref(bus);
    if(data->pipeline){
        gst_element_set_state(data->pipeline,GST_STATE_NULL);
        gst_object_unref(data->pipeline);
    }
    if (loop)
        g_main_loop_unref (loop);
}

// HANDLERS
void RtmpStreamer::pad_added_handler (GstElement *src, GstPad *new_pad, CustomData *data) {
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

gboolean RtmpStreamer::eos_cb (GstBus * bus, GstMessage * message, CustomData *data)
{
    g_main_loop_quit (loop);
    return true;
}

// METHODS
void RtmpStreamer::Lounch(){
    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);
}

void RtmpStreamer::Play(){
    gst_element_set_state(data->pipeline, GST_STATE_PLAYING);
}

void RtmpStreamer::Pause(){
    gst_element_set_state(data->pipeline, GST_STATE_PAUSED);
}

void RtmpStreamer::Stop(){
    gst_element_send_event(data->pipeline, gst_event_new_eos());
    g_main_loop_quit (loop);
}
