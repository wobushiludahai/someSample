#include "service.h"

#define SERVICE_PREFIX "hai.testdbus.%s"
#define PATH_PREFIX "/org/hai/%s"

#define info(fmt...)
#define error(fmt...)
#define debug(fmt, ...)

#define PROPERTY_CHANGED_HANDLE_THREAD_NUM (4)
typedef struct
{
    gpointer proxy;
    gchar *service_name;
    gchar *if_name; // interface name
    GThreadPool *pool;
    GHashTable *signal_callback_id;
    GHashTable *property_changed_callback;
    gboolean is_have_monitor_property; // 是否有属性被监听
    gulong property_changed_signal_id;
} PROXY_INFO_T;

typedef struct
{
    GHashTable *method_callback_id;
    gpointer instance;
} SERVER_INFO_T;

typedef struct
{
    guint owner_id;
    gchar *service_name;
    GDBusConnection *connection;
    GHashTable *proxy;
    GHashTable *server;
} SERVICE_T;

// 需要考虑多线程安全
static SERVICE_T g_service = { 0 };

/**
 * @brief      dbus请求成功回调
 *
 * @param[in]  connection
 * @param[in]  name
 * @param[in]  user_data
 */
static void on_bus_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    debug("on_bus_acquired %s\n", name);
    g_service.connection = connection;

    if (user_data != NULL)
    {
        service_register_success_call call_back = (service_register_success_call)user_data;
        call_back();
    }
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

    g_free(proxy_info->if_name);
    g_free(proxy_info->service_name);
    g_thread_pool_free(proxy_info->pool, FALSE, TRUE);

    g_free(proxy_info);
}

static void _destroy_server_hashtable(gpointer data)
{
    SERVER_INFO_T *server_info = (SERVER_INFO_T *)data;
    g_hash_table_destroy(server_info->method_callback_id);
    g_object_unref(server_info->instance);
}

/**
 * @brief      服务初始化
 *
 * @param[in]  service_name
 * @param[in]  callback
 */
void service_init(const char *service_name, service_register_success_call callback)
{
    debug("service_init %s\n", service_name);

    gchar *service = g_strdup_printf(SERVICE_PREFIX, service_name);

    g_service.owner_id = g_bus_own_name(G_BUS_TYPE_SESSION, service, G_BUS_NAME_OWNER_FLAGS_NONE, on_bus_acquired,
        on_name_acquired, on_name_lost, callback, NULL);
    g_service.service_name = g_strdup(service_name);
    g_service.proxy = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, _destory_proxy_hashtable);
    g_service.server = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, _destroy_server_hashtable);

    g_free(service);
}

/**
 * @brief      退出服务
 *
 */
void service_exit(void)
{
    g_free(g_service.service_name);
    g_hash_table_destroy(g_service.proxy);
    g_hash_table_destroy(g_service.server);
    g_bus_unown_name(g_service.owner_id);

    memset(&g_service, 0, sizeof(g_service));
}

/**
 * @brief      注册接口服务
 *
 * @param[in]  server_hash_name
 * @param[in]  call
 * @return     gboolean
 */
gboolean register_server(const gchar *server_hash_name, skelete_new call)
{
    if (g_service.connection == NULL || g_service.service_name == NULL)
    {
        g_print("g_service.connection or g_service.service_name is NULL\n");
        return FALSE;
    }

    if (call == NULL || server_hash_name == NULL)
    {
        g_print("call or server_hash_name is NULL\n");
        return FALSE;
    }

    SERVER_INFO_T *server_info = (SERVER_INFO_T *)g_hash_table_lookup(g_service.server, server_hash_name);
    if (server_info != NULL)
    {
        g_print("server %s already exist\n", server_hash_name);
        return FALSE;
    }

    gchar *path = g_strdup_printf(PATH_PREFIX, g_service.service_name);
    gpointer instance = call();

    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(instance), g_service.connection, path, NULL);
    g_free(path);

    server_info = g_malloc0(sizeof(SERVER_INFO_T));
    server_info->instance = instance;
    server_info->method_callback_id = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    g_hash_table_insert(g_service.server, g_strdup(server_hash_name), server_info);

    return TRUE;
}

/**
 * @brief      获取服务实例
 *
 * @param[in]  server_hash_name
 * @return     gpointer
 */
gpointer get_server_instance(const gchar *server_hash_name)
{
    SERVER_INFO_T *server_info = (SERVER_INFO_T *)g_hash_table_lookup(g_service.server, server_hash_name);
    if (server_info == NULL)
    {
        error("can not find %s\n", server_hash_name);
        return NULL;
    }

    return server_info->instance;
}

/**
 * @brief      绑定服务端方法回调
 *
 * @param[in]  server_hash_name
 * @param[in]  method_name
 * @param[in]  callback
 * @return     gboolean
 */
