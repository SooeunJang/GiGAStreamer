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

#include <std_micro_service.hpp>
#include "microsvc_controller.hpp"
#include "glsm_utils.hpp"

using namespace web;
using namespace http;
using namespace glsm;

void MicroserviceController::initRestOpHandlers() {
    _listener.support(methods::GET, std::bind(&MicroserviceController::handleGet, this, std::placeholders::_1));
    _listener.support(methods::PUT, std::bind(&MicroserviceController::handlePut, this, std::placeholders::_1));
    _listener.support(methods::POST, std::bind(&MicroserviceController::handlePost, this, std::placeholders::_1));
    _listener.support(methods::DEL, std::bind(&MicroserviceController::handleDelete, this, std::placeholders::_1));
}

void MicroserviceController::handleGet(http_request message) {
    auto path = requestPath(message); //same as "path = http::uri::split_path(http::uri::decode(message.relative_uri().path()));"
    if (!path.empty()) {
    	std::string command;
    	std::string camid;
    	web::json::value response;

    	std::map<std::string, json::value> incommingStreamList;

		json::value temp;
		json::value incommingStream;
		temp["RTMP"] = json::value::number(0);
		temp["MPEGDASH"] = json::value::number(0);
		temp["CUPERTINO"] = json::value::number(0);
		temp["SANJOSE"] = json::value::number(0);
		temp["SMOOTH"] = json::value::number(0);
		temp["RTP"] = json::value::number(1);
//    				connectionCount.push_back(temp);
		incommingStream["serverName"] = json::value::string("_defaultServer_");
		incommingStream["applicationInstance"] = json::value::string("_definst_");
		incommingStream["name"] = json::value::string("cam00001.stream");
		incommingStream["uptime"] = json::value::number(610335);
		incommingStream["bytesIn"] = json::value::number(214354352366);
		incommingStream["bytesOut"] = json::value::number(109897525);
		incommingStream["bytesInRate"] = json::value::number(358423);
		incommingStream["bytesOutRate"] = json::value::number(122058);
		incommingStream["totalConnections"] = json::value::number(1);
		incommingStream["connectionCount"] = temp;
		incommingStreamList.insert(std::make_pair("cam00001", incommingStream));

		json::value incommingStream2;
		incommingStream2["serverName"] = json::value::string("_defaultServer_");
		incommingStream2["applicationInstance"] = json::value::string("_definst_");
		incommingStream2["name"] = json::value::string("cam00002.stream");
		incommingStream2["uptime"] = json::value::number(610335);
		incommingStream2["bytesIn"] = json::value::number(214354352366);
		incommingStream2["bytesOut"] = json::value::number(109897525);
		incommingStream2["bytesInRate"] = json::value::number(358423);
		incommingStream2["bytesOutRate"] = json::value::number(122058);
		incommingStream2["totalConnections"] = json::value::number(1);
		incommingStream2["connectionCount"] = temp;
		incommingStreamList.insert(std::make_pair("cam00002", incommingStream2));

		json::value incommingStream3;
		incommingStream3["serverName"] = json::value::string("_defaultServer_");
		incommingStream3["applicationInstance"] = json::value::string("_definst_");
		incommingStream3["name"] = json::value::string("cam00003.stream");
		incommingStream3["uptime"] = json::value::number(610335);
		incommingStream3["bytesIn"] = json::value::number(214354352366);
		incommingStream3["bytesOut"] = json::value::number(109897525);
		incommingStream3["bytesInRate"] = json::value::number(358423);
		incommingStream3["bytesOutRate"] = json::value::number(122058);
		incommingStream3["totalConnections"] = json::value::number(1);
		incommingStream3["connectionCount"] = temp;
		incommingStreamList.insert(std::make_pair("cam00003", incommingStream3));


//http://${streamer_ip}:8087/v2/servers/_defaultServer_/vhosts/_defaultVHost_/applications/gigaeyeslive/instances/_definst_/incommingStreams/${streamfile}.stream/monitoring/current
    	for(std::vector<std::string>::reverse_iterator riter = path.rbegin(); riter != path.rend(); ++riter)
    	{
    		if(*riter == "_definst_" && riter == path.rbegin() )
			{
    			if(*(riter+1) == "instances")
    			{
					std::vector<web::json::value> incommingStreams;
					json::value temp;
					temp["name"] = json::value::string("cam00001.stream");
					temp["applicationInstance"] = json::value::string("_definst_");
					temp["isStreamManagerStream"] = json::value::boolean(true);
					incommingStreams.push_back(temp);

					json::value temp2;
					temp2["name"] = json::value::string("cam00002.stream");
					temp2["applicationInstance"] = json::value::string("_definst_");
					temp2["isStreamManagerStream"] = json::value::boolean(true);
					incommingStreams.push_back(temp2);

					json::value temp3;
					temp3["name"] = json::value::string("cam00003.stream");
					temp3["applicationInstance"] = json::value::string("_definst_");
					temp3["isStreamManagerStream"] = json::value::boolean(true);
					incommingStreams.push_back(temp3);

					response["incommingStreams"] = json::value::array(incommingStreams);

			    	response["name"] = json::value::string("_definst_");
			    	response["outgoingStreams"] = web::json::value::null();
			    	response["serverName"] = json::value::string("_defaultServer_");
			    	response["streamGroups"] = web::json::value::null();
					break;
    			}
			}
    		else if(*riter == "current" && riter == path.rbegin())
    		{
    			if(*(riter+3) == "incommingstreams")
    			{
    				std::string streamName = *(riter+2);
    				camid = (*(riter+2)).substr(0, (*(riter+2)).find('.'));
    				std::cout<<"camid :"<<camid<<std::endl;
    				response = incommingStreamList.find(camid)->second;
    				break;
    			}
    		}
    	}



    	message.reply(status_codes::OK, response);
//    	for( auto const iter : path )
//    	{
//			if(iter.compare("monitoring") != 0 || iter.compare("current") != 0)
//			{
//				auto response = json::value::object();
//				response["byteIn"] = json::value::string("123456");
//				response["status"] = json::value::string("ready!");
//				message.reply(status_codes::OK, response);
//			}
//			elseif
//			else
//			{
//				message.reply(status_codes::NotFound);
//			}
//    	}
    }
    else {
    	std::cout<<"host: "<<message.request_uri().host()<<std::endl;
    	std::cout<<"user_info: "<<message.request_uri().user_info()<<std::endl;
    	std::cout<<"content_type: "<<message.headers().content_type()<<std::endl;
    	auto response = json::value::object();
		response["Message"] = json::value::string("GLSM Streaming Engine 4 Perpetual Pro Edition 4.7.1 build20635");
		message.reply(status_codes::OK, response);
//        message.reply(status_codes::NotFound);
    }
}

