#pragma once

#include "cpprest/http_client.h"
#include <cpprest/json.h>
#include <iostream>
#include <random>
#include <boost/date_time/local_time/local_date_time.hpp>
#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>


using namespace std;
using namespace web;
using namespace utility;
using namespace http;
using namespace http::client;

#define FROMJSON_INT(key) result.key = item[U(#key)].as_integer();
#define FROMJSON_BOOL(key) result.key = item[U(#key)].as_bool();
#define FROMJSON_STR(key) result.key = item[U(#key)].as_string();
#define FROMJSON_CHILD(key, childclass) result.key = childclass::fromJson(item[U(#key)]);

#define ASJSON_INT(key) result[U(#key)] = web::json::value::number(key);
#define ASJSON_BOOL(key) result[U(#key)] = web::json::value::boolean(key);
#define ASJSON_STR(key) result[U(#key)] = web::json::value::string(key);
#define ASJSON_CHILD(key, child) result[U(#key)] = child.asJson();
#define TRIM_SPACE ' '

namespace cfx {

using namespace std;

	http_response CheckResponse(const std::string &url, const http_response &response)
	{
		ucout << response.to_string() << endl;
		return response;
	}

//	http_response CheckResponse(const std::string &url, const http_response &response, bool &refresh)
//	{
//		ucout << response.to_string() << endl;
//		BJPutResponse answer = BJPutResponse::FromJSON(response.extract_json().get());
//		refresh = answer.Status == ST_Refresh;
//		return response;
//	}

	inline int getRandomNumber ( int min, int max )
	{
		//< 1단계. 시드 설정
		random_device rn;
		mt19937_64 rnd( rn() );

		//< 2단계. 분포 설정 ( 정수 )
		uniform_int_distribution<int> range(min, max);

		//< 3단계. 값 추출
		return range( rnd );
	}

	inline std::string current_time() {

		const boost::posix_time::ptime now =
							   boost::posix_time::second_clock::local_time();

		const boost::format f = boost::format("%s%02d%02d%02d%02d%02d")
			% now.date().year_month_day().year
			% now.date().year_month_day().month.as_number()
			% now.date().year_month_day().day.as_number()
			% now.time_of_day().hours()
			% now.time_of_day().minutes()
			% now.time_of_day().seconds();

		const std::string result = f.str();
	 return result;
	}

	typedef struct _PlatformEvent
	{
		utility::string_t event_id;
		utility::string_t event_type;
		utility::string_t event_time;
		utility::string_t thumb_image_path;

		_PlatformEvent() {}
		_PlatformEvent(utility::string_t _event_id, utility::string_t _event_type, utility::string_t _event_time, utility::string_t _thumb_image_path)
		: event_id(_event_id), event_type(_event_type), event_time(_event_time), thumb_image_path(_thumb_image_path) {}
		~_PlatformEvent() {}

		web::json::value asJson() const
		{
			web::json::value result = web::json::value::object();
			ASJSON_STR(event_id)
			ASJSON_STR(event_type)
			ASJSON_STR(event_time)
			ASJSON_STR(thumb_image_path)
			return result;
		}

	}PlatformEvent;

