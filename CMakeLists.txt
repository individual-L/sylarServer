cmake_minimum_required(VERSION 2.8.3)
project(gaiya)

set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -ggdb -Wall -rdynamic  -std=c++11 -Wno-deprecated -Werror -Wno-unused-function")

set(LIB_SRC src/Log/log.cpp)
add_library(gaiya SHARED ${LIB_SRC})
add_executable(test tester/testLog.cpp)
target_include_directories(test PUBLIC ${PROJECT_SOURCE_DIR}/src/Log)

#这个命令确保在构建目标之前，先构建它所依赖的其他目标。
add_dependencies(test gaiya)

target_link_libraries(test gaiya)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
