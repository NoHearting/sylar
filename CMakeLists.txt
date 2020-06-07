cmake_minimum_required(VERSION 2.8)
project(sylar)

include(cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror \
	 -Wno-unused-function -Wno-builtin-macro-redefined")

include_directories(.)
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

find_library(YAMLCPP yaml-cpp)
message("***", ${YAMLCPP})

set(LIB_SRC
	sylar/log.cpp
    sylar/util.cpp
	sylar/config.cpp
	)

add_library(sylar SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(sylar) #__FILE__
# add_library(sylay_static STATIC ${LIB_SRC})
#SET_TARGET_PROPERTIES (sylar_static PROPERTIES OUTPUT_NAME "sylar")


add_executable(test tests/test.cpp)
add_dependencies(test sylar)
force_redefine_file_macro_for_sources(test) #__FILE__
target_link_libraries(test sylar ${YAMLCPP})


add_executable(test_config tests/test_config.cpp)
add_dependencies(test_config sylar)
force_redefine_file_macro_for_sources(test_config) #__FILE__
# target_link_libraries(test_config sylar -L/usr/local/lib/libyaml-cpp.so -lyaml-cpp)
target_link_libraries(test_config sylar ${YAMLCPP})


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
