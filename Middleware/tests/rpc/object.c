#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <dbus/dbus.h>
#include "rpc_middleware.h"

#define info(fmt...)
#define error(fmt...)
#define debug(fmt...)

#define DBUS_INTERFACE_OBJECT_MANAGER "org.freedesktop.DBus.ObjectManager"

#ifndef DBUS_ERROR_UNKNOWN_PROPERTY
#define DBUS_ERROR_UNKNOWN_PROPERTY "org.freedesktop.DBus.Error.UnknownProperty"
#endif

#ifndef DBUS_ERROR_PROPERTY_READ_ONLY
#define DBUS_ERROR_PROPERTY_READ_ONLY "org.freedesktop.DBus.Error.PropertyReadOnly"
#endif

struct generic_data
{
    unsigned int refcount;
    DBusConnection *conn;
    char *path;
    GSList *interfaces;
    GSList *objects;
    GSList *added;
    GSList *removed;
    guint process_id;
    gboolean pending_prop;
    char *introspect;
    struct generic_data *parent;
};

struct interface_data
{
    char *name;
    const RpcMethodTable *methods;
    const RpcSignalTable *signals;
    const RpcPropertyTable *properties;
    GSList *pending_prop;
    void *user_data;
    RpcDestroyFunction destroy;
};

struct security_data
{
    RpcPendingReply pending;
    DBusMessage *message;
    const RpcMethodTable *method;
    void *iface_user_data;
};

struct property_data
{
    DBusConnection *conn;
    RpcPendingPropertySet id;
    DBusMessage *message;
};

static int global_flags = 0;
static struct generic_data *root;
static GSList *pending = NULL;

static gboolean process_changes(gpointer user_data);
static void process_properties_from_interface(struct generic_data *data, struct interface_data *iface);
static void process_property_changes(struct generic_data *data);

static void print_arguments(GString *gstr, const RpcArgInfo *args, const char *direction)
{
    for (; args && args->name; args++)
    {
        g_string_append_printf(gstr, "<arg name=\"%s\" type=\"%s\"", args->name, args->signature);

        if (direction)
            g_string_append_printf(gstr, " direction=\"%s\"/>\n", direction);
        else
            g_string_append_printf(gstr, "/>\n");
    }
}

#define RPC_ANNOTATE(name_, value_)                                                                                    \
    "<annotation name=\"org.freedesktop.DBus." name_ "\" "                                                             \
    "value=\"" value_ "\"/>"

#define RPC_ANNOTATE_DEPRECATED RPC_ANNOTATE("Deprecated", "true")

#define RPC_ANNOTATE_NOREPLY RPC_ANNOTATE("Method.NoReply", "true")

static gboolean check_experimental(int flags, int flag)
{
    if (!(flags & flag))
        return FALSE;

    return !(global_flags & RPC_FLAG_ENABLE_EXPERIMENTAL);
}

static void generate_interface_xml(GString *gstr, struct interface_data *iface)
{
    const RpcMethodTable *method;
    const RpcSignalTable *signal;
    const RpcPropertyTable *property;

    for (method = iface->methods; method && method->name; method++)
    {
        if (check_experimental(method->flags, RPC_METHOD_FLAG_EXPERIMENTAL))
            continue;

        g_string_append_printf(gstr, "<method name=\"%s\">", method->name);
        print_arguments(gstr, method->in_args, "in");
        print_arguments(gstr, method->out_args, "out");

        if (method->flags & RPC_METHOD_FLAG_DEPRECATED)
            g_string_append_printf(gstr, RPC_ANNOTATE_DEPRECATED);

        if (method->flags & RPC_METHOD_FLAG_NOREPLY)
            g_string_append_printf(gstr, RPC_ANNOTATE_NOREPLY);

        g_string_append_printf(gstr, "</method>");
    }

    for (signal = iface->signals; signal && signal->name; signal++)
    {
        if (check_experimental(signal->flags, RPC_SIGNAL_FLAG_EXPERIMENTAL))
            continue;

        g_string_append_printf(gstr, "<signal name=\"%s\">", signal->name);
        print_arguments(gstr, signal->args, NULL);

        if (signal->flags & RPC_SIGNAL_FLAG_DEPRECATED)
            g_string_append_printf(gstr, RPC_ANNOTATE_DEPRECATED);

        g_string_append_printf(gstr, "</signal>\n");
    }

    for (property = iface->properties; property && property->name; property++)
    {
        if (check_experimental(property->flags, RPC_PROPERTY_FLAG_EXPERIMENTAL))
            continue;

        g_string_append_printf(gstr,
            "<property name=\"%s\""
            " type=\"%s\" access=\"%s%s\">",
            property->name, property->type, property->get ? "read" : "", property->set ? "write" : "");

        if (property->flags & RPC_PROPERTY_FLAG_DEPRECATED)
            g_string_append_printf(gstr, RPC_ANNOTATE_DEPRECATED);

        g_string_append_printf(gstr, "</property>");
    }
}

static void generate_introspection_xml(DBusConnection *conn, struct generic_data *data, const char *path)
{
    GSList *list;
    GString *gstr;
    char **children;
    int i;

    g_free(data->introspect);

    gstr = g_string_new(DBUS_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE);

    g_string_append_printf(gstr, "<node>");

    for (list = data->interfaces; list; list = list->next)
    {
        struct interface_data *iface = list->data;

        g_string_append_printf(gstr, "<interface name=\"%s\">", iface->name);

        generate_interface_xml(gstr, iface);

        g_string_append_printf(gstr, "</interface>");
    }

    if (!dbus_connection_list_registered(conn, path, &children))
        goto done;

    for (i = 0; children[i]; i++)
        g_string_append_printf(gstr, "<node name=\"%s\"/>", children[i]);

    dbus_free_string_array(children);

done:
    g_string_append_printf(gstr, "</node>");

    data->introspect = g_string_free(gstr, FALSE);
}

