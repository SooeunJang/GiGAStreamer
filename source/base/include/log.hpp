#pragma once

#include <boost/move/utility.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/attributes/timer.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/filesystem.hpp>
#include <cassert>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define DEBUG
//#define TEST

#ifdef TEST
#define LOG_NORM(format, arg ...) BOOST_LOG_FUNCTION(); glsm::Log::writeLog(glsm::severity_level::normal, format, ##arg)
#define LOG_NOTI(format, arg ...) BOOST_LOG_FUNCTION(); glsm::Log::writeLog(glsm::severity_level::notification, format, ##arg)
#define LOG_WARN(format, arg ...) BOOST_LOG_FUNCTION(); glsm::Log::writeLog(glsm::severity_level::warning, format, ##arg)
#define LOG_ERR(format, arg ...) BOOST_LOG_FUNCTION(); glsm::Log::writeLog(glsm::severity_level::error, format, ##arg)
#define LOG_ERRNO(format, arg ...) BOOST_LOG_FUNCTION(); glsm::Log::writeLog(glsm::severity_level::error, 	format "(%d,%s)\n", ##arg, errno, strerror(errno))
#define LOG_CRI(format, arg ...) BOOST_LOG_FUNCTION(); glsm::Log::writeLog(glsm::severity_level::critical, format, ##arg)
#else
#define LOG_NORM(format, arg ...) glsm::Log::writeLog(glsm::severity_level::normal, "%s:%d " format "\n", __FILE__, __LINE__, ##arg)
#define LOG_NOTI(format, arg ...) glsm::Log::writeLog(glsm::severity_level::notification, "%s:%d " format "\n", __FILE__, __LINE__, ##arg)
#define LOG_WARN(format, arg ...) glsm::Log::writeLog(glsm::severity_level::warning, "%s:%d " format "\n", __FILE__, __LINE__, ##arg)
#define LOG_ERR(format, arg ...) glsm::Log::writeLog(glsm::severity_level::error, "%s:%d " format "\n", __FILE__, __LINE__, ##arg)
#define LOG_ERRNO(format, arg ...) glsm::Log::writeLog(glsm::severity_level::error, 	"%s:%d " format "(%d,%s)\n", __FILE__, __LINE__, ##arg, errno, strerror(errno))
#define LOG_CRI(format, arg ...) glsm::Log::writeLog(glsm::severity_level::critical, "%s:%d " format "\n", __FILE__, __LINE__, ##arg)
#endif

namespace glsm{
	enum severity_level
	{
		normal,
		notification,
		warning,
		error,
		critical
	};

