#ifndef RtmpStreamer_hpp
#define RtmpStreamer_hpp

#include <gst/gst.h>

static GMainLoop *loop;

typedef struct _CustomData {
    GstElement *pipeline;
    
    GstElement *audio_src;
    GstElement *video_src;
    GstElement *clock_overlay;
    GstElement *text_overlay;
    GstElement *file_src;
    GstElement *mp4_demux;
    GstElement *video_queue_in;
    GstElement *video_decoder;
    GstElement *video_encoder;
    GstElement *video_filter_in;
    GstElement *video_identity;
    GstElement *video_parse_in;
    GstElement *video_queue_out;
    GstElement *audio_queue_in;
    GstElement *audio_decoder;
    GstElement *audio_resample;
    GstElement *audio_convert;
    GstElement *audio_encoder;
    GstElement *audio_parse;
    GstElement *audio_queue_out;
    GstElement *flv_muxer;
    GstElement *rtmp_sink;
    // SRC
    gboolean add_test_src(){
        if (!pipeline){
            g_print("no pipeline");
            return false;
        }
        
        video_src = gst_element_factory_make ("videotestsrc", "video_src");
        audio_src = gst_element_factory_make ("audiotestsrc", "audio_src");
        if (!video_src || !audio_src ){
            g_print("create src troubles");
            return false;
        }
        
        gst_bin_add_many (GST_BIN (pipeline), video_src, audio_src, NULL);
        
        g_object_set (G_OBJECT(video_src),
                      "do-timestamp",true,
                      "is-live", true, NULL);
        g_object_set (G_OBJECT(audio_src),
                      "is-live", true,
                      "do-timestamp",true, NULL);
        
        return true;
    }
    gboolean add_file_src_with_mp4_demux(const gchar *in){
        if (!pipeline){
            g_print("no pipeline");
            return false;
        }
        
        file_src = gst_element_factory_make ("filesrc", "file_src");
        mp4_demux = gst_element_factory_make ("qtdemux", "demux");
        if (!file_src || !mp4_demux){
            g_print("create elements troubles");
            return false;
        }
        
        g_object_set (G_OBJECT (file_src),
                      "location", in, NULL);
        
        gst_bin_add_many (GST_BIN (pipeline),
                          file_src,
                          mp4_demux, NULL);
        
        if (!gst_element_link_many(file_src, mp4_demux, NULL)){
            g_print("filesrc link failure");
            return false;
        }
        
        g_signal_connect (mp4_demux, "pad-added", G_CALLBACK (pad_added_handler), this);
        
        return true;
    }
    
    // DECODERS
    gboolean add_video_decoder(){
        if (!pipeline){
            g_print("no pipeline");
            return false;
        }
        
        video_queue_in = gst_element_factory_make ("queue", "video_queue_in");
        video_parse_in = gst_element_factory_make ("h264parse", "video_parse_in");
        video_decoder = gst_element_factory_make ("avdec_h264", "video_decoder");
        
        if (!video_queue_in || !video_parse_in || !video_decoder){
            g_print("create video decoder troubles");
            return false;
        }
        
        gst_bin_add_many (GST_BIN (pipeline),
                          video_queue_in,
                          video_parse_in,
                          video_decoder, NULL);
        
        if (!gst_element_link_many(video_queue_in, video_parse_in, video_decoder ,NULL)){
            g_print("filesrc link failure");
            return false;
        }
        
        return true;
    }
    
    gboolean add_audio_decoder(){
        if (!pipeline){
            g_print("no pipeline");
            return false;
        }
        
        audio_queue_in = gst_element_factory_make ("queue", "audio_queue_in");
        audio_parse = gst_element_factory_make ("aacparse", "audio_parse_in");
        audio_decoder = gst_element_factory_make ("avdec_aac", "audio_decoder");
        audio_convert = gst_element_factory_make ("audioconvert", "audio_convert");
        audio_resample = gst_element_factory_make ("audioresample", "audio_resample");
        
        if (!audio_queue_in || !audio_parse || !audio_decoder || !audio_convert || !audio_resample){
            g_print("create audio decoder troubles");
            return false;
        }
        
        gst_bin_add_many (GST_BIN (pipeline),
                          audio_queue_in,
                          audio_parse,
                          audio_decoder,
                          audio_convert,
                          audio_resample,
                          NULL);
        
        if (!gst_element_link_many(audio_queue_in, audio_parse, audio_decoder, audio_convert, audio_resample, NULL)){
            g_print("link failure");
            return false;
        }
        
        return true;
    }
    // ENCODERS
    gboolean add_video_encoder_with_text_labels(){
        if (!pipeline){
            g_print("no pipeline");
            return false;
        }
        
        clock_overlay = gst_element_factory_make ("clockoverlay", "clock_overlay");
        text_overlay = gst_element_factory_make ("textoverlay", "text_overlay");
        video_filter_in = gst_element_factory_make ("capsfilter", "video_filter_in");
        video_encoder = gst_element_factory_make ("x264enc", "video_encoder");
        video_queue_out = gst_element_factory_make ("queue", "video_queue_out");
        video_identity = gst_element_factory_make ("identity", "video_identity");
        
        if (!clock_overlay || !text_overlay ||
            !video_filter_in || !video_encoder ||
            !video_queue_out || !video_identity ){
            g_print("create video encoder troubles");
            return false;
        }
        
        g_object_set (G_OBJECT (video_encoder),
                      "bitrate", 3000,
                      "speed-preset", 1,
                      "tune", 4, NULL);
        g_object_set (G_OBJECT(clock_overlay),
                      "halignment", 2, NULL);
        
        GstCaps* filtercaps_in = gst_caps_from_string("video/x-raw,format=I420");
        g_object_set (G_OBJECT (video_filter_in), "caps", filtercaps_in, NULL);
        gst_caps_unref(filtercaps_in);
        
        gst_bin_add_many (GST_BIN (pipeline),
                          video_filter_in,
                          clock_overlay,
                          text_overlay,
                          video_identity,
                          video_encoder,
                          video_queue_out, NULL);
        
        if (!gst_element_link_many(video_filter_in,
                                   clock_overlay,
                                   text_overlay,
                                   video_identity,
                                   video_encoder,
                                   video_queue_out, NULL)){
            g_print("link failure");
            return false;
        }
        
        g_signal_connect_data(video_identity, "handoff",
                              G_CALLBACK(pts_analysis_cb),
                              this, NULL, GConnectFlags());
        
        return true;
    }
    
    gboolean add_audio_encoder(){
        if (!pipeline){
            g_print("no pipeline");
            return false;
        }
        
        audio_encoder = gst_element_factory_make ("avenc_aac", "audio_encoder");
        audio_queue_out = gst_element_factory_make ("queue", "audio_queue_out");
        if (!audio_encoder || !audio_queue_out ){
            g_print("create audio encoder troubles");
            return false;
        }
        
        g_object_set (G_OBJECT (audio_encoder),
                      "bitrate", 128, NULL);
        
        gst_bin_add_many (GST_BIN (pipeline),
                          audio_encoder,
                          audio_queue_out, NULL);
        
        if (!gst_element_link(audio_encoder, audio_queue_out)){
            g_print("link failure");
            return false;
        }
        
        return true;
    }
    // SINKS
    gboolean add_rtmp_muxer_and_sink(const gchar *out){
        if (!pipeline){
            g_print("no pipeline");
            return false;
        }
        
        flv_muxer = gst_element_factory_make ("flvmux", "mux");
        rtmp_sink = gst_element_factory_make ("rtmpsink", "rtmp_sink");
        if (!flv_muxer || !rtmp_sink){
            g_print("create elements troubles");
            return false;
        }
        
        g_object_set (G_OBJECT (rtmp_sink),
                      "location", out,
                      "async", false,
                      NULL);
        g_object_set (G_OBJECT(flv_muxer),
                      "streamable", true,
                      NULL);
        
        gst_bin_add_many (GST_BIN (pipeline),
                          flv_muxer,
                          rtmp_sink, NULL);
        
        if (!gst_element_link_many(flv_muxer, rtmp_sink, NULL)){
            g_print("flv link failure");
            return false;
        }
        
        return true;
    }
    
    // BUILDERS
    gboolean build_test_src(const gchar *out){
        if (!pipeline){
            g_print("no pipeline");
            return false;
        }
        // add elements
        if (!add_test_src()){
            g_print("add_test_src failure");
            return false;
        }
        if (!add_video_encoder_with_text_labels()){
            g_print("add_video_encoder failure");
            return false;
        }
        if (!add_audio_encoder()){
            g_print("add_audio_encoder failure");
            return false;
        }
        if (!add_rtmp_muxer_and_sink(out)){
            g_print("add_rtmp_muxer_and_streamer failure");
            return false;
        }
        
        // link elements
        if (!gst_element_link(video_src, video_filter_in)){
            g_print("video link failure");
            return false;
        }
        if (!gst_element_link(audio_src,audio_encoder)){
            g_print("audio link failure");
            return false;
        }
        if (!gst_element_link(video_queue_out, flv_muxer)){
            g_print("video to flv link failure");
            return false;
        }
        if (!gst_element_link(audio_queue_out, flv_muxer)){
            g_print("audio to flv link failure");
            return false;
        }
        
        return true;
    }
    
    gboolean build_file_src(const gchar *filepath, const gchar *out){
        if (!pipeline){
            g_print("no pipeline");
            return false;
        }
        
        // add elements
        if (!add_file_src_with_mp4_demux(filepath)){
            g_print("add_file_src_with_mp4_demux failure");
            return false;
        }
        if (!add_video_decoder()){
            g_print("add_video_decoder failure");
            return false;
        }
        if (!add_audio_decoder()){
            g_print("add_audio_decoder failure");
            return false;
        }
        if (!add_video_encoder_with_text_labels()){
            g_print("add_video_encoder failure");
            return false;
        }
        if (!add_audio_encoder()){
            g_print("add_audio_encoder failure");
            return false;
        }
        if (!add_rtmp_muxer_and_sink(out)){
            g_print("add_rtmp_muxer_and_streamer failure");
            return false;
        }
        
        // link elements
        if (!gst_element_link(video_decoder, video_filter_in)){
            g_print("video link failure");
            return false;
        }
        if (!gst_element_link(audio_resample,audio_encoder)){
            g_print("audio link failure");
            return false;
        }
        if (!gst_element_link(video_queue_out, flv_muxer)){
            g_print("video to flv link failure");
            return false;
        }
        if (!gst_element_link(audio_queue_out, flv_muxer)){
            g_print("audio to flv link failure");
            return false;
        }
       
        return true;
    }
    
    // HANDLERS
    static void pts_analysis_cb(GstElement *identity, GstBuffer *buffer, _CustomData *data) {
        if (!data->text_overlay)
            return;
        
        gchar *text = g_strdup_printf ("PTS: %" GST_TIME_FORMAT,
                                GST_TIME_ARGS (GST_BUFFER_PTS (buffer)));
        g_object_set (G_OBJECT (data->text_overlay),
                      "text", text, NULL);
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

        if (g_str_has_prefix (new_pad_type,"audio/mpeg")){
            ret = gst_pad_link (new_pad, sink_pad_audio);
            if (GST_PAD_LINK_FAILED (ret)) {
               g_print ("Type is '%s' but link failed.\n", new_pad_type);
             }
        }
        else if (g_str_has_prefix (new_pad_type, "video/x-h264")){
            ret = gst_pad_link (new_pad, sink_pad_video);
            if (GST_PAD_LINK_FAILED (ret)) {
               g_print ("Type is '%s' but link failed.\n", new_pad_type);
             }
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
        g_main_loop_quit (loop);
        return true;
    }
    
} CustomData;

class RtmpStreamer{
private:
    CustomData *data;
    GstBus *bus;
public:
    RtmpStreamer(const gchar *file_path, const gchar *rtmp_address);
    ~RtmpStreamer();
    
    void Play();
    void Pause();
    void Stop();
    void Launch();
};

#endif /* RtmpStreamer_hpp */