static DBusMessage *introspect(DBusConnection *connection, DBusMessage *message, void *user_data)
{
    struct generic_data *data = user_data;
    DBusMessage *reply;

    if (data->introspect == NULL)
        generate_introspection_xml(connection, data, dbus_message_get_path(message));

    reply = dbus_message_new_method_return(message);
    if (reply == NULL)
        return NULL;

    dbus_message_append_args(reply, DBUS_TYPE_STRING, &data->introspect, DBUS_TYPE_INVALID);

    return reply;
}

static DBusHandlerResult process_message(
    DBusConnection *connection, DBusMessage *message, const RpcMethodTable *method, void *iface_user_data)
{
    DBusMessage *reply;

    reply = method->function(connection, message, iface_user_data);

    if (method->flags & RPC_METHOD_FLAG_NOREPLY || dbus_message_get_no_reply(message))
    {
        if (reply != NULL)
            dbus_message_unref(reply);
        return DBUS_HANDLER_RESULT_HANDLED;
    }

    if (method->flags & RPC_METHOD_FLAG_ASYNC)
    {
        if (reply == NULL)
            return DBUS_HANDLER_RESULT_HANDLED;
    }

    if (reply == NULL)
        return DBUS_HANDLER_RESULT_NEED_MEMORY;

    rpc_send_message(connection, reply);

    return DBUS_HANDLER_RESULT_HANDLED;
}

static RpcPendingReply next_pending = 1;
static GSList *pending_security = NULL;

static const RpcSecurityTable *security_table = NULL;

void rpc_pending_success(DBusConnection *connection, RpcPendingReply pending)
{
    GSList *list;

    for (list = pending_security; list; list = list->next)
    {
        struct security_data *secdata = list->data;

        if (secdata->pending != pending)
            continue;

        pending_security = g_slist_remove(pending_security, secdata);

        process_message(connection, secdata->message, secdata->method, secdata->iface_user_data);

        dbus_message_unref(secdata->message);
        g_free(secdata);
        return;
    }
}

void rpc_pending_error_valist(
    DBusConnection *connection, RpcPendingReply pending, const char *name, const char *format, va_list args)
{
    GSList *list;

    for (list = pending_security; list; list = list->next)
    {
        struct security_data *secdata = list->data;

        if (secdata->pending != pending)
            continue;

        pending_security = g_slist_remove(pending_security, secdata);

        rpc_send_error_valist(connection, secdata->message, name, format, args);

        dbus_message_unref(secdata->message);
        g_free(secdata);
        return;
    }
}

void rpc_pending_error(DBusConnection *connection, RpcPendingReply pending, const char *name, const char *format, ...)
{
    va_list args;

    va_start(args, format);

    rpc_pending_error_valist(connection, pending, name, format, args);

    va_end(args);
}

int polkit_check_authorization(DBusConnection *conn, const char *action, gboolean interaction,
    void (*function)(dbus_bool_t authorized, void *user_data), void *user_data, int timeout);

struct builtin_security_data
{
    DBusConnection *conn;
    RpcPendingReply pending;
};

static void builtin_security_result(dbus_bool_t authorized, void *user_data)
{
    struct builtin_security_data *data = user_data;

    if (authorized == TRUE)
        rpc_pending_success(data->conn, data->pending);
    else
        rpc_pending_error(data->conn, data->pending, DBUS_ERROR_AUTH_FAILED, NULL);

    g_free(data);
}

static void builtin_security_function(
    DBusConnection *conn, const char *action, gboolean interaction, RpcPendingReply pending)
{
    struct builtin_security_data *data;

    data = g_new0(struct builtin_security_data, 1);
    data->conn = conn;
    data->pending = pending;

    if (polkit_check_authorization(conn, action, interaction, builtin_security_result, data, 30000) < 0)
        rpc_pending_error(conn, pending, NULL, NULL);
}

static gboolean check_privilege(
    DBusConnection *conn, DBusMessage *msg, const RpcMethodTable *method, void *iface_user_data)
{
    const RpcSecurityTable *security;

    for (security = security_table; security && security->privilege; security++)
    {
        struct security_data *secdata;
        gboolean interaction;

        if (security->privilege != method->privilege)
            continue;

        secdata = g_new(struct security_data, 1);
        secdata->pending = next_pending++;
        secdata->message = dbus_message_ref(msg);
        secdata->method = method;
        secdata->iface_user_data = iface_user_data;

        pending_security = g_slist_prepend(pending_security, secdata);

        if (security->flags & RPC_SECURITY_FLAG_ALLOW_INTERACTION)
            interaction = TRUE;
        else
            interaction = FALSE;

        if (!(security->flags & RPC_SECURITY_FLAG_BUILTIN) && security->function)
            security->function(conn, security->action, interaction, secdata->pending);
        else
            builtin_security_function(conn, security->action, interaction, secdata->pending);

        return TRUE;
    }

    return FALSE;
}

static RpcPendingPropertySet next_pending_property = 1;
static GSList *pending_property_set;

static struct property_data *remove_pending_property_data(RpcPendingPropertySet id)
{
    struct property_data *propdata;
    GSList *l;

    for (l = pending_property_set; l != NULL; l = l->next)
    {
        propdata = l->data;
        if (propdata->id != id)
            continue;

        break;
    }

    if (l == NULL)
        return NULL;

    pending_property_set = g_slist_delete_link(pending_property_set, l);

    return propdata;
}

