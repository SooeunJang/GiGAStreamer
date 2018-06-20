#pragma once

#include <unordered_map>
#include "glsm_utils.hpp"
#include "streaming-server.h"
#include "streaming-object.h"

namespace glsm
{
typedef enum _StreamStatus
{
	SUSPENDED = 0,
	READY,
	RUNNING,
	STOPPED,
	MODIFIED,
} StreamStatus;


typedef struct _StreamState
{
	std::string stream_name;
	std::string url;
	bool is_streaming;
	bool is_recording;

	_StreamState() : is_streaming(false), is_recording(false)
	{}
	_StreamState(const gchar* _stream_name, const gchar* _url, gboolean _is_streaming, gboolean _is_recording)
	: stream_name(_stream_name), url(_url), is_streaming(_is_streaming), is_recording(_is_recording)
	{
	}
	void print_info()
	{
        g_print ("stream_name: %s,\n url: %s,\n streaming: %u, recording: %u\n",stream_name.c_str(), url.c_str(), is_streaming, is_recording);
	}
} StreamState;

class GLSinterface
{
public:
	GLSinterface(ServerInfo* serverconf) : gls_handle(nullptr), main_loop(nullptr), error(nullptr), status(SUSPENDED)
	{
		::gst_init (0, nullptr);
		gls_handle = streaming_server_new ();

		GValue rtsp_mounts = G_VALUE_INIT;
		g_value_init (&rtsp_mounts, G_TYPE_PTR_ARRAY);
		g_assert (G_VALUE_HOLDS_BOXED (&rtsp_mounts));

		GPtrArray *rtsp_mounts_array = g_ptr_array_new ();
		g_ptr_array_add (rtsp_mounts_array, (gpointer) serverconf->stream1.c_str());
		g_ptr_array_add (rtsp_mounts_array, (gpointer) serverconf->stream2.c_str());
		g_ptr_array_add (rtsp_mounts_array, (gpointer) serverconf->stream3.c_str());

		g_value_set_boxed (&rtsp_mounts, rtsp_mounts_array);
		g_object_set_property (G_OBJECT (gls_handle), "rtsp-mounts", &rtsp_mounts);

		g_ptr_array_unref (rtsp_mounts_array);
		g_value_unset (&rtsp_mounts);

		GValue rtsp_port = G_VALUE_INIT;
		g_value_init (&rtsp_port, G_TYPE_STRING);
		g_value_set_static_string (&rtsp_port, serverconf->streamerPort.c_str());
		g_assert (G_VALUE_HOLDS_STRING (&rtsp_port));

		g_object_set_property (G_OBJECT (gls_handle), "rtsp-port", &rtsp_port);
		g_value_unset (&rtsp_port);
	}
	~GLSinterface()
	{
		g_main_loop_quit (main_loop);
		g_main_loop_unref (main_loop);
		streaming_server_stop (gls_handle, &error);
		streaming_server_unref (gls_handle);
	}
	bool run_streamer()
	{
		streaming_server_start (gls_handle, &error);

		main_loop = g_main_loop_new (nullptr, FALSE);
		g_main_loop_run (main_loop);
		return true;
	}
	bool create_stream(std::string camid)
	{
		return streaming_session_new (gls_handle, camid.c_str(), NULL);
	}
	bool delete_stream(std::string camid)
	{
		streaming_session_unref (gls_handle, camid.c_str(), NULL);
		return true;
	}
	bool modify_stream(std::string camid, const char* uri)
	{
		StreamingObject *object = streaming_session_get (gls_handle, camid.c_str(), &error);
		if (object == NULL)
		{
			g_printerr ("Unable to get the session: %s\n", camid.c_str());
			return false;
		}

		GValue rtsp_url = G_VALUE_INIT;
		g_value_init (&rtsp_url, G_TYPE_STRING);
		g_value_set_static_string (&rtsp_url, uri);
		g_assert (G_VALUE_HOLDS_STRING (&rtsp_url));
		g_object_set_property (G_OBJECT (object), "rtsp-url", &rtsp_url);
		g_value_unset (&rtsp_url);
		return true;
	}

	bool start_stream(std::string camid)
	{
		StreamingObject *object = streaming_session_get (gls_handle, camid.c_str(), &error);
		return streaming_object_start (object, &error);
	}
	bool stop_stream(std::string camid)
	{
		StreamingObject *object = streaming_session_get (gls_handle, camid.c_str(), &error);
		if (object == NULL)
		{
			g_printerr ("Unable to get the session: %s\n", camid.c_str());
			return false;
		}

		streaming_object_stop (object, &error);
		return true;
	}

	bool start_record(std::string camid, RecordParameters* recordParameters)
	{
		auto outputPath = recordParameters->outputPath.c_str();

		StreamingObject *object = streaming_session_get (gls_handle, camid.c_str(), &error);
		if (object == NULL)
		{
			g_printerr ("Unable to get the session: %s\n", camid.c_str());
			return false;
		}

		GValue record_path = G_VALUE_INIT;
		g_value_init (&record_path, G_TYPE_STRING);
		g_value_set_static_string (&record_path, outputPath);
		g_assert (G_VALUE_HOLDS_STRING (&record_path));
		g_object_set_property (G_OBJECT (object), "record-path", &record_path);
		g_value_unset (&record_path);

		return streaming_object_record (object, &error);
	}
	bool stop_record(std::string camid)
	{
		StreamingObject *object = streaming_session_get (gls_handle, camid.c_str(), &error);
		if (object == NULL) {
			g_printerr ("Unable to get the session: %s\n", camid.c_str());
			return false;
		}

		streaming_object_unrecord (object, &error);
		return true;
	}

	bool update_status(std::vector<StreamState>* vector)
	{
	      GList *l;
	      for (l = streaming_session_list (gls_handle, NULL); l != NULL; l = l->next)
	      {
	        GError *error = NULL;
	        StreamingObject *object = streaming_session_get (gls_handle, (gchar *) l->data, &error);
	        if (object == NULL)
	        {
	          continue;
	        }

	        GValue val = G_VALUE_INIT;
	        g_value_init (&val, G_TYPE_STRING);
	        g_object_get_property (G_OBJECT (object), "rtsp-url", &val);

	        auto statedst = StreamState((const gchar *)l->data, (const gchar*) g_value_get_string (&val)
	        		,streaming_object_connected (object, NULL),streaming_object_recorded (object, NULL));
#ifdef DEBUG
	        statedst.print_info();
#endif
	        vector->push_back(statedst);
	        g_value_unset (&val);
	      }
	      g_list_free (l);
	      return true;
	}

	bool split_record(std::string camid)
	{
		StreamingObject *object = streaming_session_get (gls_handle, camid.c_str(), &error);
		if (object == NULL)
		{
			g_printerr ("Unable to get the session: %s\n", camid.c_str());
			return false;
		}

		return streaming_object_split_now (object, &error);
	}

	void set_status(StreamStatus _status)
	{
		status = _status;
	}

	StreamStatus get_status()
	{
		return status;
	}

private:
	GMainLoop* main_loop;
	GError* error;
	StreamingServer* gls_handle;
	StreamStatus status;
};

}