#include "stream_controller.hpp"
#include <algorithm>
#include <stdexcept>
#include <mutex>

using namespace web;
using namespace http;
using namespace glsm;

//template<typename T>
//inline static bool operator ==(const std::vector<T>& v, const T& elem)
//{
//  return (std::find(v.begin(), v.end(), elem) != v.end());
//}
//
//template<typename T>
//inline static bool operator !=(const std::vector<T>& v, const T& elem)
//{
//  return (std::find(v.begin(), v.end(), elem) == v.end());
//}
void StreamController::initRestOpHandlers() {
    _listener.support(methods::GET, std::bind(&StreamController::handleGet, this, std::placeholders::_1));
    _listener.support(methods::PUT, std::bind(&StreamController::handlePut, this, std::placeholders::_1));
    _listener.support(methods::POST, std::bind(&StreamController::handlePost, this, std::placeholders::_1));
    _listener.support(methods::DEL, std::bind(&StreamController::handleDelete, this, std::placeholders::_1));
}

void StreamController::setListenerPath(uri_builder& routing_path)
{
    if(!serverInfo->serverName.empty())
    {
    	routing_path.append_path("/v" + serverInfo->version);
    }
//    if(!serverInfo->serverHost.empty())
//	{
//    	routing_path.append_path("/vhosts/");
//    	routing_path.append_path(serverInfo->serverHost);
//	}
//    if(!serverInfo->appName.empty())
//	{
//    	routing_path.append_path("/applications/");
//    	routing_path.append_path(serverInfo->appName);
//	}
//    LOG_NOTI("%s",routing_path.to_string().c_str());
}

void StreamController::handleGet(http_request message) {
	auto path = requestPath(message);
	if (!path.empty())
	{
		auto response = json::value::object();
		bool result = false;
		auto appname = path[URL_Separator::application];
		if(path[URL_Separator::machine] == "machine")
		{
			SystemMonitorConfiguration monitor_config;
			RuntimeUtils* runtimeUtils = new RuntimeUtils(&monitor_config, true);
			runtimeUtils->monitor_systemResource();
			message.reply(status_codes::OK, response);
			delete runtimeUtils;
		}
		else if(path[URL_Separator::streamfiles] == "streamfiles")
		{
			LOG_NOTI("get stream profile IP: [%s]", message.remote_address().c_str());
			if(path.size() -1 != URL_Separator::streamfiles)
			{
				message.reply(status_codes::NotImplemented);
			}
			else
			{
				response["serverName"] = web::json::value::string(serverInfo->serverName);
				auto temp = json::value::array();
				int idx = 0;
				for( auto streamfile : streamfilelist )
				{
					temp[idx++] = streamfile.second.asJson();
				}
				response["streamFiles"] = temp;
			}
			message.reply(status_codes::OK, response);
			LOG_NOTI("get stream profile - result : %d, \n - request : %s \n - response: %s ", result, uri::decode(message.relative_uri().path()).c_str(), response.serialize().c_str());
		}
		else if(path[URL_Separator::instances] == "instances")
		{
			if(path.size() -1 == (int) URL_Separator::instance)
			{
				response["serverName"] = web::json::value::string(serverInfo->serverName);
				auto temp = json::value::array();
				int idx = 0;

				std::vector<StreamState> list;
				result = get_streamer()->update_status(&list, appname);
				for( auto stream : list)
				{
					ProfileStatusListV0 status = ProfileStatusListV0(stream.url, false, serverInfo->appInstance,
							stream.stream_name, true, true, false, stream.is_streaming, 2000, 1000);
					temp[idx++] = status.asJson();
				}
				response["incomingStreams"] = temp;
			}
			else
			{
				message.reply(status_codes::NotImplemented, responseNotImpl(methods::GET));
			}
			if (result)
			{
				message.reply(status_codes::OK, response);
			}
			else
			{
				message.reply(status_codes::InternalError, response);
			}
			LOG_NOTI("select stream profile - result : %d, \n - request : %s \n - response: %s ", result, uri::decode(message.relative_uri().path()).c_str(), response.serialize().c_str());
		}
		else
		{
			message.reply(status_codes::NotFound, responseNotFound(methods::GET));
		}
	}
	else
	{
		auto response = json::value::object();
		response["Message"] = json::value::string("GLSM Streaming Engine ");
		message.reply(status_codes::OK, response);
	}
}

