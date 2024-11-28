#ifndef __HOOK_H__
#define __HOOK_H__

#include<unistd.h>
#include<time.h>//nano

namespace gaiya{
  bool isHook();

  void setThreadHook(bool isHook);

}

//告诉c++编译器用c语言的方式进行链接
extern "C"{

//定义不同精度的sleep函数
typedef unsigned int (*sleep_fun)(unsigned int seconds);
extern sleep_fun sleep_f;

typedef int (*usleep_fun)(useconds_t usec);
extern usleep_fun usleep_f;

typedef int (*nanosleep_fun)(const struct timespec *duration,
                              struct timespec *rem);
extern nanosleep_fun nanosleep_f;


}

#endif
