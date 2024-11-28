#include"hook.hpp"
#include <dlfcn.h>

static thread_local bool t_isHook = false;

bool isHook(){
  return t_isHook;
}

void setThreadHook(bool isHook){
  t_isHook = isHook;
}

#define HOOK_INIT(XX) \
  XX(sleep) \
  XX(usleep) \
  XX(nanosleep)
#undef HOOK_INIT

void hook_init(){
  static bool isInit = false;
  if(!isInit){
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT,name); \
  HOOK_INIT(XX)
#undef XX
  isInit = true;
  }
}

struct HookIniter{
  HookIniter(){
    hook_init();
  }
};

static HookIniter hookiniter;

extern "C"{
#define XX(name) name ## _fun name ## _f = nullptr; \
  HOOK_INIT(XX) 
#undef XX
} 
