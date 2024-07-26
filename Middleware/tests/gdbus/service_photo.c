#include <glib.h>
#include <gio/gio.h>

#include "service.h"
#include "photo.h"

#define MODULE_NAME "PHOTO"
Photo *photo = NULL;

#include "video.h"
gpointer thread_test(gpointer data)
{
    GError *proxyerror = NULL;
    Video *proxy = (Video *)register_proxy("VIDEO", (proxy_new_sync)video_proxy_new_for_bus_sync);

    g_print("thread_test  %p\n", proxy);

    while (1)
    {
        g_usleep(1000000); // 1s
        video_call_stop_video_sync(get_service_proxy_by_name("VIDEO"), NULL, &proxyerror);
        if (proxyerror != NULL)
        {
            g_printerr("Error calling method: %s\n", proxyerror->message);
            // return NULL;
        }
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    GMainLoop *loop = NULL;

    photo = photo_skeleton_new();
    service_init(MODULE_NAME, photo);

    g_thread_new("thread_test", thread_test, NULL);

    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    return 0;
}
