
#include "controller_manager.hpp"
#include <iostream>

namespace cfx {

ControllerManager::ControllerManager(const char* config_path) : gls_interface(nullptr), state(SUSPENDED)
{
	parse_serverConfig(config_path);
}
ControllerManager::~ControllerManager()
{
	if(state != STOPPED)
	{
		stop_streamer();
	}
}

void ControllerManager::initialize()
{
	gls_interface = new GLSinterface(&serverInfo);
}

void
ControllerManager::parse_serverConfig(const char* filename)
{
	std::ifstream ifs;
	ifs.open(filename, std::ifstream::in);
	ifs.is_open();
	while (ifs.good())
	{
		std::string line;
		std::getline(ifs, line);
		std::istringstream iss(line);
		std::string line_type;
		iss >> line_type;
		if (line_type == "version:")
		{
			iss >> serverInfo.version;
		}
		else if (line_type == "servers:")
		{
			iss >> serverInfo.serverName;
		}
		else if (line_type == "vhosts:")
		{
			iss >> serverInfo.serverHost;
		}
		else if (line_type == "applications:")
		{
			iss >> serverInfo.appName;
		}
		else if (line_type == "stream1:")
		{
			iss >> serverInfo.stream1;
		}
		else if (line_type == "stream2:")
		{
			iss >> serverInfo.stream2;
		}
		else if (line_type == "stream3:")
		{
			iss >> serverInfo.stream3;
		}
		else if (line_type == "instances:")
		{
			iss >> serverInfo.appInstance;
		}
		else if (line_type == "connection_timeout:")
		{
			iss >> serverInfo.connection_timeout;
		}
		else if (line_type == "serviceport:")
		{
			iss >> serverInfo.servicePort;
		}
		else if (line_type == "streamerport:")
		{
			iss >> serverInfo.streamerPort;
		}
	}
	ifs.close();
}
void
ControllerManager::add_server(const char* name, BasicController* _server)
{
	serverList.insert(std::make_pair(std::string(name), _server));
}

BasicController* ControllerManager::getServer(std::string name)
{
	return serverList.at(name);
}

void ControllerManager::Clear()
{
	serverList.clear();
}

void ControllerManager::run_services()
{
	for ( auto& iter : serverList)
	{
		iter.second->accept().wait();
		LOG_NOTI("GLSM server name :[%s] is now listening at %s",iter.first.c_str(), iter.second->get_endpoint().c_str());
	}
}

void ControllerManager::stop_services()
{
	for ( auto& iter : serverList)
	{
		iter.second->shutdown().wait();
		LOG_NOTI("GLSM server name :[%s] is shutdown",iter.first.c_str());
	}
}

void ControllerManager::set_eventQueue()
{
	for ( auto iter : serverList)
	{
		iter.second->set_eventQueue(&eventQueue);
	}
}

void ControllerManager::get_eventQueue()
{
	std::this_thread::sleep_for(std::chrono::seconds(2));
//	while(1)
//	{
//		if(!eventQueue.empty())
//		{
//			try{
//				for( auto iter = eventQueue.begin(); iter != eventQueue.end(); ++iter )
//				{
////							if(!websocket->send_message(iter->get_id(), iter->get_data()))
////							{
////								throw std::invalid_argument("No camera available for session");
////							}
//////							restclient->send_message(iter->get_id(), iter->get_data());
//				}
//			}
//			catch (websocketpp::exception const & e)
//			{
//				std::cout << e.what() << std::endl;
//			} catch (websocketpp::lib::error_code& e) {
//				std::cout << e.message() << std::endl;
//			} catch (...)
//			{
//				std::cerr << "no connection. connect device first on own cam id :"<< std::endl;
//			}
//			if(mapmutex.try_lock())
//			{
//				eventQueue.clear();
//				mapmutex.unlock();
//			}
//			continue;
//		}
//	}
}

void ControllerManager::run_streamer()
{
	state = RUNNING;
	gls_interface->run_streamer();
}

void ControllerManager::set_streamer()
{
	for ( auto iter : serverList)
	{
		iter.second->set_streamer(gls_interface);
	}
	state = READY;
}
void ControllerManager::stop_streamer()
{
	if(gls_interface != nullptr)
	{
		delete gls_interface;
	}
	state = STOPPED;
}

ServerState ControllerManager::get_state()
{
	return state;
}
ServerInfo* ControllerManager::get_serverInfo()
{
	return &serverInfo;
}

}
