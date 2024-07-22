#ifndef __RPC_MIDDLEWARE_H
#define __RPC_MIDDLEWARE_H

#include <dbus/dbus.h>
#include <glib.h>

typedef struct RpcArgInfo RpcArgInfo;
typedef struct RpcMethodTable RpcMethodTable;
typedef struct RpcSignalTable RpcSignalTable;
typedef struct RpcPropertyTable RpcPropertyTable;
typedef struct RpcSecurityTable RpcSecurityTable;

typedef void (*RpcWatchFunction)(DBusConnection *connection, void *user_data);

typedef void (*RpcMessageFunction)(DBusConnection *connection, DBusMessage *message, void *user_data);

typedef gboolean (*RpcSignalFunction)(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusConnection *rpc_setup_bus(DBusBusType type, const char *name, DBusError *error);

DBusConnection *rpc_setup_private(DBusBusType type, const char *name, DBusError *error);

gboolean rpc_request_name(DBusConnection *connection, const char *name, DBusError *error);

gboolean rpc_set_disconnect_function(
    DBusConnection *connection, RpcWatchFunction function, void *user_data, DBusFreeFunction destroy);

typedef void (*RpcDestroyFunction)(void *user_data);

typedef DBusMessage *(*RpcMethodFunction)(DBusConnection *connection, DBusMessage *message, void *user_data);

typedef gboolean (*RpcPropertyGetter)(const RpcPropertyTable *property, DBusMessageIter *iter, void *data);

typedef guint32 RpcPendingPropertySet;

typedef void (*RpcPropertySetter)(
    const RpcPropertyTable *property, DBusMessageIter *value, RpcPendingPropertySet id, void *data);

typedef gboolean (*RpcPropertyExists)(const RpcPropertyTable *property, void *data);

typedef guint32 RpcPendingReply;

typedef void (*RpcSecurityFunction)(
    DBusConnection *connection, const char *action, gboolean interaction, RpcPendingReply pending);

enum RpcFlags
{
    RPC_FLAG_ENABLE_EXPERIMENTAL = (1 << 0),
};

enum RpcMethodFlags
{
    RPC_METHOD_FLAG_DEPRECATED = (1 << 0),
    RPC_METHOD_FLAG_NOREPLY = (1 << 1),
    RPC_METHOD_FLAG_ASYNC = (1 << 2),
    RPC_METHOD_FLAG_EXPERIMENTAL = (1 << 3),
};

enum RpcSignalFlags
{
    RPC_SIGNAL_FLAG_DEPRECATED = (1 << 0),
    RPC_SIGNAL_FLAG_EXPERIMENTAL = (1 << 1),
};

enum RpcPropertyFlags
{
    RPC_PROPERTY_FLAG_DEPRECATED = (1 << 0),
    RPC_PROPERTY_FLAG_EXPERIMENTAL = (1 << 1),
};

enum RpcSecurityFlags
{
    RPC_SECURITY_FLAG_DEPRECATED = (1 << 0),
    RPC_SECURITY_FLAG_BUILTIN = (1 << 1),
    RPC_SECURITY_FLAG_ALLOW_INTERACTION = (1 << 2),
};

enum RpcPropertyChangedFlags
{
    RPC_PROPERTY_CHANGED_FLAG_FLUSH = (1 << 0),
};

typedef enum RpcMethodFlags RpcMethodFlags;
typedef enum RpcSignalFlags RpcSignalFlags;
typedef enum RpcPropertyFlags RpcPropertyFlags;
typedef enum RpcSecurityFlags RpcSecurityFlags;
typedef enum RpcPropertyChangedFlags RpcPropertyChangedFlags;

struct RpcArgInfo
{
    const char *name;
    const char *signature;
};

struct RpcMethodTable
{
    const char *name;
    RpcMethodFunction function;
    RpcMethodFlags flags;
    unsigned int privilege;
    const RpcArgInfo *in_args;
    const RpcArgInfo *out_args;
};

struct RpcSignalTable
{
    const char *name;
    RpcSignalFlags flags;
    const RpcArgInfo *args;
};

struct RpcPropertyTable
{
    const char *name;
    const char *type;
    RpcPropertyGetter get;
    RpcPropertySetter set;
    RpcPropertyExists exists;
    RpcPropertyFlags flags;
};

struct RpcSecurityTable
{
    unsigned int privilege;
    const char *action;
    RpcSecurityFlags flags;
    RpcSecurityFunction function;
};

#define RPC_ARGS(args...)                                                                                              \
    (const RpcArgInfo[])                                                                                               \
    {                                                                                                                  \
        args,                                                                                                          \
        {                                                                                                              \
        }                                                                                                              \
    }

#define RPC_METHOD(_name, _in_args, _out_args, _function)                                                              \
    .name = _name, .in_args = _in_args, .out_args = _out_args, .function = _function

#define RPC_ASYNC_METHOD(_name, _in_args, _out_args, _function)                                                        \
    .name = _name, .in_args = _in_args, .out_args = _out_args, .function = _function, .flags = RPC_METHOD_FLAG_ASYNC

#define RPC_DEPRECATED_METHOD(_name, _in_args, _out_args, _function)                                                   \
    .name = _name, .in_args = _in_args, .out_args = _out_args, .function = _function,                                  \
    .flags = RPC_METHOD_FLAG_DEPRECATED

#define RPC_DEPRECATED_ASYNC_METHOD(_name, _in_args, _out_args, _function)                                             \
    .name = _name, .in_args = _in_args, .out_args = _out_args, .function = _function,                                  \
    .flags = RPC_METHOD_FLAG_ASYNC | RPC_METHOD_FLAG_DEPRECATED

#define RPC_EXPERIMENTAL_METHOD(_name, _in_args, _out_args, _function)                                                 \
    .name = _name, .in_args = _in_args, .out_args = _out_args, .function = _function,                                  \
    .flags = RPC_METHOD_FLAG_EXPERIMENTAL

#define RPC_EXPERIMENTAL_ASYNC_METHOD(_name, _in_args, _out_args, _function)                                           \
    .name = _name, .in_args = _in_args, .out_args = _out_args, .function = _function,                                  \
    .flags = RPC_METHOD_FLAG_ASYNC | RPC_METHOD_FLAG_EXPERIMENTAL

#define RPC_NOREPLY_METHOD(_name, _in_args, _out_args, _function)                                                      \
    .name = _name, .in_args = _in_args, .out_args = _out_args, .function = _function, .flags = RPC_METHOD_FLAG_NOREPLY

#define RPC_SIGNAL(_name, _args) .name = _name, .args = _args

void rpc_set_flags(int flags);
int rpc_get_flags(void);

gboolean rpc_register_interface(DBusConnection *connection, const char *path, const char *name,
    const RpcMethodTable *methods, const RpcSignalTable *signals, const RpcPropertyTable *properties, void *user_data,
    RpcDestroyFunction destroy);
gboolean rpc_unregister_interface(DBusConnection *connection, const char *path, const char *name);

gboolean rpc_register_security(const RpcSecurityTable *security);
gboolean rpc_unregister_security(const RpcSecurityTable *security);

void rpc_pending_success(DBusConnection *connection, RpcPendingReply pending);
void rpc_pending_error(DBusConnection *connection, RpcPendingReply pending, const char *name, const char *format, ...)
    __attribute__((format(printf, 4, 5)));
void rpc_pending_error_valist(
    DBusConnection *connection, RpcPendingReply pending, const char *name, const char *format, va_list args);

DBusMessage *rpc_create_error(DBusMessage *message, const char *name, const char *format, ...)
    __attribute__((format(printf, 3, 4)));
DBusMessage *rpc_create_error_valist(DBusMessage *message, const char *name, const char *format, va_list args);
DBusMessage *rpc_create_reply(DBusMessage *message, int type, ...);
DBusMessage *rpc_create_reply_valist(DBusMessage *message, int type, va_list args);

gboolean rpc_send_message(DBusConnection *connection, DBusMessage *message);
gboolean rpc_send_message_with_reply(
    DBusConnection *connection, DBusMessage *message, DBusPendingCall **call, int timeout);
gboolean rpc_send_error(DBusConnection *connection, DBusMessage *message, const char *name, const char *format, ...)
    __attribute__((format(printf, 4, 5)));
gboolean rpc_send_error_valist(
    DBusConnection *connection, DBusMessage *message, const char *name, const char *format, va_list args);
gboolean rpc_send_reply(DBusConnection *connection, DBusMessage *message, int type, ...);
gboolean rpc_send_reply_valist(DBusConnection *connection, DBusMessage *message, int type, va_list args);

gboolean rpc_emit_signal(
    DBusConnection *connection, const char *path, const char *interface, const char *name, int type, ...);
gboolean rpc_emit_signal_valist(
    DBusConnection *connection, const char *path, const char *interface, const char *name, int type, va_list args);

guint rpc_add_service_watch(DBusConnection *connection, const char *name, RpcWatchFunction connect,
    RpcWatchFunction disconnect, void *user_data, RpcDestroyFunction destroy);
guint rpc_add_disconnect_watch(DBusConnection *connection, const char *name, RpcWatchFunction function, void *user_data,
    RpcDestroyFunction destroy);
guint rpc_add_signal_watch(DBusConnection *connection, const char *sender, const char *path, const char *interface,
    const char *member, RpcSignalFunction function, void *user_data, RpcDestroyFunction destroy);
guint rpc_add_properties_watch(DBusConnection *connection, const char *sender, const char *path, const char *interface,
    RpcSignalFunction function, void *user_data, RpcDestroyFunction destroy);
gboolean rpc_remove_watch(DBusConnection *connection, guint tag);
void rpc_remove_all_watches(DBusConnection *connection);

void rpc_pending_property_success(RpcPendingPropertySet id);
void rpc_pending_property_error_valist(RpcPendingReply id, const char *name, const char *format, va_list args);
void rpc_pending_property_error(RpcPendingReply id, const char *name, const char *format, ...);
void rpc_emit_property_changed(DBusConnection *connection, const char *path, const char *interface, const char *name);
void rpc_emit_property_changed_full(DBusConnection *connection, const char *path, const char *interface,
    const char *name, RpcPropertyChangedFlags flags);
gboolean rpc_get_properties(DBusConnection *connection, const char *path, const char *interface, DBusMessageIter *iter);

gboolean rpc_attach_object_manager(DBusConnection *connection);
gboolean rpc_detach_object_manager(DBusConnection *connection);

typedef struct RpcClient RpcClient;
typedef struct RpcProxy RpcProxy;

RpcProxy *rpc_proxy_new(RpcClient *client, const char *path, const char *interface);

RpcProxy *rpc_proxy_ref(RpcProxy *proxy);
void rpc_proxy_unref(RpcProxy *proxy);

const char *rpc_proxy_get_path(RpcProxy *proxy);
const char *rpc_proxy_get_interface(RpcProxy *proxy);

gboolean rpc_proxy_get_property(RpcProxy *proxy, const char *name, DBusMessageIter *iter);

gboolean rpc_proxy_refresh_property(RpcProxy *proxy, const char *name);

typedef void (*RpcResultFunction)(const DBusError *error, void *user_data);

gboolean rpc_proxy_set_property_basic(RpcProxy *proxy, const char *name, int type, const void *value,
    RpcResultFunction function, void *user_data, RpcDestroyFunction destroy);

gboolean rpc_proxy_set_property_array(RpcProxy *proxy, const char *name, int type, const void *value, size_t size,
    RpcResultFunction function, void *user_data, RpcDestroyFunction destroy);

typedef void (*RpcSetupFunction)(DBusMessageIter *iter, void *user_data);
typedef void (*RpcReturnFunction)(DBusMessage *message, void *user_data);

gboolean rpc_proxy_method_call(RpcProxy *proxy, const char *method, RpcSetupFunction setup, RpcReturnFunction function,
    void *user_data, RpcDestroyFunction destroy);

typedef void (*RpcClientFunction)(RpcClient *client, void *user_data);
typedef void (*RpcProxyFunction)(RpcProxy *proxy, void *user_data);
typedef void (*RpcPropertyFunction)(RpcProxy *proxy, const char *name, DBusMessageIter *iter, void *user_data);

gboolean rpc_proxy_set_property_watch(RpcProxy *proxy, RpcPropertyFunction function, void *user_data);

gboolean rpc_proxy_set_removed_watch(RpcProxy *proxy, RpcProxyFunction destroy, void *user_data);

RpcClient *rpc_client_new(DBusConnection *connection, const char *service, const char *path);
RpcClient *rpc_client_new_full(
    DBusConnection *connection, const char *service, const char *path, const char *root_path);

RpcClient *rpc_client_ref(RpcClient *client);
void rpc_client_unref(RpcClient *client);

gboolean rpc_client_set_connect_watch(RpcClient *client, RpcWatchFunction function, void *user_data);
gboolean rpc_client_set_disconnect_watch(RpcClient *client, RpcWatchFunction function, void *user_data);
gboolean rpc_client_set_signal_watch(RpcClient *client, RpcMessageFunction function, void *user_data);
gboolean rpc_client_set_ready_watch(RpcClient *client, RpcClientFunction ready, void *user_data);
gboolean rpc_client_set_proxy_handlers(RpcClient *client, RpcProxyFunction proxy_added, RpcProxyFunction proxy_removed,
    RpcPropertyFunction property_changed, void *user_data);

#endif /* __RPC_MIDDLEWARE_H */