	typedef struct _IIFEvent
	{
		utility::string_t result;
		utility::string_t msg;
		utility::string_t acc;
		utility::string_t url;
		utility::string_t face_id;
		utility::string_t json;
		utility::string_t acc_top;

		static _IIFEvent fromJson(const web::json::value& object)
		{
			_IIFEvent result;
			auto item = object.as_object();
			FROMJSON_STR(msg)
			FROMJSON_STR(acc)
			FROMJSON_STR(url)
			FROMJSON_STR(face_id)
			FROMJSON_STR(json)
			FROMJSON_STR(acc_top)
			return result;
		}

		web::json::value asJson() const
		{
			web::json::value result = web::json::value::object();
			ASJSON_STR(msg)
			ASJSON_STR(acc)
			ASJSON_STR(url)
			ASJSON_STR(face_id)
			ASJSON_STR(json)
			ASJSON_STR(acc_top)

//			result["event_time"] = web::json::value::string(current_time());
//			ASJSON_STR(thumb_image_path)
//			result["face_info"] = web::json::value::null();
//			ASJSON_CHILD(recognition_info, recognition_info)
			return result;
		}

	}IIFEvent;
/*
	inline std::string trimStr(std::string& s,char drop = TRIM_SPACE)
	{
		std::string r=s.erase(s.find_last_not_of(drop)+1);
		return r.erase(0,r.find_first_not_of(drop));
	}
	inline std::map<utility::string_t, utility::string_t> splitStr( utility::string_t &query)
	{
		std::map<utility::string_t, utility::string_t> results;

		// Split into key value pairs separated by '&'.
		size_t prev_amp_index = 0;
		while(prev_amp_index != utility::string_t::npos)
		{
			size_t amp_index = query.find_first_of(_XPLATSTR(','), prev_amp_index);
			if (amp_index == utility::string_t::npos)
				amp_index = query.find_first_of(_XPLATSTR(';'), prev_amp_index);

			utility::string_t key_value_pair = query.substr(
				prev_amp_index,
				amp_index == utility::string_t::npos ? query.size() - prev_amp_index : amp_index - prev_amp_index);
			prev_amp_index = amp_index == utility::string_t::npos ? utility::string_t::npos : amp_index + 1;

			size_t equals_index = key_value_pair.find_first_of(_XPLATSTR(':'));
			if(equals_index == utility::string_t::npos)
			{
				continue;
			}
			else if (equals_index == 0)
			{
				utility::string_t value(key_value_pair.begin() + equals_index + 1, key_value_pair.end());
				results[_XPLATSTR("")] = value;
			}
			else
			{
				utility::string_t key(key_value_pair.begin(), key_value_pair.begin() + equals_index);
				utility::string_t value(key_value_pair.begin() + equals_index + 1, key_value_pair.end());
			results[key] = value;
		}
		}

		return results;
	}
*/
	class Client
	{
	public :
		Client() {}
		Client(utility::string_t _address, utility::string_t _port) : address(_address), port(_port)
		{
			address.append(port);
		}
		~Client() {}
		void send_message(int id, json::value response)
		{
//			utility::string_t uri_camid = U("/v2");

			utility::string_t uri_camid = U("/V100/CTRL-52105");
			switch(id)
			{
			case 1:
				uri_camid.append("/cam0000000417");
				break;
			case 2:
				uri_camid.append("/cam0000000420");
				break;
			case 3:
				break;
			case 4:
				break;
			default :
				break;
			}

			http::uri uri = http::uri(address);
			http_client client(http::uri_builder(uri).append_path(uri_camid).to_uri());
			std::cout<<"from camid: "<<id<<", send to : "<<client.base_uri().to_string()<<std::endl;
			std::cout<<"json data : "<<response.serialize()<<std::endl;

			IIFEvent iifEvent = IIFEvent::fromJson(response);

//			auto acc = response["acc"].as_string();
//			auto json = response["url"].as_string();
//			auto face_id = response["face_id"].as_string();

//			auto temp = trimStr(json);
//			temp.erase(std::remove(temp.begin(), temp.end(), ' '), temp.end());
//			std::map<utility::string_t, utility::string_t> map = splitStr(temp);
//			auto face_id = map["\'face_id\'"];
			auto completeData = json::value::object();
//utility::string_t _event_id, utility::string_t _event_type, utility::string_t _event_time, utility::string_t _thumb_image_path)
			auto eventid = getRandomNumber (0, 999999999);
			char num[10];
			snprintf(num, 10, "%9d", eventid);
			PlatformEvent platformEvent = PlatformEvent(utility::string_t(num), "180", utility::string_t(current_time()), iifEvent.url);

			auto eventData = platformEvent.asJson();

			eventData["face_info"] = web::json::value::null();

			auto recognition_info = json::value::object();
			recognition_info["face_id"] = web::json::value::string(iifEvent.face_id);
			recognition_info["acc"] = web::json::value::string(iifEvent.acc);

			eventData["recognition_info"] = recognition_info;
			completeData=eventData;
			std::cout<<"send data : "<<eventData.serialize()<<std::endl;
			CheckResponse(client.base_uri().to_string(), client.request(methods::POST,U(""), completeData).get());
		}


	protected:
		utility::string_t address;
		utility::string_t port;
	};

}