void rpc_pending_property_success(RpcPendingPropertySet id)
{
    struct property_data *propdata;

    propdata = remove_pending_property_data(id);
    if (propdata == NULL)
        return;

    rpc_send_reply(propdata->conn, propdata->message, DBUS_TYPE_INVALID);
    dbus_message_unref(propdata->message);
    g_free(propdata);
}

void rpc_pending_property_error_valist(RpcPendingReply id, const char *name, const char *format, va_list args)
{
    struct property_data *propdata;

    propdata = remove_pending_property_data(id);
    if (propdata == NULL)
        return;

    rpc_send_error_valist(propdata->conn, propdata->message, name, format, args);

    dbus_message_unref(propdata->message);
    g_free(propdata);
}

void rpc_pending_property_error(RpcPendingReply id, const char *name, const char *format, ...)
{
    va_list args;

    va_start(args, format);

    rpc_pending_property_error_valist(id, name, format, args);

    va_end(args);
}

static void reset_parent(gpointer data, gpointer user_data)
{
    struct generic_data *child = data;
    struct generic_data *parent = user_data;

    child->parent = parent;
}

static void append_property(struct interface_data *iface, const RpcPropertyTable *p, DBusMessageIter *dict)
{
    DBusMessageIter entry, value;

    dbus_message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &p->name);
    dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, p->type, &value);

    p->get(p, &value, iface->user_data);

    dbus_message_iter_close_container(&entry, &value);
    dbus_message_iter_close_container(dict, &entry);
}

static void append_properties(struct interface_data *data, DBusMessageIter *iter)
{
    DBusMessageIter dict;
    const RpcPropertyTable *p;

    dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
        DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_VARIANT_AS_STRING
            DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
        &dict);

    for (p = data->properties; p && p->name; p++)
    {
        if (check_experimental(p->flags, RPC_PROPERTY_FLAG_EXPERIMENTAL))
            continue;

        if (p->get == NULL)
            continue;

        if (p->exists != NULL && !p->exists(p, data->user_data))
            continue;

        append_property(data, p, &dict);
    }

    dbus_message_iter_close_container(iter, &dict);
}

static void append_interface(gpointer data, gpointer user_data)
{
    struct interface_data *iface = data;
    DBusMessageIter *array = user_data;
    DBusMessageIter entry;

    dbus_message_iter_open_container(array, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &iface->name);
    append_properties(data, &entry);
    dbus_message_iter_close_container(array, &entry);
}

static void emit_interfaces_added(struct generic_data *data)
{
    DBusMessage *signal;
    DBusMessageIter iter, array;

    if (root == NULL || data == root)
        return;

    signal = dbus_message_new_signal(root->path, DBUS_INTERFACE_OBJECT_MANAGER, "InterfacesAdded");
    if (signal == NULL)
        return;

    dbus_message_iter_init_append(signal, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_OBJECT_PATH, &data->path);

    dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,
        DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_ARRAY_AS_STRING
            DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_VARIANT_AS_STRING
                DBUS_DICT_ENTRY_END_CHAR_AS_STRING DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
        &array);

    g_slist_foreach(data->added, append_interface, &array);
    g_slist_free(data->added);
    data->added = NULL;

    dbus_message_iter_close_container(&iter, &array);

    /* Use dbus_connection_send to avoid recursive calls to rpc_flush */
    dbus_connection_send(data->conn, signal, NULL);
    dbus_message_unref(signal);
}

static struct interface_data *find_interface(GSList *interfaces, const char *name)
{
    GSList *list;

    if (name == NULL)
        return NULL;

    for (list = interfaces; list; list = list->next)
    {
        struct interface_data *iface = list->data;
        if (!strcmp(name, iface->name))
            return iface;
    }

    return NULL;
}

static gboolean rpc_args_have_signature(const RpcArgInfo *args, DBusMessage *message)
{
    const char *sig = dbus_message_get_signature(message);
    const char *p = NULL;

    for (; args && args->signature && *sig; args++)
    {
        p = args->signature;

        for (; *sig && *p; sig++, p++)
        {
            if (*p != *sig)
                return FALSE;
        }
    }

    if (*sig || (p && *p) || (args && args->signature))
        return FALSE;

    return TRUE;
}

static void add_pending(struct generic_data *data)
{
    if (data->process_id > 0)
        return;

    data->process_id = g_idle_add(process_changes, data);

    pending = g_slist_append(pending, data);
}

static gboolean remove_interface(struct generic_data *data, const char *name)
{
    struct interface_data *iface;

    iface = find_interface(data->interfaces, name);
    if (iface == NULL)
        return FALSE;

    process_properties_from_interface(data, iface);

    data->interfaces = g_slist_remove(data->interfaces, iface);

    if (iface->destroy)
    {
        iface->destroy(iface->user_data);
        iface->user_data = NULL;
    }

    /*
     * Interface being removed was just added, on the same mainloop
     * iteration? Don't send any signal
     */
    if (g_slist_find(data->added, iface))
    {
        data->added = g_slist_remove(data->added, iface);
        g_free(iface->name);
        g_free(iface);
        return TRUE;
    }

    if (data->parent == NULL)
    {
        g_free(iface->name);
        g_free(iface);
        return TRUE;
    }

    data->removed = g_slist_prepend(data->removed, iface->name);
    g_free(iface);

    add_pending(data);

    return TRUE;
}

static struct generic_data *invalidate_parent_data(DBusConnection *conn, const char *child_path)
{
    struct generic_data *data = NULL, *child = NULL, *parent = NULL;
    char *parent_path, *slash;

    parent_path = g_strdup(child_path);
    slash = strrchr(parent_path, '/');
    if (slash == NULL)
        goto done;

    if (slash == parent_path && parent_path[1] != '\0')
        parent_path[1] = '\0';
    else
        *slash = '\0';

