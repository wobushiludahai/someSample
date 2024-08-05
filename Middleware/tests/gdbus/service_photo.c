#include <glib.h>
#include <gio/gio.h>

#include "service.h"
#include "photo.h"

#define MODULE_NAME "PHOTO"
#include "video.h"

static void record_stop_signal(Video *object)
{
    g_print("Received signal\n");
}

void test_property_changed_callback(GVariant *property)
{
    g_print("Property changed: %s\n", g_variant_get_type_string(property));
}

int test = 0;

#define PROXY_VIDEO_HASH_NAME "video"
#define PROXY_VIDEO2_HASH_NAME "video2"
gpointer thread_test(gpointer data)
{
    GError *proxyerror = NULL;
    register_proxy("VIDEO", "video", PROXY_VIDEO_HASH_NAME, (proxy_new_sync)video_proxy_new_for_bus_sync);
    bind_signal_callback(PROXY_VIDEO_HASH_NAME, "RecordStop", (gpointer)record_stop_signal);

    while (1)
    {
        g_usleep(1000 * 1000); // 1s
        test++;
        video_call_stop_video_sync(get_proxy_instance(PROXY_VIDEO_HASH_NAME), NULL, &proxyerror);
        if (proxyerror != NULL)
        {
            g_printerr("Error calling method: %s\n", proxyerror->message);
            // return NULL;
        }

        if (test == 2)
        {
            g_print("bind property changed callback\n");
            bind_property_changed_callback(PROXY_VIDEO_HASH_NAME, "Int16Property", test_property_changed_callback);
            bind_property_changed_callback(PROXY_VIDEO_HASH_NAME, "Uint16Property", test_property_changed_callback);
            bind_property_changed_callback(PROXY_VIDEO_HASH_NAME, "Int32Property", test_property_changed_callback);
            bind_property_changed_callback(PROXY_VIDEO_HASH_NAME, "Uint32Property", test_property_changed_callback);
            bind_property_changed_callback(PROXY_VIDEO_HASH_NAME, "Int64Property", test_property_changed_callback);
            bind_property_changed_callback(PROXY_VIDEO_HASH_NAME, "Uint64Property", test_property_changed_callback);
        }
    }

    return NULL;
}

#define SERVER_PHOTO_HASH_NAME "photo"
extern gboolean register_server(const gchar *hash_name, skelete_new call);
static void service_register_success_callback(void)
{
    register_server(SERVER_PHOTO_HASH_NAME, (skelete_new)photo_skeleton_new);
}

int main(int argc, char *argv[])
{
    GMainLoop *loop = NULL;

    service_init(MODULE_NAME, service_register_success_callback);

    g_thread_new("thread_test", thread_test, NULL);

    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    return 0;
}
