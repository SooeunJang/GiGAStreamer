#pragma once

#include "basic_controller.hpp"

namespace cfx {
	typedef enum _ServerState
	{
		SUSPENDED = 0,
		READY,
		RUNNING,
		STOPPED,
	} ServerState;
	class ControllerManager
	{
	public:
		typedef std::unordered_map<std::string, BasicController*> ServerList;
		ControllerManager(const char* config_path);
		~ControllerManager();
		void initialize();
		void add_server(const char* name, BasicController* _server);
		BasicController* getServer(std::string name);
		void Clear();
		void run_services();
		void stop_services();
		void set_eventQueue();
		void get_eventQueue();
		void set_streamer();
		void run_streamer();
		void stop_streamer();
		ServerState get_state();
		ServerInfo* get_serverInfo();
	private:
		void parse_serverConfig(const char* filename);
		ServerList serverList;
		EventQueue eventQueue;
		ServerInfo serverInfo;
		GLSinterface* gls_interface;
		ServerState state;
	};
}
