/*
  GiGAeyes Streamer version 1.0

  Copyright ⓒ 2015 kt corp. All rights reserved.

  This is a proprietary software of kt corp, and you may not use this file except in
  compliance with license agreement with kt corp. Any redistribution or use of this
  software, with or without modification shall be strictly prohibited without prior written
  approval of kt corp, and the copyright notice above does not evidence any actual or
  intended publication of such software.
*/

#include "streaming-server.h"

#include <sys/stat.h>

#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <gst/rtsp-server/rtsp-server.h>

#include "streaming-object.h"

struct _StreamingServer
{
  GObject parent_instance;

  gchar *rtsp_port;
  GstRTSPServer *rtsp_server;
  GHashTable *applications;
};

G_DEFINE_TYPE (StreamingServer, streaming_server, G_TYPE_OBJECT)

enum
{
  PROP_RTSP_PORT = 1,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void
streaming_server_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  StreamingServer *self = STREAMING_SERVER (object);

  switch (property_id)
  {
    case PROP_RTSP_PORT:
      g_free (self->rtsp_port);
      self->rtsp_port = g_value_dup_string (value);
      break;

    default:
      /* We don't have any other property... */
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
streaming_server_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  StreamingServer *self = STREAMING_SERVER (object);

  switch (property_id)
  {
    case PROP_RTSP_PORT:
      g_value_set_string (value, self->rtsp_port);
      break;

    default:
      /* We don't have any other property... */
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
streaming_server_dispose (GObject *gobject)
{
  /* Always chain up to the parent class; there is no need to check if
   * the parent class implements the dispose() virtual function: it is
   * always guaranteed to do so
   */
  G_OBJECT_CLASS (streaming_server_parent_class)->dispose (gobject);
}

static void
streaming_server_finalize (GObject *gobject)
{
  StreamingServer *self = STREAMING_SERVER (gobject);

  g_free (self->rtsp_port);
  g_hash_table_destroy (self->applications);

  /* Always chain up to the parent class; as with dispose(), finalize()
   * is guaranteed to exist on the parent's class virtual function table
   */
  G_OBJECT_CLASS (streaming_server_parent_class)->finalize (gobject);
}

static void
streaming_server_class_init (StreamingServerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = streaming_server_dispose;
  object_class->finalize = streaming_server_finalize;

  object_class->set_property = streaming_server_set_property;
  object_class->get_property = streaming_server_get_property;

  obj_properties[PROP_RTSP_PORT] =
      g_param_spec_string ("rtsp-port",
                           "RTSP Port",
                           "Port of the rtsp server.",
                           "1935"  /* default value */,
                           G_PARAM_READWRITE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
streaming_server_init (StreamingServer *self)
{
  self->applications = g_hash_table_new (g_str_hash, g_str_equal);
}

StreamingServer *
streaming_server_new (void) {
  gst_init (NULL, NULL);
  return STREAMING_SERVER (g_object_new (STREAMING_TYPE_SERVER, NULL));
}

void
streaming_server_unref (StreamingServer *server) {
  g_return_if_fail (STREAMING_IS_SERVER (server));
  g_object_unref (server);
}

gboolean
streaming_server_start (StreamingServer  *self,
                        GError           **error)
{
  g_return_val_if_fail (STREAMING_IS_SERVER (self), false);
  g_return_val_if_fail (error == NULL || *error == NULL, false);

  GstRTSPServer *server = gst_rtsp_server_new ();
  g_object_set (server, "service", self->rtsp_port, NULL);
  gst_rtsp_server_attach (server, NULL);
  self->rtsp_server = server;

  return true;
}

void
streaming_server_stop (StreamingServer  *self,
                       GError           **error)
{
  g_return_if_fail (STREAMING_IS_SERVER (self));
  g_return_if_fail (error == NULL || *error == NULL);

  g_clear_object (&self->rtsp_server);
}

GHashTable *
streaming_application_get (StreamingServer *self,
                           const gchar     *application,
                           GError          **error)
{
  g_return_val_if_fail (STREAMING_IS_SERVER (self), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);
  g_return_val_if_fail (self->applications != NULL, NULL);
  g_return_val_if_fail (application != NULL, NULL);

  GHashTable *application_hash = (GHashTable *) g_hash_table_lookup (self->applications, application);
  if (application_hash == NULL)
  {
    return NULL;
  }

  return application_hash;
}

StreamingObject *
streaming_session_new (StreamingServer *self,
                       const gchar     *application,
                       const gchar     *name,
                       GError          **error)
{
  g_return_val_if_fail (STREAMING_IS_SERVER (self), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);
  g_return_val_if_fail (self->rtsp_server != NULL, NULL);
  g_return_val_if_fail (self->applications != NULL, NULL);

  GHashTable* application_hash = streaming_application_get (self, application, error);
  if (application_hash == NULL)
  {
    application_hash = g_hash_table_new (g_str_hash, g_str_equal);
    g_hash_table_insert (self->applications, g_strdup (application), (gpointer) application_hash);

    gchar shm_path[1024];
    g_sprintf (shm_path, "/tmp/%s", application);
    g_mkdir (shm_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    g_print ("%s application is created\n", application);
  }
  else
  {
    if (g_hash_table_lookup (application_hash, name) != NULL)
    {
      g_printerr ("%s object is existed\n", name);
      return NULL;
    }
  }

  StreamingObject *object = streaming_object_new ();

  GValue application_name = G_VALUE_INIT;
  g_value_init (&application_name, G_TYPE_STRING);
  g_value_set_static_string (&application_name, application);
  g_assert (G_VALUE_HOLDS_STRING (&application_name));
  g_object_set_property (G_OBJECT (object), "application", &application_name);
  g_value_unset (&application_name);

  GValue object_name = G_VALUE_INIT;
  g_value_init (&object_name, G_TYPE_STRING);
  g_value_set_static_string (&object_name, name);
  g_assert (G_VALUE_HOLDS_STRING (&object_name));
  g_object_set_property (G_OBJECT (object), "object-name", &object_name);
  g_value_unset (&object_name);

  GstRTSPMountPoints *rtsp_mounts = gst_rtsp_server_get_mount_points (self->rtsp_server);

  gchar shm_path[1024];
  g_sprintf (shm_path, "/tmp/%s/%s", application, name);
  
  gchar rtsp_src[1024];
  // for standalone test
  // g_sprintf (rtsp_src, "videotestsrc ! queue ! videoconvert ! x264enc ! rtph264pay name=pay0 pt=96");
  g_sprintf (rtsp_src,
             "shmsrc socket-path=%s do-timestamp=true is-live=true"
             " ! application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264"
             " ! rtph264depay"
             " ! rtph264pay name=pay0 pt=97",
             shm_path);

  GstRTSPMediaFactory *rtsp_factory = gst_rtsp_media_factory_new ();
  gst_rtsp_media_factory_set_shared (rtsp_factory, TRUE);
  gst_rtsp_media_factory_set_launch (rtsp_factory, rtsp_src);

  gchar rtsp_mount[1024];
  g_sprintf (rtsp_mount, "/%s/%s", application, name);
  gst_rtsp_mount_points_add_factory (rtsp_mounts, rtsp_mount, rtsp_factory);
  g_object_unref (rtsp_mounts);

  g_hash_table_insert(application_hash, g_strdup (name), object);

  return object;
}

void
streaming_session_unref (StreamingServer *self,
                         const gchar     *application,
                         const gchar     *name,
                         GError          **error)
{
  g_return_if_fail (STREAMING_IS_SERVER (self));
  g_return_if_fail (error == NULL || *error == NULL);
  g_return_if_fail (name != NULL);

  GHashTable* application_hash = streaming_application_get (self, application, error);
  if (application_hash == NULL)
  {
    g_printerr ("%s application isn't existed\n", application);
    return;
  }

  StreamingObject *object = streaming_session_get (self, application, name, error);
  if (object == NULL)
  {
    g_printerr ("%s object isn't existed\n", name);
    return;
  }

  streaming_object_unref (object);

  g_hash_table_remove (application_hash, name);
  if (g_hash_table_size (application_hash) == 0) {
    g_hash_table_remove (self->applications, application);

    gchar shm_path[1024];
    g_sprintf (shm_path, "/tmp/%s", application);
    g_remove (shm_path);
  }
}

StreamingObject *
streaming_session_get (StreamingServer *self,
                       const gchar     *application,
                       const gchar     *name,
                       GError          **error)
{
  g_return_val_if_fail (STREAMING_IS_SERVER (self), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);
  g_return_val_if_fail (name != NULL, NULL);

  GHashTable* application_hash = streaming_application_get (self, application, error);
  if (application_hash == NULL)
  {
    g_printerr ("%s application isn't existed\n", application);
    return NULL;
  }

  return (StreamingObject *) g_hash_table_lookup (application_hash, name);
}

GList *
streaming_session_list (StreamingServer *self,
                        const gchar     *application,
                        GError          **error)
{
  g_return_val_if_fail (STREAMING_IS_SERVER (self), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  GHashTable* application_hash = streaming_application_get (self, application, error);
  if (application_hash == NULL)
  {
    g_printerr ("%s application isn't existed\n", application);
    return NULL;
  }

  return g_hash_table_get_keys (application_hash);
}
