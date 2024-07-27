#include <glib.h>
#include <gio/gio.h>

#include "service.h"
#include "video.h"

#define MODULE_NAME "VIDEO"
Video *video = NULL;

gboolean handle_stop_video(Video *object, GDBusMethodInvocation *invocation)
{
    g_print("Received method call\n");

    video_complete_stop_video(object, invocation);

    return TRUE;
}

gint16 test = 0;
gpointer thread_test(gpointer data)
{
    while (1)
    {
        g_usleep(1000); // 1s
        video_emit_record_stop(get_service_instance());
        video_set_int16_property(get_service_instance(), test++);
        video_set_uint16_property(get_service_instance(), test++);
        video_set_int32_property(get_service_instance(), test++);
        video_set_uint32_property(get_service_instance(), test++);
        video_set_int64_property(get_service_instance(), test++);
        video_set_uint64_property(get_service_instance(), test++);
        g_print("Emitted signal\n");
    }

    return NULL;
}


int main(int argc, char *argv[])
{
    GMainLoop *loop = NULL;

    video = video_skeleton_new();
    service_init(MODULE_NAME, video);

    bind_service_method_callback("StopVideo", (gpointer)handle_stop_video);
    g_thread_new("thread_test", thread_test, NULL);

    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    service_exit();

    return 0;
}
