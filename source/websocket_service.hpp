/*
 * websocket_service.hpp
 *
 *  Created on: 2018. 6. 21.
 *      Author: jangsueun
 */
#pragma once

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/extensions/permessage_deflate/enabled.hpp>
#include <iostream>
#include <map>

struct testee_config : public websocketpp::config::asio
{
	// pull default settings from our core config
	typedef websocketpp::config::asio core;

	typedef core::concurrency_type concurrency_type;
	typedef core::request_type request_type;
	typedef core::response_type response_type;
	typedef core::message_type message_type;
	typedef core::con_msg_manager_type con_msg_manager_type;
	typedef core::endpoint_msg_manager_type endpoint_msg_manager_type;

	typedef core::alog_type alog_type;
	typedef core::elog_type elog_type;
	typedef core::rng_type rng_type;
	typedef core::endpoint_base endpoint_base;

	static bool const enable_multithreading = true;

	struct transport_config : public core::transport_config {
		typedef core::concurrency_type concurrency_type;
		typedef core::elog_type elog_type;
		typedef core::alog_type alog_type;
		typedef core::request_type request_type;
		typedef core::response_type response_type;

		static bool const enable_multithreading = true;
	};

	typedef websocketpp::transport::asio::endpoint<transport_config>
		transport_type;
//
//	static const websocketpp::log::level elog_level =
//		websocketpp::log::elevel::none;
//	static const websocketpp::log::level alog_level =
//		websocketpp::log::alevel::none;

	/// permessage_compress extension
	struct permessage_deflate_config {};

	typedef websocketpp::extensions::permessage_deflate::enabled
		<permessage_deflate_config> permessage_deflate_type;
};
typedef websocketpp::server<testee_config> server;
typedef server::message_ptr message_ptr;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::thread> thread_ptr;
//typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using websocketpp::connection_hdl;

struct connection_data {
    int sessionid;
    std::string name;
    connection_hdl hdl;
};

class WebSocketService
{
public:
	WebSocketService(uint16_t _port, int _thread) : websocket_server(), port(_port), threads_num(_thread)
	{

	}
	~WebSocketService()
	{
		if(log.is_open())
		{
			log.close();
		}
	}
	void initialize()
	{

	    try
	    {
//	        // Set logging settings
	        websocket_server.set_access_channels(websocketpp::log::alevel::all);
	        websocket_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

	        log.open("./log");
	        websocket_server.get_alog().set_ostream(&log);
	        websocket_server.get_elog().set_ostream(&log);

	        // Initialize ASIO
	        websocket_server.init_asio();
	        websocket_server.set_reuse_addr(true);
	        websocket_server.set_listen_backlog(8086);


	        // Register our message handler
	        websocket_server.set_message_handler(bind(&WebSocketService::on_message,this,::_1,::_2));
	        websocket_server.set_socket_init_handler(bind(&WebSocketService::on_socket_init, this,::_1,::_2));
	        websocket_server.set_open_handler(bind(&WebSocketService::on_open,this,::_1));
	        websocket_server.set_close_handler(bind(&WebSocketService::on_close,this,::_1));

	    } catch (websocketpp::exception const & e)
	    {
	        std::cout << e.what() << std::endl;
	    } catch (websocketpp::lib::error_code& e) {
	        std::cout << e.message() << std::endl;
	    } catch (...)
	    {
	        std::cout << "other exception" << std::endl;
	    }
	}

