#pragma once 

#include <basic_controller.hpp>

using namespace cfx;

class StreamController : public BasicController, Controller {
public:
    StreamController(const char* _serverName, const char* _serverHost, const char* _appName, const char* _appInstance)
		: BasicController(_serverName, _serverHost, _appName, _appInstance) {}
    virtual ~StreamController() {}

    void handleGet(http_request message) override;
    void handlePut(http_request message) override;
    void handlePost(http_request message) override;
    void handleDelete(http_request message) override;

protected:
    virtual void setListenerPath(uri_builder& routing_path);
    void initRestOpHandlers() override;
};