    if (!strlen(parent_path))
        goto done;

    if (dbus_connection_get_object_path_data(conn, parent_path, (void *)&data) == FALSE)
    {
        goto done;
    }

    parent = invalidate_parent_data(conn, parent_path);

    if (data == NULL)
    {
        data = parent;
        if (data == NULL)
            goto done;
    }

    g_free(data->introspect);
    data->introspect = NULL;

    if (!dbus_connection_get_object_path_data(conn, child_path, (void *)&child))
        goto done;

    if (child == NULL || g_slist_find(data->objects, child) != NULL)
        goto done;

    data->objects = g_slist_prepend(data->objects, child);
    child->parent = data;

done:
    g_free(parent_path);
    return data;
}

static inline const RpcPropertyTable *find_property(const RpcPropertyTable *properties, const char *name)
{
    const RpcPropertyTable *p;

    for (p = properties; p && p->name; p++)
    {
        if (strcmp(name, p->name) != 0)
            continue;

        if (check_experimental(p->flags, RPC_PROPERTY_FLAG_EXPERIMENTAL))
            break;

        return p;
    }

    return NULL;
}

static DBusMessage *properties_get(DBusConnection *connection, DBusMessage *message, void *user_data)
{
    struct generic_data *data = user_data;
    struct interface_data *iface;
    const RpcPropertyTable *property;
    const char *interface, *name;
    DBusMessageIter iter, value;
    DBusMessage *reply;

    if (!dbus_message_get_args(message, NULL, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID))
        return NULL;

    iface = find_interface(data->interfaces, interface);
    if (iface == NULL)
        return rpc_create_error(message, DBUS_ERROR_INVALID_ARGS, "No such interface '%s'", interface);

    property = find_property(iface->properties, name);
    if (property == NULL)
        return rpc_create_error(message, DBUS_ERROR_INVALID_ARGS, "No such property '%s'", name);

    if (property->exists != NULL && !property->exists(property, iface->user_data))
        return rpc_create_error(message, DBUS_ERROR_INVALID_ARGS, "No such property '%s'", name);

    if (property->get == NULL)
        return rpc_create_error(message, DBUS_ERROR_INVALID_ARGS, "Property '%s' is not readable", name);

    reply = dbus_message_new_method_return(message);
    if (reply == NULL)
        return NULL;

    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT, property->type, &value);

    if (!property->get(property, &value, iface->user_data))
    {
        dbus_message_unref(reply);
        return NULL;
    }

    dbus_message_iter_close_container(&iter, &value);

    return reply;
}

static DBusMessage *properties_get_all(DBusConnection *connection, DBusMessage *message, void *user_data)
{
    struct generic_data *data = user_data;
    struct interface_data *iface;
    const char *interface;
    DBusMessageIter iter;
    DBusMessage *reply;

    if (!dbus_message_get_args(message, NULL, DBUS_TYPE_STRING, &interface, DBUS_TYPE_INVALID))
        return NULL;

    iface = find_interface(data->interfaces, interface);
    if (iface == NULL)
        return rpc_create_error(message, DBUS_ERROR_INVALID_ARGS, "No such interface '%s'", interface);

    reply = dbus_message_new_method_return(message);
    if (reply == NULL)
        return NULL;

    dbus_message_iter_init_append(reply, &iter);

    append_properties(iface, &iter);

    return reply;
}

static DBusMessage *properties_set(DBusConnection *connection, DBusMessage *message, void *user_data)
{
    struct generic_data *data = user_data;
    DBusMessageIter iter, sub;
    struct interface_data *iface;
    const RpcPropertyTable *property;
    const char *name, *interface;
    struct property_data *propdata;
    gboolean valid_signature;
    char *signature;

    if (!dbus_message_iter_init(message, &iter))
        return rpc_create_error(message, DBUS_ERROR_INVALID_ARGS, "No arguments given");

    if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_STRING)
        return rpc_create_error(
            message, DBUS_ERROR_INVALID_ARGS, "Invalid argument type: '%c'", dbus_message_iter_get_arg_type(&iter));

    dbus_message_iter_get_basic(&iter, &interface);
    dbus_message_iter_next(&iter);

    if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_STRING)
        return rpc_create_error(
            message, DBUS_ERROR_INVALID_ARGS, "Invalid argument type: '%c'", dbus_message_iter_get_arg_type(&iter));

    dbus_message_iter_get_basic(&iter, &name);
    dbus_message_iter_next(&iter);

    if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT)
        return rpc_create_error(
            message, DBUS_ERROR_INVALID_ARGS, "Invalid argument type: '%c'", dbus_message_iter_get_arg_type(&iter));

    dbus_message_iter_recurse(&iter, &sub);

    iface = find_interface(data->interfaces, interface);
    if (iface == NULL)
        return rpc_create_error(message, DBUS_ERROR_INVALID_ARGS, "No such interface '%s'", interface);

    property = find_property(iface->properties, name);
    if (property == NULL)
        return rpc_create_error(message, DBUS_ERROR_UNKNOWN_PROPERTY, "No such property '%s'", name);

    if (property->set == NULL)
        return rpc_create_error(message, DBUS_ERROR_PROPERTY_READ_ONLY, "Property '%s' is not writable", name);

    if (property->exists != NULL && !property->exists(property, iface->user_data))
        return rpc_create_error(message, DBUS_ERROR_UNKNOWN_PROPERTY, "No such property '%s'", name);

    signature = dbus_message_iter_get_signature(&sub);
    valid_signature = strcmp(signature, property->type) ? FALSE : TRUE;
    dbus_free(signature);
    if (!valid_signature)
        return rpc_create_error(message, DBUS_ERROR_INVALID_SIGNATURE, "Invalid signature for '%s'", name);

    propdata = g_new(struct property_data, 1);
    propdata->id = next_pending_property++;
    propdata->message = dbus_message_ref(message);
    propdata->conn = connection;
    pending_property_set = g_slist_prepend(pending_property_set, propdata);

    property->set(property, &sub, propdata->id, iface->user_data);

    return NULL;
}

