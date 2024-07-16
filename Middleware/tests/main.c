#include <stdio.h>
#include <stdlib.h>
#include "glib.h"

#include "test_cases.h"
#include "dbus/dbus.h"

DBusMessage *test_signal;
dbus_uint32_t serial;
DBusConnection *conn;

gboolean test_timeout(gpointer data)
{
    // g_print("Timeout\n");

    dbus_connection_send(conn, test_signal, &serial);
    return true;
}

DBusHandlerResult get_property(DBusConnection *connection, const char *service, const char *path,
    const char *interface_name, const char *property_name)
{
    DBusMessage *message, *reply;
    DBusError error;
    // dbus_int32_t value;
    // dbus_bool_t success;

    // 初始化DBus错误
    dbus_error_init(&error);

    // 创建一个新的DBus消息
    message = dbus_message_new_method_call(service, // 目标服务名称
        path, // 目标对象路径
        "org.freedesktop.DBus.Properties", // 接口名称
        "Get" // 方法名称
    );
    if (!message)
    {
        fprintf(stderr, "Message is NULL!\n");
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }

    // 添加参数到消息中
    dbus_message_append_args(
        message, DBUS_TYPE_STRING, &interface_name, DBUS_TYPE_STRING, &property_name, DBUS_TYPE_INVALID);

    // 发送消息并获取回复
    reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
    if (dbus_error_is_set(&error))
    {
        fprintf(stderr, "Failed to send message: %s\n", error.message);
        dbus_error_free(&error);
        dbus_message_unref(message);
        return -1;
    }

    if (!reply)
    {
        fprintf(stderr, "No reply received!\n");
        dbus_message_unref(message);
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    fprintf(stderr, "Reply serial:%u\n", dbus_message_get_serial(reply));
    fprintf(stderr, "Message signature:%s\n", dbus_message_get_signature(reply));
    fprintf(stderr, "Reply serial:%u\n", dbus_message_get_serial(message));
    fprintf(stderr, "Message signature:%s\n", dbus_message_get_signature(message));

    GVariant *test_value = NULL;

    // 读取回复中的值
    if (!dbus_message_get_args(reply, &error, DBUS_TYPE_VARIANT, &test_value, DBUS_TYPE_INVALID))
    {
        fprintf(stderr, "Failed to get arguments: %s\n", error.message);
        dbus_error_free(&error);
        dbus_message_unref(message);
        dbus_message_unref(reply);
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    // if (success)
    // {
    //     // 注意：这里你需要处理DBusVariant，并提取出实际的属性值
    //     // 由于我们使用dbus_int32_t作为示例，这里直接打印它（这实际上是不正确的）
    //     printf("Property value: %d\n", value); // 这只是一个示例，你需要根据实际的类型来解析DBusVariant
    // }
    // else
    // {
    //     fprintf(stderr, "Property not found or invalid type\n");
    // }

    // 清理
    dbus_message_unref(message);
    dbus_message_unref(reply);

    return DBUS_HANDLER_RESULT_HANDLED;
}

#include <gio/gio.h>
void on_get_property_reply(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    GVariant *result;
    GError *error = NULL;

    result = g_dbus_proxy_call_finish(G_DBUS_PROXY(source_object), res, &error);
    if (result == NULL)
    {
        g_print("Error calling Get on %s: %s\n", (char *)user_data, error->message);
        g_error_free(error);
        return;
    }

    g_print("Type:%s\n", g_variant_get_type_string(result));

    result = g_variant_get_child_value(result, 0);
    if (result == NULL) {
        g_print("Error getting child value failed\n");
    }
    g_print("Type:%s\n", g_variant_get_type_string(result));
    result = g_variant_get_child_value(result, 0);
    g_print("Type:%s\n", g_variant_get_type_string(result));


    g_print("Property value: %s\n", g_variant_get_string(result, NULL));
    g_variant_unref(result);
}

int main(void)
{
    // GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    // DBusMessage *msg = dbus_message_new(DBUS_MESSAGE_TYPE_METHOD_RETURN);

    // conn = dbus_bus_get(DBUS_BUS_SESSION, NULL);

    // get_property(conn, "org.freedesktop.systemd1", "/org/freedesktop/systemd1/unit/dev_2dram4_2edevice",
    //     "org.freedesktop.systemd1.Unit", "ActiveEnterTimestampMonotonic");

    // test_signal = dbus_message_new_signal(DBUS_PATH_DBUS, DBUS_INTERFACE_PROPERTIES, "PropertiesChanged");
    // if (test_signal == NULL)
    // {
    //     printf("Unable to allocate new " DBUS_INTERFACE_PROPERTIES ".PropertiesChanged signal\n");
    //     return -1;
    // }

    // g_timeout_add(1000, test_timeout, NULL);

    // g_main_loop_run(loop);

    // test_config_mgmt();

    GDBusProxy *proxy;
    GDBusConnection *connection;
    GError *error = NULL;

    // 初始化 GIO 库
    // g_type_init();

    // 获取系统或会话 D-Bus 连接
    connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
    if (connection == NULL)
    {
        g_print("Error getting session bus: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

    // 创建 D-Bus 代理对象
    proxy = g_dbus_proxy_new_sync(connection, // 连接
        G_DBUS_PROXY_FLAGS_NONE, // 标志
        NULL, // 接口信息（可以为 NULL，如果知道接口名称和对象路径）
        "org.freedesktop.systemd1", // 服务名称
        "/org/freedesktop/systemd1/unit/dev_2dram4_2edevice", // 对象路径
        "org.freedesktop.systemd1.Unit", // 接口名称
        NULL, // 取消回调
        &error // 错误
    );
    if (proxy == NULL)
    {
        g_print("Error creating proxy: %s\n", error->message);
        g_error_free(error);
        g_object_unref(connection);
        return 1;
    }

    // 调用 Get 方法获取属性
    const gchar *property_name = "CollectMode";
    g_dbus_proxy_call(proxy, "org.freedesktop.DBus.Properties.Get",
        g_variant_new("(ss)", "org.freedesktop.systemd1.Unit", property_name), G_DBUS_CALL_FLAGS_NONE,
        -1, // 超时（毫秒），-1 表示无限制
        NULL, // 取消回调
        on_get_property_reply, // 异步回调
        (gpointer)property_name // 用户数据
    );

    // 因为我们使用了异步调用，所以这里需要进入主循环等待回调
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop); // 这会阻塞，直到你退出主循环

    // 清理
    g_object_unref(proxy);
    g_object_unref(connection);
    g_main_loop_unref(loop);

    return 0;
}