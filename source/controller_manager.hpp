#pragma once

#include <unordered_map>
#include <iostream>
#include <thread>
#include <mutex>
#include "basic_controller.hpp"

namespace cfx {
	extern std::mutex mapmutex;
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
			for ( auto iter : serverList)
			{
				iter.second->accept().wait();
				std::cout << "server name :[" << iter.first<<"] is now listening at \""<<iter.second->get_endpoint()<<"\""<<std::endl;
			}
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
				std::cout<<&eventQueue<<std::endl;
				iter.second->set_eventQueue(&eventQueue);
			}
//			std::thread event_thread([&]()
//					{
//						get_eventQueue();
//					});
//			event_thread.join();
		}

		void get_eventQueue()
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
			while(1)
			{
				if(mapmutex.try_lock())
				{
					if(eventQueue.empty())
					{
						mapmutex.unlock();
						continue;
					}
					std::cout<<"!!!!!!!!!!!!!!!!!!!"<<std::endl;
					for( auto iter = eventQueue.begin(); iter != eventQueue.end(); ++iter )
					{
						Event* event = *iter;
						std::cout<<"id: "<<event->get_id()<<", str: "<<event->get_data()<<std::endl;
						if(mapmutex.try_lock())
						{
	//						eventQueue.erase();
							eventQueue.erase(iter);
							mapmutex.unlock();
						}
					}
				}
				else
				{
					std::cout<<"mutex is locked !!!"<<std::endl;
				}
//				continue;
			}
		}
	protected:
		std::unordered_map<std::string, BasicController*> serverList;
		EventQueue eventQueue;
	};
}
