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

// 返回值为 true则执行后续修改动作，返回值为false则不会去执行后续修改动作
gboolean test_server_property_before_change_callback(gpointer value)
{
    gchar *test_value = *(gchar **)value;
    g_print("func:%s %s\n", "test_server_property_before_change_callback", test_value);

    return TRUE;
}
// 值不同，且被修改后触发回调
void test_server_property_after_changed_callback(gpointer value)
{
    g_print("func:%s.\n", "test_server_property_after_changed_callback");
}

static void service_register_success_callback(void)
{
    // 注册服务
    register_server(SERVER_VIDEO_HASH_NAME, (skelete_new)video_skeleton_new);

    // 绑定方法回调
    bind_service_method_callback(SERVER_VIDEO_HASH_NAME, "StopVideo", G_CALLBACK(handle_stop_video));

    video_server_set_str_property_before_change_callback(test_server_property_before_change_callback);
    video_server_set_str_property_after_changed_callback(test_server_property_after_changed_callback);
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
