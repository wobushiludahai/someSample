#include "glib.h"
#include "gio/gio.h"
#include "dbus/dbus.h"

void get_perperty_by_gdbus(const gchar *service, const gchar *path, const gchar *interface, const gchar *property_name)
{
    GDBusMessage *call_msg;
    GDBusMessage *reply_msg;
    GDBusConnection *c;
    GError *error = NULL;
    GVariant *body;

    c = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
    call_msg = g_dbus_message_new_method_call(service, path, "org.freedesktop.DBus.Properties", "Get");

    // 设置参数
    g_dbus_message_set_body(call_msg, g_variant_new("(ss)", interface, property_name));

    reply_msg = g_dbus_connection_send_message_with_reply_sync(
        c, call_msg, G_DBUS_SEND_MESSAGE_FLAGS_NONE, -1, NULL, NULL, &error);

    if (reply_msg == NULL)
    {
        g_print("Some Error occuerd\n");
    }

    if (error != NULL)
    {
        g_print("Some Error occuerd\n");
    }

    body = g_dbus_message_get_body(reply_msg);

    // g_print("Type:%s\n", g_variant_get_type_string(body));
    body = g_variant_get_child_value(body, 0);
    // g_print("Type:%s\n", g_variant_get_type_string(body));
    body = g_variant_get_child_value(body, 0);

    g_print("\n");
    g_print("Type:%s\n", g_variant_get_type_string(body));
    g_print("Result:%s\n", g_variant_get_string(body, NULL));
    g_print("\n");

    g_object_unref(call_msg);
    g_object_unref(reply_msg);
    g_object_unref(c);
}

void parse_variant(DBusMessageIter *arg_iter)
{
    DBusMessageIter variant_iter;
    const char *contents;
    dbus_bool_t value;
    int type;

    // 从 variant 类型参数中提取实际的值
    dbus_message_iter_recurse(arg_iter, &variant_iter);

    // 获取 variant 内部值的类型
    type = dbus_message_iter_get_arg_type(&variant_iter);

    // 根据类型处理不同的数据类型
    switch (type)
    {
        case DBUS_TYPE_STRING:
            dbus_message_iter_get_basic(&variant_iter, &contents);
            g_print("Received string: %s\n", contents);
            break;
        case DBUS_TYPE_BOOLEAN:
            dbus_message_iter_get_basic(&variant_iter, &value);
            g_print("Received boolean: %s\n", value ? "true" : "false");
            break;

        case DBUS_TYPE_VARIANT:
            g_print("Received variant\n");
            parse_variant(&variant_iter);
            break;

        // 添加其他需要处理的类型
        default:
            g_print("Unhandled type: %c\n", type);
            break;
    }
}

void get_property_by_dbus(const gchar *service, const gchar *path, const gchar *interface, const gchar *property_name)
{
    DBusConnection *conn = NULL;
    DBusMessage *call_msg = NULL;
    DBusMessage *reply_msg = NULL;

    conn = dbus_bus_get(DBUS_BUS_SESSION, NULL);
    if (conn == NULL)
    {
        g_print("New Dbus connection failed\n");
        return;
    }

    call_msg = dbus_message_new_method_call(service, path, "org.freedesktop.DBus.Properties", "Get");
    if (call_msg == NULL)
    {
        g_print("Message Null\n");
    }

    dbus_message_append_args(
        call_msg, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &property_name, DBUS_TYPE_INVALID);

    reply_msg = dbus_connection_send_with_reply_and_block(conn, call_msg, -1, NULL);

    g_print("Type:%s\n", dbus_message_get_signature(reply_msg));

    DBusMessageIter arg_iter;
    // 假设我们知道 variant 类型参数位于消息的第一个位置
    dbus_message_iter_init(reply_msg, &arg_iter);
    parse_variant(&arg_iter);

    dbus_message_unref(call_msg);
    dbus_message_unref(reply_msg);
    dbus_connection_unref(conn);
}

void test_get_property(void)
{
    get_perperty_by_gdbus("org.freedesktop.systemd1", "/org/freedesktop/systemd1/unit/dev_2dram4_2edevice",
        "org.freedesktop.systemd1.Unit", "Description");

    g_print("\nget_property_by_dbus\n");
    get_property_by_dbus("org.freedesktop.systemd1", "/org/freedesktop/systemd1/unit/dev_2dram4_2edevice",
        "org.freedesktop.systemd1.Unit", "Description");
}