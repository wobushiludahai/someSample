#include "glib.h"
#include "gio/gio.h"

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

#include "dbus/dbus.h"
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

    // call_msg = dbus_message_new_method_call(service, path, "org.freedesktop.DBus.Properties", "Get");
    call_msg = dbus_message_new_method_call("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "GetId");
    if (call_msg == NULL)
    {

        g_print("Message Null\n");
    }

    // dbus_message_append_args(
    //     call_msg, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &property_name, DBUS_TYPE_INVALID);

    reply_msg = dbus_connection_send_with_reply_and_block(conn, call_msg, -1, NULL);

    // dbus_message_get_args(reply_msg, NULL, DBUS_TYPE_VARIANT, &body, DBUS_TYPE_INVALID);
    // if (body == NULL)
    // {
    //     g_print("body is null\n");
    // }

    g_print("Type:%s\n", dbus_message_get_signature(reply_msg));
    

    // DBusError *error = NULL;

    gchar *body = NULL;
    dbus_bool_t result = dbus_message_get_args(reply_msg, NULL, DBUS_TYPE_STRING, &body, DBUS_TYPE_INVALID);
    if (result == FALSE)
    {
        g_print("Error occured\n");
    }

    // body = g_variant_get_child_value(body, 0);
    g_print("Type:%s\n", body);
    // body = g_variant_get_child_value(body, 0);
    // g_print("Type:%s\n", g_variant_get_type_string(body));
    // g_print("Result:%s\n", g_variant_get_string(body, NULL));

    // g_print("\nResult:%s\n", g_variant_get_string(body, NULL));

    // dbus_error_free(error);
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