#ifndef __DBUS_SERVICE_H__
#define __DBUS_SERVICE_H__

#include <gio/gio.h>
#include <glib.h>

#ifdef G_OS_UNIX
#include <gio/gunixfdlist.h>
/* For STDOUT_FILENO */
#include <unistd.h>
#endif

#include <pthread.h>
#include <semaphore.h>

G_BEGIN_DECLS

typedef gpointer (*proxy_new_sync)(GBusType bus_type, GDBusProxyFlags flags, const gchar *name,
    const gchar *object_path, GCancellable *cancellable, GError **error);
typedef void (*proxy_property_changed_callback)(GVariant *property);

// 返回值为 true则执行后续修改动作，返回值为false则不会去执行后续修改动作
typedef gboolean (*server_property_before_change_callback)(gpointer value);
// 值不同，且被修改后触发回调
typedef void (*server_property_after_changed_callback)(gpointer value);

typedef void (*property_changed_callback)(GVariant *property);
typedef gpointer (*skelete_new)(void);
typedef void (*service_register_success_call)(void);

/**
 * @brief      服务初始化
 *
 * @param[in]  service_name
 * @param[in]  callback
 */
void service_init(const char *service_name, service_register_success_call callback);

/**
 * @brief      退出服务
 *
 */
void service_exit(void);

/**
 * @brief      注册接口服务
 *
 * @param[in]  server_hash_name
 * @param[in]  call
 * @return     gboolean
 */
gboolean register_server(const gchar *server_hash_name, skelete_new call);

/**
 * @brief      获取服务实例
 *
 * @param[in]  server_hash_name
 * @return     gpointer
 */
gpointer get_server_instance(const gchar *server_hash_name);

/**
 * @brief      绑定服务端方法回调
 *
 * @param[in]  server_hash_name
 * @param[in]  method_name
 * @param[in]  callback
 * @return     gboolean
 */
gboolean bind_service_method_callback(const gchar *server_hash_name, const gchar *method_name, gpointer callback);

/**
 * @brief      解绑服务端的一个方法处理回调
 *
 * @param[in]  server_hash_name
 * @param[in]  method_name
 * @return     gboolean
 */
gboolean unbind_service_method_callback(const gchar *server_hash_name, const gchar *method_name);

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
gboolean bind_signal_callback(const gchar *proxy_hash_name, const gchar *signal_name, gpointer callback);

/**
 * @brief      解绑一个信号监听
 *
 * @param[in]  proxy_hash_name
 * @param[in]  signal_name
 * @return     gboolean
 */
gboolean unbind_signal_callback(const gchar *proxy_hash_name, const gchar *signal_name);

/**
 * @brief      绑定一个属性回调
 *             1、请确保对应service已经注册
 *             2、请控制回调函数执行时间，防止出现大量执行排队 *
 * @param[in]  proxy_hash_name
 * @param[in]  property_name
 * @param[in]  callback
 * @return     gboolean
 */
gboolean bind_proxy_property_changed_callback(
    const gchar *proxy_hash_name, const gchar *property_name, proxy_property_changed_callback callback);

/**
 * @brief      解绑一个属性回调
 *
 * @param[in]  proxy_hash_name
 * @param[in]  property_name
 * @return     gboolean
 */
gboolean unbind_proxy_property_changed_callback(const gchar *proxy_hash_name, const gchar *property_name);

/**
 * @brief      注册一个代理
 *
 * @param[in]  service_name
 * @param[in]  if_name
 * @param[in]  hash_name
 * @param[in]  call
 * @return     gpointer
 */
gpointer register_proxy(const gchar *service_name, const gchar *if_name, const gchar *hash_name, proxy_new_sync call);

/**
 * @brief      获取一个代理实例
 *
 * @param[in]  hash_name
 * @return     gpointer
 */
gpointer get_proxy_instance(const gchar *hash_name);

G_END_DECLS

#endif