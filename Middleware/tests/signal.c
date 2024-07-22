#include "glib.h"
#include "gio/gio.h"
#include "dbus/dbus.h"

// dbus_message_new_signal

void send_signal_by_gdbus(void)
{
    // g_dbus_connection_emit_signal
}

void send_signal_by_dbus(void)
{
    // dbus_message_new_signal
}

void subscribe_signal_by_gdbus(void)
{
    // g_dbus_connection_signal_subscribe
}

void subscribe_signal_by_dbus(void)
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

    // dbus_message_set_sender(signal, "org.freedesktop.DBus");

    // dbus_message_set_destination(signal, "org.freedesktop.DBus");

    // dbus_message_set_path(signal, "/org/freedesktop/DBus");

    // dbus_message_set_interface(signal, "org.freedesktop.DBus");

    // dbus_message_set_signature(signal, "sss");

    g_print("Send signal\n");

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

void test_signal(void)
{
    g_print("%s\n", __func__);
    subscribe_signal_by_dbus();
}