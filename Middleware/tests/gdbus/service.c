#include "service.h"

#define SERVICE_PREFIX "hai.testdbus.%s"
#define PATH_PREFIX "/org/hai/%s"

#define info(fmt...)
#define error(fmt...)
#define debug(fmt...)

typedef struct
{
    guint owner_id;
    gchar *service_name;
    gpointer *instance;
    GHashTable *method_callback_id;
    GHashTable *proxy;
} SERVICE_T;

static SERVICE_T g_service = { .instance = NULL };

/*
1、完成并行初始化
2、封装接口方便使用 （connect以及disconnect）
*/
static void on_bus_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    debug("on_bus_acquired %s\n", name);

    gchar *path = g_strdup_printf(PATH_PREFIX, g_service.service_name);
    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(user_data), connection, path, NULL);

    g_free(path);
}

static void on_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    debug("on_name_acquired %s\n", name);
}

static void on_name_lost(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    exit(1);
}

// 自身服务初始化
void service_init(const char *service_name, gpointer instance)
{
    debug("service_init %s\n", service_name);

    gchar *service = g_strdup_printf(SERVICE_PREFIX, service_name);

    g_service.owner_id = g_bus_own_name(G_BUS_TYPE_SESSION, service, G_BUS_NAME_OWNER_FLAGS_NONE, on_bus_acquired,
        on_name_acquired, on_name_lost, instance, NULL);
    g_service.service_name = g_strdup(service_name);
    g_service.method_callback_id = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    g_service.proxy = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    g_service.instance = instance;

    g_free(service);
}

void service_exit(void)
{
    g_free(g_service.service_name);
    g_bus_unown_name(g_service.owner_id);
    g_hash_table_destroy(g_service.method_callback_id);
    g_hash_table_destroy(g_service.proxy);
}

gboolean bind_service_method_callback(const gchar *method_name, gpointer callback)
{
    if (g_service.instance == NULL)
    {
        error("g_service.instance is NULL\n");
        return FALSE;
    }

    gulong id = g_signal_connect(g_service.instance, method_name, G_CALLBACK(callback), NULL);

    debug("bind_service_method_callback %s   %lu\n", method_name, id);

    g_hash_table_insert(g_service.method_callback_id, g_strdup(method_name), GUINT_TO_POINTER(id));

    return TRUE;
}

gboolean unbind_service_method_callback(const gchar *method_name)
{
    gpointer value = g_hash_table_lookup(g_service.method_callback_id, method_name);

    if (value == NULL)
    {
        debug("can not find %s\n", method_name);
        return FALSE;
    }

    g_signal_handler_disconnect(g_service.instance, GPOINTER_TO_UINT(value));
    debug("unbind_service_method_callback %s   %u\n", method_name, GPOINTER_TO_UINT(value));

    g_hash_table_remove(g_service.method_callback_id, method_name);

    return TRUE;
}

gpointer get_service_instance(void)
{
    return g_service.instance;
}

gpointer register_proxy(const gchar *service_name, proxy_new_sync call)
{
    gpointer proxy = g_hash_table_lookup(g_service.proxy, service_name);
    if (proxy != NULL)
    {
        debug("proxy %s already exist\n", service_name);
        return proxy;
    }

    gchar *service = g_strdup_printf(SERVICE_PREFIX, service_name);
    gchar *path = g_strdup_printf(PATH_PREFIX, service_name);
    GError *proxyerror = NULL;

    proxy = call(G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, service, path, NULL, &proxyerror);
    if (proxy == NULL)
    {
        error("Error creating proxy: %s\n", proxyerror->message);
        g_error_free(proxyerror);
        return NULL;
    }

    g_hash_table_insert(g_service.proxy, g_strdup(service_name), proxy);

    g_free(service);
    g_free(path);

    return proxy;
}

gpointer get_service_proxy_by_name(const gchar *service_name)
{
    gpointer proxy = g_hash_table_lookup(g_service.proxy, service_name);
    if (proxy == NULL)
    {
        error("can not find %s\n", service_name);
        return NULL;
    }

    return proxy;
}
