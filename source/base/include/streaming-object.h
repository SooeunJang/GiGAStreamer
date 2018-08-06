/*
  GiGAeyes Streamer version 1.0

  Copyright ⓒ 2015 kt corp. All rights reserved.

  This is a proprietary software of kt corp, and you may not use this file except in
  compliance with license agreement with kt corp. Any redistribution or use of this
  software, with or without modification shall be strictly prohibited without prior written
  approval of kt corp, and the copyright notice above does not evidence any actual or
  intended publication of such software.
*/

#ifndef __STREAMING_OBJECT_H__
#define __STREAMING_OBJECT_H__

#include <glib-object.h>

G_BEGIN_DECLS

/*
 * Type declaration.
 */
#define STREAMING_TYPE_OBJECT streaming_object_get_type ()
G_DECLARE_FINAL_TYPE (StreamingObject, streaming_object, STREAMING, OBJECT, GObject)

/*
 * Method definitions.
 */
StreamingObject *streaming_object_new (void);
void streaming_object_unref (StreamingObject *);

gboolean streaming_object_start (StreamingObject  *self,
                                 GError           **error);
void streaming_object_stop (StreamingObject  *self,
                            GError           **error);
gboolean streaming_object_connected (StreamingObject  *self,
                                     GError           **error);

gboolean streaming_object_record (StreamingObject  *self,
                                 GError           **error);
void streaming_object_unrecord (StreamingObject  *self,
                                GError           **error);
gboolean streaming_object_recorded (StreamingObject  *self,
                                     GError           **error);
gboolean streaming_object_split_now (StreamingObject  *self,
                                     GError           **error);

G_END_DECLS

#endif /* __STREAMING_OBJECT_H__ */
