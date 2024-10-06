#ifndef __UTIL_H__
#define __UTIL_H__

#include<sys/syscall.h>
#include<unistd.h>
#include<sys/types.h>
#include<pthread.h>

namespace gaiya{

pid_t GetThreadId();
int32_t GetCoroutineId();

}
#endif
