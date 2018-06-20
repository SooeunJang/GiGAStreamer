#include "machine_controller.hpp"

using namespace web;
using namespace http;
using namespace glsm;

void MachineController::initRestOpHandlers() {
    this->_listener.support(methods::GET, std::bind(&MachineController::handleGet, this, std::placeholders::_1));
}

void MachineController::handleGet(http_request message)
{
	 auto path = requestPath(message);
	 if (!path.empty()) {
		 message.reply(status_codes::NotImplemented);
	 }
	 else
	 {
		LOG_NOTI("get machine status check from IP: [%s]", message.remote_address().c_str());
		auto response = json::value::object();
		response["Message"] = json::value::string("GLSM Streaming Engine");
		message.reply(status_codes::OK, response);
	 }
}