	void on_open(connection_hdl hdl)
	{
		try
		{
			connection_data data;
			std::cout<<"client connected with hdl: " << hdl.lock().get()
					 <<", camid: "<<websocket_server.get_con_from_hdl(hdl)->get_uri()->get_queryParam("camid")
					 <<std::endl;
			data.sessionid = std::stoi(websocket_server.get_con_from_hdl(hdl)->get_uri()->get_queryParam("camid"));
			data.name.clear();
			data.name = "CAM" + std::to_string(data.sessionid);
			data.hdl = hdl;
			m_connections[hdl] = data;
		}
		catch(websocketpp::exception const & e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	void on_close(connection_hdl hdl)
	{
		connection_data& data = get_data_from_hdl(hdl);

		std::cout << "Closing connection " << data.name
				  << " with sessionid " << data.sessionid << std::endl;

		m_connections.erase(hdl);
	}
    void on_message(connection_hdl hdl, server::message_ptr msg)
    {
        connection_data& data = get_data_from_hdl(hdl);

//        if (data.name.empty()) {
//            data.name = msg->get_payload();
//            std::cout << "Setting name of connection with sessionid "
//                      << data.sessionid << " to " << data.name << std::endl;
//        } else {
        std::cout << "on_message called with hdl: " << hdl.lock().get()
				  << "Got a message from connection " << data.name
                  << " with sessionid " << data.sessionid << std::endl;
//        }
    }
    bool send_message( int id, json::value responseData)
    {
    	connection_hdl hdl;
    	bool found = false;
		if(m_connections.size() != 0)
		{
			for( auto data : m_connections)
			{
				std::cout<<"data.second.sessionid: "<<data.second.sessionid<<", id:"<<id<<std::endl;
				if( data.second.sessionid == id)
				{
					found = true;
					hdl = data.first;
					break;
				}
			}
			if(found)
			{
				websocket_server.send(hdl, responseData.serialize(), websocketpp::frame::opcode::text);
			}
		}
		return found;
    }
//    void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg)
//    {
//    	std::cout<<"remote ip:"<<s->get_connection()->get_remote_endpoint()<<std::endl;
//    	std::cout<<"client ip"<<s->get_con_from_hdl(hdl)->get_raw_socket().remote_endpoint().address()<<std::endl;
//    //	std::cout<<"get_host() ip"<<s->get_con_from_hdl(hdl)->get_origin()<<std::endl;
//
//        std::cout << "on_message called with hdl: " << hdl.lock().get()
//                  << " and message: " << msg->get_payload()
//                  << std::endl;\
//    	if (msg->get_payload() == "stop-listening") {
//    		  s->stop_listening();
//    		  return;
//    	  }
//        try {
//            s->send(hdl, msg->get_payload(), msg->get_opcode());
//        } catch (const websocketpp::lib::error_code& e) {
//            std::cout << "Echo failed because: " << e
//                      << "(" << e.message() << ")" << std::endl;
//        }
//    }

    void on_socket_init(websocketpp::connection_hdl, boost::asio::ip::tcp::socket & s) {
        boost::asio::ip::tcp::no_delay option(true);
        s.set_option(option);
    }

    connection_data& get_data_from_hdl(connection_hdl hdl)
    {
        auto it = m_connections.find(hdl);

        if (it == m_connections.end()) {
            // this connection is not in the list. This really shouldn't happen
            // and probably means something else is wrong.
            throw std::invalid_argument("No data available for session");
        }

        return it->second;
    }
	void run()
	{
		websocket_server.listen(port);
		websocket_server.start_accept();
		if( threads_num == 1)
		{
			websocket_server.run();
		}
		else
		{
			for (size_t i = 0; i < threads_num; i++) {
				ts.push_back(websocketpp::lib::make_shared<websocketpp::lib::thread>(&server::run, &websocket_server));
			}

			for (size_t i = 0; i < threads_num; i++) {
				ts[i]->join();
			}
		}
        std::cout<<"websocket run !!"<<std::endl;
	}
private:
    typedef std::map<connection_hdl,connection_data,std::owner_less<connection_hdl>> con_list;
//    server m_server;
    server websocket_server;
    con_list m_connections;
    std::vector<thread_ptr> ts;
    std::ofstream log;
    uint16_t port;
    int threads_num;
};
// Define a callback to handle incoming messages
