cmake_minimum_required(VERSION 3.10)
project(gaiya)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -ggdb -lstdc++fs -Wall -rdynamic  -std=c++17 -Wno-deprecated -Werror -Wno-unused-function")


add_definitions(-DRELATIVE_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
set(LOG_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/Log)
set(UTIL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/Util)
set(SINGLEM_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/Singlem)
set(CONFIG_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/Config)
set(LOCK_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/Lock)
set(THREAD_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/Thread)
set(THREAD_COROUTINE_DIR ${PROJECT_SOURCE_DIR}/src/Coroutine)


set(LIB_SRC 
  ${UTIL_INCLUDE_DIR}/util.cpp 
  ${LOG_INCLUDE_DIR}/log.cpp 
  ${CONFIG_INCLUDE_DIR}/config.cpp
  ${SINGLEM_INCLUDE_DIR}/singlem.hpp
  ${LOCK_INCLUDE_DIR}/lock.cpp
  ${LOCK_INCLUDE_DIR}/Noncopyable.hpp
  ${THREAD_INCLUDE_DIR}/thread.cpp
  ${THREAD_COROUTINE_DIR}/coroutine.cpp
)
include_directories(
  ${LOG_INCLUDE_DIR}
  ${UTIL_INCLUDE_DIR}
  ${SINGLEM_INCLUDE_DIR}
  ${CONFIG_INCLUDE_DIR} 
  ${LOCK_INCLUDE_DIR}
  ${THREAD_INCLUDE_DIR}
  ${THREAD_COROUTINE_DIR}
)
add_library(gaiya SHARED ${LIB_SRC})
target_link_libraries(gaiya yaml-cpp)
# add_executable(testThread tester/testThread.cpp)
# add_dependencies(testThread gaiya)
# target_link_libraries(testThread gaiya)

# add_executable(testLog tester/testLog.cpp)
# add_dependencies(testLog gaiya)
# target_link_libraries(testLog gaiya)

add_executable(testCoroutine tester/testCoroutine.cpp)
add_dependencies(testCoroutine gaiya)
target_link_libraries(testCoroutine gaiya)

#这个命令确保在构建目标之前，先构建它所依赖的其他目标。

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
