/*
 * This file is automatically generated, do not modify it.
 *
 * This file Needs to be Used as a dynamic link library.
 */

#ifndef __PHOTO_H__
#define __PHOTO_H__

#include <gio/gio.h>

#include "service.h"
G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for com.example.photo */

#define TYPE_PHOTO (photo_get_type ())
#define PHOTO(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_PHOTO, Photo))
#define IS_PHOTO(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_PHOTO))
#define PHOTO_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), TYPE_PHOTO, PhotoIface))

struct _Photo;
typedef struct _Photo Photo;
typedef struct _PhotoIface PhotoIface;

struct _PhotoIface
{
  GTypeInterface parent_iface;



  gboolean (*handle_get_photo_info) (
    Photo *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_stop_photo) (
    Photo *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_take_photo) (
    Photo *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_in_arg1,
    gint arg_in_arg2);

  const gchar *const * (*get_array_str_property) (Photo *object);

  GVariant * (*get_array_uint32_property) (Photo *object);

  gboolean  (*get_bool_property) (Photo *object);

  guchar  (*get_byte_property) (Photo *object);

  gdouble  (*get_double_property) (Photo *object);

  gint16  (*get_int16_property) (Photo *object);

  gint  (*get_int32_property) (Photo *object);

  gint64  (*get_int64_property) (Photo *object);

  const gchar * (*get_str_property) (Photo *object);

  guint16  (*get_uint16_property) (Photo *object);

  guint  (*get_uint32_property) (Photo *object);

  guint64  (*get_uint64_property) (Photo *object);

  void (*start) (
    Photo *object,
    guint arg_time);

  void (*stop) (
    Photo *object);

};

GType photo_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *photo_interface_info (void);
guint photo_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void photo_complete_take_photo (
    Photo *object,
    GDBusMethodInvocation *invocation,
    const gchar *out_arg1,
    gint out_arg2);

void photo_complete_stop_photo (
    Photo *object,
    GDBusMethodInvocation *invocation);

void photo_complete_get_photo_info (
    Photo *object,
    GDBusMethodInvocation *invocation,
    guint width,
    guint height,
    const gchar *format,
    guint size,
    guint date,
    const gchar *location,
    const gchar *color);



/* D-Bus signal emissions functions: */
void photo_emit_start (
    Photo *object,
    guint arg_time);

void photo_emit_stop (
    Photo *object);



