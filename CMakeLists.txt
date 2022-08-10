cmake_minimum_required(VERSION 2.8)
project(JKYi)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
#include一般用于语句的复用，对于那些会在CMakeLists中多次用到的宏/函数，可以将它们写入到.cmake文件中去，然后使用include对它们进行加载，这样在编译时就可以使用
include(cmake/utils.cmake)

#set用来设置变量
set(CMAKE_VERBOSE_MAKEFILE ON)

#添加g++ 的编译选项
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

#设置头文件的查找路径
include_directories(.)
include_directories(/root/yaml-cpp-master/include)

#添加要需要链接的库的路径，相当于在gcc时的-L，就相当于将对应目录下的库加到搜索范围内
link_directories(/root/yaml-cpp-master/lib)

#查找下来的开源库的相关信息
find_package(Boost REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

#find_package(MySQL REQUIRED)
#    include_directories(${MySQL_INCLUDE_DIRS})
#endif()

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()


set (LIB_SRC 
    JKYi/log.cc
    JKYi/util.cc
    JKYi/config.cc
    JKYi/thread.cc
	JKYi/mutex.cc
	JKYi/fiber.cc
	JKYi/scheduler.cc
	JKYi/timer.cc
	JKYi/iomanager.cc
	JKYi/fdmanager.cc
	JKYi/hook.cc
    JKYi/address.cc
    JKYi/socket.cc
    JKYi/daemon.cc
    JKYi/env.cc
    JKYi/bytearray.cc
    JKYi/http/http.cc
    JKYi/http/http_parser.cc
    JKYi/http/http_session.cc
    JKYi/http/http_server.cc
    JKYi/http/servlet.cc
    JKYi/http/http_connection.cc
    JKYi/http/ws_session.cc
    JKYi/http/ws_server.cc
    JKYi/http/ws_connection.cc
    JKYi/http/ws_servlet.cc
    JKYi/stream.cc
    JKYi/stream/socket_stream.cc
    JKYi/tcp_server.cc
    JKYi/application.cc
    JKYi/worker.cc
    )

ragelmaker(JKYi/http/http11_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/JKYi/http)
ragelmaker(JKYi/http/httpclient_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/JKYi/http)
ragelmaker(JKYi/uri.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/JKYi)

#生成库文件，这里就是通过LIB_SRC生成共享库libJKYI.SO
add_library(JKYi SHARED ${LIB_SRC})
#add_library(JKYi_static ${LIB_SRC})
#SET_TARGET_PROPERTLES(JKYi_static PROPERTIEL OUTPUT_NAME "JKYi")
#生成目标文件时需要连接的库
set(LIBS
      JKYi
      pthread
      yaml-cpp  
	  dl
      ${OPENSSL_LIBRARIES}
   )

# utils.cmake文件中添加了一个function(生成的可执行文件名称 源文件名称 依赖名称 需要的库的名称)
JKYi_add_executable(test "tests/test.cc" JKYi "${LIBS}")
#
JKYi_add_executable(test_config "tests/test_config.cc" JKYi "${LIBS}")

JKYi_add_executable(test_thread "tests/test_thread.cc" JKYi "${LIBS}")

JKYi_add_executable(test_util "tests/test_util.cc" JKYi "${LIBS}")

JKYi_add_executable(test_fiber "tests/test_fiber.cc" JKYi "${LIBS}")

JKYi_add_executable(test_scheduler "tests/test_scheduler.cc" JKYi "${LIBS}")

JKYi_add_executable(test_iomanager "tests/test_iomanager.cc" JKYi "${LIBS}")

JKYi_add_executable(test_hook "tests/test_hook.cc" JKYi "${LIBS}")

JKYi_add_executable(test_address "tests/test_address.cc" JKYi "${LIBS}")

JKYi_add_executable(test_socket "tests/test_socket.cc" JKYi "${LIBS}")

JKYi_add_executable(test_bytearray "tests/test_bytearray.cc" JKYi "${LIBS}")

JKYi_add_executable(test_http "tests/test_http.cc" JKYi "${LIBS}")

JKYi_add_executable(test_http_parser "tests/test_http_parser.cc" JKYi "${LIBS}")

JKYi_add_executable(test_http_server "tests/test_http_server.cc" JKYi "${LIBS}")

JKYi_add_executable(test_http_connection "tests/test_http_connection.cc" JKYi "${LIBS}")

JKYi_add_executable(my_http_server "sample/my_http_server.cc" JKYi "${LIBS}")

JKYi_add_executable(test_daemon "tests/test_daemon.cc" JKYi "${LIBS}")

JKYi_add_executable(test_env "tests/test_env.cc" JKYi "${LIBS}")

JKYi_add_executable(test_application "tests/test_application.cc" JKYi "${LIBS}")

JKYi_add_executable(test_ws_server "tests/test_ws_server.cc" JKYi "${LIBS}")

JKYi_add_executable(test_ws_client "tests/test_ws_client.cc" JKYi "${LIBS}")

#将生成的目标文件放到bin目录下
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#将生成的动态库放到lib目录下
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)


