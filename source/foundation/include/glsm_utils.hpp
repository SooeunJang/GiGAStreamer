/*
 * glsm_utils.hpp
 *
 *  Created on: 2018. 6. 12.
 *      Author: jangsueun
 */

#ifndef SOURCE_GLSM_UTILS_HPP_
#define SOURCE_GLSM_UTILS_HPP_

#include <cpprest/json.h>
//#include "json/json.h"
#include <map>

namespace glsm {
//
#pragma pack(push, 1)

//typedef struct _ProtocolCount
//{
//	int RTMP;
//	int MPEGDASH;
//	int CUPERTINO;
//	int SANJOSE;
//	int SMOOTH;
//	int RTP;
//}ProtocolCount;
//
//typedef struct _ServerStatus
//{
//	long int byteIn;
//	long int byteOut;
//	int byteInRate;
//	int byteOutRate;
//	int totalConnection;
//	ProtocolCount connectionCount;
//}ServerStatus;
//

typedef struct _streamInfo
{
	const char* applicationInstance;
	bool isConnected;
	bool isPTZEnabled;
	bool isPublishedToVOD;
	bool isRecordingSet;
	bool isStreamManagerStream;
	const char* name;
	int ptzPollingInterval;
	int ptzPollingIntervalMinimum;
	std::string sourceIp;
	            /*"isConnected": false,
	            "isPTZEnabled": false,
	            "isPublishedToVOD": false,
	            "isRecordingSet": false,
	            "isStreamManagerStream": true,
	            "name": "cam0000000404.stream",
	            "ptzPollingInterval": 2000,
	            "ptzPollingIntervalMinimum": 1000,
	            "sourceIp": "{uri : \"rtsp://admin:ktt15880112%21@192.168.0.240:553/onvif/media?profile=Profile1\",rtspStreamAudioTrack : \"false\",rtspFilterUnknownTracks : \"true\",rtpTransportMode : \"tcp\"}"*/
}StreamInfo;
//
//typedef struct _totalStreamInfo
//{
//	std::vector<StreamInfo*> streamVector;
//
//};

class CJsonObject
{
public:
	std::unordered_map<std::string, std::string>	m_mStrList;
	std::unordered_map<std::string, int>		m_mIntList;
	std::unordered_map<std::string, double>	m_mDblList;
	std::unordered_map<std::string, bool>		m_mBoolList;
	std::vector<CJsonObject>			m_vArrayList;
	std::vector<std::string>			m_vKeyList;
public:
	CJsonObject() {}
	CJsonObject(web::json::value p_jValue, std::vector<std::string> p_vKeyList)
	{
		parse(p_jValue, p_vKeyList);
	}

	void parse(web::json::value p_jValue, std::vector<std::string> p_vKeyList)
	{
		if (!p_vKeyList.empty())
			m_vKeyList = p_vKeyList;

		if (p_jValue.is_array())
		{
			web::json::array A = p_jValue.as_array();
			for (auto v : A)
			{
				m_vArrayList.emplace_back(v, p_vKeyList);
			}
		}
		else if (p_jValue.is_object())
		{
			std::vector<web::json::value> V;

			for (auto sKey : p_vKeyList)
			{
				web::json::value v = p_jValue.at(sKey);
				std::cout<<sKey<<":"<<v.serialize()<<std::endl;

				if (v.is_array())
				{
					web::json::array A = v.as_array();
					for (auto v : A)
					{
						parse(v, p_vKeyList);
					}
				}
				else
				{
					if (v.is_string())
						m_mStrList.emplace(sKey, v.as_string());
					else if (v.is_integer())
						m_mIntList.emplace(sKey, v.as_integer());
					else if (v.is_double())
						m_mDblList.emplace(sKey, v.as_double());
					else if (v.is_boolean())
					{
						m_mBoolList.emplace(sKey, v.as_bool());
					}
				}
			}
		}
	}

	bool hasStr()  { return !m_mStrList.empty();  }
	bool hasInt()  { return !m_mIntList.empty();  }
	bool hasDbl()  { return !m_mDblList.empty();  }
	bool hasBool() { return !m_mBoolList.empty(); }
	bool isArray() { return !m_vArrayList.empty(); }

	std::string	GetString(std::string p_sKey)  { return m_mStrList.at(p_sKey); }
	int		GetInt   (std::string p_sKey)	{ return m_mIntList.at(p_sKey); }
	double		GetDouble(std::string p_sKey)	{ return m_mDblList.at(p_sKey); }
	bool		GetBool  (std::string p_sKey)	{ return m_mBoolList.at(p_sKey); }

	void Clear()
	{
		m_mStrList.clear();
		m_mIntList.clear();
		m_mDblList.clear();
		m_mBoolList.clear();
		m_vArrayList.clear();
	}

	web::json::value GetJsonValue()
	{
		web::json::value V;
		if (isArray())
		{
			std::vector<web::json::value> VJ;
			for (auto JO : m_vArrayList)
				VJ.emplace_back(JO.GetJsonValue());
			V = web::json::value::array(VJ);
		}
		else
		{
			for (auto PS : m_mStrList)
				V[PS.first] = web::json::value::string(PS.second);
			for (auto PI : m_mIntList)
				V[PI.first] = web::json::value::number(PI.second);
			for (auto PD : m_mDblList)
				V[PD.first] = web::json::value::number(PD.second);
			for (auto PB : m_mBoolList)
				V[PB.first] = web::json::value::boolean(PB.second);
		}
		return V;
	}

