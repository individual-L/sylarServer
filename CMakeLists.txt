cmake_minimum_required(VERSION 3.10)
project(gaiya)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -ggdb -Wall -rdynamic  -std=c++11 -Wno-deprecated -Werror -Wno-unused-function")

set(LOG_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/Log)
set(UTIL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/Util)
set(SINGLEM_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/Singlem)
set(CONFIG_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/Config)

set(LIB_SRC 
  ${UTIL_INCLUDE_DIR}/util.cpp 
  ${LOG_INCLUDE_DIR}/log.cpp 
  ${CONFIG_INCLUDE_DIR}/config.h
  ${SINGLEM_INCLUDE_DIR}/singlem.h
  )

add_library(gaiya SHARED ${LIB_SRC})
add_executable(testConfig tester/testConfig.cpp)
# add_executable(testLog tester/testLog.cpp)

target_include_directories(gaiya PUBLIC 
  ${LOG_INCLUDE_DIR}
  ${UTIL_INCLUDE_DIR}
  ${SINGLEM_INCLUDE_DIR}
  ${CONFIG_INCLUDE_DIR} 
  )
# target_include_directories(testLog PUBLIC 
#   ${LOG_INCLUDE_DIR}
#   ${UTIL_INCLUDE_DIR}
#   ${CONFIG_INCLUDE_DIR} 

#   )
# target_include_directories(testConfig PUBLIC 
# ${LOG_INCLUDE_DIR}
# ${UTIL_INCLUDE_DIR}
# ${CONFIG_INCLUDE_DIR} 

# )
#这个命令确保在构建目标之前，先构建它所依赖的其他目标。
add_dependencies(testConfig gaiya)
target_link_libraries(testConfig gaiya)
target_link_libraries(testConfig yaml-cpp)
# add_dependencies(testLog gaiya)
# target_link_libraries(testLog gaiya)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
