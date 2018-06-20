
#include "basic_controller.hpp"
#include "network_utils.hpp"
using namespace utility;

namespace cfx {
    BasicController::BasicController(const char* _serverName, const char* _serverHost, const char* _appName, const char* _appInstance)
    {
    	serverInfo = new ServerInfo(_serverName, _serverHost, _appName, _appInstance);
    }
    BasicController::BasicController(ServerInfo* _serverInfo)
	{
		serverInfo = _serverInfo;
	}
    BasicController::BasicController(void)
	{
		serverInfo = NULL;
	}
    BasicController::~BasicController()
    {
    	delete serverInfo;
    }
    void BasicController::initialize(const std::string & value, utility::seconds t)
    {
    	setEndpoint(value, t);
    }

    void BasicController::setListenerPath(uri_builder& routing_path)
    {
    	return;
    }

    void BasicController::setEndpoint(const std::string & value, utility::seconds t) {

    	if(!uri::validate(value))
        {
        	throw;
        }

    	auto endpointURI = uri(value);
        auto endpointBuilder = uri_builder();

        endpointBuilder.set_scheme(endpointURI.scheme());
        if (endpointURI.host() == "host_auto_ip4") {
            endpointBuilder.set_host(NetworkUtils::hostIP4(endpointURI.port()));
        }
        else if (endpointURI.host() == "host_auto_ip6") {
            endpointBuilder.set_host(NetworkUtils::hostIP6(endpointURI.port()));
        }
        else
        {
        	endpointBuilder.set_host(NetworkUtils::hostIP(AF_UNSPEC, endpointURI.port()));
        }
        endpointBuilder.set_port(endpointURI.port());
        endpointBuilder.set_path(endpointURI.path());

        setListenerPath(endpointBuilder);
        _listener = http_listener(endpointBuilder.to_uri());
        _config.set_timeout(t);
    }

    pplx::task<void> BasicController::accept() {
        initRestOpHandlers();
        return _listener.open();
    }

    pplx::task<void> BasicController::shutdown() {
        return _listener.close();
    }

    std::vector<utility::string_t> BasicController::requestPath(const http_request & message) {
        auto relativePath = uri::decode(message.relative_uri().path());
        return uri::split_path(relativePath);        
    }
    json::value BasicController::responseNotImpl(const http::method & method) {
        auto response = json::value::object();
        utility::string_t res_msg= _listener.uri().to_string();
        response["uri_info"] = json::value::string(res_msg);
        response["serviceName"] = json::value::string("GLSM Service");
        response["http_method"] = json::value::string(method);
        return response ;
    }
}
