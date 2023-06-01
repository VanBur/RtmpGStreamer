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
        if (!data->build_test_src(rtmp_address)){
            g_printerr ("Could not build pipline with test src");
            return;
        }
    } else {
        if (!data->build_file_src(file_path,rtmp_address)){
            g_printerr ("Could not build pipline with test src");
            return;
        }
    }
    
    bus = gst_element_get_bus(data->pipeline);
    
    gst_bus_add_signal_watch(bus);
    g_object_set(GST_BIN(data->pipeline), "message-forward", TRUE, NULL);
    g_signal_connect(G_OBJECT(bus), "message::eos", G_CALLBACK(data->eos_cb), data);
    gst_object_unref(bus);
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

void RtmpStreamer::Launch(){
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
