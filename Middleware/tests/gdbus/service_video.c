#include <glib.h>
#include <gio/gio.h>

#include "service.h"
#include "video.h"

#define MODULE_NAME "VIDEO"
#define SERVER_VIDEO_HASH_NAME "video"
#define SERVER_VIDEO2_HASH_NAME "video2"

gboolean handle_stop_video(Video *object, GDBusMethodInvocation *invocation)
{
    g_print("Received method call\n");

    video_complete_stop_video(object, invocation);

    return TRUE;
}

gint16 test = 0;
extern gboolean register_server(const gchar *hash_name, skelete_new call);
gpointer thread_test(gpointer data)
{
    while (1)
    {
        g_usleep(2000 * 1000); // 1s
    }

    return NULL;
}

extern gboolean bind_server_property_changed_callback(
    const gchar *server_hash_name, const gchar *property_name, property_changed_callback callback);
static void service_register_success_callback(void)
{
    // 注册服务
    register_server(SERVER_VIDEO_HASH_NAME, (skelete_new)video_skeleton_new);
    register_server(SERVER_VIDEO2_HASH_NAME, (skelete_new)video2_skeleton_new);

    // 绑定方法回调
    bind_service_method_callback(SERVER_VIDEO_HASH_NAME, "StopVideo", G_CALLBACK(handle_stop_video));

    bind_server_property_changed_callback(SERVER_VIDEO_HASH_NAME, "Int16Property", NULL);
}

#include "config_mgmt.h"
int main(int argc, char *argv[])
{
    GMainLoop *loop = NULL;

    config_mgmt_init();
    config_mgmt_set_uint32_value("video_Int32Property", 1999);

    service_init(MODULE_NAME, service_register_success_callback);
    g_thread_new("thread_test", thread_test, NULL);

    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    service_exit();

    return 0;
}
