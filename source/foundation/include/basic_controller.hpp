
#pragma once

#include <string>
#include <cpprest/http_listener.h>
#include <pplx/pplxtasks.h>
#include "controller.hpp"
#include "event.hpp"
#include <thread>
#include <mutex>

using namespace web;
using namespace http::experimental::listener;

namespace cfx
{
	extern std::mutex mapmutex;
	typedef struct _ServerInfo
	{
		const char* serverName;
		const char* serverHost;
		const char* appName;
		const char* appInstance;

		_ServerInfo(const char* _serverName, const char* _serverHost, const char* _appName, const char* _appInstance)
		{
			serverName = _serverName;
			serverHost = _serverHost;
			appName = _appName;
			appInstance = _appInstance;
		}
		const char* get_serverName() { return serverName; }
		const char* get_serverHost() { return serverHost; }
		const char* get_appName() { return appName; }
		const char* get_appInstance() { return appInstance; }

		void printInfo(std::iostream &ss)
		{
			ss<<"serverName: "<<serverName<<" , serverHost:"<<serverHost<<" , appName:"<<appName<<" , appInstance:"<<appInstance<<std::endl;
		}
	}ServerInfo;

    class BasicController {
    public:
        BasicController(const char* _serverName, const char* _serverHost, const char* _appName, const char* _appInstance);
        BasicController(ServerInfo* _serverInfo);
        BasicController(void);
        virtual ~BasicController();

        void initialize(const std::string & value, utility::seconds t);
        pplx::task<void> accept();
        pplx::task<void> shutdown();

        //getter & setter
        ServerInfo* get_serverInfo() { return serverInfo; }
        http_listener* get_listener() { return &_listener; }
        http_listener_config* get_listenerConfig() { return &_config; }
        std::string get_endpoint() const { return _listener.uri().to_string(); }
        virtual void set_eventQueue(EventQueue* _eventQueue)
        {
        	this->eventQueue = _eventQueue;
        }

    protected:
        http_listener _listener;
        http_listener_config _config;
        ServerInfo* serverInfo;
        EventQueue* eventQueue;
        std::vector<utility::string_t> requestPath(const http_request & message);
        std::map<utility::string_t, utility::string_t> parseQuery(const http_request & message);
        json::value responseNotImpl(const http::method & method);
        virtual void setListenerPath(uri_builder& routing_path);
        virtual void initRestOpHandlers() = 0;

    private:
        void setEndpoint(const std::string & value, utility::seconds t);
    };
}
