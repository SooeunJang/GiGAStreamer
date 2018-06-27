cmake_minimum_required(VERSION 3.0)

project(glsm)

option (ADD_WEBSOCKET "Build websocketpp together." FALSE)
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

# Override from command line "CMake -D<OPTION>=TRUE/FALSE/0/1/ON/OFF"

# Configure compiler options ...
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")

    message("-- configuring clang options")
    set(CMAKE_CXX_FLAGS "-arch x86_64 -std=c++11 -stdlib=libc++ -DBOOST_LOG_DYN_LINK -Wno-deprecated-declarations")

elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")

    message("-- configuring gcc options")
	if(ADD_WEBSOCKET)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WEBSOCKETPP_FLAGS} -lz -lpthread -std=gnu++11 -DBOOST_LOG_DYN_LINK")
else()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread -std=gnu++11 -DBOOST_LOG_DYN_LINK")
endif()
endif()

# Project construction ...

# source files ...
add_executable(${PROJECT_NAME} ./source/main.cpp
                               ./source/stream_controller.cpp
                               ./source/foundation/network_utils.cpp
                               ./source/machine_controller.cpp
                               ./source/foundation/basic_controller.cpp)

# headers search paths ...

##############3#for websocket!!###########
#set(WEBSOCKETPP_ROOT "/home/gigasurv/Dev/websocketpp")
set(WEBSOCKETPP_ROOT "libs/cpprestsdk/Release/libs/websocketpp/")
set(WEBSOCKETPP_LIB ${WEBSOCKETPP_ROOT}/lib)
set(WEBSOCKETPP_INCLUDE ${WEBSOCKETPP_ROOT})
set(WEBSOCKETPP_PLATFORM_LIBS -lpthread -lrt -lz)
set(WEBSOCKETPP_PLATFORM_TLS_LIBS -lssl -lcrypto)
set(WEBSOCKETPP_BOOST_LIBS -lboost_system -lboost_thread -lboost_chrono -lboost_timer)
set(WEBSOCKETPP_FLAGS ${WEBSOCKETPP_PLATFORM_LIBS} ${WEBSOCKETPP_PLATFORM_TLS_LIBS} ${WEBSOCKETPP_BOOST_LIBS})
##########################################

set(CPPRESTSDK_INCLUDE_DIR "./libs/cpprestsdk/Release/include")
set(GLSMSERVICE_INCLUDE_DIR "./source/foundation/include")
message(GLSMSERVICE " ${GLSMSERVICE_INCLUDE_DIR}")
if(ADD_WEBSOCKET)
	set(HEADER_SEARCH_PATHS ${CPPRESTSDK_INCLUDE_DIR} ${GLSMSERVICE_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${WEBSOCKETPP_INCLUDE})
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
    set(CPPRESTSDK_LIBRARY "${PROJECT_SOURCE_DIR}/libs/cpprestsdk/Release/build.debug/Binaries/libcpprest.so")
#if(ADD_WEBSOCKET)
#	set(LIBRARIES_SEARCH_PATHS ${CPPRESTSDK_LIBRARY} ${OPENSSL_LIBS} ${Boost_LIBRARIES} ${WEBSOCKETPP_LIB})
#else()
	set(LIBRARIES_SEARCH_PATHS ${CPPRESTSDK_LIBRARY} ${OPENSSL_LIBS} ${Boost_LIBRARIES})
#endif()
endif()

if(ADD_WEBSOCKET)
message(HEADER_SEARCH_PATHS: " ${HEADER_SEARCH_PATHS}")
message(WEBSOCKETPP_FLAGS: " ${WEBSOCKETPP_FLAGS}")
message(CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS}")
message(ADD_WEBSOCKET: " ${ADD_WEBSOCKET}")
message(BOOST_LIBS " ${Boost_LIBRARIES}")
message(OPENSSL_LIBS " ${OPENSSL_LIBRARIES}")
message(CPPRESTSDK_LIBRARY " ${CPPRESTSDK_LIBRARY}")
message(LIBRARIES_SEARCH_PATHS " ${LIBRARIES_SEARCH_PATHS}")
endif()

include_directories(${HEADER_SEARCH_PATHS})
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework CoreFoundation")
    target_link_libraries(${PROJECT_NAME} "-framework Security")
    target_link_libraries(${PROJECT_NAME} ${LIBRARIES_SEARCH_PATHS})
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-W1, -F/Library/Frameworks")
else()
    target_link_libraries(${PROJECT_NAME} ${LIBRARIES_SEARCH_PATHS})
endif()
