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
typedef void (*property_changed_callback)(GVariant *property);

/**
 * @brief      自身服务初始化
 *
 * @param[in]  service_name
 * @param[in]  instance
 */
void service_init(const char *service_name, gpointer instance);

/**
 * @brief      获取自身服务实例
 *
 * @return     gpointer
 */
gpointer get_service_instance(void);

/**
 * @brief      退出服务
 *
 */
void service_exit(void);

/**
 * @brief      绑定服务端方法回调
 *
 * @param[in]  method_name
 * @param[in]  callback
 * @return     gboolean
 */
gboolean bind_service_method_callback(const gchar *method_name, gpointer callback);

/**
 * @brief      解绑服务端的一个方法处理回调
 *
 * @param[in]  method_name
 * @return     gboolean
 */
gboolean unbind_service_method_callback(const gchar *method_name);

/**
 * @brief      绑定一个属性回调
 *             1、请确保对应service已经注册
 *             2、目前暂不支持监听自身信号
 *             3、请控制回调函数执行时间，防止出现大量执行排队
 *
 * @param[in]  service
 * @param[in]  signal_name
 * @param[in]  callback
 * @return     gboolean
 */
gboolean bind_signal_callback(const gchar *service, const gchar *signal_name, gpointer callback);

/**
 * @brief      解绑一个信号监听
 *
 * @param[in]  service
 * @param[in]  signal_name
 * @return     gboolean
 */
gboolean unbind_signal_callback(const gchar *service, const gchar *signal_name);

/**
 * @brief      绑定一个属性回调
 *             1、请确保对应service已经注册
 *             2、目前暂不支持监听自身属性
 *             3、请控制回调函数执行时间，防止出现大量执行排队
 *
 * @param[in]  service
 * @param[in]  property_name
 * @param[in]  callback
 * @return     gboolean
 */
gboolean bind_property_changed_callback(
    const gchar *service, const gchar *property_name, property_changed_callback callback);

/**
 * @brief      解绑一个属性回调
 *
 * @param[in]  service
 * @param[in]  property_name
 * @return     gboolean
 */
gboolean unbind_property_changed_callback(const gchar *service, const gchar *property_name);

/**
 * @brief      注册一个代理
 *
 * @param[in]  service_name
 * @param[in]  call
 * @return     gpointer
 */
gpointer register_proxy(const gchar *service_name, proxy_new_sync call);

/**
 * @brief      Get the service proxy by name object
 *
 * @param[in]  service_name
 * @return     gpointer
 */
gpointer get_service_proxy_by_name(const gchar *service_name);

G_END_DECLS

#endif