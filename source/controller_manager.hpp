#pragma once

#include <unordered_map>
#include <iostream>
#include <thread>
#include <mutex>
#include "basic_controller.hpp"
#include "websocket_service.hpp"
#include "rest_client.hpp"

namespace cfx {
	extern std::mutex mapmutex;
	class ControllerManager
	{
	public:
		ControllerManager()
		{
			websocket = new WebSocketService(8080, 1);
			websocket->initialize();
			restclient = new Client("http://211.54.3.139:", "28080");
		}
		~ControllerManager()
		{
			delete websocket;
			delete restclient;
		}
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
			for ( auto iter : serverList)
			{
				iter.second->accept().wait();
				std::cout << "server name :[" << iter.first<<"] is now listening at \""<<iter.second->get_endpoint()<<"\""<<std::endl;
			}
			websocket->run();
		}

		void stop_services()
		{
			for ( auto iter : serverList)
			{
				iter.second->shutdown().wait();
				std::cout << "server name :[" << iter.first<<"] is shutdown"<<std::endl;
			}
		}

		void set_eventQueue()
		{
			for ( auto iter : serverList)
			{
				iter.second->set_eventQueue(&eventQueue);
			}
		}

		void get_eventQueue()
		{
			std::this_thread::sleep_for(std::chrono::seconds(2));
			while(1)
			{
				if(!eventQueue.empty())
				{
					std::cout<<"get message from IIF server!!!"<<std::endl;
					try{
						for( auto iter = eventQueue.begin(); iter != eventQueue.end(); ++iter )
						{
							if(!websocket->send_message(iter->get_id(), iter->get_data()))
							{
								throw std::invalid_argument("No camera available for session");
							}
							restclient->send_message(iter->get_id(), iter->get_data());
						}
					}
					catch (websocketpp::exception const & e)
					{
						std::cout << e.what() << std::endl;
					} catch (websocketpp::lib::error_code& e) {
						std::cout << e.message() << std::endl;
					} catch (...)
					{
						std::cerr << "no connection. connect device first on own cam id :"<< std::endl;
					}
					if(mapmutex.try_lock())
					{
						eventQueue.clear();
						mapmutex.unlock();
					}
					continue;
				}
			}
		}
	protected:
		std::unordered_map<std::string, BasicController*> serverList;
		EventQueue eventQueue;
		WebSocketService* websocket;
		Client* restclient;
	};
}
