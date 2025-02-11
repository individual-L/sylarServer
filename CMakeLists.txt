cmake_minimum_required(VERSION 3.10)
project(gaiya)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -ggdb -lstdc++fs -Wall -rdynamic  -std=c++17 -Wno-deprecated -Werror -Wno-unused-function")


set(LOG_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/Log)
set(UTIL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/Util)
set(SINGLEM_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/Singlem)
set(CONFIG_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/Config)
set(LOCK_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/Lock)
set(THREAD_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/Thread)
set(COROUTINE_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/Coroutine)
set(SCHEDULER_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/Scheduler)
set(IOMANAGER_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/IOmanager)
set(TIMER_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/Timer)
set(HOOK_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/Hook)
set(ADDRESS_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/Address)
set(SOCKET_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/Socket)
set(BYTEARRAY_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/ByteArray)
set(HTTP_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/Http)


set(LIB_SRC 
  ${UTIL_INCLUDE_DIR}/util.cpp 
  ${LOG_INCLUDE_DIR}/log.cpp 
  ${CONFIG_INCLUDE_DIR}/config.cpp
  ${SINGLEM_INCLUDE_DIR}/singlem.hpp
  ${LOCK_INCLUDE_DIR}/lock.cpp
  ${LOCK_INCLUDE_DIR}/Noncopyable.hpp
  ${THREAD_INCLUDE_DIR}/thread.cpp
  ${COROUTINE_INCLUDE_DIR}/coroutine.cpp
  ${SCHEDULER_INCLUDE_DIR}/schedule.cpp
  ${IOMANAGER_INCLUDE_DIR}/iomanager.cpp
  ${TIMER_INCLUDE_DIR}/timer.cpp
  ${HOOK_INCLUDE_DIR}/hook.cpp
  ${HOOK_INCLUDE_DIR}/fdMng.cpp
  ${ADDRESS_INCLUDE_DIR}/address.cpp
  ${UTIL_INCLUDE_DIR}/endian.hpp
  ${SOCKET_INCLUDE_DIR}/socket.cpp
  ${BYTEARRAY_INCLUDE_DIR}/bytearray.cpp
  ${HTTP_INCLUDE_DIR}/http.cpp
)
include_directories(
  ${LOG_INCLUDE_DIR}
  ${UTIL_INCLUDE_DIR}
  ${SINGLEM_INCLUDE_DIR}
  ${CONFIG_INCLUDE_DIR} 
  ${LOCK_INCLUDE_DIR}
  ${THREAD_INCLUDE_DIR}
  ${COROUTINE_INCLUDE_DIR}
  ${SCHEDULER_INCLUDE_DIR}
  ${IOMANAGER_INCLUDE_DIR}
  ${TIMER_INCLUDE_DIR}
  ${HOOK_INCLUDE_DIR}
  ${ADDRESS_INCLUDE_DIR}
  ${SOCKET_INCLUDE_DIR}
  ${BYTEARRAY_INCLUDE_DIR}
  ${HTTP_INCLUDE_DIR}
)
add_library(gaiya SHARED ${LIB_SRC})

add_executable(main tester/main.cpp)
target_link_libraries(main pthread)
target_link_libraries(main gaiya)
target_link_libraries(main yaml-cpp)
add_dependencies(main gaiya)

#这个命令确保在构建目标之前，先构建它所依赖的其他目标。

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
