#include "service.h"

#define SERVICE_PREFIX "hai.testdbus.%s"
#define PATH_PREFIX "/org/hai/%s"

#define info(fmt...)
#define error(fmt...)
#define debug(fmt...)

typedef struct
{
    gpointer proxy;
    gchar *service_name;
    GThreadPool *pool;
    GHashTable *signal_callback_id;
    GHashTable *property_changed_callback;
    gboolean is_have_monitor_property; // 是否有属性被监听
    gulong property_changed_signal_id;
} PROXY_INFO_T;

typedef struct
{
    guint owner_id;
    gchar *service_name;
    gpointer *instance;
    GHashTable *method_callback_id;
    GHashTable *proxy;
} SERVICE_T;

// 需要考虑多线程安全
static SERVICE_T g_service = { .instance = NULL };

/*
    1、完成并行初始化
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

static void _destory_proxy_hashtable(gpointer data)
{
    PROXY_INFO_T *proxy_info = (PROXY_INFO_T *)data;

    g_hash_table_destroy(proxy_info->signal_callback_id);
    g_hash_table_destroy(proxy_info->property_changed_callback);
    g_free(proxy_info->service_name);
    g_thread_pool_free(proxy_info->pool, FALSE, TRUE);

    g_free(proxy_info);
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
    g_service.proxy = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, _destory_proxy_hashtable);

    g_service.instance = instance;

    g_free(service);
}

gpointer get_service_instance(void)
{
    return g_service.instance;
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

// 请确保 service 已经 register
// 是否需要支持监听自身 signal ?
gboolean bind_signal_callback(const gchar *service, const gchar *signal_name, gpointer callback)
{
    PROXY_INFO_T *proxy_info = (PROXY_INFO_T *)g_hash_table_lookup(g_service.proxy, service);
    if (proxy_info == NULL)
    {
        error("can not find %s\n", service);
        return FALSE;
    }

    gchar *signal_key = g_strdup_printf("%s_%s", service, signal_name);
    gpointer value = g_hash_table_lookup(proxy_info->signal_callback_id, signal_key);
    if (value != NULL)
    {
        debug("The signal %s for %s has been binded\n", signal_name, service);
        g_free(signal_key);
        return FALSE;
    }

    gulong id = g_signal_connect(proxy_info->proxy, signal_name, G_CALLBACK(callback), NULL);
    g_hash_table_insert(proxy_info->signal_callback_id, g_strdup(signal_key), GUINT_TO_POINTER(id));
    g_free(signal_key);

    return TRUE;
}

gboolean unbind_signal_callback(const gchar *service, const gchar *signal_name)
{
    PROXY_INFO_T *proxy_info = (PROXY_INFO_T *)g_hash_table_lookup(g_service.proxy, service);
    if (proxy_info != NULL)
    {
        g_print("can not find %s\n", service);
        return FALSE;
    }

    gchar *signal_key = g_strdup_printf("%s_%s", service, signal_name);
    gpointer value = g_hash_table_lookup(proxy_info->signal_callback_id, signal_key);
    if (value == NULL)
    {
        g_print("can not find %s:%s\n", service, signal_name);
        g_free(signal_key);
        return FALSE;
    }

    g_signal_handler_disconnect(proxy_info->proxy, GPOINTER_TO_UINT(value));
    g_hash_table_remove(proxy_info->signal_callback_id, signal_key);

    g_free(signal_key);
    return TRUE;
}

static void _property_changed_thread_handle(gpointer data, gpointer user_data)
{
    PROXY_INFO_T *proxy_info = (PROXY_INFO_T *)user_data;
    GVariant *changed_properties = (GVariant *)data;

    GVariantIter iter;
    g_variant_iter_init(&iter, changed_properties);

    gchar *key = NULL;
    GVariant *value = NULL;
    while (g_variant_iter_next(&iter, "{sv}", &key, &value))
    {
        gchar *callback_key = g_strdup_printf("%s_%s", proxy_info->service_name, key);
        gpointer callback = g_hash_table_lookup(proxy_info->property_changed_callback, callback_key);
        if (callback != NULL)
        {
            ((property_changed_callback)callback)(value);
        }

        g_free(callback_key);
        g_variant_unref(value);
        g_free(key);
    }

    g_variant_unref(changed_properties);
}

static void _properties_changed(
    GDBusProxy *proxy, GVariant *changed_properties, const gchar *const *invalidated_properties)
{
    gchar *service_name = g_strdup_printf("%s", strrchr(g_dbus_proxy_get_object_path(proxy), '/') + 1);

    PROXY_INFO_T *proxy_info = (PROXY_INFO_T *)g_hash_table_lookup(g_service.proxy, service_name);
    if (proxy_info == NULL || proxy_info->pool == NULL)
    {
        error("can not find %s\n", service_name);
        g_free(service_name);
        return;
    }

    g_variant_ref(changed_properties);
    g_thread_pool_push(proxy_info->pool, changed_properties, NULL);

    g_free(service_name);
}

// 请确保 service 已经 register
gboolean bind_property_changed_callback(
    const gchar *service, const gchar *property_name, property_changed_callback callback)
{
    PROXY_INFO_T *proxy_info = (PROXY_INFO_T *)g_hash_table_lookup(g_service.proxy, service);
    if (proxy_info == NULL)
    {
        error("can not find %s\n", service);
        return FALSE;
    }

    gchar *key = g_strdup_printf("%s_%s", service, property_name);
    gpointer value = g_hash_table_lookup(proxy_info->property_changed_callback, key);
    if (value != NULL)
    {
        debug("The property %s callback for %s has been binded\n", signal_name, service);
        g_free(key);
        return FALSE;
    }

    if (proxy_info->is_have_monitor_property == FALSE)
    { // 如果还没有属性监听
        proxy_info->property_changed_signal_id
            = g_signal_connect(proxy_info->proxy, "g-properties-changed", G_CALLBACK(_properties_changed), NULL);
        proxy_info->is_have_monitor_property = TRUE;
        if (proxy_info->pool == NULL)
        {
            proxy_info->pool = g_thread_pool_new(_property_changed_thread_handle, proxy_info, 20, FALSE, NULL);
        }
    }

    g_hash_table_insert(proxy_info->property_changed_callback, g_strdup(key), (gpointer)callback);
    g_free(key);

    return TRUE;
}

gboolean unbind_property_changed_callback(const gchar *service, const gchar *property_name)
{
    PROXY_INFO_T *proxy_info = (PROXY_INFO_T *)g_hash_table_lookup(g_service.proxy, service);
    if (proxy_info == NULL)
    {
        error("can not find %s\n", service);
        return FALSE;
    }

    gchar *key = g_strdup_printf("%s_%s", service, property_name);
    gpointer value = g_hash_table_lookup(proxy_info->property_changed_callback, key);
    if (value == NULL)
    {
        error("can not find callback %s:%s\n", service, property_name);
        g_free(key);
        return FALSE;
    }

    g_hash_table_remove(proxy_info->property_changed_callback, key);
    g_free(key);

    if (g_hash_table_size(proxy_info->property_changed_callback) == 0) // 已经没有监听属性
    {
        if (proxy_info->is_have_monitor_property == TRUE)
        {
            g_signal_handler_disconnect(proxy_info->proxy, proxy_info->property_changed_signal_id);
            proxy_info->is_have_monitor_property = FALSE;
            g_thread_pool_free(proxy_info->pool, FALSE, TRUE);
            proxy_info->pool = NULL;
        }
    }

    return TRUE;
}

gpointer register_proxy(const gchar *service_name, proxy_new_sync call)
{
    PROXY_INFO_T *proxy_info = (PROXY_INFO_T *)g_hash_table_lookup(g_service.proxy, service_name);
    if (proxy_info != NULL)
    {
        debug("proxy %s already exist\n", service_name);
        return proxy_info->proxy;
    }

    gchar *service = g_strdup_printf(SERVICE_PREFIX, service_name);
    gchar *path = g_strdup_printf(PATH_PREFIX, service_name);
    GError *proxyerror = NULL;

    gpointer proxy = call(G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, service, path, NULL, &proxyerror);
    if (proxy == NULL)
    {
        error("Error creating proxy: %s\n", proxyerror->message);
        g_error_free(proxyerror);
        return NULL;
    }

    proxy_info = g_malloc0(sizeof(PROXY_INFO_T));
    proxy_info->is_have_monitor_property = FALSE;
    proxy_info->property_changed_signal_id = 0;
    proxy_info->proxy = proxy;
    proxy_info->signal_callback_id = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    proxy_info->property_changed_callback = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    proxy_info->pool = NULL;
    proxy_info->service_name = g_strdup(service_name);

    g_hash_table_insert(g_service.proxy, g_strdup(service_name), proxy_info);

    g_free(service);
    g_free(path);

    return proxy;
}

gpointer get_service_proxy_by_name(const gchar *service_name)
{
    PROXY_INFO_T *proxy_info = (PROXY_INFO_T *)g_hash_table_lookup(g_service.proxy, service_name);
    if (proxy_info == NULL)
    {
        error("can not find %s\n", service_name);
        return NULL;
    }

    return proxy_info->proxy;
}
