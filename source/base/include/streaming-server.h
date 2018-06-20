/*
 * Copyright/Licensing information.
 */

#ifndef __STREAMING_SERVER_H__
#define __STREAMING_SERVER_H__

#include <glib-object.h>
#include <glib/gprintf.h>
#include <gst/gst.h>
#include "streaming-object.h"

G_BEGIN_DECLS

/*
 * Type declaration.
 */
#define STREAMING_TYPE_SERVER streaming_server_get_type ()
G_DECLARE_FINAL_TYPE (StreamingServer, streaming_server, STREAMING, SERVER, GObject)

/*
 * Method definitions.
 */
StreamingServer *streaming_server_new (void);
void streaming_server_unref (StreamingServer *);

gboolean streaming_server_start (StreamingServer  *self,
                                 GError           **error);
void streaming_server_stop (StreamingServer  *self,
                            GError           **error);

StreamingObject *streaming_session_new (StreamingServer  *self,
                                        const gchar      *name,
                                        GError           **error);
void streaming_session_unref (StreamingServer  *self,
                             const gchar       *name,
                              GError           **error);
StreamingObject *streaming_session_get (StreamingServer  *self,
                                        const gchar      *name,
                                        GError           **error);
GList *streaming_session_list (StreamingServer  *self,
                               GError           **error);

G_END_DECLS

#endif /* __STREAMING_SERVER_H__ */