/* D-Bus method calls: */
void photo_call_take_photo (
    Photo *proxy,
    const gchar *arg_in_arg1,
    gint arg_in_arg2,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean photo_call_take_photo_finish (
    Photo *proxy,
    gchar **out_out_arg1,
    gint *out_out_arg2,
    GAsyncResult *res,
    GError **error);

gboolean photo_call_take_photo_sync (
    Photo *proxy,
    const gchar *arg_in_arg1,
    gint arg_in_arg2,
    gchar **out_out_arg1,
    gint *out_out_arg2,
    GCancellable *cancellable,
    GError **error);

void photo_call_stop_photo (
    Photo *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean photo_call_stop_photo_finish (
    Photo *proxy,
    GAsyncResult *res,
    GError **error);

gboolean photo_call_stop_photo_sync (
    Photo *proxy,
    GCancellable *cancellable,
    GError **error);

void photo_call_get_photo_info (
    Photo *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean photo_call_get_photo_info_finish (
    Photo *proxy,
    guint *out_width,
    guint *out_height,
    gchar **out_format,
    guint *out_size,
    guint *out_date,
    gchar **out_location,
    gchar **out_color,
    GAsyncResult *res,
    GError **error);

gboolean photo_call_get_photo_info_sync (
    Photo *proxy,
    guint *out_width,
    guint *out_height,
    gchar **out_format,
    guint *out_size,
    guint *out_date,
    gchar **out_location,
    gchar **out_color,
    GCancellable *cancellable,
    GError **error);



/* D-Bus property accessors: */
#define PHOTO_STRPROPERTY_DEFAULT ("111")
const gchar *photo_get_str_property (Photo *object);
gchar *photo_dup_str_property (Photo *object);
void photo_set_str_property (Photo *object, const gchar *value);
void photo_server_set_str_property_before_change_callback (server_property_before_change_callback call);
void photo_server_set_str_property_after_changed_callback (server_property_after_changed_callback call);

#define PHOTO_BYTEPROPERTY_DEFAULT ('a')
guchar photo_get_byte_property (Photo *object);
void photo_set_byte_property (Photo *object, guchar value);
void photo_server_set_byte_property_before_change_callback (server_property_before_change_callback call);
void photo_server_set_byte_property_after_changed_callback (server_property_after_changed_callback call);

#define PHOTO_BOOLPROPERTY_DEFAULT (TRUE)
gboolean photo_get_bool_property (Photo *object);
void photo_set_bool_property (Photo *object, gboolean value);
void photo_server_set_bool_property_before_change_callback (server_property_before_change_callback call);
void photo_server_set_bool_property_after_changed_callback (server_property_after_changed_callback call);

#define PHOTO_INT16PROPERTY_DEFAULT (10)
gint16 photo_get_int16_property (Photo *object);
void photo_set_int16_property (Photo *object, gint16 value);
void photo_server_set_int16_property_before_change_callback (server_property_before_change_callback call);
void photo_server_set_int16_property_after_changed_callback (server_property_after_changed_callback call);

#define PHOTO_UINT16PROPERTY_DEFAULT (10)
guint16 photo_get_uint16_property (Photo *object);
void photo_set_uint16_property (Photo *object, guint16 value);
void photo_server_set_uint16_property_before_change_callback (server_property_before_change_callback call);
void photo_server_set_uint16_property_after_changed_callback (server_property_after_changed_callback call);

#define PHOTO_INT32PROPERTY_DEFAULT (-56)
gint photo_get_int32_property (Photo *object);
void photo_set_int32_property (Photo *object, gint value);
void photo_server_set_int32_property_before_change_callback (server_property_before_change_callback call);
void photo_server_set_int32_property_after_changed_callback (server_property_after_changed_callback call);

#define PHOTO_UINT32PROPERTY_DEFAULT (10)
guint photo_get_uint32_property (Photo *object);
void photo_set_uint32_property (Photo *object, guint value);
void photo_server_set_uint32_property_before_change_callback (server_property_before_change_callback call);
void photo_server_set_uint32_property_after_changed_callback (server_property_after_changed_callback call);

#define PHOTO_INT64PROPERTY_DEFAULT (-99)
gint64 photo_get_int64_property (Photo *object);
void photo_set_int64_property (Photo *object, gint64 value);
void photo_server_set_int64_property_before_change_callback (server_property_before_change_callback call);
void photo_server_set_int64_property_after_changed_callback (server_property_after_changed_callback call);

#define PHOTO_UINT64PROPERTY_DEFAULT (113)
guint64 photo_get_uint64_property (Photo *object);
void photo_set_uint64_property (Photo *object, guint64 value);
void photo_server_set_uint64_property_before_change_callback (server_property_before_change_callback call);
void photo_server_set_uint64_property_after_changed_callback (server_property_after_changed_callback call);

#define PHOTO_DOUBLEPROPERTY_DEFAULT (1.1)
gdouble photo_get_double_property (Photo *object);
void photo_set_double_property (Photo *object, gdouble value);
void photo_server_set_double_property_before_change_callback (server_property_before_change_callback call);
void photo_server_set_double_property_after_changed_callback (server_property_after_changed_callback call);

const gchar *const *photo_get_array_str_property (Photo *object);
gchar **photo_dup_array_str_property (Photo *object);
void photo_set_array_str_property (Photo *object, const gchar *const *value);
void photo_server_set_array_str_property_before_change_callback (server_property_before_change_callback call);
void photo_server_set_array_str_property_after_changed_callback (server_property_after_changed_callback call);

GVariant *photo_get_array_uint32_property (Photo *object);
GVariant *photo_dup_array_uint32_property (Photo *object);
void photo_set_array_uint32_property (Photo *object, GVariant *value);
void photo_server_set_array_uint32_property_before_change_callback (server_property_before_change_callback call);
void photo_server_set_array_uint32_property_after_changed_callback (server_property_after_changed_callback call);


/* ---- */

#define TYPE_PHOTO_PROXY (photo_proxy_get_type ())
#define PHOTO_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_PHOTO_PROXY, PhotoProxy))
#define PHOTO_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_PHOTO_PROXY, PhotoProxyClass))
#define PHOTO_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_PHOTO_PROXY, PhotoProxyClass))
#define IS_PHOTO_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_PHOTO_PROXY))
#define IS_PHOTO_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_PHOTO_PROXY))

typedef struct _PhotoProxy PhotoProxy;
typedef struct _PhotoProxyClass PhotoProxyClass;
typedef struct _PhotoProxyPrivate PhotoProxyPrivate;

struct _PhotoProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  PhotoProxyPrivate *priv;
};

struct _PhotoProxyClass
{
  GDBusProxyClass parent_class;
};

GType photo_proxy_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (PhotoProxy, g_object_unref)
#endif

void photo_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Photo *photo_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
Photo *photo_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void photo_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Photo *photo_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
Photo *photo_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define TYPE_PHOTO_SKELETON (photo_skeleton_get_type ())
#define PHOTO_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_PHOTO_SKELETON, PhotoSkeleton))
#define PHOTO_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_PHOTO_SKELETON, PhotoSkeletonClass))
#define PHOTO_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_PHOTO_SKELETON, PhotoSkeletonClass))
#define IS_PHOTO_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_PHOTO_SKELETON))
#define IS_PHOTO_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_PHOTO_SKELETON))

typedef struct _PhotoSkeleton PhotoSkeleton;
typedef struct _PhotoSkeletonClass PhotoSkeletonClass;
typedef struct _PhotoSkeletonPrivate PhotoSkeletonPrivate;

struct _PhotoSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  PhotoSkeletonPrivate *priv;
};

struct _PhotoSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType photo_skeleton_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (PhotoSkeleton, g_object_unref)
#endif

Photo *photo_skeleton_new (void);


G_END_DECLS

#endif /* __PHOTO_H__ */
