
#pragma once

#include <string>
#include <cpprest/http_listener.h>
#include <pplx/pplxtasks.h>
#include <thread>

#include "glsm_baseheaders.hpp"
#include "runtime_utils.hpp"
#include "controller.hpp"
#include "event.hpp"
#include "gls_interface.hpp"

using namespace web;
using namespace http::experimental::listener;
using namespace glsm;

namespace cfx
{
	extern std::mutex mapmutex;

    class BasicController {
    public:
		typedef std::unordered_map<utility::string_t, StreamFiles> StreamFileList;
		typedef std::unordered_map<utility::string_t, ProfileStatusListV0> incomingStreamList;
        BasicController(ServerInfo* _serverInfo);
        BasicController(void);
        virtual ~BasicController();
        virtual void initialize(const std::string & value);
        //getter & setter
        virtual ServerInfo* get_serverInfo() { return serverInfo; }
        virtual http_listener* get_listener() { return &_listener; }
        virtual http_listener_config* get_listenerConfig() { return &_config; }
        virtual std::string get_endpoint() const { return _listener.uri().to_string(); }
        virtual void set_eventQueue(EventQueue* _eventQueue) { this->eventQueue = _eventQueue; }
        virtual void set_streamer(GLSinterface* _gls_interface) { this->gls_interface = _gls_interface; }
        virtual GLSinterface* get_streamer() { return this->gls_interface; }
        pplx::task<void> accept();
        pplx::task<void> shutdown();

    protected:
        http_listener _listener;
        http_listener_config _config;
        ServerInfo* serverInfo;
        EventQueue* eventQueue;
        GLSinterface* gls_interface;
        std::vector<utility::string_t> requestPath(const http_request & message);
        std::map<utility::string_t, utility::string_t> parseQuery(const http_request & message);
        json::value responseNotImpl(const http::method & method);
        json::value responseNotFound(const http::method & method);
        json::value make_response(bool result, utility::string_t message, const json::value& data);
        virtual void setListenerPath(uri_builder& routing_path);
        virtual void initRestOpHandlers() = 0;
        StreamFileList streamfilelist;
        incomingStreamList incomingstreamlist;
    private:
        void setEndpoint(const std::string & value, utility::seconds t);
    };
}
