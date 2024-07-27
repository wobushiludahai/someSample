#include <glib.h>
#include <gio/gio.h>

#include "service.h"
#include "photo.h"

#define MODULE_NAME "PHOTO"
Photo *photo = NULL;

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
gpointer thread_test(gpointer data)
{
    GError *proxyerror = NULL;
    Video *proxy = (Video *)register_proxy("VIDEO", (proxy_new_sync)video_proxy_new_for_bus_sync);

    g_print("thread_test  %p\n", proxy);

    bind_signal_callback("VIDEO", "RecordStop", (gpointer)record_stop_signal);

    while (1)
    {
        g_usleep(1000); // 1s
        test++;
        video_call_stop_video_sync(get_service_proxy_by_name("VIDEO"), NULL, &proxyerror);
        if (proxyerror != NULL)
        {
            g_printerr("Error calling method: %s\n", proxyerror->message);
            // return NULL;
        }

        if (test == 10)
        {
            g_print("bind property changed callback\n");
            bind_property_changed_callback("VIDEO", "Int16Property", test_property_changed_callback);
            bind_property_changed_callback("VIDEO", "Uint16Property", test_property_changed_callback);
            bind_property_changed_callback("VIDEO", "Int32Property", test_property_changed_callback);
            bind_property_changed_callback("VIDEO", "Uint32Property", test_property_changed_callback);
            bind_property_changed_callback("VIDEO", "Int64Property", test_property_changed_callback);
            bind_property_changed_callback("VIDEO", "Uint64Property", test_property_changed_callback);
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
