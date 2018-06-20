#include "machine_controller.hpp"
#include "glsm_utils.hpp"

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
		std::cout<<"host: "<<message.request_uri().host()<<std::endl;
		std::cout<<"user_info: "<<message.request_uri().user_info()<<std::endl;
		std::cout<<"content_type: "<<message.headers().content_type()<<std::endl;
		auto response = json::value::object();
		response["Message"] = json::value::string("GLSM Streaming Engine 4 Perpetual Pro Edition 4.7.1 build20635");
		message.reply(status_codes::OK, response);
	 }
}
