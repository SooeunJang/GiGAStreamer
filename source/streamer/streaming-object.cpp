/*
  GiGAeyes Streamer version 1.0

  Copyright ⓒ 2015 kt corp. All rights reserved.

  This is a proprietary software of kt corp, and you may not use this file except in
  compliance with license agreement with kt corp. Any redistribution or use of this
  software, with or without modification shall be strictly prohibited without prior written
  approval of kt corp, and the copyright notice above does not evidence any actual or
  intended publication of such software.
*/

#include "streaming-object.h"

#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <gst/gst.h>

struct _StreamingObject
{
  GObject parent_instance;

  gchar *object_name;
  gchar *rtsp_url;
  gchar *record_path;
  guint64 record_max_size;
  guint record_max_files;

  GstElement *stream_pipeline;
  GstElement *record_pipeline;
};

G_DEFINE_TYPE (StreamingObject, streaming_object, G_TYPE_OBJECT)

enum
{
  PROP_OBJECT_NAME = 1,
  PROP_RTSP_URL,
  PROP_RECORD_PATH,
  PROP_RECORD_MAX_SIZE,
  PROP_RECORD_MAX_FILES,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

enum
{
  SIG_ERROR = 1,
  SIG_EOS,
  N_SIGNALS
};

static guint obj_signals[N_SIGNALS] = { 0, }; 

static void cb_message (GstBus *bus, GstMessage *msg, StreamingObject *self);

GstState get_pipeline_state (GstElement *pipeline);

static void
streaming_object_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  StreamingObject *self = STREAMING_OBJECT (object);