gboolean bind_service_method_callback(const gchar *server_hash_name, const gchar *method_name, gpointer callback)
{
    if (server_hash_name == NULL || method_name == NULL || callback == NULL)
    {
        g_print("server_hash_name or method_name or callback is NULL\n");
        return FALSE;
    }

    SERVER_INFO_T *server_info = (SERVER_INFO_T *)g_hash_table_lookup(g_service.server, server_hash_name);
    if (server_info == NULL)
    {
        g_print("can not find %s\n", server_hash_name);
        return FALSE;
    }

    gulong id = g_signal_connect(server_info->instance, method_name, G_CALLBACK(callback), NULL);

    g_print("bind_service_method_callback %s   %lu\n", method_name, id);

    g_hash_table_insert(server_info->method_callback_id, g_strdup(method_name), GUINT_TO_POINTER(id));

    return TRUE;
}

/**
 * @brief      解绑服务端的一个方法处理回调
 *
 * @param[in]  server_hash_name
 * @param[in]  method_name
 * @return     gboolean
 */
gboolean unbind_service_method_callback(const gchar *server_hash_name, const gchar *method_name)
{
    if (server_hash_name == NULL || method_name == NULL)
    {
        g_print("server_hash_name or method_name is NULL\n");
        return FALSE;
    }

    SERVER_INFO_T *server_info = (SERVER_INFO_T *)g_hash_table_lookup(g_service.server, server_hash_name);
    if (server_info == NULL)
    {
        g_print("can not find %s\n", server_hash_name);
        return FALSE;
    }

    gpointer value = g_hash_table_lookup(server_info->method_callback_id, method_name);
    if (value == NULL)
    {
        g_print("can not find %s\n", method_name);
        return FALSE;
    }

    g_signal_handler_disconnect(server_info->instance, GPOINTER_TO_UINT(value));
    g_print("unbind_service_method_callback %s   %u\n", method_name, GPOINTER_TO_UINT(value));

    g_hash_table_remove(server_info->method_callback_id, method_name);

    return TRUE;
}

/**
 * @brief      绑定一个信号回调
 *             1、请确保对应service已经注册
 *             2、目前暂不支持监听自身信号
 *             3、请控制回调函数执行时间，防止出现大量执行排队
 *
 * @param[in]  proxy_hash_name
 * @param[in]  signal_name
 * @param[in]  callback
 * @return     gboolean
 */
gboolean bind_signal_callback(const gchar *proxy_hash_name, const gchar *signal_name, gpointer callback)
{
    PROXY_INFO_T *proxy_info = (PROXY_INFO_T *)g_hash_table_lookup(g_service.proxy, proxy_hash_name);
    if (proxy_info == NULL)
    {
        error("can not find %s\n", proxy_hash_name);
        return FALSE;
    }

    gchar *signal_key = g_strdup_printf("%s_%s", proxy_hash_name, signal_name);
    gpointer value = g_hash_table_lookup(proxy_info->signal_callback_id, signal_key);
    if (value != NULL)
    {
        debug("The signal %s for %s has been binded\n", signal_name, proxy_hash_name);
        g_free(signal_key);
        return FALSE;
    }

    gulong id = g_signal_connect(proxy_info->proxy, signal_name, G_CALLBACK(callback), NULL);
    g_hash_table_insert(proxy_info->signal_callback_id, g_strdup(signal_key), GUINT_TO_POINTER(id));
    g_free(signal_key);

    return TRUE;
}

/**
 * @brief      解绑一个信号监听
 *
 * @param[in]  proxy_hash_name
 * @param[in]  signal_name
 * @return     gboolean
 */
gboolean unbind_signal_callback(const gchar *proxy_hash_name, const gchar *signal_name)
{
    PROXY_INFO_T *proxy_info = (PROXY_INFO_T *)g_hash_table_lookup(g_service.proxy, proxy_hash_name);
    if (proxy_info != NULL)
    {
        g_print("can not find %s\n", proxy_hash_name);
        return FALSE;
    }

    gchar *signal_key = g_strdup_printf("%s_%s", proxy_hash_name, signal_name);
    gpointer value = g_hash_table_lookup(proxy_info->signal_callback_id, signal_key);
    if (value == NULL)
    {
        g_print("can not find %s:%s\n", proxy_hash_name, signal_name);
        g_free(signal_key);
        return FALSE;
    }

    g_signal_handler_disconnect(proxy_info->proxy, GPOINTER_TO_UINT(value));
    g_hash_table_remove(proxy_info->signal_callback_id, signal_key);

    g_free(signal_key);
    return TRUE;
}