void StreamController::handlePut(http_request message) {
	auto path = requestPath(message); //same as "path = http::uri::split_path(http::uri::decode(message.relative_uri().path()));"
	if (!path.empty()) {

		BOOST_LOG_SCOPED_THREAD_TAG("Tag", "PUT");
		auto value = message.extract_json().get();
		auto response = json::value::object();
		bool result = false;
		try
		{
			if(path[URL_Separator::streamfiles] == "streamfiles")
			{
				auto camid = path[URL_Separator::camid];
				auto appname = path[URL_Separator::application];
				if(path[URL_Separator::adv] == "adv")
				{
					std::string uri;
					StreamProfileModify streamProfileModify = glsm::StreamProfileModify::fromJson(value);
					for( auto setting : streamProfileModify.advancedSettings)
					{
						if(setting.name.compare("uri") == 0)
						{
							uri = uri::decode(setting.value);
						}
					}
					if(!uri.empty())
					{
						result = get_streamer()->modify_stream(camid+".stream", appname, uri.c_str());
						LOG_NOTI("modify/start stream profile camid :[%s], IP: [%s], MSG: [%s] \n URI: %s", camid.c_str(), message.remote_address().c_str(), value.serialize().c_str(), uri.c_str());
						response = make_response(result, utility::string_t(""), streamProfileModify.asJson());
					}
					else
					{
						message.reply(status_codes::NotImplemented);
					}
				}
				else if(path[URL_Separator::streamfile_actions] == "actions")
				{
					if(path[URL_Separator::streamfile_actions_cmd] == "connect")
					{
						auto paramMap = parseQuery(message);
						LOG_NOTI("connect stream profile camid :[%s], IP: [%s], MSG: [%s/%s/%s]",camid.c_str(), message.remote_address().c_str(), paramMap["connectAppName"].c_str(), paramMap["appInstance"].c_str(), paramMap["mediaCasterType"].c_str());
						StreamProfileConnect streamProfileConnect = StreamProfileConnect(paramMap["connectAppName"], paramMap["appInstance"], paramMap["mediaCasterType"]);
						result = get_streamer()->start_stream(camid+".stream", appname);
						response = make_response(result, streamProfileConnect.getResultMsg() + camid + ".stream", json::value::null());
					}
				}
				else
				{
					message.reply(status_codes::NotFound, responseNotFound(methods::PUT));
				}
			}
			else if(path[URL_Separator::instances] == "instances")
			{
				auto stream_filename = path[URL_Separator::profile_name];
				auto appname = path[URL_Separator::application];
				if(path[URL_Separator::incomingstreams] == "incomingstreams")
				{
					if(path[URL_Separator::actions] == "actions")
					{
						if (path[URL_Separator::cmd] == "disconnectStream")
						{
							LOG_NOTI("disconnect stream profile streamfile :[%s], IP: [%s]",stream_filename.c_str(), message.remote_address().c_str());
							StreamProfileDisConnect streamProfileDisConnect = StreamProfileDisConnect();
							result = get_streamer()->stop_stream(stream_filename, appname);
							response = make_response(result, streamProfileDisConnect.getResultMsg() + stream_filename, json::value::null());
						}
					}
				}
				else if(path[URL_Separator::streamrecorders] == "streamrecorders")
				{
					if(path[URL_Separator::actions] == "actions")
					{
						auto appname = path[URL_Separator::application];
						if(path[URL_Separator::cmd] == "stopRecording")
						{
							LOG_NOTI("stop recording camid :[%s], IP: [%s]", stream_filename.c_str(), message.remote_address().c_str());
							result = get_streamer()->stop_record(stream_filename, appname);
							response = make_response(result, "Recording (" + stream_filename + ") stopped", json::value::null());
						}
						else if(path[URL_Separator::cmd] == "splitRecording")
						{
							LOG_NOTI("split recording camid :[%s], IP: [%s]", stream_filename.c_str(), message.remote_address().c_str());
							result = get_streamer()->split_record(stream_filename, appname);
							response = make_response(result, "Recording (" + stream_filename + ") split", json::value::null());
						}
					}
				}
				else
				{
					message.reply(status_codes::NotFound, responseNotFound(methods::PUT));
				}
			}
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
			response = make_response(false, utility::string_t(e.what()), json::value::null());
		}
		if (result)
		{
			message.reply(status_codes::OK, response);
		}
		else
		{
			message.reply(status_codes::InternalError, response);
		}
	}
	else
	{
		message.reply(status_codes::NotFound);
	}
}

