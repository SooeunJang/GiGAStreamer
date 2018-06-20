/*
 * Copyright/Licensing information.
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