static const RpcMethodTable properties_methods[] = {
    { RPC_METHOD("Get", RPC_ARGS({ "interface", "s" }, { "name", "s" }), RPC_ARGS({ "value", "v" }), properties_get) },
    { RPC_ASYNC_METHOD(
        "Set", RPC_ARGS({ "interface", "s" }, { "name", "s" }, { "value", "v" }), NULL, properties_set) },
    { RPC_METHOD("GetAll", RPC_ARGS({ "interface", "s" }), RPC_ARGS({ "properties", "a{sv}" }), properties_get_all) },
    {}
};

static const RpcSignalTable properties_signals[]
    = { { RPC_SIGNAL("PropertiesChanged",
            RPC_ARGS({ "interface", "s" }, { "changed_properties", "a{sv}" }, { "invalidated_properties", "as" })) },
          {} };

static void append_name(gpointer data, gpointer user_data)
{
    char *name = data;
    DBusMessageIter *iter = user_data;

    dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &name);
}

static void emit_interfaces_removed(struct generic_data *data)
{
    DBusMessage *signal;
    DBusMessageIter iter, array;

    if (root == NULL || data == root)
        return;

    signal = dbus_message_new_signal(root->path, DBUS_INTERFACE_OBJECT_MANAGER, "InterfacesRemoved");
    if (signal == NULL)
        return;

    dbus_message_iter_init_append(signal, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_OBJECT_PATH, &data->path);
    dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &array);

    g_slist_foreach(data->removed, append_name, &array);
    g_slist_free_full(data->removed, g_free);
    data->removed = NULL;

    dbus_message_iter_close_container(&iter, &array);

    /* Use dbus_connection_send to avoid recursive calls to rpc_flush */
    dbus_connection_send(data->conn, signal, NULL);
    dbus_message_unref(signal);
}

static void remove_pending(struct generic_data *data)
{
    if (data->process_id > 0)
    {
        g_source_remove(data->process_id);
        data->process_id = 0;
    }

    pending = g_slist_remove(pending, data);
}

static gboolean process_changes(gpointer user_data)
{
    struct generic_data *data = user_data;

    remove_pending(data);

    if (data->added != NULL)
        emit_interfaces_added(data);

    /* Flush pending properties */
    if (data->pending_prop == TRUE)
        process_property_changes(data);

    if (data->removed != NULL)
        emit_interfaces_removed(data);

    data->process_id = 0;

    return FALSE;
}

static void generic_unregister(DBusConnection *connection, void *user_data)
{
    struct generic_data *data = user_data;
    struct generic_data *parent = data->parent;

    if (parent != NULL)
        parent->objects = g_slist_remove(parent->objects, data);

    if (data->process_id > 0)
    {
        g_source_remove(data->process_id);
        data->process_id = 0;
        process_changes(data);
    }

    g_slist_foreach(data->objects, reset_parent, data->parent);
    g_slist_free(data->objects);

    dbus_connection_unref(data->conn);
    g_free(data->introspect);
    g_free(data->path);
    g_free(data);
}

static DBusHandlerResult generic_message(DBusConnection *connection, DBusMessage *message, void *user_data)
{
    struct generic_data *data = user_data;
    struct interface_data *iface;
    const RpcMethodTable *method;
    const char *interface;

    interface = dbus_message_get_interface(message);

    iface = find_interface(data->interfaces, interface);
    if (iface == NULL)
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    for (method = iface->methods; method && method->name && method->function; method++)
    {

        if (dbus_message_is_method_call(message, iface->name, method->name) == FALSE)
            continue;

        if (check_experimental(method->flags, RPC_METHOD_FLAG_EXPERIMENTAL))
            return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

        if (rpc_args_have_signature(method->in_args, message) == FALSE)
            continue;

        if (check_privilege(connection, message, method, iface->user_data) == TRUE)
            return DBUS_HANDLER_RESULT_HANDLED;

        return process_message(connection, message, method, iface->user_data);
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static DBusObjectPathVTable generic_table = {
    .unregister_function = generic_unregister,
    .message_function = generic_message,
};

static const RpcMethodTable introspect_methods[]
    = { { RPC_METHOD("Introspect", NULL, RPC_ARGS({ "xml", "s" }), introspect) }, {} };

static void append_interfaces(struct generic_data *data, DBusMessageIter *iter)
{
    DBusMessageIter array;

    dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
        DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_ARRAY_AS_STRING
            DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_VARIANT_AS_STRING
                DBUS_DICT_ENTRY_END_CHAR_AS_STRING DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
        &array);

    g_slist_foreach(data->interfaces, append_interface, &array);

    dbus_message_iter_close_container(iter, &array);
}

static void append_object(gpointer data, gpointer user_data)
{
    struct generic_data *child = data;
    DBusMessageIter *array = user_data;
    DBusMessageIter entry;

    dbus_message_iter_open_container(array, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_OBJECT_PATH, &child->path);
    append_interfaces(child, &entry);
    dbus_message_iter_close_container(array, &entry);

    g_slist_foreach(child->objects, append_object, user_data);
}

static DBusMessage *get_objects(DBusConnection *connection, DBusMessage *message, void *user_data)
{
    struct generic_data *data = user_data;
    DBusMessage *reply;
    DBusMessageIter iter;
    DBusMessageIter array;

    reply = dbus_message_new_method_return(message);
    if (reply == NULL)
        return NULL;

    dbus_message_iter_init_append(reply, &iter);

    dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,
        DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING DBUS_TYPE_OBJECT_PATH_AS_STRING DBUS_TYPE_ARRAY_AS_STRING
            DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_ARRAY_AS_STRING
                DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_VARIANT_AS_STRING
                    DBUS_DICT_ENTRY_END_CHAR_AS_STRING DBUS_DICT_ENTRY_END_CHAR_AS_STRING
                        DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
        &array);

    g_slist_foreach(data->objects, append_object, &array);

    dbus_message_iter_close_container(&iter, &array);

    return reply;
}

