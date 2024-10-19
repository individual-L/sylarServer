#ifndef __UTIL_H__
#define __UTIL_H__

#include<sys/syscall.h>
#include<unistd.h>
#include<sys/types.h>
#include<pthread.h>
#include<cxxabi.h>
#include<iostream>
#include<string>

// #include"log.hpp"
// #include"config.h"

namespace gaiya{

pid_t GetThreadId();

int32_t GetCoroutineId();

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
  return nullptr;
}


}
#endif
