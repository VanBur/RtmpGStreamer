#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/check/gstcheck.h>
#include "../RtmpStreamer/RtmpStreamer.hpp"

GST_START_TEST (test_file_src)
{
    CustomData data;
    gchar *input = "/path/to/file.mp4";
    gchar *output = "rtmp://127.0.0.1:12345";
    
    gboolean res;
    res = data.build_file_src(input, output);
    fail_unless(!res, "Could not create flow");
    
    
    GObjectClass* objClass = G_OBJECT_GET_CLASS(data.rtmp_sink);
    guint n_props;
    GParamSpec** props;
    props = g_object_class_list_properties(objClass,&n_props);
    
    
    g_free (input);
    g_free (output);
}

GST_END_TEST;

GST_START_TEST (test_test_src)
{
    CustomData data;
    gchar *output = "rtmp://127.0.0.1:12345";
    
    gboolean res;
    res = data.build_test_src(output);
    fail_unless(!res, "Could not create flow");
    
    g_free (output);
}

GST_END_TEST;

static Suite *
libvisual_suite (void)
{
    Suite *s = suite_create ("RtmpStreamer");
    TCase *tc_chain = tcase_create ("streamer tests");

    tcase_set_timeout (tc_chain, 30);
    suite_add_tcase (s, tc_chain);

    tcase_add_test (tc_chain, test_file_src);
    tcase_add_test (tc_chain, test_test_src);

    return s;
}

GST_CHECK_MAIN (libvisual);
