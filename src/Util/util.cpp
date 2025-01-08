#include"util.hpp"
#include"coroutine.hpp"
#include"thread.hpp"

namespace gaiya{

gaiya::Logger::ptr logger = LOG_ROOT();

void backTrace(std::vector<std::string>& bt,const int size,const int skip){
  void ** buff = (void **) malloc(sizeof(void*) * size);
  int n = ::backtrace(buff,size);
  char ** strings = ::backtrace_symbols(buff,n);

  if(strings == nullptr){
    LOG_ERROR(logger) << "backtrace_symbols error";
    std::cerr << "backtrace_symbols";
  }
  for(int i = skip;i < n;++i){
    bt.push_back(strings[i]);
  }
  free(buff);
  free(strings);
}

std::string backTraceToString(const int size,const int skip,const std::string& prefix){
  std::vector<std::string> bt;
  backTrace(bt,size,skip);
  std::stringstream ss;
  ss <<std::endl;
  for(auto& it : bt){
    ss << prefix << it <<std::endl;
  }
  return ss.str();
}


pid_t GetThreadId(){
  return syscall(SYS_gettid);
}

int32_t GetCoroutineId(){
  return gaiya::Coroutine::GetCurCoroId();
}

std::string GetThreadName(){
  return gaiya::Thread::GetName();
}

//删除path文件
bool Unlink(std::string path){
  return ::unlink(path.c_str()) == 0;
}

}