  switch (property_id)
  {
    case PROP_OBJECT_NAME:
      g_free (self->object_name);
      self->object_name = g_value_dup_string (value);
      break;

    case PROP_RTSP_URL:
      g_free (self->rtsp_url);
      self->rtsp_url = g_value_dup_string (value);
      break;

    case PROP_RECORD_PATH:
      g_free (self->record_path);
      self->record_path = g_value_dup_string (value);
      break;

    case PROP_RECORD_MAX_SIZE:
      self->record_max_size = g_value_get_uint64 (value);
      break;

    case PROP_RECORD_MAX_FILES:
      self->record_max_files = g_value_get_uint (value);
      break;

    default:
      /* We don't have any other property... */
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
streaming_object_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  StreamingObject *self = STREAMING_OBJECT (object);

  switch (property_id)
  {
    case PROP_OBJECT_NAME:
      g_value_set_string (value, self->object_name);
      break;

    case PROP_RTSP_URL:
      g_value_set_string (value, self->rtsp_url);
      break;

    case PROP_RECORD_PATH:
      g_value_set_string (value, self->record_path);
      break;

    case PROP_RECORD_MAX_SIZE:
      g_value_set_uint64 (value, self->record_max_size);
      break;

    case PROP_RECORD_MAX_FILES:
      g_value_set_uint (value, self->record_max_files);
      break;

    default:
      /* We don't have any other property... */
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
streaming_object_dispose (GObject *gobject)
{
  /* Always chain up to the parent class; there is no need to check if
   * the parent class implements the dispose() virtual function: it is
   * always guaranteed to do so
   */
  G_OBJECT_CLASS (streaming_object_parent_class)->dispose (gobject);
}

static void
streaming_object_finalize (GObject *gobject)
{
  StreamingObject *obj = STREAMING_OBJECT (gobject);

  g_free (obj->object_name);
  g_free (obj->rtsp_url);
  g_free (obj->record_path);

  /* Always chain up to the parent class; as with dispose(), finalize()
   * is guaranteed to exist on the parent's class virtual function table
   */
  G_OBJECT_CLASS (streaming_object_parent_class)->finalize (gobject);
}

static void
streaming_object_class_init (StreamingObjectClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = streaming_object_dispose;
  object_class->finalize = streaming_object_finalize;

  object_class->set_property = streaming_object_set_property;
  object_class->get_property = streaming_object_get_property;

  obj_properties[PROP_OBJECT_NAME] =
      g_param_spec_string ("object-name",
                           "Object Name",
                           "Name of the object to load and display from.",
                           NULL  /* default value */,
                           G_PARAM_READWRITE);

  obj_properties[PROP_RTSP_URL] =
      g_param_spec_string ("rtsp-url",
                           "RTSP URL",
                           "Location of the RTSP url to read.",
                           NULL  /* default value */,
                           G_PARAM_READWRITE);

  obj_properties[PROP_RECORD_PATH] =
      g_param_spec_string ("record-path",
                           "Record Path",
                           "Path of record files to save.",
                           NULL  /* default value */,
                           G_PARAM_READWRITE);

  obj_properties[PROP_RECORD_MAX_SIZE] =
      g_param_spec_uint64 ("record-max-size",
                           "Record Max size",
                           "Max. amount of time per record file in ns.",
                           0  /* minimum value */,
                           60000000000 /* maximum value */,
                           30000000000  /* default value */,
                           G_PARAM_READWRITE);

  obj_properties[PROP_RECORD_MAX_FILES] =
      g_param_spec_uint ("record-max-files",
                         "Record Max files",
                         "Maximum number of record files to keep on disk.",
                         0  /* minimum value */,
                         10 /* maximum value */,
                         5  /* default value */,
                         G_PARAM_READWRITE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);

  obj_signals[SIG_ERROR] = 
      g_signal_newv ("error",
                     G_TYPE_FROM_CLASS (object_class),
                     (GSignalFlags) (G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS),
                     NULL /* closure */,
                     NULL /* accumulator */,
                     NULL /* accumulator data */,
                     NULL /* C marshaller */,
                     G_TYPE_NONE /* return_type */,
                     0     /* n_params */,
                     NULL  /* param_types */);

  obj_signals[SIG_EOS] = 
      g_signal_newv ("end-of-stream",
                     G_TYPE_FROM_CLASS (object_class),
                     (GSignalFlags) (G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS),
                     NULL /* closure */,
                     NULL /* accumulator */,
                     NULL /* accumulator data */,
                     NULL /* C marshaller */,
                     G_TYPE_NONE /* return_type */,
                     0     /* n_params */,
                     NULL  /* param_types */);
}

static void
streaming_object_init (StreamingObject *self)
{
  // /* initialize all public and private members to reasonable default values.
  //  * They are all automatically initialized to 0 to begin with. */
}

StreamingObject *
streaming_object_new (void) {
  return (StreamingObject *) g_object_new (STREAMING_TYPE_OBJECT, NULL);
}

void
streaming_object_unref (StreamingObject *object) {
  g_return_if_fail (STREAMING_IS_OBJECT (object));
  g_object_unref (object);
}

gboolean
streaming_object_start (StreamingObject  *self,
                        GError           **error)
{
  g_return_val_if_fail (STREAMING_IS_OBJECT (self), false);
  g_return_val_if_fail (error == NULL || *error == NULL, false);

  gchar shm_path[1024];
  g_sprintf (shm_path, "/tmp/%s", self->object_name);
  g_remove (shm_path);

  gchar cmd[1024];
  g_sprintf (cmd,
             "rtspsrc name=rtspsrc-%s location=%s protocols=tcp latency=500 is-live=true"
             " ! shmsink socket-path=/tmp/%s shm-size=10000000 wait-for-connection=false",
             self->object_name, self->rtsp_url, self->object_name);
  self->stream_pipeline = gst_parse_launch (cmd, NULL);

  GstBus *bus = gst_element_get_bus (self->stream_pipeline);
  gst_bus_add_signal_watch (bus);
  g_signal_connect (bus, "message", G_CALLBACK (cb_message), self);
  gst_object_unref (bus);

  GstStateChangeReturn ret = gst_element_set_state (self->stream_pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE)
  {
    g_printerr ("Unable to set the stream_pipeline to the playing state.\n");
    return false;
  }

  return true;
}

void
streaming_object_stop (StreamingObject  *self,
                       GError           **error)
{
  g_return_if_fail (STREAMING_IS_OBJECT (self));
  g_return_if_fail (error == NULL || *error == NULL);
  g_return_if_fail (self->stream_pipeline != NULL);

  if (self->record_pipeline != NULL &&
      get_pipeline_state (self->record_pipeline) == GST_STATE_PLAYING) {
    streaming_object_unrecord (self, error);
  }

  GstBus *bus = gst_element_get_bus (self->stream_pipeline);
  gst_bus_remove_signal_watch (bus);
  gst_object_unref (bus);

  GstStateChangeReturn ret = gst_element_set_state (self->stream_pipeline, GST_STATE_NULL);
  if (ret == GST_STATE_CHANGE_FAILURE)
  {
    g_printerr ("Unable to set the stream_pipeline to the paused state.\n");
    return;
  }

  gst_object_unref (self->stream_pipeline);
  self->stream_pipeline = NULL;
}

gboolean
streaming_object_connected (StreamingObject  *self,
                            GError           **error)
{
  g_return_val_if_fail (STREAMING_IS_OBJECT (self), false);
  g_return_val_if_fail (error == NULL || *error == NULL, false);

  return (self->stream_pipeline != NULL) &&
      (get_pipeline_state (self->stream_pipeline) == GST_STATE_PLAYING);
}

void
set_record_location (gchar *location, const gchar *path, const gchar *prefix)
{
  GTimeZone *tz = g_time_zone_new_local ();
  GDateTime *now = g_date_time_new_now (tz);
  g_time_zone_unref (tz);

  gchar *datetime = g_date_time_format (now, "%Y%m%d_%H%M%S");
  g_date_time_unref (now);

  g_sprintf (location, "%s/%s_%s.mp4", path, prefix, datetime);
  g_free (datetime);
}

gboolean
streaming_object_record (StreamingObject *self,
                         GError          **error)
{
  g_return_val_if_fail (STREAMING_IS_OBJECT (self), false);
  g_return_val_if_fail (error == NULL || *error == NULL, false);
  g_return_val_if_fail (self->stream_pipeline != NULL &&
                        get_pipeline_state (self->stream_pipeline) == GST_STATE_PLAYING &&
                        self->record_pipeline == NULL,
                        false);

  gchar location[1024];
  set_record_location (location, self->record_path, self->object_name);

  gchar cmd[1024];
  g_sprintf (cmd,
             "shmsrc socket-path=/tmp/%s do-timestamp=true is-live=true"
             " ! application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264"
             " ! rtph264depay"
             " ! h264parse"
             " ! splitmuxsink name=recorder max-size-time=%lu max_files=%u location=%s",
             self->object_name, self->record_max_size, self->record_max_files, location);
  self->record_pipeline = gst_parse_launch (cmd, NULL);

  GstBus *bus = gst_element_get_bus (self->record_pipeline);
  gst_bus_add_signal_watch (bus);
  g_signal_connect (bus, "message", G_CALLBACK (cb_message), self);
  gst_object_unref (bus);

  GstStateChangeReturn ret = gst_element_set_state (self->record_pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the record_pipeline to the playing state.\n");
    return false;
  }

  return true;
}

void
streaming_object_unrecord (StreamingObject *self,
                           GError          **error)
{
  g_return_if_fail (STREAMING_IS_OBJECT (self));
  g_return_if_fail (error == NULL || *error == NULL);
  g_return_if_fail (self->record_pipeline != NULL);

  GstBus *bus = gst_element_get_bus (self->record_pipeline);
  gst_bus_remove_signal_watch (bus);
  gst_object_unref (bus);

  GstStateChangeReturn ret = gst_element_set_state (self->record_pipeline, GST_STATE_NULL);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the record_pipeline to the paused state.\n");
    return;
  }

  gst_object_unref (self->record_pipeline);
  self->record_pipeline = NULL;
}

gboolean
streaming_object_recorded (StreamingObject  *self,
                           GError           **error)
{
  g_return_val_if_fail (STREAMING_IS_OBJECT (self), false);
  g_return_val_if_fail (error == NULL || *error == NULL, false);

  return (self->record_pipeline != NULL) &&
      (get_pipeline_state (self->record_pipeline) == GST_STATE_PLAYING);
}

gboolean
streaming_object_split_now (StreamingObject  *self,
                            GError           **error)
{
  g_return_val_if_fail (STREAMING_IS_OBJECT (self), false);
  g_return_val_if_fail (error == NULL || *error == NULL, false);
  g_return_val_if_fail (self->stream_pipeline != nullptr, false);
  g_return_val_if_fail (self->record_pipeline != nullptr, false);

  if (get_pipeline_state (self->record_pipeline) != GST_STATE_PLAYING)
  {
    g_printerr ("%s isn't recording\n", self->object_name);
    return false;
  }

  GstElement *sink = gst_bin_get_by_name (GST_BIN(self->record_pipeline), "recorder");
  g_signal_emit_by_name (sink, "split-now", NULL);
  g_object_unref (sink);

  return true;
}

GstState
get_pipeline_state (GstElement *pipeline)
{
  GstState old_state, new_state;
  GstStateChangeReturn ret = gst_element_get_state (pipeline, &old_state, &new_state, GST_MSECOND);
  if (ret == GST_STATE_CHANGE_FAILURE)
  {
    g_printerr ("Unable to get the state of pipeline.\n");
  }

  g_print ("get state: from %s to %s\n",
           gst_element_state_get_name (old_state),
           gst_element_state_get_name (new_state));
  return old_state;
}

static void
cb_message (GstBus *bus, GstMessage *msg, StreamingObject *self)
{
  switch (GST_MESSAGE_TYPE (msg))
  {
    case GST_MESSAGE_ERROR: {
      GError *err;
      gchar *debug;
      gst_message_parse_error (msg, &err, &debug);
      g_print ("%s: Error(%s %s %s)\n", self->object_name, GST_OBJECT_NAME (msg->src), err->message, (debug) ? debug : "none");
      g_error_free (err);
      g_free (debug);

      g_signal_emit (self, obj_signals[SIG_ERROR], 0);

      streaming_object_stop (self, NULL);
      streaming_object_start (self, NULL);
    } break;

    case GST_MESSAGE_EOS:
      /* end-of-stream */
      gst_element_set_state (self->stream_pipeline, GST_STATE_READY);

      if (self->record_pipeline != NULL)
      {
        gst_element_set_state (self->record_pipeline, GST_STATE_READY);
      }

      g_print ("%s: EOS\n", self->object_name);
      g_signal_emit (self, obj_signals[SIG_EOS], 0);
      break;

    case GST_MESSAGE_CLOCK_LOST:
      /* Get a new clock */
      gst_element_set_state (self->stream_pipeline, GST_STATE_PAUSED);
      gst_element_set_state (self->stream_pipeline, GST_STATE_PLAYING);

      if (self->record_pipeline != NULL)
      {
        gst_element_set_state (self->record_pipeline, GST_STATE_PAUSED);
        gst_element_set_state (self->record_pipeline, GST_STATE_PLAYING);
      }
      break;

    case GST_MESSAGE_ASYNC_DONE: {
      GstState state;
      GstState pending;
      GstStateChangeReturn ret = gst_element_get_state(self->stream_pipeline, &state, &pending, 0);
      if (ret != GST_STATE_CHANGE_FAILURE)
      {
        g_print ("[%s] Async Done: %s from %s to %s\n",
                 self->object_name,
                 GST_OBJECT_NAME (msg->src),
                 gst_element_state_get_name (state),
                 gst_element_state_get_name (pending));
      }

      if (self->record_pipeline != NULL)
      {
        ret = gst_element_get_state (self->record_pipeline, &state, &pending, 0);
        if (ret != GST_STATE_CHANGE_FAILURE)
        {
          g_print ("[%s] Async Done: %s from %s to %s\n",
                   self->object_name,
                   GST_OBJECT_NAME (msg->src),
                   gst_element_state_get_name (state),
                   gst_element_state_get_name (pending));
        }
      }
    } break;

    case GST_MESSAGE_STATE_CHANGED: {
      // GstState old_state, new_state;
      // gst_message_parse_state_changed (msg, &old_state, &new_state, NULL);
      // g_print ("[%03d] Element %s changed state from %s to %s.\n",
      //          self->idx,
      //          GST_OBJECT_NAME (msg->src),
      //          gst_element_state_get_name (old_state),
      //          gst_element_state_get_name (new_state));
    } break;

    case GST_MESSAGE_ELEMENT:
      g_message ("[%s] Get Message: %s", self->object_name,
                 gst_structure_get_name (gst_message_get_structure(msg)));

      if (g_strcmp0 (gst_structure_get_name (gst_message_get_structure (msg)),
                     "splitmuxsink-fragment-closed") == 0)
      {
        gchar location[1024];
        set_record_location (location, self->record_path, self->object_name);
        GstElement *sink = gst_bin_get_by_name (GST_BIN (self->record_pipeline), "recorder");
        g_object_set (sink, "location", &location, NULL);
        g_object_unref (sink);
      }

      break;

    default:
      /* Unhandled message */
      //g_print ("cb_message %s %d\n", GST_MESSAGE_TYPE_NAME(msg), self->idx);
      break;
  }
}