static const RpcMethodTable manager_methods[]
    = { { RPC_METHOD("GetManagedObjects", NULL, RPC_ARGS({ "objects", "a{oa{sa{sv}}}" }), get_objects) }, {} };

static const RpcSignalTable manager_signals[]
    = { { RPC_SIGNAL("InterfacesAdded", RPC_ARGS({ "object", "o" }, { "interfaces", "a{sa{sv}}" })) },
          { RPC_SIGNAL("InterfacesRemoved", RPC_ARGS({ "object", "o" }, { "interfaces", "as" })) }, {} };

static gboolean add_interface(struct generic_data *data, const char *name, const RpcMethodTable *methods,
    const RpcSignalTable *signals, const RpcPropertyTable *properties, void *user_data, RpcDestroyFunction destroy)
{
    struct interface_data *iface;
    const RpcMethodTable *method;
    const RpcSignalTable *signal;
    const RpcPropertyTable *property;

    for (method = methods; method && method->name; method++)
    {
        if (!check_experimental(method->flags, RPC_METHOD_FLAG_EXPERIMENTAL))
            goto done;
    }

    for (signal = signals; signal && signal->name; signal++)
    {
        if (!check_experimental(signal->flags, RPC_SIGNAL_FLAG_EXPERIMENTAL))
            goto done;
    }

    for (property = properties; property && property->name; property++)
    {
        if (!check_experimental(property->flags, RPC_PROPERTY_FLAG_EXPERIMENTAL))
            goto done;
    }

    /* Nothing to register */
    return FALSE;

done:
    iface = g_new0(struct interface_data, 1);
    iface->name = g_strdup(name);
    iface->methods = methods;
    iface->signals = signals;
    iface->properties = properties;
    iface->user_data = user_data;
    iface->destroy = destroy;

    data->interfaces = g_slist_append(data->interfaces, iface);
    if (data->parent == NULL)
        return TRUE;

    data->added = g_slist_append(data->added, iface);

    add_pending(data);

    return TRUE;
}

static struct generic_data *object_path_ref(DBusConnection *connection, const char *path)
{
    struct generic_data *data;

    if (dbus_connection_get_object_path_data(connection, path, (void *)&data) == TRUE)
    {
        if (data != NULL)
        {
            data->refcount++;
            return data;
        }
    }

    data = g_new0(struct generic_data, 1);
    data->conn = dbus_connection_ref(connection);
    data->path = g_strdup(path);
    data->refcount = 1;

    data->introspect = g_strdup(DBUS_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE "<node></node>");

    if (!dbus_connection_register_object_path(connection, path, &generic_table, data))
    {
        dbus_connection_unref(data->conn);
        g_free(data->path);
        g_free(data->introspect);
        g_free(data);
        return NULL;
    }

    invalidate_parent_data(connection, path);

    add_interface(data, DBUS_INTERFACE_INTROSPECTABLE, introspect_methods, NULL, NULL, data, NULL);

    return data;
}

static void object_path_unref(DBusConnection *connection, const char *path)
{
    struct generic_data *data = NULL;

    if (dbus_connection_get_object_path_data(connection, path, (void *)&data) == FALSE)
        return;

    if (data == NULL)
        return;

    data->refcount--;

    if (data->refcount > 0)
        return;

    remove_interface(data, DBUS_INTERFACE_INTROSPECTABLE);
    remove_interface(data, DBUS_INTERFACE_PROPERTIES);

    invalidate_parent_data(data->conn, data->path);

    dbus_connection_unregister_object_path(data->conn, data->path);
}

static gboolean check_signal(
    DBusConnection *conn, const char *path, const char *interface, const char *name, const RpcArgInfo **args)
{
    struct generic_data *data = NULL;
    struct interface_data *iface;
    const RpcSignalTable *signal;

    *args = NULL;
    if (!dbus_connection_get_object_path_data(conn, path, (void *)&data) || data == NULL)
    {
        error("dbus_connection_emit_signal: path %s isn't registered", path);
        return FALSE;
    }

    iface = find_interface(data->interfaces, interface);
    if (iface == NULL)
    {
        error("dbus_connection_emit_signal: %s does not implement %s", path, interface);
        return FALSE;
    }

    for (signal = iface->signals; signal && signal->name; signal++)
    {
        if (strcmp(signal->name, name) != 0)
            continue;

        if (signal->flags & RPC_SIGNAL_FLAG_EXPERIMENTAL)
        {
            const char *env = g_getenv("RPC_EXPERIMENTAL");
            if (g_strcmp0(env, "1") != 0)
                break;
        }

        *args = signal->args;
        return TRUE;
    }

    error("No signal named %s on interface %s", name, interface);
    return FALSE;
}

