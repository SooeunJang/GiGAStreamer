#pragma once

#include <unordered_map>
#include <iostream>
#include "basic_controller.hpp"

namespace cfx {
	class ControllerManager
	{
	public:
		ControllerManager() {}
		~ControllerManager() {}
		void add_server(const char* name, BasicController* _server)
		{
			serverList.insert(std::make_pair(std::string(name), _server));
		}

		BasicController* getServer(std::string name)  { return serverList.at(name); }

		void Clear()
		{
			serverList.clear();
		}

		void run_services()
		{
			std::cout << " GLSM(GiGAeyes Live Streaming Manager) service loading... "<<std::endl;
			for ( auto& iter : serverList)
			{
				iter.second->accept().wait();
				std::cout << "server name :[" << iter.first<<"] is now listening at \""<<iter.second->get_endpoint()<<"\""<<std::endl;
			}
		}

		void stop_services()
		{
			for ( auto& iter : serverList)
			{
				iter.second->shutdown().wait();
				std::cout << "server name :[" << iter.first<<"] is shutdown"<<std::endl;
			}
		}
	protected:
		std::unordered_map<std::string, BasicController*> serverList;
	};
}
