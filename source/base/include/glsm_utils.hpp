#pragma once

#include <cpprest/json.h>
#include <cpprest/http_msg.h>
#include <iostream>
#include <map>
#include <vector>

#define FROMJSON_INT(key) if(!item[U(#key)].is_null()) {result.key = item[U(#key)].as_integer();}
#define FROMJSON_BOOL(key) if(!item[U(#key)].is_null()) {result.key = item[U(#key)].as_bool();}
#define FROMJSON_STR(key) if(!item[U(#key)].is_null()) {result.key = item[U(#key)].as_string();}
#define FROMJSON_CHILD(key, childclass) if(!item[U(#key)].is_null()) {result.key = childclass::fromJson(item[U(#key)]);}
#define FROMJSON_CHILDLIST(key, ins, childclass) \
			auto key##list = item[U(#key)].as_array(); \
			for( auto ins : key##list ) \
			{ \
				result.key.push_back(childclass::fromJson(ins)); \
			}

#define ASJSON_INT(key) if(!web::json::value::number(key).is_null()) result[U(#key)] = web::json::value::number(key); else result[U(#key)] = web::json::value::null();
//#define ASJSON_INT64(key) if(!web::json::value(key).is_null()) result[U(#key)] = web::json::value::number(key); else result[U(#key)] = web::json::value::null();
#define ASJSON_BOOL(key) if(!web::json::value::boolean(key).is_null()) result[U(#key)] = web::json::value::boolean(key); else result[U(#key)] = web::json::value::null();
#define ASJSON_STR(key) if(!web::json::value::string(key).is_null()) result[U(#key)] = web::json::value::string(key); else result[U(#key)] = web::json::value::null();
#define ASJSON_CHILD(key, child) if(!child.asJson().is_null()) result[U(#key)] = child.asJson(); else result[U(#key)] = web::json::value::null();
#define ASJSON_CHILDLIST(key, child)		\
				int idx = 0; \
				auto key##list = web::json::value::array();\
				for( auto child : key) { \
				if(!child.asJson().is_null())\
					key##list[idx++] = child.asJson(); \
				else \
					key##list[idx++] = web::json::value::null(); \
				} \
				result[U(#key)] = key##list;

namespace glsm {
//
//#pragma pack(push, 1)
	typedef struct _ServerInfo
	{
		std::string version;
		std::string serverName;
		std::string serverHost;
		std::string appName;
		std::string appInstance;
		std::string stream1;
		std::string stream2;
		std::string stream3;
		std::string servicePort;
		std::string streamerPort;
		unsigned int connection_timeout;


		_ServerInfo() : connection_timeout(DAFAULT_CONNECTION_TIMEOUT){}
		_ServerInfo(const char* _version, const char* _serverName, const char* _serverHost, const char* _appName,
				const char* _stream1, const char* _stream2, const char* _stream3, const char* _appInstance, const char* _servicePort, const char* _streamerPort)
		: version(_version), serverName(_serverName), serverHost(_serverHost), appName(_appName), appInstance(_appInstance),
		stream1(_stream1), stream2(_stream2), stream3(_stream3), servicePort(_servicePort), streamerPort(_streamerPort), connection_timeout(DAFAULT_CONNECTION_TIMEOUT)
		{
		}

		void printInfo(std::iostream &ss)
		{
			ss<<"version: "<<version
			<<", serverName: "<<serverName
			<<", serverHost:"<<serverHost
			<<", appName:"<<appName
			<<", appInstance:"<<appInstance
			<<", stream1:"<<stream1
			<<", stream2:"<<stream2
			<<", stream3:"<<stream3
			<<", servicePort:"<<servicePort
			<<", streamerPort:"<<streamerPort
			<<", connection_timeout:"<<connection_timeout
			<<std::endl;
		}
	}ServerInfo;

	typedef enum _URL_Separator
	{
		servers = 0,
		machine = 0,
		vhosts = 2,
		applications = 4,
		application = 5,
		streamfiles = 6,
		camid = 7,
		adv = 8,
		instances = 6,
		instance = 7,
		streamfile_actions = 8,
		profile_name = 9,
		streamfile_actions_cmd = 9,
		incomingstreams = 8,
		streamrecorders = 8,
		actions = 10,
		cmd = 11,
	} URL_Separator;



	typedef struct _Stream
	{
		utility::string_t name;
		utility::string_t serverName;
		utility::string_t uri;
		_Stream() {}
		_Stream(utility::string_t _name, utility::string_t _serverName, utility::string_t _uri) : name(_name), serverName(_serverName), uri(_uri) {}
		~_Stream() {}

		static _Stream fromJson(const web::json::value& object)
		{
			_Stream result;
			for( auto iter : object.as_array() )
			{
				auto item = iter.as_object();
				FROMJSON_STR(name)
				FROMJSON_STR(serverName)
				FROMJSON_STR(uri)
			}
			return result;
		}

		web::json::value asJson() const
		{
			web::json::value result = web::json::value::object();
			ASJSON_STR(name)
			ASJSON_STR(serverName)
			ASJSON_STR(uri)
			return result;
		}
	} Stream;


	typedef struct _StreamFiles
	{
		utility::string_t id;
		utility::string_t href;

		_StreamFiles() {}
		_StreamFiles(utility::string_t _id, utility::string_t _href) : id(_id), href(_href) {}
		~_StreamFiles() {}

		static _StreamFiles fromJson(const web::json::value& object)
		{
			_StreamFiles result;
			auto item = object.as_object();
			FROMJSON_STR(id)
			FROMJSON_STR(href)
			return result;
		}

		web::json::value asJson() const
		{
			web::json::value result = web::json::value::object();
			ASJSON_STR(id)
			ASJSON_STR(href)
			return result;
		}
	} StreamFiles;

	typedef struct _StreamProfile
	{
		utility::string_t restURI;
		std::vector<StreamFiles> streamFiles;

		_StreamProfile() {}
		~_StreamProfile() {}

		static _StreamProfile fromJson(const web::json::value& object)
		{
			_StreamProfile result;
			auto item = object.as_object();
			FROMJSON_STR(restURI)
			FROMJSON_CHILDLIST(streamFiles, streamFile, StreamFiles)
			return result;
		}

		web::json::value asJson() const
		{
			web::json::value result = web::json::value::object();
			ASJSON_STR(restURI)
			ASJSON_CHILDLIST(streamFiles, StreamFilesS)
			return result;
		}
	} StreamProfile;

	typedef struct _AdvancedSettings
	{
		bool enabled;
		bool canRemove;
		utility::string_t name;
		utility::string_t value;
		utility::string_t defaultValue;
		utility::string_t type;
		utility::string_t sectionName;
		utility::string_t section;
		bool documented;

		_AdvancedSettings() : enabled(false), canRemove(false), documented(false){}
		~_AdvancedSettings() {}

		static _AdvancedSettings fromJson(const web::json::value& object)
		{
			_AdvancedSettings result;
			auto item = object.as_object();
			FROMJSON_BOOL(enabled)
			FROMJSON_BOOL(canRemove)
			FROMJSON_STR(name)
			FROMJSON_STR(value)
			FROMJSON_STR(defaultValue)
			FROMJSON_STR(type)
			FROMJSON_STR(sectionName)
			FROMJSON_STR(section)
			FROMJSON_BOOL(documented)
			return result;
		}

		web::json::value asJson() const
		{
			web::json::value result = web::json::value::object();
			ASJSON_BOOL(enabled)
			ASJSON_BOOL(canRemove)
			ASJSON_STR(name)
			ASJSON_STR(value)
			ASJSON_STR(defaultValue)
			ASJSON_STR(type)
			ASJSON_STR(sectionName)
			ASJSON_STR(section)
			ASJSON_BOOL(documented)
			return result;
		}
	} AdvancedSettings;

	typedef struct _StreamProfileModify
	{
		utility::string_t restURI;
		std::vector<AdvancedSettings> advancedSettings;
		_StreamProfileModify() {}
		~_StreamProfileModify() {}

		static _StreamProfileModify fromJson(const web::json::value& object)
		{
			_StreamProfileModify result;
			auto item = object.as_object();
			FROMJSON_STR(restURI)
			FROMJSON_CHILDLIST(advancedSettings, advancedSetting, AdvancedSettings)
			return result;
		}

		web::json::value asJson() const
		{
			web::json::value result = web::json::value::object();
			ASJSON_STR(restURI)
			ASJSON_CHILDLIST(advancedSettings, advancedSetting)
			return result;
		}
	}StreamProfileModify;

	typedef struct _StreamProfileConnect
	{
		utility::string_t connectAppName;
		utility::string_t appInstance;
		utility::string_t mediaCasterType;

		_StreamProfileConnect() {}
		_StreamProfileConnect(utility::string_t _connectAppName, utility::string_t _appInstance, utility::string_t _mediaCasterType) :
			connectAppName(_connectAppName), appInstance(_appInstance), mediaCasterType(_mediaCasterType) {}
		~_StreamProfileConnect() {}

		utility::string_t getResultMsg()
		{
			utility::string_t msg("Publish stream successfully started");
			msg += "[" + connectAppName + "/" + appInstance + "]:";
			msg += "mp4:";
			return msg;
		}

		static _StreamProfileConnect fromJson(const web::json::value& object)
		{
			_StreamProfileConnect result;
			auto item = object.as_object();
			FROMJSON_STR(connectAppName)
			FROMJSON_STR(appInstance)
			FROMJSON_STR(mediaCasterType)
			return result;
		}

		web::json::value asJson() const
		{
			web::json::value result = web::json::value::object();
			ASJSON_STR(connectAppName)
			ASJSON_STR(appInstance)
			ASJSON_STR(mediaCasterType)
			return result;
		}
	}StreamProfileConnect;

	typedef struct _StreamProfileDisConnect
	{
		utility::string_t connectAppName;
		utility::string_t appInstance;
		utility::string_t mediaCasterType;

		_StreamProfileDisConnect() : connectAppName("gigaeyeslive"), appInstance("_definst_"), mediaCasterType("rtp") {}
		_StreamProfileDisConnect(utility::string_t _connectAppName, utility::string_t _appInstance, utility::string_t _mediaCasterType) :
			connectAppName(_connectAppName), appInstance(_appInstance), mediaCasterType(_mediaCasterType) {}
		~_StreamProfileDisConnect() {}

		utility::string_t getResultMsg()
		{
			utility::string_t msg("Publish stream successfully stopped");
			msg += "[" + connectAppName + "/" + appInstance + "]:";
			msg += ":";
			return msg;
		}

		static _StreamProfileDisConnect fromJson(const web::json::value& object)
		{
			_StreamProfileDisConnect result;
			auto item = object.as_object();
			FROMJSON_STR(connectAppName)
			FROMJSON_STR(appInstance)
			FROMJSON_STR(mediaCasterType)
			return result;
		}

		web::json::value asJson() const
		{
			web::json::value result = web::json::value::object();
			ASJSON_STR(connectAppName)
			ASJSON_STR(appInstance)
			ASJSON_STR(mediaCasterType)
			return result;
		}
	} StreamProfileDisConnect;

	typedef struct _RecordParameters
	{
		utility::string_t restURI;
		utility::string_t recorderName;
		utility::string_t instanceName;
		utility::string_t recorderState;
		bool defaultRecorder;
		utility::string_t segmentationType;
		utility::string_t outputPath;
		utility::string_t baseFile;
		utility::string_t fileFormat;
		utility::string_t fileVersionDelegateName;
		utility::string_t fileTemplate;
		utility::string_t segmentDuration;
		int	segmentSize;
		bool recordData;
		bool startOnKeyFrame;
		utility::string_t option;
		bool moveFirstVideoFrameToZero;
		utility::string_t recordingStartTime;
		_RecordParameters() : defaultRecorder(true), segmentSize(0), recordData(false), startOnKeyFrame(true), moveFirstVideoFrameToZero(true){}
		_RecordParameters(utility::string_t _restURI, utility::string_t _recorderName, utility::string_t _instanceName,
				utility::string_t _recorderState, bool _defaultRecorder, utility::string_t _segmentationType, utility::string_t _outputPath,
				utility::string_t _baseFile, utility::string_t _fileFormat, utility::string_t _fileVersionDelegateName,	utility::string_t _fileTemplate,
				utility::string_t _segmentDuration, int _segmentSize, bool _recordData, bool _startOnKeyFrame, utility::string_t _option,
				bool _moveFirstVideoFrameToZero, utility::string_t _recordingStartTime)
			: restURI(_restURI), recorderName(_recorderName), instanceName(_instanceName), recorderState(_recorderState), defaultRecorder(_defaultRecorder),
			  segmentationType(_segmentationType), outputPath(_outputPath), baseFile(_baseFile), fileFormat(_fileFormat), fileVersionDelegateName(_fileVersionDelegateName),
			  fileTemplate(_fileTemplate), segmentDuration(_segmentDuration), segmentSize(_segmentSize), recordData(_recordData), startOnKeyFrame(_startOnKeyFrame),
			  option(_option), moveFirstVideoFrameToZero(_moveFirstVideoFrameToZero), recordingStartTime(_recordingStartTime)
		{}
		~_RecordParameters(){}

		utility::string_t getResultMsg()
		{
			utility::string_t msg("Recorder Created");
			return msg;
		}

		static _RecordParameters fromJson(const web::json::value& object)
		{
			_RecordParameters result;
			auto item = object.as_object();
			FROMJSON_STR(restURI)
			FROMJSON_STR(recorderName)
			FROMJSON_STR(instanceName)
			FROMJSON_STR(recorderState)
			FROMJSON_BOOL(defaultRecorder)
			FROMJSON_STR(segmentationType)
			FROMJSON_STR(outputPath)
			FROMJSON_STR(baseFile)
			FROMJSON_STR(fileFormat)
			FROMJSON_STR(fileVersionDelegateName)
			FROMJSON_STR(fileTemplate)
			FROMJSON_INT(segmentDuration)
			FROMJSON_INT(segmentSize)
			FROMJSON_BOOL(recordData)
			FROMJSON_BOOL(startOnKeyFrame)
			FROMJSON_STR(option)
			FROMJSON_BOOL(moveFirstVideoFrameToZero)
			FROMJSON_STR(recordingStartTime)

			return result;
		}

		web::json::value asJson() const
		{
			web::json::value result = web::json::value::object();
			ASJSON_STR(restURI)
			ASJSON_STR(recorderName)
			ASJSON_STR(instanceName)
			ASJSON_STR(recorderState)
			ASJSON_BOOL(defaultRecorder)
			ASJSON_STR(segmentationType)
			ASJSON_STR(outputPath)
			ASJSON_STR(baseFile)
			ASJSON_STR(fileFormat)
			ASJSON_STR(fileVersionDelegateName)
			ASJSON_STR(fileTemplate)
			ASJSON_STR(segmentDuration)
			ASJSON_INT(segmentSize)
			ASJSON_BOOL(recordData)
			ASJSON_BOOL(startOnKeyFrame)
			ASJSON_STR(option)
			ASJSON_BOOL(moveFirstVideoFrameToZero)
			ASJSON_STR(recordingStartTime)
			return result;
		}
	} RecordParameters;

	typedef struct _ProfileStatusListV0
	{
		utility::string_t sourceIp;
		bool isPTZEnabled;
		utility::string_t applicationInstance;
		utility::string_t name;
		bool isRecordingSet;
		bool isStreamManagerStream;
		bool isPublishedToVOD;
		bool isConnected;
		int ptzPollingInterval;
		int ptzPollingIntervalMinimum;
		_ProfileStatusListV0() : isPTZEnabled(false),
				  isRecordingSet(false), isStreamManagerStream(true), isPublishedToVOD(false),
				  isConnected(false), ptzPollingInterval(2000), ptzPollingIntervalMinimum(1000) {}
		_ProfileStatusListV0(utility::string_t _sourceIp, bool _isPTZEnabled, utility::string_t _applicationInstance,
				utility::string_t _name, bool _isRecordingSet, bool _isStreamManagerStream,	bool _isPublishedToVOD,
				bool _isConnected, int _ptzPollingInterval, int _ptzPollingIntervalMinimum)
		: sourceIp(_sourceIp), isPTZEnabled(_isPTZEnabled), applicationInstance(_applicationInstance), name(_name),
		  isRecordingSet(_isRecordingSet), isStreamManagerStream(_isStreamManagerStream), isPublishedToVOD(_isPublishedToVOD),
		  isConnected(_isConnected), ptzPollingInterval(_ptzPollingInterval), ptzPollingIntervalMinimum(_ptzPollingIntervalMinimum)
		{}
		~_ProfileStatusListV0() {}

		static _ProfileStatusListV0 fromJson(const web::json::value& object)
		{
			_ProfileStatusListV0 result;
			for( auto iter : object.as_array() )
			{
				auto item = iter.as_object();
				FROMJSON_STR(sourceIp)
				FROMJSON_BOOL(isPTZEnabled)
				FROMJSON_STR(applicationInstance)
				FROMJSON_STR(name)
				FROMJSON_BOOL(isRecordingSet)
				FROMJSON_BOOL(isStreamManagerStream)
				FROMJSON_BOOL(isPublishedToVOD)
				FROMJSON_BOOL(isConnected)
				FROMJSON_INT(ptzPollingInterval)
				FROMJSON_INT(ptzPollingIntervalMinimum)
			}
			return result;
		}

		web::json::value asJson() const
		{
			web::json::value result = web::json::value::object();

			ASJSON_STR(sourceIp)
			ASJSON_BOOL(isPTZEnabled)
			ASJSON_STR(applicationInstance)
			ASJSON_STR(name)
			ASJSON_BOOL(isRecordingSet)
			ASJSON_BOOL(isStreamManagerStream)
			ASJSON_BOOL(isPublishedToVOD)
			ASJSON_BOOL(isConnected)
			ASJSON_INT(ptzPollingInterval)
			ASJSON_INT(ptzPollingIntervalMinimum)
			return result;
		}
	} ProfileStatusListV0;

	typedef struct _ProfileStatusInfoV0
	{
		utility::string_t serverName;
		std::vector<ProfileStatusListV0> incomingStreams;
		_ProfileStatusInfoV0() {}
		_ProfileStatusInfoV0(utility::string_t _serverName, std::vector<ProfileStatusListV0> _incomingStreams)
		: serverName(_serverName), incomingStreams(_incomingStreams)
		{}
		~_ProfileStatusInfoV0() {}

//		static _ProfileStatusInfoV0 fromJson(const web::json::value& object)
//		{
//			_ProfileStatusInfoV0 result;
//			FROMJSON_STR(serverName)
//			if(!item["serverName"].is_null())
//			{
//				result.serverName = item["serverName"].as_string();
//				std::cout<<"serverName"<<":"<<result.serverName<<std::endl;
//			}
//			std::vector<_ProfileStatusListV0> profileStatusList;
//			auto items = object.as_array();
//			for( auto item : items)
//			{
//				profileStatusList.push_back(ProfileStatusListV0::fromJson(item));
//			}
//			result.incomingStreams = profileStatusList;
//			return result;
//		}

		web::json::value asJson() const
		{
			web::json::value result = web::json::value::object();
			auto list = web::json::value::array();
			ASJSON_STR(serverName)
//			ASJSON_CHILD(incomingStreams, ProfileStatusListV0);
			int idx = 0;
			for( auto incomingStream : incomingStreams )
			{
				if(!incomingStream.asJson().is_null())
					list[idx++] = incomingStream.asJson();
				else
					list[idx++] = web::json::value::null();
			}
			result["incomingStreams"] = list;
			return result;
		}
	} ProfileStatusInfoV0;

	typedef struct _ConnectionCountVO
	{
		int rtmp;
		int mpegdash;
		int cupertino;
		int sanjose;
		int smooth;
		int rtp;
		static _ConnectionCountVO fromJson(const web::json::value& object)
		{
			_ConnectionCountVO result;
			auto item = object.as_object();

			FROMJSON_INT(rtmp)
			FROMJSON_INT(mpegdash)
			FROMJSON_INT(cupertino)
			FROMJSON_INT(sanjose)
			FROMJSON_INT(smooth)
			FROMJSON_INT(rtp)

			return result;
		}

		web::json::value asJson() const
		{
			web::json::value result = web::json::value::object();
			ASJSON_INT(rtmp)
			ASJSON_INT(mpegdash)
			ASJSON_INT(cupertino)
			ASJSON_INT(sanjose)
			ASJSON_INT(smooth)
			ASJSON_INT(rtp)
			return result;
		}
	} ConnectionCountVO;

	typedef struct _ProfileStatusV0
	{
		utility::string_t serverName;
		int uptime;
//		long long int bytesIn;
		int64_t bytesIn;
//		long long int bytesOut;
		int64_t bytesOut;
		int bytesInRate;
		int bytesOutRate;
		int totalConnections;
		ConnectionCountVO connectionCount;
		utility::string_t applicationInstance;
		utility::string_t name;
		static _ProfileStatusV0 fromJson(const web::json::value& object)
		{
			_ProfileStatusV0 result;
			auto item = object.as_object();

			FROMJSON_STR(serverName)
			FROMJSON_INT(uptime)
			FROMJSON_INT(bytesIn)
			FROMJSON_INT(bytesOut)
			FROMJSON_INT(bytesInRate)
			FROMJSON_INT(bytesOutRate)
			FROMJSON_INT(totalConnections)
			FROMJSON_CHILD(connectionCount, ConnectionCountVO)
			FROMJSON_STR(applicationInstance)
			FROMJSON_STR(name)

			return result;
		}

		web::json::value asJson() const
		{
			web::json::value result = web::json::value::object();
			ASJSON_STR(serverName)
			ASJSON_INT(uptime)
//			ASJSON_INT(bytesIn)
			if(!web::json::value::number(bytesIn).is_null())
				result["bytesIn"] = web::json::value(bytesIn).as_number().to_int64();
			else
				result["bytesIn"] = web::json::value::null();

			if(!web::json::value::number(bytesOut).is_null())
				result["bytesOut"] = web::json::value::number(bytesOut);
			else
				result["bytesOut"] = web::json::value::null();
//			ASJSON_INT(bytesOut)
			ASJSON_INT(bytesInRate)
			ASJSON_INT(bytesOutRate)
			ASJSON_INT(totalConnections)
			ASJSON_CHILD(connectionCount, connectionCount)
			ASJSON_STR(applicationInstance)
			ASJSON_STR(name)
			return result;
		}
	} ProfileStatusV0;
//	typedef std::unordered_map<utility::string_t, ProfileStatusListV0> incomingStreamList;
//	typedef struct _Instances
//	{
//		utility::string_t instanceName;
//
//	} Instances;

}


