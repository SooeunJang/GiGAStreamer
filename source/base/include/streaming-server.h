/*
  GiGAeyes Streamer version 1.0

  Copyright ⓒ 2015 kt corp. All rights reserved.

  This is a proprietary software of kt corp, and you may not use this file except in
  compliance with license agreement with kt corp. Any redistribution or use of this
  software, with or without modification shall be strictly prohibited without prior written
  approval of kt corp, and the copyright notice above does not evidence any actual or
  intended publication of such software.
*/

#ifndef __STREAMING_SERVER_H__
#define __STREAMING_SERVER_H__

#include <glib-object.h>

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

gboolean streaming_server_start (StreamingServer *self,
                                 GError          **error);
void streaming_server_stop (StreamingServer *self,
                            GError          **error);

StreamingObject *streaming_session_new (StreamingServer *self,
                                        const gchar     *application,
                                        const gchar     *name,
                                        GError          **error);
void streaming_session_unref (StreamingServer *self,
                              const gchar     *application,
                              const gchar     *name,
                              GError          **error);
StreamingObject *streaming_session_get (StreamingServer *self,
                                        const gchar     *application,
                                        const gchar     *name,
                                        GError          **error);
GList *streaming_session_list (StreamingServer *self,
                               const gchar     *application,
                               GError          **error);

G_END_DECLS

#endif /* __STREAMING_SERVER_H__ */