	namespace logging = boost::log;
	namespace src = boost::log::sources;
	namespace keywords = boost::log::keywords;
	namespace trival = boost::log::trivial;
	namespace attrs = boost::log::attributes;
	namespace sinks = boost::log::sinks;
	namespace expr = boost::log::expressions;
	using boost::shared_ptr;

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(my_logger, src::severity_logger_mt<glsm::severity_level>)
//BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(my_logger, src::logger_mt)
#ifndef DEBUG
typedef sinks::synchronous_sink<sinks::text_file_backend> text_sink;
#else
typedef sinks::synchronous_sink<sinks::text_ostream_backend> text_sink;
#endif

BOOST_LOG_ATTRIBUTE_KEYWORD(_line_id, "LineID", unsigned int);
BOOST_LOG_ATTRIBUTE_KEYWORD(_severity, "Severity", glsm::severity_level);
BOOST_LOG_ATTRIBUTE_KEYWORD(_timestamp, "TimeStamp", boost::posix_time::ptime);
BOOST_LOG_ATTRIBUTE_KEYWORD(_tag_attr, "Tag", std::string);
BOOST_LOG_ATTRIBUTE_KEYWORD(_scope, "Scope", attrs::named_scope::value_type);
BOOST_LOG_ATTRIBUTE_KEYWORD(_uptime, "Uptime", attrs::timer::value_type);

// The formatting logic for the severity level
template< typename CharT, typename TraitsT >
inline std::basic_ostream< CharT, TraitsT >& operator<< (
    std::basic_ostream< CharT, TraitsT >& strm, glsm::severity_level lvl)
{
    static const char* const str[] =
    {
        "normal",
        "notification",
        "warning",
        "error",
        "critical"
    };
    if (static_cast< std::size_t >(lvl) < (sizeof(str) / sizeof(*str)))
        strm << str[lvl];
    else
        strm << static_cast< int >(lvl);
    return strm;
}

class Log
{
public:
	Log() {}
	~Log()
	{
		stop();
	}
	static void initialize()
	{

#ifndef DEBUG
		boost::filesystem::path dir("./logs");
		if(!(boost::filesystem::exists(dir))){
			boost::filesystem::create_directory(dir);
		}
		pSink = shared_ptr<text_sink>(new text_sink(
				keywords::file_name = "./logs/[GLSM]_%Y%m%d_%H%M%S_%5N.log",      // file name pattern
				keywords::rotation_size = 10*1024*1024
				));
		auto pBackend = pSink->locked_backend();
		pBackend->set_file_collector(sinks::file::make_collector(
	            keywords::target = "./logs",                          // where to store rotated files
	            keywords::max_size = 16 * 1024 * 1024,              // maximum total size of the stored files, in bytes
	            keywords::min_free_space = 100 * 1024 * 1024,       // minimum free space on the drive, in bytes
	            keywords::max_files = 512                           // maximum number of stored files
	            ));
		pBackend->scan_for_files();
		pBackend->auto_flush(true);
#else
		shared_ptr< text_sink > pSink(new text_sink);
		{
			auto pBackend = pSink->locked_backend();
			pBackend = pSink->locked_backend();
			shared_ptr<std::ostream> pStream(&std::clog, boost::null_deleter());
			pBackend->add_stream(pStream);

			boost::filesystem::path dir("./logs");
			if(!(boost::filesystem::exists(dir))){
				boost::filesystem::create_directory(dir);
			}
			shared_ptr< std::ofstream > pStream2(new std::ofstream("./logs/[GLSM]_debug.log"));
			assert(pStream2->is_open());
			pBackend->add_stream(pStream2);
			pBackend->auto_flush(true);
		}
#endif
		logging::core::get()->add_sink(pSink);
		pSink->set_formatter(
		expr::stream
//		<< expr::attr< unsigned int >("RecordID")
		<< "[" << expr::format_date_time(_timestamp, "%d.%m.%Y %H:%M:%S")
		<< "][" << _severity
		<< "] [" << expr::attr< boost::posix_time::time_duration >("Uptime")
		<< "] [" // then this delimiter separates it from the rest of the line
		<< expr::if_(expr::has_attr("Tag"))
		[
			   expr::stream << expr::attr<std::string>("Tag")
	    << "] "
	    ]
//#ifdef DEBUG
//	   << " ["
//	   << expr::format_named_scope(_scope, keywords::format = "%n [%f:%l]")
//	   << "] "
//#endif

	   << expr::smessage
	   );

		logging::add_common_attributes();
	    attrs::named_scope Scope;
	    attrs::local_clock timeStamp;
	    logging::core::get()->add_global_attribute("TimeStamp", timeStamp);
	    logging::core::get()->add_global_attribute("Uptime", attrs::timer());
	    logging::core::get()->add_global_attribute("Scope", attrs::named_scope());
	    logging::core::get()->add_global_attribute("Scope", Scope);

	    LOG_NOTI("server logging is initialized");

        pSink->set_filter(
             expr::attr< glsm::severity_level >("Severity").or_default(normal) >= normal // Write all records with "warning" severity or higher
             || expr::begins_with(expr::attr< std::string >("Tag").or_default(std::string()), "IMPORTANT")); // ...or specifically tagged
	}
	static void stop()
	{
		logging::core::get()->remove_all_sinks();
	}
	static void writeLog(glsm::severity_level level, const char* msg, ...)
	{
//		BOOST_LOG_FUNCTION();
		auto slg = my_logger::get();
		char msg_buf[1024] = {'\0', };
		va_list va;
		va_start(va, msg);
		vsnprintf(msg_buf, sizeof(msg_buf), msg, va);
		BOOST_LOG_SEV(slg, level) << msg_buf;
	}
private:
	static boost::shared_ptr<text_sink> pSink;
};

}
