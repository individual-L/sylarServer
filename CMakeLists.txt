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

set(LIB_SRC 
  ${UTIL_INCLUDE_DIR}/util.cpp 
  ${LOG_INCLUDE_DIR}/log.cpp 
  ${CONFIG_INCLUDE_DIR}/config.cpp
  ${SINGLEM_INCLUDE_DIR}/singlem.hpp
  ${LOCK_INCLUDE_DIR}/lock.cpp
  ${LOCK_INCLUDE_DIR}/Noncopyable.hpp
  ${THREAD_INCLUDE_DIR}/thread.cpp
  )


add_library(gaiya SHARED ${LIB_SRC})
add_executable(testThread tester/testThread.cpp)
# add_executable(testLog tester/testLog.cpp)


target_include_directories(gaiya PUBLIC 
  ${LOG_INCLUDE_DIR}
  ${UTIL_INCLUDE_DIR}
  ${SINGLEM_INCLUDE_DIR}
  ${CONFIG_INCLUDE_DIR} 
  ${LOCK_INCLUDE_DIR}
  ${THREAD_INCLUDE_DIR}
  )
# target_include_directories(testLog PUBLIC 
#   ${LOG_INCLUDE_DIR}
#   ${UTIL_INCLUDE_DIR}
#   ${CONFIG_INCLUDE_DIR} 

#   )
target_include_directories(testThread PUBLIC 
${LOG_INCLUDE_DIR}
${UTIL_INCLUDE_DIR}
${CONFIG_INCLUDE_DIR} 
${SINGLEM_INCLUDE_DIR}
${THREAD_INCLUDE_DIR}
)

target_link_libraries(gaiya yaml-cpp)
#这个命令确保在构建目标之前，先构建它所依赖的其他目标。
add_dependencies(testThread gaiya)

# 创建一个自定义目标，用于确保 log.cpp 先被编译
# add_custom_target(singlem_hpp DEPENDS ${SINGLEM_INCLUDE_DIR}/singlem.hpp)
# add_dependencies(gaiya singlem_hpp)
target_link_libraries(testThread gaiya)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
