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
    if(this->serverInfo->appInstance != NULL)
	{
    	routing_path.append_path("/instances/");
    	routing_path.append_path(this->serverInfo->appInstance);
	}
}

void StreamController::handleGet(http_request message) {
    message.reply(status_codes::NotImplemented);
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
