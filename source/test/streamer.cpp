#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/check/gstcheck.h>

#include "../rtmp/streamer.hpp"

GST_START_TEST (test_add_file_src_with_mp4_demux)
{
    CustomData data;
    data.pipeline = gst_pipeline_new ("test_file_src");

    // is flow correct 
    fail_unless(data.add_file_src_with_mp4_demux("input.mp4"));
    // not null element
    fail_unless(data.file_src);
    fail_unless(data.mp4_demux);

    gchar *location;
    g_object_get(G_OBJECT (data.file_src), "location", &location, NULL);
    fail_unless_equals_string(location,"input.mp4");

    gst_element_set_state (data.pipeline, GST_STATE_NULL);
    gst_object_unref (data.pipeline);
    g_free(location);
}

GST_END_TEST;

GST_START_TEST (test_add_video_encoder_with_text_labels)
{
    CustomData data;
    data.pipeline = gst_pipeline_new ("test_video_encoder");

    // is flow correct 
    fail_unless(data.add_video_encoder_with_text_labels());
    // not null element
    fail_unless(data.video_encoder);

    gint bitrate;
    g_object_get(G_OBJECT (data.video_encoder), "bitrate", &bitrate, NULL);
    fail_unless_equals_int(bitrate,3000);

    gst_element_set_state (data.pipeline, GST_STATE_NULL);
    gst_object_unref (data.pipeline);
}

GST_END_TEST;

GST_START_TEST (test_add_rtmp_muxer_and_sink)
{
    CustomData data;
    data.pipeline = gst_pipeline_new ("test_rtmp_sink");

    // is flow correct 
    fail_unless(data.add_rtmp_muxer_and_sink("rtmp://127.0.0.1:12345"));
    // not null element
    fail_unless(data.rtmp_sink);

    gchar *location;
    g_object_get(G_OBJECT (data.rtmp_sink), "location", &location, NULL);
    fail_unless_equals_string(location,"rtmp://127.0.0.1:12345");

    gst_element_set_state (data.pipeline, GST_STATE_NULL);
    gst_object_unref (data.pipeline);
    g_free(location);
}

GST_END_TEST;

static Suite *
streamer_suite (void)
{
    Suite *s = suite_create ("streamer");
    TCase *tc_chain = tcase_create ("general");

    suite_add_tcase (s, tc_chain);

    tcase_add_test (tc_chain, test_add_file_src_with_mp4_demux);
    tcase_add_test (tc_chain, test_add_video_encoder_with_text_labels);
    tcase_add_test (tc_chain, test_add_rtmp_muxer_and_sink);

    return s;
}

GST_CHECK_MAIN (streamer);
