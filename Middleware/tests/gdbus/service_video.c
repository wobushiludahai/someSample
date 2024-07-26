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

int main(int argc, char *argv[])
{
    GMainLoop *loop = NULL;

    video = video_skeleton_new();
    service_init(MODULE_NAME, video);

    bind_service_method_callback("stop_video", (gpointer)handle_stop_video);

    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    service_exit();

    return 0;
}
