
#include <iostream>
#include <stdlib.h>
#include <thread>
#include <usr_interrupt_handler.hpp>
#include <map>

#include "controller_manager.hpp"
#include "stream_controller.hpp"
#include "machine_controller.hpp"


#define USERCON
#define LOG
#define EXTERNAL_IP_BLOCK 0
#define DEFAULT_CONFIGFILE_PATH "./glsm.config"

using namespace web;
using namespace cfx;
using namespace glsm;

ControllerManager* controllerManager = NULL;
std::mutex cfx::mapmutex;
boost::shared_ptr<text_sink> Log::pSink = NULL;

int main(int argc, char * argv[]) {
	utility::seconds connection_timeout(DAFAULT_CONNECTION_TIMEOUT);
#if EXTERNAL_IP_BLOCK
	utility::string_t defaultRoute = "http://host_auto_ip4:";
#else
	utility::string_t defaultRoute = "http://unspecified:";
#endif

#ifdef LOG
	Log::initialize();
	SystemMonitorConfiguration monitor_config;
	RuntimeUtils* runtimeUtils = new RuntimeUtils(&monitor_config, true);
	runtimeUtils->initialize_monitoring();
#endif
#ifdef USERCON
	InterruptHandler::hookSIGINT();
#endif
	try {
		controllerManager = new ControllerManager(DEFAULT_CONFIGFILE_PATH);
		controllerManager->initialize();

		BasicController* stream_server = new StreamController(controllerManager->get_serverInfo());
		stream_server->initialize(defaultRoute);
		controllerManager->add_server("stream", stream_server);

		BasicController* machine_server = new MachineController(controllerManager->get_serverInfo());
		machine_server->initialize(defaultRoute);
		controllerManager->add_server("machine", machine_server);

		controllerManager->run_services();
		controllerManager->set_streamer();
		controllerManager->run_streamer();

		LOG_NOTI("GLSM Initializing complete !!");

#ifdef USERCON
		InterruptHandler::waitForUserInterrupt();
#endif
	}
	catch(std::exception & e) {
		LOG_ERR("%s", e.what());
	}
	catch(...) {
		RuntimeUtils::print_callStackBacktrace(NULL);
	}

	if(controllerManager->get_state() == cfx::RUNNING)
	{
		controllerManager->stop_streamer();
		controllerManager->stop_services();
	}
#ifdef LOG
	delete(runtimeUtils);
	Log::stop();
#endif
	delete controllerManager;

    return 0;
}
