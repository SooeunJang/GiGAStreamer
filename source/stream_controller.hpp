#pragma once 

#include <basic_controller.hpp>

using namespace cfx;

class StreamController : public BasicController, Controller {
public:
    StreamController(ServerInfo* _serverInfo) : BasicController(_serverInfo) {}
    StreamController(void) : BasicController() {}
    virtual ~StreamController() {}

    void handleGet(http_request message) override;
    void handlePut(http_request message) override;
    void handlePost(http_request message) override;
    void handleDelete(http_request message) override;

protected:
    virtual void setListenerPath(uri_builder& routing_path);
    void initRestOpHandlers() override;
};
