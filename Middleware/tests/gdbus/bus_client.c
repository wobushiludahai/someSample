#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <unistd.h>

#include <time.h>

#include <gio/gio.h>
#include <gio/gunixfdlist.h>

#include "interface.h"
static MyInterface *proxy = NULL;

static gboolean onValueChanged(MyInterface *object, gchar *arg_blah, gchar *arg_boo)
{
    g_print("onValueChanged, get value: %s %s\n", arg_blah, arg_boo);
    return TRUE;
}

void onPropertyChanged(MyInterface *proxy, const gchar *object_path, guint16 property, gpointer user_data)
{
    g_print("onPropertyChanged, value: %d\n", property);
}

void onPropertyChangedtest(GDBusProxy *proxy, GVariant *changed_properties, const gchar *const *invalidated_properties)
{
    g_print("onPropertyChanged  %s  %s\n", g_dbus_proxy_get_interface_name(proxy),
        g_variant_get_type_string(changed_properties));

    GVariantIter iter;
    g_variant_iter_init(&iter, changed_properties);

    gchar *key = NULL;
    GVariant *value = NULL;
    while (g_variant_iter_next(&iter, "{sv}", &key, &value))
    {
        g_print("key: %s, value: %u\n", key, g_variant_get_uint16(value));
        g_variant_unref(value);
        g_free(key);
    }
}

guint16 temp_test = 0;
gboolean timeout_test(gpointer user_data)
{
    my_interface_set_uint16_property(proxy, temp_test++);

    guint16 ret = my_interface_get_uint16_property(proxy);
    g_print("Emit signal  %u\n", ret);

    return TRUE;
}

gpointer test_thread(gpointer data)
{
    guint32 count = 0;

    while (1)
    {
        count++;
        my_interface_set_uint16_property(proxy, temp_test++);

        char *p = "hello";
        char *q = NULL;
        gint ret_int;
        GError *proxyerror = NULL;

        my_interface_call_my_method_sync(proxy, p, 100, &q, &ret_int, NULL, &proxyerror);
        if (proxyerror != NULL)
        {
            g_printerr("Error calling method: %s\n", proxyerror->message);
            break;
        }

        if (count == 1000 * 10000)
        {
            break;
        }

        g_usleep(1000 * 1000);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    GMainLoop *loop = NULL;
    GError *connerror = NULL;
    GError *proxyerror = NULL;

    proxy = my_interface_proxy_new_for_bus_sync(
        G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, "org.hai.TestServer", "/org/hai/TestServer", NULL, &proxyerror);
    if (proxy == NULL)
    {
        g_printerr("Error creating proxy: %s\n", proxyerror->message);
        g_error_free(connerror);
        g_error_free(proxyerror);
        return 1;
    }

    // g_timeout_add(1, (GSourceFunc)timeout_test, NULL);

    g_thread_new("thread_test", (GThreadFunc)test_thread, NULL);

    g_signal_connect(proxy, "bar-signal", G_CALLBACK(onValueChanged), NULL);
    g_signal_connect(proxy, "g-properties-changed", G_CALLBACK(onPropertyChangedtest), NULL);

    char *p = "hello";
    char *q = NULL;
    gint ret_int;

    my_interface_call_my_method_sync(proxy, p, 100, &q, &ret_int, NULL, &proxyerror);
    if (proxyerror != NULL)
    {
        g_printerr("Error calling method: %s\n", proxyerror->message);
        return 1;
    }

    g_print("method return: %s, %d\n", q, ret_int);

    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    g_object_unref(proxy);

    return 0;
}