void MicroserviceController::handlePut(http_request message) {
    auto path = requestPath(message); //same as "path = http::uri::split_path(http::uri::decode(message.relative_uri().path()));"
    if (!path.empty()) {
    	std::string command;
    	std::string camid;
    	web::json::value response;
    	for(std::vector<std::string>::reverse_iterator riter = path.rbegin(); riter != path.rend(); ++riter)
    	{
    		if(*riter == "adv")
    		{
    			command = *riter;
    			camid = *(riter+1);
    			auto glsm = GlsmParserBase(command.c_str(), camid.c_str());
    			response = glsm.doProcess(message.extract_json().get());
    			break;
    		} else if(*riter == "connect")
    		{
    			command = *riter;
    			camid = *(riter+2);
    			auto glsm = GlsmParserBase(command.c_str(), camid.c_str());
    			response = glsm.doProcess(message.extract_json().get());
    			break;
    		} else if(*riter == "disconnectStream")
			{
    			command = *riter;
    			camid = (*(riter+2)).substr(0, (*(riter+2)).find('.'));
    			auto glsm = GlsmDisconnectStream(command.c_str(), camid.c_str());
    			response = glsm.doProcess(message.extract_json().get());
    			break;
			}
//    		if(iter.compare("streamfiles") != 0)
//    		{
//    			if(*(iter + 1).find(".stream") == std::string::npos)
//    			{
//    				std::string camid(*(iter+1));
//    			}
//    			if( *(iter + 2).compare("adv") != 0 )
//				{
//					auto response = json::value::object();
//					response["command"] = json::value::string("modify");
//					message.reply(status_codes::OK, response);
//				}
//				else if( iter.compare("connect") != 0 )
//				{
//
//				}
//    		}
//			else
//			{
//				message.reply(status_codes::NotFound);
//			}
    	}

		message.reply(status_codes::OK, response);
//    	auto response = json::value::object();
//		response["command"] = json::value::string(camid);
//		message.reply(status_codes::OK, response);
    }
    else {
        message.reply(status_codes::NotFound);
    }
}

void MicroserviceController::handlePost(http_request message) {
	auto path = requestPath(message); //same as "path = http::uri::split_path(http::uri::decode(message.relative_uri().path()));"
	if (!path.empty()) {

//		auto glsm = GlsmParserBase();
//		auto response = glsm.doProcess(message.extract_json().get());
		auto response = json::value::object();
		response["command"] = json::value::string("post");
		message.reply(status_codes::OK, response);
	}
	else
	{
		message.reply(status_codes::NotFound);
	}
}

void MicroserviceController::handleDelete(http_request message) {    
	auto path = requestPath(message);
	if (!path.empty()) {
		//TODO delete stream
		auto glsm = GlsmParserBase("delete","cam0002");
		auto response = glsm.doProcess(message.extract_json().get());
		message.reply(status_codes::OK, response);
	}
	else
	{
		message.reply(status_codes::NotFound);
	}
}

void MicroserviceController::handlePatch(http_request message) {
    message.reply(status_codes::NotImplemented, responseNotImpl(methods::PATCH));
}

void MicroserviceController::handleHead(http_request message) {
    message.reply(status_codes::NotImplemented, responseNotImpl(methods::HEAD));
}

void MicroserviceController::handleOptions(http_request message) {
    message.reply(status_codes::NotImplemented, responseNotImpl(methods::OPTIONS));
}

void MicroserviceController::handleTrace(http_request message) {
    message.reply(status_codes::NotImplemented, responseNotImpl(methods::TRCE));
}

void MicroserviceController::handleConnect(http_request message) {
    message.reply(status_codes::NotImplemented, responseNotImpl(methods::CONNECT));
}

void MicroserviceController::handleMerge(http_request message) {
    message.reply(status_codes::NotImplemented, responseNotImpl(methods::MERGE));
}

json::value MicroserviceController::responseNotImpl(const http::method & method) {
    auto response = json::value::object();
    response["serviceName"] = json::value::string("GLSM Service");
    response["http_method"] = json::value::string(method);
    return response ;
}
