cmake_minimum_required(VERSION 2.8)

project(JKYi)
#set用来设置变量
set(CMAKE_VERBOSE_MAKEFILE ON)
#添加g++ 的编译选项
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
#设置头文件的查找路径
include_directories(.)
include_directories(/root/yaml-cpp-master/include)
#添加要需要链接的库的路径，相当于在gcc时的-L，就相当于将对应目录下的库加到搜索范围内
link_directories(/root/yaml-cpp-master/lib)

find_library(YAMLCPP yaml-cpp)
message("***",${YAMLCPP})

set (LIB_SRC 
    JKYi/log.cc
    JKYi/util.cc
    JKYi/config.cc
    )
#生成库文件，这里就是通过LIB_SRC生成共享库libJKYI.SO
add_library(JKYi SHARED ${LIB_SRC})
#add_library(JKYi_static ${LIB_SRC})
#SET_TARGET_PROPERTLES(JKYi_static PROPERTIEL OUTPUT_NAME "JKYi")

add_executable(test tests/test.cc)
add_dependencies(test JKYi)
target_link_libraries(test JKYi ${YAMLCPP})
#
add_executable(test_config tests/test_config.cc)
add_dependencies(test_config JKYi)
#设置要连接的库的名称
target_link_libraries(test_config JKYi ${YAMLCPP})

#将生成的目标文件放到bin目录下
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#将生成的动态库放到lib目录下
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)



