#include "glib.h"
#include "gio/gio.h"

void call_method_by_dbus(void)
{
}

void test_call_method(void)
{
    GDBusMessage *call_msg;
    GDBusMessage *reply_msg;
    GDBusConnection *c;
    GError *error = NULL;
    // guint32 serial_temp;
    GVariant *body;

    g_print("%s\n", __func__);

    // session_bus_up();

    c = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
    call_msg = g_dbus_message_new_method_call(
        "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "GetId");

    // 设置参数
    // g_dbus_message_set_body();

    // g_dbus_connection_send_message(c, msg, G_DBUS_SEND_MESSAGE_FLAGS_NONE, &serial_temp, &error);

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

    g_print("Type:%s\n", g_variant_get_type_string(body));
    body = g_variant_get_child_value(body, 0);
    g_print("Type:%s\n", g_variant_get_type_string(body));
    g_print("Result:%s\n", g_variant_get_string(body, NULL));

    g_object_unref(call_msg);
    g_object_unref(reply_msg);
    g_object_unref(c);
}