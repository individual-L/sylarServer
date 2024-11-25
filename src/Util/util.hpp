#ifndef __UTIL_H__
#define __UTIL_H__

#include<sys/syscall.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/time.h>
#include<pthread.h>
#include<cxxabi.h>
#include<iostream>
#include<filesystem>
#include<string>
#include<vector>
#include<execinfo.h>

#include"log.hpp"

namespace gaiya{

inline uint64_t GetCurrentTime(){
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec * 1000ul + tv.tv_usec / 1000;
}

pid_t GetThreadId();

int32_t GetCoroutineId();

std::string GetThreadName();

void backTrace(std::vector<std::string> &bt,const int size,const int skip);

std::string backTraceToString(const int size = 64,const int skip = 2,const std::string& prefix = "     ");

inline std::string getRelativePath(std::filesystem::path p){
    // 文件路径：/home/luo/cplus
    std::filesystem::path current_file = std::filesystem::current_path().parent_path().parent_path() ; 
    //目标文件
    std::filesystem::path target_file = p; 

    // 计算目标文件p与项目路径的相对路径
    std::filesystem::path relative_path = target_file.lexically_relative(current_file); 
    return relative_path;
}

template<class T>
std::string TypeName(){
  const char * mangle_name = typeid(T).name(); //编译器定义的名称
  int status = 0;                      //// 用于接收 demangle 操作的状态
  char * demangle_name = abi::__cxa_demangle(mangle_name,nullptr,nullptr,&status);

  if(demangle_name && status == 0){
    return std::string(demangle_name);
    free(demangle_name);
  }else{
    std::cerr<<"demangle fail with status " << status << std::endl;
  }
  return {};
}




}
#endif
