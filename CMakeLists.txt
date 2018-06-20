cmake_minimum_required(VERSION 3.0)

project(glsm)

option (ADD_GSTREAMER "Build Gstreamer." FALSE)
option (DEBUG_ON "Build with debug option." FALSE)
# Configure required libraries ...
if(UNIX) # Darwing or Linux
    
    find_package(Boost REQUIRED COMPONENTS system thread log program_options chrono)
    find_package(Threads REQUIRED)

    if(APPLE)
        # Prefer a homebrew version of OpenSSL over the one in /usr/lib
	    file(GLOB OPENSSL_ROOT_DIR /usr/local/Cellar/openssl/*)
        # Prefer the latest (make the latest one first)
	    list(REVERSE OPENSSL_ROOT_DIR)

        find_package(OpenSSL 1.0.2 REQUIRED)
        set(OPENSSL_VERSION "1.0.2f")
    else()
        find_package(OpenSSL 1.0.1 REQUIRED)
        set(OPENSSL_VERSION "1.0.1")
    endif()

elseif(WIN32) # Windows systems including Win64.
    message(FATAL_ERROR "-- Windows is not supported for now.")
else()
    message(FATAL_ERROR "-- Unsupported platform sorry! :( ")
endif()

if(DEBUG_ON)
	set(DEBUG_FLAGS "-g")
else()
	set(DEBUG_FLAGS "-O2 -fPIC")
endif()
# Configure compiler options ...
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")

    message("-- configuring clang options")
    set(CMAKE_CXX_FLAGS "-arch x86_64 -fpermissive -std=c++11 -stdlib=libc++ -DBOOST_LOG_DYN_LINK -Wno-deprecated-declarations")

elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")

    message("-- configuring gcc options")
	if(ADD_GSTREAMER)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DEBUG_FLAGS} -fpermissive ${BOOST_LOG_LIBS} -lpthread -std=gnu++11 -DBOOST_LOG_DYN_LINK ${GST_FLAGS}")
	else()
	    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DEBUG_FLAGS} -fpermissive ${BOOST_LOG_LIBS} -lpthread -std=gnu++11 -DBOOST_LOG_DYN_LINK")
	endif()
endif()

# Project construction ...

# source files ...
add_executable(${PROJECT_NAME} ./source/main.cpp
			       ./source/stream_controller.cpp
                               ./source/machine_controller.cpp
                               ./source/controller_manager.cpp
                               ./source/base/network_utils.cpp
                               ./source/base/runtime_utils.cpp
                               ./source/base/basic_controller.cpp
			       ./source/streamer/streaming-object.cpp
			       ./source/streamer/streaming-server.cpp
			       )

# headers search paths ...

##############3#for websocket!!###########
if(ADD_GSTREAMER)
find_package(PkgConfig)
#pkg_check_modules(GST REQUIRED gstreamer-1.0 >= 1.5)

macro(find_gstreamer_component prefix pkg lib)
	pkg_check_modules(${prefix} REQUIRED ${pkg})
	find_library(${prefix}_LIBRARIES NAMES ${lib} HINTS ${${prefix}_LIBRARY_DIRS} ${${prefix}_LIBDIR})
	list(APPEND gstreamer_LIBRARY_DIRS -L${${prefix}_LIBRARY_DIRS})
	list(APPEND gstreamer_INCLUDE_DIRS ${${prefix}_INCLUDE_DIRS})
	list(APPEND gstreamer_LIBRARIES ${${prefix}_LIBRARIES})
	mark_as_advanced(${prefix}_LIBRARIES gstreamer_LIBRARY_DIRS gstreamer_LIBRARIES gstreamer_INCLUDE_DIRS)
endmacro()

#find_library(gstreamer NAMES gstreamer-rtsp-1.0 HINTS ${PC_gstreamer_LIBRARY_DIRS} ${PC__LIBDIR})
find_gstreamer_component(gst      gstreamer-1.0       gstreamer-1.0)
find_gstreamer_component(gst      gstreamer-rtsp-server-1.0       gstreamer-rtsp-server-1.0)
find_gstreamer_component(gst      gstreamer-rtsp-1.0       gstreamer-rtsp-1.0)
find_gstreamer_component(gobject     gobject-2.0       gobject-2.0)
find_gstreamer_component(glib     glib-2.0       glib-2.0)

message(STATUS "GSTREAMER INC: ${gstreamer_INCLUDE_DIRS}")
message(STATUS "GSTREAMER LIBS: ${gstreamer_LIBRARIES}")

#set(GST_ROOT /home/gigasurv/cerbero/build/dist/linux_x86_64/lib/)
#set(GST_INC ${gstreamer_INCLUDE_DIRS} -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include)
#set(GST_LIBS ${gstreamer_LIBRARIES} -lgobject-2.0 -lglib-2.0)
endif()
#set(Boost_LIBRARIES "/usr/local/lib")
set(BOOST_LOG_LIBS -lboost_system lboost_thread -lpthread -lboost_log)
set(CPPRESTSDK_INCLUDE_DIR "./libs/cpprestsdk/Release/include")
set(GLSMSERVICE_INCLUDE_DIR "./source/base/include")
if(ADD_GSTREAMER)
	set(HEADER_SEARCH_PATHS ${CPPRESTSDK_INCLUDE_DIR} ${GLSMSERVICE_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${gstreamer_INCLUDE_DIRS})
else()
	set(HEADER_SEARCH_PATHS ${CPPRESTSDK_INCLUDE_DIR} ${GLSMSERVICE_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR})
endif()

# library search paths ...
if(APPLE)
    set(OPENSSL_LIBS "/usr/local/Cellar/openssl/${OPENSSL_VERSION}/lib/libssl.1.0.0.dylib;/usr/local/Cellar/openssl/${OPENSSL_VERSION}/lib/libcrypto.1.0.0.dylib")
    set(CPPRESTSDK_LIBRARY "${PROJECT_SOURCE_DIR}/libs/cpprestsdk/build.release/Binaries/libcpprest.a")

    set(LIBRARIES_SEARCH_PATHS ${OPENSSL_LIBS} ${Boost_LIBRARIES} ${CPPRESTSDK_LIBRARY})
else()
    set(OPENSSL_LIBS "${OPENSSL_LIBRARIES}")
    set(CPPRESTSDK_LIBRARY "${PROJECT_SOURCE_DIR}/libs/cpprestsdk/Release/build.release/Binaries/libcpprest.a")

	if(ADD_GSTREAMER)
	    set(LIBRARIES_SEARCH_PATHS ${CPPRESTSDK_LIBRARY} ${OPENSSL_LIBS} ${Boost_LIBRARIES} ${gstreamer_LIBRARY_DIRS} ${gstreamer_LIBRARIES})
	else()
	    set(LIBRARIES_SEARCH_PATHS ${CPPRESTSDK_LIBRARY} ${OPENSSL_LIBS} ${Boost_LIBRARIES})
	endif()
endif()

#message(BOOST_LIBS " ${Boost_LIBRARIES}")
#message(OPENSSL_LIBS " ${OPENSSL_LIBRARIES}")
#message(CPPRESTSDK_LIBRARY " ${CPPRESTSDK_LIBRARY}")
#message(LIBRARIES_SEARCH_PATHS " ${LIBRARIES_SEARCH_PATHS}")
#message(HEADER_SEARCH_PATHS " ${HEADER_SEARCH_PATHS}")

include_directories(${HEADER_SEARCH_PATHS})
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework CoreFoundation")
    target_link_libraries(${PROJECT_NAME} "-framework Security")
    target_link_libraries(${PROJECT_NAME} ${LIBRARIES_SEARCH_PATHS})
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-W1, -F/Library/Frameworks")
else()
	target_link_libraries(${PROJECT_NAME} ${LIBRARIES_SEARCH_PATHS})
endif()
