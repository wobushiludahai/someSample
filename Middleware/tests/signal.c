#include "glib.h"
#include "gio/gio.h"
#include "dbus/dbus.h"

void send_signal_by_gdbus(void)
{
    GDBusConnection *c;
    c = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);

    g_dbus_connection_emit_signal(c, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus",
        "NameOwnerChanged", g_variant_new("(sss)", "test", "old", "new"), NULL);
    g_object_unref(c);
}

void send_signal_by_dbus(void)
{
    DBusConnection *conn = NULL;
    DBusMessage *signal = NULL;

    conn = dbus_bus_get(DBUS_BUS_SESSION, NULL);
    if (conn == NULL)
    {
        g_print("New Dbus connection failed\n");
        return;
    }

    signal = dbus_message_new_signal("/org/freedesktop/DBus", "org.freedesktop.DBus", "NameOwnerChanged");

    char *server = "test";
    char *old = "old";
    char *new = "new";

    dbus_message_append_args(
        signal, DBUS_TYPE_STRING, &server, DBUS_TYPE_STRING, &old, DBUS_TYPE_STRING, &new, DBUS_TYPE_INVALID);

    if (dbus_connection_send(conn, signal, NULL) == FALSE)
    {
        g_print("Send signal failed\n");
    }
    else
    {

        g_print("Send signal success\n");
    }

    dbus_message_unref(signal);

    dbus_connection_unref(conn);
}

// 假设的信号处理函数
static void on_signal_received(GDBusConnection *connection, const gchar *sender_name, const gchar *object_path,
    const gchar *interface_name, const gchar *signal_name, GVariant *parameters, gpointer user_data)
{
    g_print("Received signal: %s\n", signal_name);
    // 在这里处理信号
}

GDBusConnection *c_test = NULL;
void subscribe_signal_by_gdbus(void)
{
    c_test = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);

    g_dbus_connection_signal_subscribe(c_test, "org.freedesktop.DBus", "org.freedesktop.DBus", "NameLost",
        "/org/freedesktop/DBus", NULL, G_DBUS_SIGNAL_FLAGS_NONE, on_signal_received, NULL, NULL);

    g_print("Subscribe signal success\n");
}

void subscribe_signal_by_dbus(void)
{
    // dbus_bus_add_match
}

void test_signal(void)
{
    subscribe_signal_by_gdbus();
    g_usleep(2000 * 1000);
    g_print("%s\n", __func__);
    send_signal_by_dbus();

    g_usleep(2000 * 1000);

    send_signal_by_gdbus();
}