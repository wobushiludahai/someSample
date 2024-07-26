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
void service_exit(void);
gboolean bind_service_method_callback(const gchar *method_name, gpointer callback);
gboolean unbind_service_method_callback(const gchar *method_name);
gpointer register_proxy(const char *service_name, proxy_new_sync call);
gpointer get_service_proxy_by_name(const gchar *service_name);

#endif