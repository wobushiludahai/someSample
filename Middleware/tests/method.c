#include "glib.h"
#include "gio/gio.h"
#include "dbus/dbus.h"

void test_callback(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    g_print("\ncall_method_by_gdbus_async callback\n");
    GDBusConnection *connection = G_DBUS_CONNECTION(source_object);
    GError *error = NULL;
    GDBusMessage *reply;

    reply = g_dbus_connection_send_message_with_reply_finish(connection, res, &error);

    if (reply != NULL)
    {
        GVariant *body = g_dbus_message_get_body(reply);
        body = g_variant_get_child_value(body, 0);
        g_print("Result:%s\n", g_variant_get_string(body, NULL));
    }

    g_object_unref(connection);
    g_object_unref(reply);
}

void call_method_by_gdbus_async(const char *service, const char *path, const char *interface, const char *method)
{
    GDBusMessage *call_msg;
    GDBusConnection *c;
    GError *error = NULL;
    c = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
    call_msg = g_dbus_message_new_method_call(service, path, interface, method);

    // 设置参数
    // g_dbus_message_set_body();

    g_dbus_connection_send_message_with_reply(
        c, call_msg, G_DBUS_SEND_MESSAGE_FLAGS_NONE, -1, NULL, NULL, test_callback, NULL);

    g_object_unref(call_msg);
}

void call_method_by_gdbus_sync(const char *service, const char *path, const char *interface, const char *method)
{
    GDBusMessage *call_msg;
    GDBusMessage *reply_msg;
    GDBusConnection *c;
    GError *error = NULL;
    GVariant *body;
    g_print("%s\n", __func__);
    c = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
    call_msg = g_dbus_message_new_method_call(service, path, interface, method);

    // 设置参数
    // g_dbus_message_set_body();

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

    body = g_variant_get_child_value(body, 0);
    g_print("Result:%s\n", g_variant_get_string(body, NULL));

    g_object_unref(call_msg);
    g_object_unref(reply_msg);
    g_object_unref(c);
}

void call_method_by_dbus(const char *service, const char *path, const char *interface, const char *method)
{
    DBusConnection *conn = NULL;
    DBusMessage *call_msg = NULL;
    DBusMessage *reply_msg = NULL;

    g_print("\n%s\n", __func__);

    conn = dbus_bus_get(DBUS_BUS_SESSION, NULL);
    if (conn == NULL)
    {
        g_print("New Dbus connection failed\n");
        return;
    }

    call_msg = dbus_message_new_method_call(service, path, interface, method);
    if (call_msg == NULL)
    {
        g_print("Message Null\n");
    }

    // dbus_message_append_args(
    //     call_msg, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &property_name, DBUS_TYPE_INVALID);

    reply_msg = dbus_connection_send_with_reply_and_block(conn, call_msg, -1, NULL);

    gchar *body = NULL;
    dbus_bool_t result = dbus_message_get_args(reply_msg, NULL, DBUS_TYPE_STRING, &body, DBUS_TYPE_INVALID);
    if (result == FALSE)
    {
        g_print("Error occured\n");
    }

    g_print("Type:%s\n", body);

    dbus_message_unref(call_msg);
    dbus_message_unref(reply_msg);
    dbus_connection_unref(conn);
}

void test_call_method(void)
{
    call_method_by_gdbus_sync("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "GetId");
    call_method_by_gdbus_async("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "GetId");

    call_method_by_dbus("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "GetId");
}