	void Print()
	{
		std::cout<<GetJsonValue().serialize()<<std::endl;
	}
};



typedef struct _REST_CreateProfile
{
#define ID_ASSERT(id) auto i_##id = value2.as_object().find(#id); \
			if(i_##id == value2.as_object().end()) \
			{ \
				std::string err(#id); \
				throw web::json::json_exception((err+" key not found").c_str()); \
			} \
			std::cout<<i_##id->second.as_string()<<std::endl;
	typedef struct _StreamFiles
	{
		std::string id;
		std::string href;
		static _StreamFiles fromJson(const web::json::value &value)
		{
			_StreamFiles result;
			try
			{
				if(value.is_array())
				{
					for(size_t index = 0; index < value.as_array().size(); ++index)
					{
						const web::json::value &value2 = value.as_array().at(index);
						ID_ASSERT(id);
						result.id = i_id->second.as_string();
						ID_ASSERT(href);
						result.href = i_href->second.as_string();
					}
				}
			}
			catch (const std::exception& e)
			{
				std::cout << e.what() << std::endl;
				std::cout << "Value2 : " << value.serialize() << std::endl;
			}
			return result;
		}
	}StreamFiles;

    std::string restURI;
    StreamFiles streamFiles;

    static _REST_CreateProfile fromJson(const web::json::value &value)
    {
//    	std::vector<std::string> VS{ "restURI", "id", "href" };
//
//    	CJsonObject o(value, VS);
//    	o.Print();

    	_REST_CreateProfile result;
    	try
    	{
    		if(value.is_object())
    		{
    			auto i_resturi = value.as_object().find("restURI");
				if(i_resturi == value.as_object().end())
				{
					throw web::json::json_exception(U("i_resturi key not found"));
				}
				result.restURI = i_resturi->second.as_string();
				result.streamFiles = StreamFiles::fromJson(value.as_object().find("streamFiles")->second);
    		}
    	}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
			std::cout << "Value 1: " << value.serialize() << std::endl;
		}
		return result;
    }
//    web::json::value asJson() const
//    {
//        web::json::value result = web::json::value::object();
//        result[SUIT] = web::json::value::number(suit);
//        result[VALUE] = web::json::value::number(value);
//        return result;
//    }
#undef ID_ASSERT
}REST_CreateProfile;


#pragma pack(pop)

    class GlsmParserBase
    {
    public:
		GlsmParserBase(const char* _command, const char* _camid) : command(_command), camid(_camid)
		{
    	}
    	virtual ~GlsmParserBase()
    	{
    	}

       	virtual web::json::value doProcess(const web::json::value &request)//, web::json::value *response)
       	{
       		return set_responseData("", true, web::json::value::null());
       	}

        virtual web::json::value set_responseData(const char* message, bool is_success = true, web::json::value data = web::json::value::null())
       	{
        	auto response = json::value::object();
        	response["success"] = web::json::value::boolean(is_success);
        	response["data"] = data; //web::json::value::null();
    		response["message"] = web::json::value::string(message);
    		return response;
       	}
        const char* get_command() { return command.c_str(); }
        const char* get_camid() { return camid.c_str(); }
    protected:
        std::string command;
        std::string camid;
    };

    class GlsmDeleteStream : public GlsmParserBase
	{
	public:
    	GlsmDeleteStream(const char* _command, const char* _camid) : GlsmParserBase(_command, _camid)
		{
		}
		virtual ~GlsmDeleteStream()
		{
		}

		virtual web::json::value doProcess(const web::json::value &request)//, web::json::value *response)
		{
			auto msg = std::string("Publish stream successfully stopped [gigaeyeslive/_definst_]:");
			msg= msg + get_camid() + ".stream";
			return set_responseData(msg.c_str(), true);
		}

		virtual web::json::value set_responseData(const char* message, bool is_success = true, web::json::value data = web::json::value::null())
		{
			auto response = json::value::object();
			response["success"] = web::json::value::boolean(is_success);
			response["data"] = data; //web::json::value::null();
			response["message"] = web::json::value::string(message);
			return response;
		}
	};

    class GlsmDisconnectStream : public GlsmParserBase
	{
	public:
    	GlsmDisconnectStream(const char* _command, const char* _camid) : GlsmParserBase(_command, _camid)
		{
		}
		virtual ~GlsmDisconnectStream()
		{
		}

		virtual web::json::value doProcess(const web::json::value &request)//, web::json::value *response)
		{
			auto msg = std::string("Publish stream successfully stopped [gigaeyeslive/_definst_]:");
			msg= msg + get_camid() + ".stream";
			return set_responseData(msg.c_str(), true);
		}

		virtual web::json::value set_responseData(const char* message, bool is_success = true, web::json::value data = web::json::value::null())
		{
			auto response = json::value::object();
			response["success"] = web::json::value::boolean(is_success);
			response["data"] = data; //web::json::value::null();
			response["message"] = web::json::value::string(message);
			return response;
		}
	};
}






#endif /* SOURCE_GLSM_UTILS_HPP_ */