gboolean rpc_register_interface(DBusConnection *connection, const char *path, const char *name,
    const RpcMethodTable *methods, const RpcSignalTable *signals, const RpcPropertyTable *properties, void *user_data,
    RpcDestroyFunction destroy)
{
    struct generic_data *data;

    data = object_path_ref(connection, path);
    if (data == NULL)
        return FALSE;

    if (find_interface(data->interfaces, name))
    {
        object_path_unref(connection, path);
        return FALSE;
    }

    if (!add_interface(data, name, methods, signals, properties, user_data, destroy))
    {
        object_path_unref(connection, path);
        return FALSE;
    }

    if (properties != NULL && !find_interface(data->interfaces, DBUS_INTERFACE_PROPERTIES))
        add_interface(data, DBUS_INTERFACE_PROPERTIES, properties_methods, properties_signals, NULL, data, NULL);

    g_free(data->introspect);
    data->introspect = NULL;

    return TRUE;
}

gboolean rpc_unregister_interface(DBusConnection *connection, const char *path, const char *name)
{
    struct generic_data *data = NULL;

    if (path == NULL)
        return FALSE;

    if (dbus_connection_get_object_path_data(connection, path, (void *)&data) == FALSE)
        return FALSE;

    if (data == NULL)
        return FALSE;

    if (remove_interface(data, name) == FALSE)
        return FALSE;

    g_free(data->introspect);
    data->introspect = NULL;

    object_path_unref(connection, data->path);

    return TRUE;
}

gboolean rpc_register_security(const RpcSecurityTable *security)
{
    if (security_table != NULL)
        return FALSE;

    security_table = security;

    return TRUE;
}

gboolean rpc_unregister_security(const RpcSecurityTable *security)
{
    security_table = NULL;

    return TRUE;
}

DBusMessage *rpc_create_error_valist(DBusMessage *message, const char *name, const char *format, va_list args)
{
    char str[1024];

    if (format)
        vsnprintf(str, sizeof(str), format, args);
    else
        str[0] = '\0';

    return dbus_message_new_error(message, name, str);
}

DBusMessage *rpc_create_error(DBusMessage *message, const char *name, const char *format, ...)
{
    va_list args;
    DBusMessage *reply;

    va_start(args, format);

    reply = rpc_create_error_valist(message, name, format, args);

    va_end(args);

    return reply;
}

DBusMessage *rpc_create_reply_valist(DBusMessage *message, int type, va_list args)
{
    DBusMessage *reply;

    reply = dbus_message_new_method_return(message);
    if (reply == NULL)
        return NULL;

    if (dbus_message_append_args_valist(reply, type, args) == FALSE)
    {
        dbus_message_unref(reply);
        return NULL;
    }

    return reply;
}

DBusMessage *rpc_create_reply(DBusMessage *message, int type, ...)
{
    va_list args;
    DBusMessage *reply;

    va_start(args, type);

    reply = rpc_create_reply_valist(message, type, args);

    va_end(args);

    return reply;
}

static void rpc_flush(DBusConnection *connection)
{
    GSList *l;

    for (l = pending; l;)
    {
        struct generic_data *data = l->data;

        l = l->next;
        if (data->conn != connection)
            continue;

        process_changes(data);
    }
}

gboolean rpc_send_message(DBusConnection *connection, DBusMessage *message)
{
    dbus_bool_t result = FALSE;

    if (dbus_message_get_type(message) == DBUS_MESSAGE_TYPE_METHOD_CALL)
        dbus_message_set_no_reply(message, TRUE);
    else if (dbus_message_get_type(message) == DBUS_MESSAGE_TYPE_SIGNAL)
    {
        const char *path = dbus_message_get_path(message);
        const char *interface = dbus_message_get_interface(message);
        const char *name = dbus_message_get_member(message);
        const RpcArgInfo *args;

        if (!check_signal(connection, path, interface, name, &args))
            goto out;
    }

    /* Flush pending signal to guarantee message order */
    rpc_flush(connection);

    result = dbus_connection_send(connection, message, NULL);

out:
    dbus_message_unref(message);

    return result;
}

gboolean rpc_send_message_with_reply(
    DBusConnection *connection, DBusMessage *message, DBusPendingCall **call, int timeout)
{
    dbus_bool_t ret;

    /* Flush pending signal to guarantee message order */
    rpc_flush(connection);

    ret = dbus_connection_send_with_reply(connection, message, call, timeout);

    if (ret == TRUE && call != NULL && *call == NULL)
    {
        error("Unable to send message (passing fd blocked?)");
        return FALSE;
    }

    return ret;
}

gboolean rpc_send_error_valist(
    DBusConnection *connection, DBusMessage *message, const char *name, const char *format, va_list args)
{
    DBusMessage *error;

    error = rpc_create_error_valist(message, name, format, args);
    if (error == NULL)
        return FALSE;

    return rpc_send_message(connection, error);
}

gboolean rpc_send_error(DBusConnection *connection, DBusMessage *message, const char *name, const char *format, ...)
{
    va_list args;
    gboolean result;

    va_start(args, format);

    result = rpc_send_error_valist(connection, message, name, format, args);

    va_end(args);

    return result;
}

gboolean rpc_send_reply_valist(DBusConnection *connection, DBusMessage *message, int type, va_list args)
{
    DBusMessage *reply;

    reply = dbus_message_new_method_return(message);
    if (reply == NULL)
        return FALSE;

    if (dbus_message_append_args_valist(reply, type, args) == FALSE)
    {
        dbus_message_unref(reply);
        return FALSE;
    }

    return rpc_send_message(connection, reply);
}

gboolean rpc_send_reply(DBusConnection *connection, DBusMessage *message, int type, ...)
{
    va_list args;
    gboolean result;

    va_start(args, type);

    result = rpc_send_reply_valist(connection, message, type, args);

    va_end(args);

    return result;
}

gboolean rpc_emit_signal(
    DBusConnection *connection, const char *path, const char *interface, const char *name, int type, ...)
{
    va_list args;
    gboolean result;

    va_start(args, type);

    result = rpc_emit_signal_valist(connection, path, interface, name, type, args);

    va_end(args);

    return result;
}

