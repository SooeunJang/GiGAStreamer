
#include <iostream>
#include <stdlib.h>

#include <usr_interrupt_handler.hpp>
#include <runtime_utils.hpp>
#include <map>

#include "controller_manager.hpp"
#include "stream_controller.hpp"
#include "machine_controller.hpp"
//#include "websocket_service.hpp"

#define MAJOR_VERSION "0"
#define MINOR_VERSION "2"
#define DEFAULT_PORT_NUM 8087
#define DEFAULT_WEBSOCKET_PORT_NUM 8080

#define DAFAULT_CONNECTION_TIMEOUT 1 //sec
#define USERCON

using namespace web;
using namespace cfx;

ControllerManager* controllerManager = NULL;
std::mutex cfx::mapmutex;

int main(int argc, const char * argv[]) {

	utility::string_t port = std::to_string(DEFAULT_PORT_NUM);
	utility::seconds t(DAFAULT_CONNECTION_TIMEOUT);
	if(argc == 2)
	{
		port = argv[1];
	}
	utility::string_t defaultRoute = "http://unspecified:" + port + "/v" + MINOR_VERSION;
#ifdef USERCON
	InterruptHandler::hookSIGINT();
#endif
	controllerManager = new ControllerManager;

	try {
		BasicController* server = new StreamController();
		server->initialize(defaultRoute, t);
		controllerManager->add_server("stream", server);
		std::thread event_thread{&ControllerManager::get_eventQueue, controllerManager};
//		BasicController* machine_server = new MachineController();
//		machine_server->initialize(defaultRoute, t);
//		controllerManager->add_server("machine", machine_server);
		controllerManager->set_eventQueue();
		controllerManager->run_services();

#ifdef USERCON
		InterruptHandler::waitForUserInterrupt();
#endif
		controllerManager->stop_services();
	}
	catch(std::exception & e) {
		std::cerr << "Error : "<<e.what() << std::endl;
	}
	catch(...) {
		RuntimeUtils::printStackTrace();
	}
	delete controllerManager;

    return 0;
}
