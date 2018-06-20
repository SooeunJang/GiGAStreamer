
#include "basic_controller.hpp"
#include "network_utils.hpp"
using namespace utility;

namespace cfx {
    BasicController::BasicController(ServerInfo* _serverInfo) : serverInfo(_serverInfo), eventQueue(nullptr), gls_interface(nullptr)
	{
	}
    BasicController::BasicController(void) : serverInfo(nullptr), eventQueue(nullptr), gls_interface(nullptr)
	{
	}
    BasicController::~BasicController()
    {
    	serverInfo = nullptr;
    	delete serverInfo;
    }
    void BasicController::initialize(const std::string & value)
    {
    	std::string endpoint = value;
    	endpoint+= serverInfo->servicePort;
    	setEndpoint(endpoint, (utility::seconds)serverInfo->connection_timeout);
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
    std::map<utility::string_t, utility::string_t> BasicController::parseQuery(const http_request & message) {
		auto relativePath = uri::decode(message.relative_uri().query());
		return uri::split_query(relativePath);
	}
    json::value BasicController::responseNotImpl(const http::method & method) {
        auto response = json::value::object();
        utility::string_t res_msg= _listener.uri().to_string();
        response["uri_info"] = json::value::string(res_msg);
        response["serviceName"] = json::value::string("GLSM Service");
        response["http_method"] = json::value::string(method);
        return response ;
    }

    json::value BasicController::responseNotFound(const http::method & method) {
        auto response = json::value::object();
        response["code"] = json::value::number(status_codes::NotFound);
        response["message"] = json::value::string("The server has not found anything matching the request URI");
        response["success"] = json::value::boolean(false);
        return response ;
    }

    json::value BasicController::make_response(bool result, utility::string_t message, const json::value& data)
    {
    	auto response = json::value::object();
    	response["success"] = web::json::value::boolean(result);
    	response["message"] = web::json::value::string(message);
    	response["data"] = data;
    	return response;
    }
}