static void _property_changed_handle_thread_pool(gpointer data, gpointer user_data)
{
    PROXY_INFO_T *proxy_info = (PROXY_INFO_T *)user_data;
    GVariant *changed_properties = (GVariant *)data;

    GVariantIter iter;
    g_variant_iter_init(&iter, changed_properties);

    gchar *key = NULL;
    GVariant *value = NULL;
    while (g_variant_iter_next(&iter, "{sv}", &key, &value))
    {
        gchar *callback_key = g_strdup_printf("%s_%s", proxy_info->if_name, key);
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

static void _properties_changed(GDBusProxy *proxy, GVariant *changed_properties,
    const gchar *const *invalidated_properties, PROXY_INFO_T *proxy_info)
{
    g_variant_ref(changed_properties);
    g_thread_pool_push(proxy_info->pool, changed_properties, NULL);
}

/**
 * @brief      绑定一个属性回调
 *             1、请确保对应service已经注册
 *             2、目前暂不支持监听自身属性
 *             3、请控制回调函数执行时间，防止出现大量执行排队 *
 * @param[in]  proxy_hash_name
 * @param[in]  property_name
 * @param[in]  callback
 * @return     gboolean
 */
gboolean bind_property_changed_callback(
    const gchar *proxy_hash_name, const gchar *property_name, property_changed_callback callback)
{
    PROXY_INFO_T *proxy_info = (PROXY_INFO_T *)g_hash_table_lookup(g_service.proxy, proxy_hash_name);
    if (proxy_info == NULL)
    {
        error("can not find %s\n", proxy_hash_name);
        return FALSE;
    }

    gchar *key = g_strdup_printf("%s_%s", proxy_hash_name, property_name);
    gpointer value = g_hash_table_lookup(proxy_info->property_changed_callback, key);
    if (value != NULL)
    {
        debug("The property %s callback for %s has been binded\n", property_name, proxy_hash_name);
        g_free(key);
        return FALSE;
    }

    if (proxy_info->is_have_monitor_property == FALSE)
    { // 如果还没有属性监听
        proxy_info->property_changed_signal_id
            = g_signal_connect(proxy_info->proxy, "g-properties-changed", G_CALLBACK(_properties_changed), proxy_info);
        proxy_info->is_have_monitor_property = TRUE;
        if (proxy_info->pool == NULL)
        {
            proxy_info->pool = g_thread_pool_new(
                _property_changed_handle_thread_pool, proxy_info, PROPERTY_CHANGED_HANDLE_THREAD_NUM, FALSE, NULL);
        }
    }

    g_hash_table_insert(proxy_info->property_changed_callback, g_strdup(key), (gpointer)callback);
    g_free(key);

    return TRUE;
}

/**
 * @brief      解绑一个属性回调
 *
 * @param[in]  proxy_hash_name
 * @param[in]  property_name
 * @return     gboolean
 */
gboolean unbind_property_changed_callback(const gchar *proxy_hash_name, const gchar *property_name)
{
    PROXY_INFO_T *proxy_info = (PROXY_INFO_T *)g_hash_table_lookup(g_service.proxy, proxy_hash_name);
    if (proxy_info == NULL)
    {
        error("can not find %s\n", proxy_hash_name);
        return FALSE;
    }

    gchar *key = g_strdup_printf("%s_%s", proxy_hash_name, property_name);
    gpointer value = g_hash_table_lookup(proxy_info->property_changed_callback, key);
    if (value == NULL)
    {
        error("can not find callback %s:%s\n", proxy_hash_name, property_name);
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

/**
 * @brief      注册一个代理
 *
 * @param[in]  service_name
 * @param[in]  if_name
 * @param[in]  hash_name
 * @param[in]  call
 * @return     gpointer
 */
gpointer register_proxy(const gchar *service_name, const gchar *if_name, const gchar *hash_name, proxy_new_sync call)
{
    PROXY_INFO_T *proxy_info = (PROXY_INFO_T *)g_hash_table_lookup(g_service.proxy, hash_name);
    if (proxy_info != NULL)
    {
        debug("proxy %s already exist\n", hash_name);
        return proxy_info->proxy;
    }

    gchar *service = g_strdup_printf(SERVICE_PREFIX, service_name);
    gchar *path = g_strdup_printf(PATH_PREFIX, service_name);
    GError *proxyerror = NULL;

    gpointer proxy = call(G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, service, path, NULL, &proxyerror);
    g_free(service);
    g_free(path);
    if (proxy == NULL)
    {
        g_print("Error creating proxy: %s\n", proxyerror->message);
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
    proxy_info->if_name = g_strdup(if_name);

    g_hash_table_insert(g_service.proxy, g_strdup(hash_name), proxy_info);

    return proxy;
}

/**
 * @brief      获取一个代理实例
 *
 * @param[in]  hash_name
 * @return     gpointer
 */
gpointer get_proxy_instance(const gchar *hash_name)
{
    PROXY_INFO_T *proxy_info = (PROXY_INFO_T *)g_hash_table_lookup(g_service.proxy, hash_name);
    if (proxy_info == NULL)
    {
        g_print("can not find %s\n", hash_name);
        return NULL;
    }

    return proxy_info->proxy;
}