#include <std_micro_service.hpp>
#include "glsm_utils.hpp"
#include "stream_controller.hpp"

using namespace web;
using namespace http;
using namespace glsm;

void StreamController::initRestOpHandlers() {
    _listener.support(methods::GET, std::bind(&StreamController::handleGet, this, std::placeholders::_1));
    _listener.support(methods::PUT, std::bind(&StreamController::handlePut, this, std::placeholders::_1));
    _listener.support(methods::POST, std::bind(&StreamController::handlePost, this, std::placeholders::_1));
    _listener.support(methods::DEL, std::bind(&StreamController::handleDelete, this, std::placeholders::_1));
}


void StreamController::setListenerPath(uri_builder& routing_path)
{
    if(this->serverInfo->serverName != NULL)
    {
    	routing_path.append_path("/servers/");
    	routing_path.append_path(this->serverInfo->serverName);
    }
    if(this->serverInfo->serverHost != NULL)
	{
    	routing_path.append_path("/vHosts/");
    	routing_path.append_path(this->serverInfo->serverHost);
	}
    if(this->serverInfo->appName != NULL)
	{
    	routing_path.append_path("/applications/");
    	routing_path.append_path(this->serverInfo->appName);
	}
}

void StreamController::handleGet(http_request message) {
	auto path = requestPath(message);
	if (!path.empty())
	{
		auto first = path.begin();
		auto last = path.end();

		web::json::value response;
		for(std::vector<utility::string_t>::iterator iter = path.begin(); iter != path.end(); ++iter)
		{
//			std::cout<<"distance:"<<std::distance(std::next(iter, 1),last)<<", count:"<<path.size()<<std::endl;
			if(*iter == "instances")
			{
				if(std::distance(std::next(iter, 2), last) == 0)
				{
//servers/_defaultServer_/vHosts/_defaultVHost_/applications/gigaeyeslive/instances/_definst_
					std::vector<web::json::value> incomingStreams;
					json::value temp;
					temp["name"] = json::value::string("cam00001.stream");
					temp["applicationInstance"] = json::value::string(serverInfo->get_appInstance());
					temp["isStreamManagerStream"] = json::value::boolean(true);
					incomingStreams.push_back(temp);

					json::value temp2;
					temp2["name"] = json::value::string("cam00002.stream");
					temp2["applicationInstance"] = json::value::string(serverInfo->get_appInstance());
					temp2["isStreamManagerStream"] = json::value::boolean(true);
					incomingStreams.push_back(temp2);

					json::value temp3;
					temp3["name"] = json::value::string("cam00003.stream");
					temp3["applicationInstance"] = json::value::string(serverInfo->get_appInstance());
					temp3["isStreamManagerStream"] = json::value::boolean(true);
					incomingStreams.push_back(temp3);

					response["incomingStreams"] = json::value::array(incomingStreams);

					response["name"] = json::value::string(serverInfo->get_appInstance());
					response["outgoingStreams"] = web::json::value::null();
					response["serverName"] = json::value::string(serverInfo->get_serverName());
					response["streamGroups"] = web::json::value::null();
					break;
				}
				else
				{
					if(std::next(iter, 2) == "incomingstreams")
					{

					}
				}
			}

		}
		message.reply(status_codes::OK, response);
	}
	else
	{

	}
}

void StreamController::handlePut(http_request message) {
	message.reply(status_codes::NotImplemented);
}

void StreamController::handlePost(http_request message) {
	message.reply(status_codes::NotImplemented);
}

void StreamController::handleDelete(http_request message) {    
	message.reply(status_codes::NotImplemented);
}
