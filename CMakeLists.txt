cmake_minimum_required(VERSION 3.0)
project(sylar)

include(cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror \
	 -Wno-unused-function -Wno-unused-result -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c11 -Wall -Wno-deprecated -Werror \
	  -Wno-unused-function -Wno-unused-result -Wno-builtin-macro-redefined -Wno-deprecated-declarations")





include_directories(.)
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

option(BUILD_TEST "ON for complile test" ON)
message("defined BUILD_TEST: ${BUILD_TEST}")
set(BUILD_TEST ON)
message("defined BUILD_TEST: ${BUILD_TEST}")


find_package(Protobuf REQUIRED)
if(Protobuf_FOUND)
	include_directories(${Protobuf_INCLUDE_DIRS})
endif()

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
	include_directories(${OPENSSL_INCLUDE_DIR})
endif()


find_package(Boost REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIRS})
endif()

set(LIB_SRC
	sylar/http/http_connection.cpp
	sylar/http/http_parser.cpp
	sylar/http/http_session.cpp
	sylar/http/http_server.cpp
	sylar/http/servlet.cpp
	sylar/http/ws_connection.cpp
	sylar/http/ws_server.cpp
	sylar/http/ws_servlet.cpp
	sylar/http/ws_session.cpp
	sylar/http/http.cpp
	sylar/http/http11_parser.rl.cpp
	sylar/http/httpclient_parser.rl.cpp
	sylar/streams/async_socket_stream.cpp
	sylar/streams/socket_stream.cpp
	sylar/streams/zlib_stream.cpp
	sylar/util/hash_util.cpp
	sylar/util/json_util.cpp
	sylar/util/crypto_util.cpp
	sylar/address.cpp
	sylar/application.cpp
	sylar/bytearray.cpp
	sylar/config.cpp
	sylar/daemon.cpp
	sylar/env.cpp
	sylar/fiber.cpp
	sylar/iomanager.cpp
	sylar/library.cpp
	sylar/log.cpp
	sylar/module.cpp
	sylar/mutex.cpp
	sylar/protocol.cpp
	sylar/rock/rock_protocol.cpp
	sylar/scheduler.cpp
	sylar/socket.cpp
	sylar/stream.cpp
	sylar/util.cpp
	sylar/tcp_server.cpp
	sylar/thread.cpp
	sylar/timer.cpp
	sylar/hook.cpp
	sylar/fd_manager.cpp
	sylar/worker.cpp
	)

ragelmaker(sylar/http/http11_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/sylar/http)
ragelmaker(sylar/http/httpclient_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/sylar/http)
ragelmaker(sylar/uri.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/sylar/)

add_library(sylar SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(sylar) #__FILE__
# add_library(sylay_static STATIC ${LIB_SRC})
#SET_TARGET_PROPERTIES (sylar_static PROPERTIES OUTPUT_NAME "sylar")

find_library(YAMLCPP yaml-cpp)
find_library(PTHREAD pthread)
find_library(JSONCPP jsoncpp)

set(LIBS
		sylar
		dl			#动态加载so
		pthread		#多线程
		yaml-cpp	#yaml-cpp
		jsoncpp	#jsoncpp
		${ZLIB_LIBRARIES}	#zlib
		${OPENSSL_LIBRARIES} #openssl
		${PROTOBUF_LIBRARIES}
	)	

# message("***", ${LIBS})




if(BUILD_TEST)
# sylar_add_executable(test "tests/test.cpp" sylar "${LIBS}")
sylar_add_executable(test_config "tests/test_config.cpp" sylar "${LIBS}")
sylar_add_executable(test_thread "tests/test_thread.cpp" sylar "${LIBS}")
sylar_add_executable(test_util "tests/test_util.cpp" sylar "${LIBS}")
sylar_add_executable(test_fiber "tests/test_fiber.cpp" sylar "${LIBS}")
sylar_add_executable(test_scheduler "tests/test_scheduler.cpp" sylar "${LIBS}")
sylar_add_executable(test_iomanager "tests/test_iomanager.cpp" sylar "${LIBS}")
sylar_add_executable(test_hook "tests/test_hook.cpp" sylar "${LIBS}")
sylar_add_executable(test_address "tests/test_address.cpp" sylar "${LIBS}")
sylar_add_executable(test_socket "tests/test_socket.cpp" sylar "${LIBS}")
sylar_add_executable(test_bytearray "tests/test_bytearray.cpp" sylar "${LIBS}")
sylar_add_executable(test_http "tests/test_http.cpp" sylar "${LIBS}")
sylar_add_executable(test_http_parser "tests/test_http_parser.cpp" sylar "${LIBS}")
sylar_add_executable(test_tcp_server "tests/test_tcp_server.cpp" sylar "${LIBS}")
sylar_add_executable(echo_server "examples/echo_server.cpp" sylar "${LIBS}")
sylar_add_executable(test_http_server "tests/test_http_server.cpp" sylar "${LIBS}")
sylar_add_executable(test_http_connection "tests/test_http_connection.cpp" sylar "${LIBS}")
sylar_add_executable(test_uri "tests/test_uri.cpp" sylar "${LIBS}")
sylar_add_executable(test_daemon "tests/test_daemon.cpp" sylar "${LIBS}")
sylar_add_executable(my_http_server "examples/my_http_server.cpp" sylar "${LIBS}")
sylar_add_executable(test_env "tests/test_env.cpp" sylar "${LIBS}")
sylar_add_executable(test_application "tests/test_application.cpp" sylar "${LIBS}")
sylar_add_executable(test_config_dir "tests/test_config_dir.cpp" sylar "${LIBS}")
sylar_add_executable(echo_server_udp "examples/echo_server_udp.cpp" sylar "${LIBS}")
sylar_add_executable(echo_udp_client "examples/echo_udp_client.cpp" sylar "${LIBS}")
sylar_add_executable(test_ws_server "tests/test_ws_server.cpp" sylar "${LIBS}")
sylar_add_executable(test_ws_client "tests/test_ws_client.cpp" sylar "${LIBS}")
sylar_add_executable(test_fileop "tests/test_fileop.cpp" sylar "${LIBS}")
sylar_add_executable(test_timed_cache "tests/test_timed_cache.cpp" sylar "${LIBS}")
sylar_add_executable(test_lru "tests/test_lru.cpp" sylar "${LIBS}")
sylar_add_executable(test_timed_lru_cache "tests/test_timed_lru_cache.cpp" sylar "${LIBS}")
sylar_add_executable(test_zlib_stream "tests/test_zlib_stream.cpp" sylar "${LIBS}")
sylar_add_executable(test_crypto "tests/test_crypto.cpp" sylar "${LIBS}")


endif()

# add_library(test SHARED tests/test_module.cpp)

sylar_add_executable(bin_sylar "sylar/main.cpp" sylar "${LIBS}")
set_target_properties(bin_sylar PROPERTIES OUTPUT_NAME "sylar")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
