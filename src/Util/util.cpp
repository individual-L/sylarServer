#include"util.h"

namespace gaiya{

pid_t GetThreadId(){
  return syscall(SYS_gettid);
}
int32_t GetCoroutineId(){
  return 1;
}

}