gboolean rpc_emit_signal_valist(
    DBusConnection *connection, const char *path, const char *interface, const char *name, int type, va_list args)
{
    DBusMessage *signal;
    dbus_bool_t ret;
    const RpcArgInfo *args_info;

    if (!check_signal(connection, path, interface, name, &args_info))
        return FALSE;

    signal = dbus_message_new_signal(path, interface, name);
    if (signal == NULL)
    {
        error("Unable to allocate new %s.%s signal", interface, name);
        return FALSE;
    }

    ret = dbus_message_append_args_valist(signal, type, args);
    if (!ret)
        goto fail;

    if (rpc_args_have_signature(args_info, signal) == FALSE)
    {
        error("%s.%s: got unexpected signature '%s'", interface, name, dbus_message_get_signature(signal));
        ret = FALSE;
        goto fail;
    }

    return rpc_send_message(connection, signal);

fail:
    dbus_message_unref(signal);

    return ret;
}

static void process_properties_from_interface(struct generic_data *data, struct interface_data *iface)
{
    GSList *l;
    DBusMessage *signal;
    DBusMessageIter iter, dict, array;
    GSList *invalidated;

    data->pending_prop = FALSE;

    if (iface->pending_prop == NULL)
        return;

    signal = dbus_message_new_signal(data->path, DBUS_INTERFACE_PROPERTIES, "PropertiesChanged");
    if (signal == NULL)
    {
        error("Unable to allocate new " DBUS_INTERFACE_PROPERTIES ".PropertiesChanged signal");
        return;
    }

    iface->pending_prop = g_slist_reverse(iface->pending_prop);

    dbus_message_iter_init_append(signal, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &iface->name);
    dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,
        DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_VARIANT_AS_STRING
            DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
        &dict);

    invalidated = NULL;

    for (l = iface->pending_prop; l != NULL; l = l->next)
    {
        RpcPropertyTable *p = l->data;

        if (p->get == NULL)
            continue;

        if (p->exists != NULL && !p->exists(p, iface->user_data))
        {
            invalidated = g_slist_prepend(invalidated, p);
            continue;
        }

        append_property(iface, p, &dict);
    }

    dbus_message_iter_close_container(&iter, &dict);

    dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &array);
    for (l = invalidated; l != NULL; l = g_slist_next(l))
    {
        RpcPropertyTable *p = l->data;

        dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING, &p->name);
    }
    g_slist_free(invalidated);
    dbus_message_iter_close_container(&iter, &array);

    g_slist_free(iface->pending_prop);
    iface->pending_prop = NULL;

    /* Use dbus_connection_send to avoid recursive calls to rpc_flush */
    dbus_connection_send(data->conn, signal, NULL);
    dbus_message_unref(signal);
}

static void process_property_changes(struct generic_data *data)
{
    GSList *l;

    for (l = data->interfaces; l != NULL; l = l->next)
    {
        struct interface_data *iface = l->data;

        process_properties_from_interface(data, iface);
    }
}

void rpc_emit_property_changed_full(DBusConnection *connection, const char *path, const char *interface,
    const char *name, RpcPropertyChangedFlags flags)
{
    const RpcPropertyTable *property;
    struct generic_data *data;
    struct interface_data *iface;

    if (path == NULL)
        return;

    if (!dbus_connection_get_object_path_data(connection, path, (void **)&data) || data == NULL)
        return;

    iface = find_interface(data->interfaces, interface);
    if (iface == NULL)
        return;

    /*
     * If ObjectManager is attached, don't emit property changed if
     * interface is not yet published
     */
    if (root && g_slist_find(data->added, iface))
        return;

    property = find_property(iface->properties, name);
    if (property == NULL)
    {
        error("Could not find property %s in %p", name, iface->properties);
        return;
    }

    if (g_slist_find(iface->pending_prop, (void *)property) != NULL)
        return;

    data->pending_prop = TRUE;
    iface->pending_prop = g_slist_prepend(iface->pending_prop, (void *)property);

    if (flags & RPC_PROPERTY_CHANGED_FLAG_FLUSH)
        process_property_changes(data);
    else
        add_pending(data);
}

void rpc_emit_property_changed(DBusConnection *connection, const char *path, const char *interface, const char *name)
{
    rpc_emit_property_changed_full(connection, path, interface, name, 0);
}

gboolean rpc_get_properties(DBusConnection *connection, const char *path, const char *interface, DBusMessageIter *iter)
{
    struct generic_data *data;
    struct interface_data *iface;

    if (path == NULL)
        return FALSE;

    if (!dbus_connection_get_object_path_data(connection, path, (void **)&data) || data == NULL)
        return FALSE;

    iface = find_interface(data->interfaces, interface);
    if (iface == NULL)
        return FALSE;

    append_properties(iface, iter);

    return TRUE;
}

gboolean rpc_attach_object_manager(DBusConnection *connection)
{
    struct generic_data *data;

    data = object_path_ref(connection, "/");
    if (data == NULL)
        return FALSE;

    add_interface(data, DBUS_INTERFACE_OBJECT_MANAGER, manager_methods, manager_signals, NULL, data, NULL);
    root = data;

    return TRUE;
}

gboolean rpc_detach_object_manager(DBusConnection *connection)
{
    if (!rpc_unregister_interface(connection, "/", DBUS_INTERFACE_OBJECT_MANAGER))
        return FALSE;

    root = NULL;

    return TRUE;
}

void rpc_set_flags(int flags)
{
    global_flags = flags;
}

int rpc_get_flags(void)
{
    return global_flags;
}
