//
//  Created by Ivan Mejia on 12/24/16.
//
// MIT License
//
// Copyright (c) 2016 ivmeroLabs.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//


#include <iostream>
#include <stdlib.h>

#include <usr_interrupt_handler.hpp>
#include <runtime_utils.hpp>

#include "microsvc_controller.hpp"

#define MAJOR_VERSION "0"
#define MINOR_VERSION "1"
#define DEFAULT_PORT_NUM "8087"
#define SERVER_IP_ADDR "221.148.54.130"
#define DAFAULT_CONNECTION_TIMEOUT 1 //sec

using namespace web;
using namespace cfx;

int main(int argc, const char * argv[]) {

	utility::string_t port = U(DEFAULT_PORT_NUM);
	utility::seconds t(DAFAULT_CONNECTION_TIMEOUT);
	if(argc == 2)
	{
		port = argv[1];
	}
	utility::string_t defaultRoute = "http://host_auto_ip4:" + port;// + "/v" + MINOR_VERSION;

	InterruptHandler::hookSIGINT();

	MicroserviceController server;

	try {
		server.setEndpoint(defaultRoute, t);
		// wait for server initialization...
		server.accept().wait();
		std::cout << "GLSM now listening for requests at: " << server.endpoint() << '\n';

		InterruptHandler::waitForUserInterrupt();

		server.shutdown().wait();
	}
	catch(std::exception & e) {
		std::cerr << "Error : "<<e.what() << std::endl;
	}
	catch(...) {
		RuntimeUtils::printStackTrace();
	}
    return 0;
}