void StreamController::handlePost(http_request message) {
	auto path = requestPath(message);
	if (!path.empty()) {
		BOOST_LOG_SCOPED_THREAD_TAG("Tag", "POST");
		auto value = message.extract_json().get();
		auto response = json::value::object();
		bool result = false;
		try
		{
			if(path[URL_Separator::streamfiles] == "streamfiles")
			{
				auto camid = path[URL_Separator::camid];
				auto appname = path[URL_Separator::application];
				LOG_NOTI("create stream profile camid:[%s], IP: [%s], MSG: [%s]",camid.c_str(), message.remote_address().c_str(), value.serialize().c_str());
				StreamProfile streamprofile = glsm::StreamProfile::fromJson(value);
				result = get_streamer()->create_stream(camid+".stream", appname);
				response = make_response(result, utility::string_t(""), streamprofile.asJson());
			}
			else if(path[URL_Separator::instances] == "instances")
			{
				if(path[URL_Separator::streamrecorders] == "streamrecorders")
				{
					auto profileName = path[URL_Separator::profile_name];
					auto appname = path[URL_Separator::application];
					LOG_NOTI("record stream profile camid :[%s], IP: [%s], MSG: [%s]", profileName.c_str(), message.remote_address().c_str(), value.serialize().c_str());
					RecordParameters recordParameters = glsm::RecordParameters::fromJson(value);
					result = get_streamer()->start_record(profileName, appname, &recordParameters);
					LOG_NOTI("Start record profile - result : %d, \n - request : %s \n - response: %s ", result, uri::decode(message.relative_uri().path()).c_str(), response.serialize().c_str());
					response = make_response(result, recordParameters.getResultMsg(), json::value::null());
				}
			}
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
			response = make_response(false, utility::string_t(e.what()), json::value::null());
		}
		if (result)
		{
			message.reply(status_codes::OK, response);
		}
		else
		{
			message.reply(status_codes::InternalError, response);
		}
	}
	else
	{
		message.reply(status_codes::NotFound);
	}
}

void StreamController::handleDelete(http_request message) {
	auto path = requestPath(message);
	if (!path.empty()) {
		BOOST_LOG_SCOPED_THREAD_TAG("Tag", "DELETE");
		auto value = message.extract_json().get();
		auto response = json::value::object();
		bool result = false;
		try
		{
			if(path[URL_Separator::streamfiles] == "streamfiles")
			{
				auto camid = path[URL_Separator::camid];
				auto appname = path[URL_Separator::application];
				LOG_NOTI("delete stream profile camid:[%s], IP: [%s], MSG: [%s]",camid.c_str(), message.remote_address().c_str(), value.serialize().c_str());
				result = get_streamer()->delete_stream(camid+".stream", appname);
				response = make_response(result, utility::string_t(""), web::json::value::null());
			}
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
			response = make_response(false, utility::string_t(e.what()), json::value::null());
		}
		if (result)
		{
			message.reply(status_codes::OK, response);
		}
		else
		{
			message.reply(status_codes::InternalError, response);
		}
	}
	else
	{
		message.reply(status_codes::NotFound);
	}
}
