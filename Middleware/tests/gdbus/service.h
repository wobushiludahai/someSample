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

typedef gpointer (*proxy_new_sync)(GBusType bus_type, GDBusProxyFlags flags, const gchar *name,
    const gchar *object_path, GCancellable *cancellable, GError **error);


void service_init(const char *service_name, gpointer interface);
gpointer get_service_instance(void);
void service_exit(void);
gboolean bind_service_method_callback(const gchar *method_name, gpointer callback);
gboolean unbind_service_method_callback(const gchar *method_name);
gpointer register_proxy(const char *service_name, proxy_new_sync call);
gpointer get_service_proxy_by_name(const gchar *service_name);
// 请确保 service 已经 register
gboolean bind_signal_callback(const gchar *service, const gchar *signal_name, gpointer callback);
gboolean unbind_signal_callback(const gchar *service, const gchar *signal_name);


typedef void (*property_changed_callback)(GVariant *property);
// 请确保 service 已经 register
gboolean bind_property_changed_callback(
    const gchar *service, const gchar *property_name, property_changed_callback callback);

gboolean unbind_property_changed_callback(const gchar *service, const gchar *property_name);

#endif