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
}

void StreamController::handleGet(http_request message) {
	message.reply(status_codes::NotImplemented);
}

void StreamController::handlePut(http_request message) {
	message.reply(status_codes::NotImplemented);
}

void StreamController::handlePost(http_request message) {
	auto paramMap = parseQuery(message);
	int camid = std::stoi(paramMap["camid"]);

	std::cout<<camid<<std::endl;
	utility::string_t data = message.extract_json().get().serialize();
	std::cout<<data<<std::endl;

	Event* event = new Event;
	event->set_event(camid, data);
	std::cout<<event->get_data()<<std::endl;
	std::cout<<eventQueue<<std::endl;
//	if(mapmutex.try_lock())
//	{
		eventQueue->push_back(event);
//		mapmutex.unlock();
//	}
//		std::cout<<eventQueue<<std::endl;
	std::cout<<event->get_id()<<std::endl;

	auto response = json::value::object();
     utility::string_t res_msg= _listener.uri().to_string();
     response["uri_info"] = json::value::string(res_msg);
     response["serviceName"] = json::value::string("GLSM Service");
     response["camid"] = json::value::string(paramMap["camid"]);
//     response["json"] = message.extract_json().get();
	message.reply(status_codes::OK, response);
}

void StreamController::handleDelete(http_request message) {    
	message.reply(status_codes::NotImplemented);
}
