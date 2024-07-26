#include <gio/gio.h>
#include <stdlib.h>

#ifdef G_OS_UNIX
#include <gio/gunixfdlist.h>
/* For STDOUT_FILENO */
#include <unistd.h>
#endif

#include "interface.h"

static MyInterface *skeleton = NULL;

gboolean handle_my_method(
    MyInterface *object, GDBusMethodInvocation *invocation, const gchar *arg_in_arg1, gint arg_in_arg2)
{
    g_print("Received method call\n");

    g_print("arg_in_arg1: %s, arg_in_arg2: %d\n", arg_in_arg1, arg_in_arg2);

    my_interface_complete_my_method(object, invocation, "Hello", 42);

    return TRUE;
}

static void on_bus_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    g_print("Acquired name %s\n", name);

    GError *error = NULL;
    skeleton = my_interface_skeleton_new();

    g_signal_connect(skeleton, "handle-my-method", G_CALLBACK(handle_my_method), NULL);

    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(skeleton), connection, "/org/hai/TestServer", &error);
    if (error != NULL)
    {
        g_printerr("Error exporting object: %s\n", error->message);
        g_error_free(error);
    }
}

static void on_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    g_print("Acquired name %s\n", name);
}

static void on_name_lost(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    exit(1);
}

static gboolean emit_test_status_signal(gpointer user_data)
{
    // my_interface_emit_bar_signal(skeleton, "Hello", "World");

    gboolean ret = my_interface_get_bool_property(skeleton);
    g_print("Emit signal  %d\n", ret);

    return TRUE;
}

int main(int argc, char *argv[])
{
    guint owner_id;
    GMainLoop *loop;

    owner_id = g_bus_own_name(G_BUS_TYPE_SESSION, "org.hai.TestServer", G_BUS_NAME_OWNER_FLAGS_NONE, on_bus_acquired,
        on_name_acquired, on_name_lost, NULL, NULL);

    g_timeout_add(1000, (GSourceFunc)emit_test_status_signal, NULL);

    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    g_bus_unown_name(owner_id);

    return 0;
}
